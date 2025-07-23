#ifndef ANNOTATION_CLIENT_HPP
#define ANNOTATION_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>
#include "common.pb.h"
#include "annotation.pb.h"
#include "annotation.grpc.pb.h"

using Timestamp = ::Timestamp;
using Attribute = ::Attribute;
using EventMetadata = ::EventMetadata;
using DataColumn = ::DataColumn;
using DataTimestamps = ::DataTimestamps;

// Annotation service protobuf types
using CreateDataSetRequest = dp::service::annotation::CreateDataSetRequest;
using CreateDataSetResponse = dp::service::annotation::CreateDataSetResponse;
using QueryDataSetsRequest = dp::service::annotation::QueryDataSetsRequest;
using QueryDataSetsResponse = dp::service::annotation::QueryDataSetsResponse;
using CreateAnnotationRequest = dp::service::annotation::CreateAnnotationRequest;
using CreateAnnotationResponse = dp::service::annotation::CreateAnnotationResponse;
using QueryAnnotationsRequest = dp::service::annotation::QueryAnnotationsRequest;
using QueryAnnotationsResponse = dp::service::annotation::QueryAnnotationsResponse;
using ExportDataRequest = dp::service::annotation::ExportDataRequest;
using ExportDataResponse = dp::service::annotation::ExportDataResponse;
using DataSet = dp::service::annotation::DataSet;
using DataBlock = dp::service::annotation::DataBlock;
using Calculations = dp::service::annotation::Calculations;

// Time range helper structure
struct TimeRange {
    uint64_t start_epoch_sec;
    uint64_t start_nano;
    uint64_t end_epoch_sec;
    uint64_t end_nano;

    TimeRange(uint64_t start_sec = 0, uint64_t start_ns = 0, 
              uint64_t end_sec = 0, uint64_t end_ns = 0)
        : start_epoch_sec(start_sec), start_nano(start_ns),
          end_epoch_sec(end_sec), end_nano(end_ns) {}

    Timestamp getStartTimestamp() const;
    Timestamp getEndTimestamp() const;
};

// Configuration structure
struct AnnotationClientConfig {
    std::string server_address = "localhost:50051";
    int connection_timeout_seconds = 5;
    int max_message_size_mb = 64;

    static AnnotationClientConfig fromConfigFile(const std::string& config_path);
    static AnnotationClientConfig fromJson(const nlohmann::json& j);
};

// Main annotation client class
class AnnotationClient {
public:
    explicit AnnotationClient(const AnnotationClientConfig& config);
    explicit AnnotationClient(const std::string& server_address);
    ~AnnotationClient();

    // ===================== Core RPC Methods =====================
    
    CreateDataSetResponse createDataSet(const CreateDataSetRequest& request);
    QueryDataSetsResponse queryDataSets(const QueryDataSetsRequest& request);
    CreateAnnotationResponse createAnnotation(const CreateAnnotationRequest& request);
    QueryAnnotationsResponse queryAnnotations(const QueryAnnotationsRequest& request);
    ExportDataResponse exportData(const ExportDataRequest& request);

    // ===================== Connection Management =====================
    
    void updateConfig(const AnnotationClientConfig& config);
    const AnnotationClientConfig& getConfig() const { return config_; }
    bool isConnected() const;
    void reconnect();
    const std::string& getLastError() const { return last_error_; }

private:
    AnnotationClientConfig config_;
    std::unique_ptr<dp::service::annotation::DpAnnotationService::Stub> stub_;
    std::string last_error_;
    
    // Internal methods
    void initializeConnection();
};

// ===================== Helper Function Declarations =====================

// Timestamp helpers
Timestamp makeTimestamp(uint64_t epoch, uint64_t nano);

// DataSet helpers
DataSet makeDataSet(const std::string& name, const std::string& owner_id, 
                   const std::string& description, const std::vector<DataBlock>& data_blocks);
DataBlock makeDataBlock(const TimeRange& time_range, const std::vector<std::string>& pv_names);

// Request builders
CreateDataSetRequest makeCreateDataSetRequest(const DataSet& dataset);
CreateAnnotationRequest makeCreateAnnotationRequest(const std::string& owner_id, const std::string& name,
                                                   const std::vector<std::string>& dataset_ids);

// Query builders
QueryDataSetsRequest makeQueryDataSetsByOwner(const std::string& owner_id);
QueryDataSetsRequest makeQueryDataSetsByText(const std::string& text);
QueryDataSetsRequest makeQueryDataSetsByPvName(const std::string& pv_name);
QueryDataSetsRequest makeQueryDataSetsById(const std::string& dataset_id);

QueryAnnotationsRequest makeQueryAnnotationsByOwner(const std::string& owner_id);
QueryAnnotationsRequest makeQueryAnnotationsByDataSet(const std::string& dataset_id);
QueryAnnotationsRequest makeQueryAnnotationsByText(const std::string& text);
QueryAnnotationsRequest makeQueryAnnotationsByTag(const std::string& tag);
QueryAnnotationsRequest makeQueryAnnotationsById(const std::string& annotation_id);

// Export builders
ExportDataRequest makeExportDataRequest(const std::string& dataset_id, 
                                       ExportDataRequest::ExportOutputFormat format);

// Utility functions
namespace AnnotationUtils {
    TimeRange createTimeRange(uint64_t start_epoch_sec, uint64_t end_epoch_sec,
                             uint64_t start_nano = 0, uint64_t end_nano = 0);
    uint64_t getCurrentEpochSeconds();
    TimeRange createRecentTimeRange(uint64_t duration_seconds);
}

#endif // ANNOTATION_CLIENT_HPP