#ifndef ANNOTATION_CLIENT_HPP
#define ANNOTATION_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
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
    bool enable_data_validation = true;
    bool enable_progress_monitoring = true;
    std::string default_owner_id = "";

    static AnnotationClientConfig fromConfigFile(const std::string& config_path);
    static AnnotationClientConfig fromJson(const nlohmann::json& j);
};

// Callback types
using ProgressCallback = std::function<void(const std::string& operation, const std::string& status)>;
using ErrorCallback = std::function<void(const std::string& error, const std::string& context)>;

// Result structures
struct DataSetResult {
    bool success = false;
    std::string dataset_id;
    std::string error_message;
    double processing_time_seconds = 0.0;
};

struct AnnotationResult {
    bool success = false;
    std::string annotation_id;
    std::string error_message;
    double processing_time_seconds = 0.0;
};

struct ExportResult {
    bool success = false;
    std::string file_path;
    std::string file_url;
    std::string error_message;
    double processing_time_seconds = 0.0;
};

// Query result structures
struct QueryDataSetsResult {
    bool success = false;
    std::vector<DataSet> datasets;
    size_t total_results = 0;
    std::string error_message;
    double processing_time_seconds = 0.0;
};

struct QueryAnnotationsResult {
    bool success = false;
    std::vector<QueryAnnotationsResponse::AnnotationsResult::Annotation> annotations;
    size_t total_results = 0;
    std::string error_message;
    double processing_time_seconds = 0.0;
};

// DataSet building helper
class DataSetBuilder {
public:
    DataSetBuilder(const std::string& name, const std::string& owner_id);
    
    DataSetBuilder& setDescription(const std::string& description);
    DataSetBuilder& addDataBlock(const TimeRange& time_range, const std::vector<std::string>& pv_names);
    DataSetBuilder& addDataBlock(const DataBlock& data_block);
    
    DataSet build() const;

private:
    DataSet dataset_;
};

// Annotation building helper
class AnnotationBuilder {
public:
    AnnotationBuilder(const std::string& owner_id, const std::string& name);
    
    AnnotationBuilder& addDataSetId(const std::string& dataset_id);
    AnnotationBuilder& addDataSetIds(const std::vector<std::string>& dataset_ids);
    AnnotationBuilder& addAnnotationId(const std::string& annotation_id);
    AnnotationBuilder& setComment(const std::string& comment);
    AnnotationBuilder& addTag(const std::string& tag);
    AnnotationBuilder& addTags(const std::vector<std::string>& tags);
    AnnotationBuilder& addAttribute(const std::string& key, const std::string& value);
    AnnotationBuilder& addAttributes(const std::vector<Attribute>& attributes);
    AnnotationBuilder& setEventMetadata(const EventMetadata& event_metadata);
    AnnotationBuilder& setCalculations(const Calculations& calculations);
    
    CreateAnnotationRequest build() const;

private:
    CreateAnnotationRequest request_;
};

// Main annotation client class
class AnnotationClient {
public:
    explicit AnnotationClient(const AnnotationClientConfig& config);
    explicit AnnotationClient(const std::string& server_address);
    explicit AnnotationClient(const std::string& config_path, bool is_config_file);
    ~AnnotationClient();

    // ===================== DataSet Operations =====================
    
    // Create DataSets
    DataSetResult createDataSet(const CreateDataSetRequest& request);
    DataSetResult createDataSet(const DataSet& dataset);
    DataSetResult createDataSet(const std::string& name, const std::string& owner_id,
                               const std::vector<DataBlock>& data_blocks,
                               const std::string& description = "");
    
    // Query DataSets
    QueryDataSetsResult queryDataSets(const QueryDataSetsRequest& request);
    QueryDataSetsResult queryDataSetsByOwner(const std::string& owner_id);
    QueryDataSetsResult queryDataSetsByText(const std::string& text_search);
    QueryDataSetsResult queryDataSetsByPvName(const std::string& pv_name);
    QueryDataSetsResult queryDataSetsById(const std::string& dataset_id);
    
    // ===================== Annotation Operations =====================
    
    // Create Annotations
    AnnotationResult createAnnotation(const CreateAnnotationRequest& request);
    AnnotationResult createAnnotation(const std::string& owner_id, const std::string& name,
                                     const std::vector<std::string>& dataset_ids,
                                     const std::string& comment = "",
                                     const std::vector<std::string>& tags = {},
                                     const std::vector<Attribute>& attributes = {});
    
    // Query Annotations
    QueryAnnotationsResult queryAnnotations(const QueryAnnotationsRequest& request);
    QueryAnnotationsResult queryAnnotationsByOwner(const std::string& owner_id);
    QueryAnnotationsResult queryAnnotationsByDataSet(const std::string& dataset_id);
    QueryAnnotationsResult queryAnnotationsByText(const std::string& text_search);
    QueryAnnotationsResult queryAnnotationsByTag(const std::string& tag);
    QueryAnnotationsResult queryAnnotationsById(const std::string& annotation_id);
    
    // ===================== Export Operations =====================
    
    ExportResult exportData(const ExportDataRequest& request);
    ExportResult exportDataSet(const std::string& dataset_id, 
                              ExportDataRequest::ExportOutputFormat format = ExportDataRequest::EXPORT_FORMAT_CSV);
    ExportResult exportToHDF5(const std::string& dataset_id);
    ExportResult exportToCSV(const std::string& dataset_id);
    ExportResult exportToExcel(const std::string& dataset_id);
    
    // ===================== Convenience Methods =====================
    
    // Create DataSet from time range and PV list
    DataSetResult createDataSetFromTimeRange(const std::string& name, const std::string& owner_id,
                                            const TimeRange& time_range, const std::vector<std::string>& pv_names,
                                            const std::string& description = "");
    
    // Create annotation with calculations
    AnnotationResult createAnnotationWithCalculations(const std::string& owner_id, const std::string& name,
                                                      const std::vector<std::string>& dataset_ids,
                                                      const Calculations& calculations,
                                                      const std::string& comment = "");
    
    // Find related annotations
    std::vector<std::string> findRelatedAnnotations(const std::string& annotation_id);
    
    // Get all DataSets for owner
    std::vector<DataSet> getAllDataSetsForOwner(const std::string& owner_id);
    
    // Get all annotations for DataSet
    std::vector<QueryAnnotationsResponse::AnnotationsResult::Annotation> 
        getAnnotationsForDataSet(const std::string& dataset_id);

    // ===================== Configuration and Control =====================
    
    void updateConfig(const AnnotationClientConfig& config);
    const AnnotationClientConfig& getConfig() const { return config_; }
    
    void setProgressCallback(ProgressCallback callback) { progress_callback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { error_callback_ = callback; }
    
    bool isConnected() const;
    void reconnect();
    
    const std::string& getLastError() const { return last_error_; }

private:
    AnnotationClientConfig config_;
    std::unique_ptr<dp::service::annotation::DpAnnotationService::Stub> stub_;
    
    ProgressCallback progress_callback_;
    ErrorCallback error_callback_;
    std::string last_error_;
    
    // Internal methods
    void initializeConnection();
    void notifyProgress(const std::string& operation, const std::string& status) const;
    void notifyError(const std::string& error, const std::string& context) const;
    
    // Helper methods for processing responses
    DataSetResult processCreateDataSetResponse(const CreateDataSetResponse& response, double processing_time);
    AnnotationResult processCreateAnnotationResponse(const CreateAnnotationResponse& response, double processing_time);
    ExportResult processExportResponse(const ExportDataResponse& response, double processing_time);
    QueryDataSetsResult processQueryDataSetsResponse(const QueryDataSetsResponse& response, double processing_time);
    QueryAnnotationsResult processQueryAnnotationsResponse(const QueryAnnotationsResponse& response, double processing_time);
};

// ===================== Helper Function Declarations =====================

// Timestamp helpers
Timestamp makeTimestamp(uint64_t epoch, uint64_t nano);

// DataSet helpers
DataSet makeDataSet(const std::string& name, const std::string& owner_id, 
                   const std::string& description, const std::vector<DataBlock>& data_blocks);
DataBlock makeDataBlock(const TimeRange& time_range, const std::vector<std::string>& pv_names);
DataBlock makeDataBlock(const Timestamp& begin_time, const Timestamp& end_time, 
                       const std::vector<std::string>& pv_names);

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
    TimeRange createTimeRangeFromDuration(uint64_t start_epoch_sec, uint64_t duration_seconds,
                                         uint64_t start_nano = 0);
    uint64_t getCurrentEpochSeconds();
    TimeRange createRecentTimeRange(uint64_t duration_seconds);
    
    std::string generateDataSetName(const std::string& prefix = "dataset");
    std::string generateAnnotationName(const std::string& prefix = "annotation");
    
    bool validatePvNames(const std::vector<std::string>& pv_names);
    bool validateTimeRange(const TimeRange& time_range);
    std::vector<std::string> extractPvNamesFromDataBlocks(const std::vector<DataBlock>& data_blocks);
}

#endif // ANNOTATION_CLIENT_HPP
