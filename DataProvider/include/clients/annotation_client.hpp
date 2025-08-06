#ifndef ANNOTATION_CLIENT_HPP
#define ANNOTATION_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <map>
#include <grpcpp/grpcpp.h>

#include "common_client.hpp"  // Use our common client for shared types
#include "annotation.pb.h"
#include "annotation.grpc.pb.h"

// Annotation-specific type aliases
using DataSet = ::dp::service::annotation::DataSet;
using DataBlock = ::dp::service::annotation::DataBlock;

using CreateDataSetRequest = ::dp::service::annotation::CreateDataSetRequest;
using CreateDataSetResponse = ::dp::service::annotation::CreateDataSetResponse;
using CreateDataSetResult = ::dp::service::annotation::CreateDataSetResponse::CreateDataSetResult;

using QueryDataSetsRequest = ::dp::service::annotation::QueryDataSetsRequest;
using QueryDataSetsCriterion = ::dp::service::annotation::QueryDataSetsRequest::QueryDataSetsCriterion;
using QueryDataSetsResponse = ::dp::service::annotation::QueryDataSetsResponse;
using DataSetsResult = ::dp::service::annotation::QueryDataSetsResponse::DataSetsResult;

using Calculations = ::dp::service::annotation::Calculations;
using CalculationsDataFrame = ::dp::service::annotation::Calculations::CalculationsDataFrame;

using CreateAnnotationRequest = ::dp::service::annotation::CreateAnnotationRequest;
using CreateAnnotationResponse = ::dp::service::annotation::CreateAnnotationResponse;
using CreateAnnotationResult = ::dp::service::annotation::CreateAnnotationResponse::CreateAnnotationResult;

using QueryAnnotationsRequest = ::dp::service::annotation::QueryAnnotationsRequest;
using QueryAnnotationsCriterion = ::dp::service::annotation::QueryAnnotationsRequest::QueryAnnotationsCriterion;
using QueryAnnotationsResponse = ::dp::service::annotation::QueryAnnotationsResponse;
using AnnotationsResult = ::dp::service::annotation::QueryAnnotationsResponse::AnnotationsResult;
using Annotation = ::dp::service::annotation::QueryAnnotationsResponse::AnnotationsResult::Annotation;

using ExportDataRequest = ::dp::service::annotation::ExportDataRequest;
using ExportOutputFormat = ::dp::service::annotation::ExportDataRequest::ExportOutputFormat;
using ExportDataResponse = ::dp::service::annotation::ExportDataResponse;
using ExportDataResult = ::dp::service::annotation::ExportDataResponse::ExportDataResult;

using DpAnnotationService = ::dp::service::annotation::DpAnnotationService;

class AnnotationClient {
public:
    // Constructor with channel
    explicit AnnotationClient(std::shared_ptr<grpc::Channel> channel);
    
    // Constructor with address
    explicit AnnotationClient(const std::string& server_address);
    
    ~AnnotationClient();

    // ========== DataSet Operations ==========
    
    // Create a new dataset
    std::optional<std::string> CreateDataSet(
        const std::string& name,
        const std::string& owner_id,
        const std::vector<DataBlock>& data_blocks,
        const std::string& description = "");
    
    // Create dataset with full control
    CreateDataSetResponse CreateDataSetWithResponse(const DataSet& dataset);
    
    // Helper to create DataBlock
    DataBlock CreateDataBlock(
        const Timestamp& begin_time,
        const Timestamp& end_time,
        const std::vector<std::string>& pv_names);
    
    // Helper to create DataSet
    DataSet CreateDataSetObject(
        const std::string& name,
        const std::string& owner_id,
        const std::vector<DataBlock>& data_blocks,
        const std::string& description = "");
    
    // ========== DataSet Query Operations ==========
    
    // Query all datasets
    std::vector<DataSet> QueryAllDataSets();
    
    // Query datasets by ID
    std::vector<DataSet> QueryDataSetById(const std::string& dataset_id);
    
    // Query datasets by owner
    std::vector<DataSet> QueryDataSetsByOwner(const std::string& owner_id);
    
    // Query datasets by text search (searches name and description)
    std::vector<DataSet> QueryDataSetsByText(const std::string& search_text);
    
    // Query datasets by PV name
    std::vector<DataSet> QueryDataSetsByPvName(const std::string& pv_name);
    
    // Query with multiple criteria
    QueryDataSetsResponse QueryDataSets(const std::vector<QueryDataSetsCriterion>& criteria);
    
    // Helper methods to create criteria
    QueryDataSetsCriterion CreateDataSetIdCriterion(const std::string& id);
    QueryDataSetsCriterion CreateDataSetOwnerCriterion(const std::string& owner_id);
    QueryDataSetsCriterion CreateDataSetTextCriterion(const std::string& text);
    QueryDataSetsCriterion CreateDataSetPvNameCriterion(const std::string& pv_name);
    
    // ========== Annotation Operations ==========
    
    // Create a simple annotation
    std::optional<std::string> CreateAnnotation(
        const std::string& owner_id,
        const std::vector<std::string>& dataset_ids,
        const std::string& name,
        const std::string& comment = "",
        const std::vector<std::string>& tags = {},
        const std::vector<Attribute>& attributes = {});
    
    // Create annotation with calculations
    std::optional<std::string> CreateAnnotationWithCalculations(
        const std::string& owner_id,
        const std::vector<std::string>& dataset_ids,
        const std::string& name,
        const Calculations& calculations,
        const std::string& comment = "",
        const std::vector<std::string>& tags = {},
        const std::vector<Attribute>& attributes = {});
    
    // Create annotation with full control
    CreateAnnotationResponse CreateAnnotationWithResponse(const CreateAnnotationRequest& request);
    
    // Helper to create Calculations
    Calculations CreateCalculations(
        const std::string& id,
        const std::vector<CalculationsDataFrame>& frames);
    
    // Helper to create CalculationsDataFrame
    CalculationsDataFrame CreateCalculationsDataFrame(
        const std::string& name,
        const DataTimestamps& timestamps,
        const std::vector<DataColumn>& columns);
    
    // ========== Annotation Query Operations ==========
    
    // Query all annotations
    std::vector<Annotation> QueryAllAnnotations();
    
    // Query annotations by ID
    std::vector<Annotation> QueryAnnotationById(const std::string& annotation_id);
    
    // Query annotations by owner
    std::vector<Annotation> QueryAnnotationsByOwner(const std::string& owner_id);
    
    // Query annotations by dataset ID
    std::vector<Annotation> QueryAnnotationsByDataSet(const std::string& dataset_id);
    
    // Query annotations by associated annotation ID
    std::vector<Annotation> QueryAnnotationsByAnnotation(const std::string& annotation_id);
    
    // Query annotations by text search (searches name, comment, event description)
    std::vector<Annotation> QueryAnnotationsByText(const std::string& search_text);
    
    // Query annotations by tag
    std::vector<Annotation> QueryAnnotationsByTag(const std::string& tag);
    
    // Query annotations by attribute
    std::vector<Annotation> QueryAnnotationsByAttribute(
        const std::string& key,
        const std::string& value);
    
    // Query with multiple criteria
    QueryAnnotationsResponse QueryAnnotations(const std::vector<QueryAnnotationsCriterion>& criteria);
    
    // Helper methods to create criteria
    QueryAnnotationsCriterion CreateAnnotationIdCriterion(const std::string& id);
    QueryAnnotationsCriterion CreateAnnotationOwnerCriterion(const std::string& owner_id);
    QueryAnnotationsCriterion CreateAnnotationDataSetCriterion(const std::string& dataset_id);
    QueryAnnotationsCriterion CreateAnnotationAnnotationCriterion(const std::string& annotation_id);
    QueryAnnotationsCriterion CreateAnnotationTextCriterion(const std::string& text);
    QueryAnnotationsCriterion CreateAnnotationTagCriterion(const std::string& tag);
    QueryAnnotationsCriterion CreateAnnotationAttributeCriterion(
        const std::string& key,
        const std::string& value);
    
    // ========== Export Operations ==========
    
    // Export dataset to file
    std::optional<std::string> ExportDataSetToFile(
        const std::string& dataset_id,
        ExportOutputFormat format = ExportOutputFormat::ExportDataRequest_ExportOutputFormat_EXPORT_FORMAT_CSV);
    
    // Export with URL response
    std::optional<std::string> ExportDataSetToUrl(
        const std::string& dataset_id,
        ExportOutputFormat format = ExportOutputFormat::ExportDataRequest_ExportOutputFormat_EXPORT_FORMAT_CSV);
    
    // Export with full response
    ExportDataResponse ExportData(
        const std::string& dataset_id,
        ExportOutputFormat format = ExportOutputFormat::ExportDataRequest_ExportOutputFormat_EXPORT_FORMAT_CSV);
    
    // ========== Utility Methods ==========
    
    // Extract all PV names from a dataset
    std::vector<std::string> ExtractPvNamesFromDataSet(const DataSet& dataset);
    
    // Extract time range from a dataset
    std::pair<Timestamp, Timestamp> ExtractTimeRangeFromDataSet(const DataSet& dataset);
    
    // Validate dataset has required fields
    bool ValidateDataSet(const DataSet& dataset);
    
    // Validate annotation request
    bool ValidateAnnotationRequest(const CreateAnnotationRequest& request);
    
    // Check if dataset exists
    bool DataSetExists(const std::string& dataset_id);
    
    // Check if annotation exists
    bool AnnotationExists(const std::string& annotation_id);
    
    // Get dataset metadata
    std::optional<DataSet> GetDataSetMetadata(const std::string& dataset_id);
    
    // Get annotation metadata
    std::optional<Annotation> GetAnnotationMetadata(const std::string& annotation_id);
    
    // Connection management
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
        uint64_t datasets_created = 0;
        uint64_t datasets_queried = 0;
        uint64_t annotations_created = 0;
        uint64_t annotations_queried = 0;
        uint64_t exports_performed = 0;
        uint64_t errors = 0;
    };
    
    ClientStats GetStats() const;
    void ResetStats();
    
    // Access to common client
    CommonClient& GetCommonClient();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // ANNOTATION_CLIENT_HPP
