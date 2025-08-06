#ifndef INGEST_CLIENT_HPP
#define INGEST_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <future>
#include <map>
#include <queue>
#include <grpcpp/grpcpp.h>

#include "common_client.hpp"  // Use our common client for shared types
#include "ingestion.pb.h"
#include "ingestion.grpc.pb.h"

// Ingestion-specific type aliases
using RegisterProviderRequest = ::dp::service::ingestion::RegisterProviderRequest;
using RegisterProviderResponse = ::dp::service::ingestion::RegisterProviderResponse;
using RegistrationResult = ::dp::service::ingestion::RegisterProviderResponse::RegistrationResult;

using IngestDataRequest = ::dp::service::ingestion::IngestDataRequest;
using IngestDataResponse = ::dp::service::ingestion::IngestDataResponse;
using IngestionDataFrame = ::dp::service::ingestion::IngestDataRequest::IngestionDataFrame;
using AckResult = ::dp::service::ingestion::IngestDataResponse::AckResult;

using IngestDataStreamResponse = ::dp::service::ingestion::IngestDataStreamResponse;
using IngestDataStreamResult = ::dp::service::ingestion::IngestDataStreamResponse::IngestDataStreamResult;

using QueryRequestStatusRequest = ::dp::service::ingestion::QueryRequestStatusRequest;
using QueryRequestStatusCriterion = ::dp::service::ingestion::QueryRequestStatusRequest::QueryRequestStatusCriterion;
using QueryRequestStatusResponse = ::dp::service::ingestion::QueryRequestStatusResponse;
using RequestStatusResult = ::dp::service::ingestion::QueryRequestStatusResponse::RequestStatusResult;
using RequestStatus = ::dp::service::ingestion::QueryRequestStatusResponse::RequestStatusResult::RequestStatus;

using SubscribeDataRequest = ::dp::service::ingestion::SubscribeDataRequest;
using NewSubscription = ::dp::service::ingestion::SubscribeDataRequest::NewSubscription;
using CancelSubscription = ::dp::service::ingestion::SubscribeDataRequest::CancelSubscription;
using SubscribeDataResponse = ::dp::service::ingestion::SubscribeDataResponse;
using SubscribeDataResult = ::dp::service::ingestion::SubscribeDataResponse::SubscribeDataResult;

using IngestionRequestStatus = ::dp::service::ingestion::IngestionRequestStatus;
using DpIngestionService = ::dp::service::ingestion::DpIngestionService;

class IngestionClient {
public:
    // Constructor with channel
    explicit IngestionClient(std::shared_ptr<grpc::Channel> channel);
    
    // Constructor with address
    explicit IngestionClient(const std::string& server_address);
    
    ~IngestionClient();

    // ========== Provider Registration ==========
    
    // Register a new provider
    std::optional<std::string> RegisterProvider(
        const std::string& provider_name,
        const std::string& description = "",
        const std::vector<std::string>& tags = {},
        const std::vector<Attribute>& attributes = {});
    
    // Register provider with full response details
    RegisterProviderResponse RegisterProviderWithDetails(
        const std::string& provider_name,
        const std::string& description = "",
        const std::vector<std::string>& tags = {},
        const std::vector<Attribute>& attributes = {});
    
    // ========== Data Ingestion (Unary) ==========
    
    // Simple data ingestion
    bool IngestData(
        const std::string& provider_id,
        const std::string& client_request_id,
        const std::vector<DataColumn>& columns,
        const DataTimestamps& timestamps,
        const std::vector<std::string>& tags = {},
        const std::vector<Attribute>& attributes = {},
        const std::optional<EventMetadata>& event = std::nullopt);
    
    // Ingest data with full response
    IngestDataResponse IngestDataWithResponse(
        const std::string& provider_id,
        const std::string& client_request_id,
        const IngestionDataFrame& data_frame,
        const std::vector<std::string>& tags = {},
        const std::vector<Attribute>& attributes = {},
        const std::optional<EventMetadata>& event = std::nullopt);
    
    // Helper to create data frame
    IngestionDataFrame CreateDataFrame(
        const DataTimestamps& timestamps,
        const std::vector<DataColumn>& columns);
    
    // Helper to create data frame from sampling clock
    IngestionDataFrame CreateDataFrameFromClock(
        const SamplingClock& clock,
        const std::vector<DataColumn>& columns);
    
    // ========== Streaming Data Ingestion ==========
    
    // Client-side streaming ingestion
    class StreamIngestionSession {
    public:
        StreamIngestionSession(std::shared_ptr<grpc::ClientWriter<IngestDataRequest>> writer,
                              std::shared_ptr<grpc::ClientContext> context);
        
        bool SendData(const IngestDataRequest& request);
        bool SendData(const std::string& provider_id,
                     const std::string& client_request_id,
                     const IngestionDataFrame& data_frame);
        
        IngestDataStreamResponse Finish();
        void Cancel();
        
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;
    };
    
    std::unique_ptr<StreamIngestionSession> CreateStreamIngestionSession();
    
    // Bidirectional streaming ingestion
    class BidiStreamIngestionSession {
    public:
        BidiStreamIngestionSession(
            std::shared_ptr<grpc::ClientReaderWriter<IngestDataRequest, IngestDataResponse>> stream,
            std::shared_ptr<grpc::ClientContext> context);
        
        bool SendData(const IngestDataRequest& request);
        std::optional<IngestDataResponse> ReadResponse();
        bool WaitForResponse(IngestDataResponse& response, int timeout_ms = 1000);
        
        void CloseSending();
        std::vector<IngestDataResponse> ReadAllResponses();
        
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;
    };
    
    std::unique_ptr<BidiStreamIngestionSession> CreateBidiStreamIngestionSession();
    
    // ========== Request Status Query ==========
    
    // Query by provider ID
    std::vector<RequestStatus> QueryRequestStatusByProviderId(const std::string& provider_id);
    
    // Query by provider name
    std::vector<RequestStatus> QueryRequestStatusByProviderName(const std::string& provider_name);
    
    // Query by request ID
    std::vector<RequestStatus> QueryRequestStatusByRequestId(const std::string& request_id);
    
    // Query by status
    std::vector<RequestStatus> QueryRequestStatusByStatus(IngestionRequestStatus status);
    
    // Query by time range
    std::vector<RequestStatus> QueryRequestStatusByTimeRange(
        const Timestamp& start_time,
        const Timestamp& end_time);
    
    // Query with multiple criteria
    QueryRequestStatusResponse QueryRequestStatus(
        const std::vector<QueryRequestStatusCriterion>& criteria);
    
    // Helper to create criteria
    QueryRequestStatusCriterion CreateProviderIdCriterion(const std::string& provider_id);
    QueryRequestStatusCriterion CreateProviderNameCriterion(const std::string& provider_name);
    QueryRequestStatusCriterion CreateRequestIdCriterion(const std::string& request_id);
    QueryRequestStatusCriterion CreateStatusCriterion(IngestionRequestStatus status);
    QueryRequestStatusCriterion CreateTimeRangeCriterion(const Timestamp& start, const Timestamp& end);
    
    // ========== Data Subscription ==========
    
    class SubscriptionSession {
    public:
        using DataCallback = std::function<void(const SubscribeDataResult&)>;
        using ErrorCallback = std::function<void(const ExceptionalResult&)>;
        
        SubscriptionSession(
            std::shared_ptr<grpc::ClientReaderWriter<SubscribeDataRequest, SubscribeDataResponse>> stream,
            std::shared_ptr<grpc::ClientContext> context);
        
        // Subscribe to PVs
        bool Subscribe(const std::vector<std::string>& pv_names);
        
        // Cancel subscription
        bool CancelSubscription();
        
        // Read responses
        std::optional<SubscribeDataResponse> ReadResponse();
        
        // Start async reading with callbacks
        void StartAsyncReading(DataCallback on_data, ErrorCallback on_error = nullptr);
        void StopAsyncReading();
        
        // Check if subscription is active
        bool IsActive() const;
        
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;
    };
    
    std::unique_ptr<SubscriptionSession> CreateSubscriptionSession();
    
    // Simple subscription with callback
    std::future<void> SubscribeToData(
        const std::vector<std::string>& pv_names,
        std::function<void(const SubscribeDataResult&)> callback,
        std::function<void(const std::string&)> error_callback = nullptr);
    
    // ========== Utility Methods ==========
    
    // Check if connected
    bool IsConnected() const;
    
    // Get channel state
    grpc_connectivity_state GetChannelState() const;
    
    // Wait for connection
    bool WaitForConnection(int timeout_seconds = 10);
    
    // Get/set timeout for operations
    void SetDefaultTimeout(int seconds);
    int GetDefaultTimeout() const;
    
    // Error handling
    std::string GetLastError() const;
    void ClearLastError();
    
    // Statistics
    struct ClientStats {
        uint64_t providers_registered = 0;
        uint64_t data_ingested = 0;
        uint64_t stream_sessions = 0;
        uint64_t subscriptions = 0;
        uint64_t errors = 0;
    };
    
    ClientStats GetStats() const;
    void ResetStats();
    
    // Access to common client for shared operations
    CommonClient& GetCommonClient();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif