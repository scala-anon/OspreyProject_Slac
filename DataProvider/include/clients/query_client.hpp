#ifndef QUERY_CLIENT_HPP
#define QUERY_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <future>
#include <map>
#include <grpcpp/grpcpp.h>

#include "common_client.hpp"  // Use our common client for shared types
#include "query.pb.h"
#include "query.grpc.pb.h"

// Query-specific type aliases
using QueryDataRequest = ::dp::service::query::QueryDataRequest;
using QuerySpec = ::dp::service::query::QueryDataRequest::QuerySpec;
using CursorOperation = ::dp::service::query::QueryDataRequest::CursorOperation;
using CursorOperationType = ::dp::service::query::QueryDataRequest::CursorOperation::CursorOperationType;

using QueryDataResponse = ::dp::service::query::QueryDataResponse;
using QueryData = ::dp::service::query::QueryDataResponse::QueryData;
using DataBucket = ::dp::service::query::QueryDataResponse::QueryData::DataBucket;

using QueryTableRequest = ::dp::service::query::QueryTableRequest;
using TableResultFormat = ::dp::service::query::QueryTableRequest::TableResultFormat;
using QueryTableResponse = ::dp::service::query::QueryTableResponse;
using TableResult = ::dp::service::query::QueryTableResponse::TableResult;
using ColumnTable = ::dp::service::query::QueryTableResponse::ColumnTable;
using RowMapTable = ::dp::service::query::QueryTableResponse::RowMapTable;
using DataRow = ::dp::service::query::QueryTableResponse::RowMapTable::DataRow;

using QueryPvMetadataRequest = ::dp::service::query::QueryPvMetadataRequest;
using QueryPvMetadataResponse = ::dp::service::query::QueryPvMetadataResponse;
using PvMetadataResult = ::dp::service::query::QueryPvMetadataResponse::MetadataResult;
using PvInfo = ::dp::service::query::QueryPvMetadataResponse::MetadataResult::PvInfo;

using PvNameList = ::dp::service::query::PvNameList;
using PvNamePattern = ::dp::service::query::PvNamePattern;

using QueryProvidersRequest = ::dp::service::query::QueryProvidersRequest;
using ProviderCriterion = ::dp::service::query::QueryProvidersRequest::Criterion;
using QueryProvidersResponse = ::dp::service::query::QueryProvidersResponse;
using ProvidersResult = ::dp::service::query::QueryProvidersResponse::ProvidersResult;
using ProviderInfo = ::dp::service::query::QueryProvidersResponse::ProvidersResult::ProviderInfo;

using QueryProviderMetadataRequest = ::dp::service::query::QueryProviderMetadataRequest;
using QueryProviderMetadataResponse = ::dp::service::query::QueryProviderMetadataResponse;
using ProviderMetadataResult = ::dp::service::query::QueryProviderMetadataResponse::MetadataResult;
using ProviderMetadata = ::dp::service::query::QueryProviderMetadataResponse::MetadataResult::ProviderMetadata;

using DpQueryService = ::dp::service::query::DpQueryService;

class QueryClient {
public:
    // Constructor with channel
    explicit QueryClient(std::shared_ptr<grpc::Channel> channel);
    
    // Constructor with address
    explicit QueryClient(const std::string& server_address);
    
    ~QueryClient();

    // ========== Time Series Data Query (Unary) ==========
    
    // Simple query returning all data
    std::vector<DataBucket> QueryData(
        const Timestamp& begin_time,
        const Timestamp& end_time,
        const std::vector<std::string>& pv_names,
        bool use_serialized = false);
    
    // Query with full response
    QueryDataResponse QueryDataWithResponse(
        const Timestamp& begin_time,
        const Timestamp& end_time,
        const std::vector<std::string>& pv_names,
        bool use_serialized = false);
    
    // Query with QuerySpec
    QueryDataResponse QueryDataWithSpec(const QuerySpec& spec);
    
    // Helper to create QuerySpec
    QuerySpec CreateQuerySpec(
        const Timestamp& begin_time,
        const Timestamp& end_time,
        const std::vector<std::string>& pv_names,
        bool use_serialized = false);
    
    // ========== Time Series Data Query (Server Streaming) ==========
    
    class StreamQuerySession {
    public:
        StreamQuerySession(
            std::shared_ptr<grpc::ClientReader<QueryDataResponse>> reader,
            std::shared_ptr<grpc::ClientContext> context);
        
        // Read next response
        std::optional<QueryDataResponse> ReadNext();
        
        // Read all remaining responses
        std::vector<QueryDataResponse> ReadAll();
        
        // Extract all data buckets
        std::vector<DataBucket> ReadAllDataBuckets();
        
        // Cancel the stream
        void Cancel();
        
        // Check if stream is done
        bool IsDone() const;
        
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;
    };
    
    std::unique_ptr<StreamQuerySession> QueryDataStream(
        const Timestamp& begin_time,
        const Timestamp& end_time,
        const std::vector<std::string>& pv_names,
        bool use_serialized = false);
    
    // ========== Time Series Data Query (Bidirectional Streaming) ==========
    
    class BidiQuerySession {
    public:
        BidiQuerySession(
            std::shared_ptr<grpc::ClientReaderWriter<QueryDataRequest, QueryDataResponse>> stream,
            std::shared_ptr<grpc::ClientContext> context);
        
        // Send initial query
        bool SendQuery(const QuerySpec& spec);
        
        // Send cursor operation to get next batch
        bool RequestNext();
        
        // Read response
        std::optional<QueryDataResponse> ReadResponse();
        
        // Combined request next and read
        std::optional<QueryDataResponse> GetNext();
        
        // Close the sending side
        void CloseSending();
        
        // Cancel the stream
        void Cancel();
        
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;
    };
    
    std::unique_ptr<BidiQuerySession> QueryDataBidiStream();
    
    // ========== Tabular Query ==========
    
    // Query returning column-oriented table
    std::optional<ColumnTable> QueryTableColumns(
        const Timestamp& begin_time,
        const Timestamp& end_time,
        const std::vector<std::string>& pv_names);
    
    // Query returning row-oriented table
    std::optional<RowMapTable> QueryTableRows(
        const Timestamp& begin_time,
        const Timestamp& end_time,
        const std::vector<std::string>& pv_names);
    
    // Query with pattern matching
    QueryTableResponse QueryTableWithPattern(
        const Timestamp& begin_time,
        const Timestamp& end_time,
        const std::string& pv_pattern,
        TableResultFormat format = TableResultFormat::QueryTableRequest_TableResultFormat_TABLE_FORMAT_COLUMN);
    
    // Query with full control
    QueryTableResponse QueryTable(const QueryTableRequest& request);
    
    // ========== PV Metadata Query ==========
    
    // Query metadata for specific PVs
    std::vector<PvInfo> QueryPvMetadata(const std::vector<std::string>& pv_names);
    
    // Query metadata with pattern
    std::vector<PvInfo> QueryPvMetadataWithPattern(const std::string& pattern);
    
    // Query with full response
    QueryPvMetadataResponse QueryPvMetadataWithResponse(const QueryPvMetadataRequest& request);
    
    // ========== Provider Query ==========
    
    // Query all providers
    std::vector<ProviderInfo> QueryAllProviders();
    
    // Query providers by ID
    std::vector<ProviderInfo> QueryProviderById(const std::string& provider_id);
    
    // Query providers by text search
    std::vector<ProviderInfo> QueryProvidersByText(const std::string& search_text);
    
    // Query providers by tag
    std::vector<ProviderInfo> QueryProvidersByTag(const std::string& tag);
    
    // Query providers by attribute
    std::vector<ProviderInfo> QueryProvidersByAttribute(
        const std::string& key, 
        const std::string& value);
    
    // Query with multiple criteria
    QueryProvidersResponse QueryProviders(const std::vector<ProviderCriterion>& criteria);
    
    // Helper methods to create criteria
    ProviderCriterion CreateIdCriterion(const std::string& id);
    ProviderCriterion CreateTextCriterion(const std::string& text);
    ProviderCriterion CreateTagsCriterion(const std::string& tag);
    ProviderCriterion CreateAttributesCriterion(const std::string& key, const std::string& value);
    
    // ========== Provider Metadata Query ==========
    
    // Query provider metadata
    std::optional<ProviderMetadata> QueryProviderMetadata(const std::string& provider_id);
    
    // Query with full response
    QueryProviderMetadataResponse QueryProviderMetadataWithResponse(const std::string& provider_id);
    
    // ========== Utility Methods ==========
    
    // Extract data values from buckets
    std::vector<DataValue> ExtractDataValues(const std::vector<DataBucket>& buckets);
    
    // Extract timestamps from buckets
    std::vector<Timestamp> ExtractTimestamps(const std::vector<DataBucket>& buckets);
    
    // Convert column table to map
    std::map<std::string, std::vector<DataValue>> ColumnTableToMap(const ColumnTable& table);
    
    // Convert row table to vector of maps
    std::vector<std::map<std::string, DataValue>> RowTableToVector(const RowMapTable& table);
    
    // Deserialize data bucket if using serialized columns
    DataColumn DeserializeDataBucket(const DataBucket& bucket);
    
    // Check connection
    bool IsConnected() const;
    grpc_connectivity_state GetChannelState() const;
    bool WaitForConnection(int timeout_seconds = 10);
    
    // Timeout management
    void SetDefaultTimeout(int seconds);
    int GetDefaultTimeout() const;
    
    // Error handling
    std::string GetLastError() const;
    void ClearLastError();
    
    // Statistics
    struct ClientStats {
        uint64_t queries_executed = 0;
        uint64_t stream_queries = 0;
        uint64_t table_queries = 0;
        uint64_t metadata_queries = 0;
        uint64_t provider_queries = 0;
        uint64_t errors = 0;
        uint64_t total_buckets_received = 0;
    };
    
    ClientStats GetStats() const;
    void ResetStats();
    
    // Access to common client
    CommonClient& GetCommonClient();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // QUERY_CLIENT_HPP
