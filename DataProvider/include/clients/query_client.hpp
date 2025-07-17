#ifndef QUERY_CLIENT_HPP
#define QUERY_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>
#include <grpcpp/grpcpp.h>
#include "common.pb.h"
#include "query.pb.h"
#include "query.grpc.pb.h"

using Timestamp = ::Timestamp;
using Attribute = ::Attribute;
using DataColumn = ::DataColumn;
using DataValue = ::DataValue;
using QueryDataRequest = dp::service::query::QueryDataRequest;
using QueryDataResponse = dp::service::query::QueryDataResponse;
using QueryTableRequest = dp::service::query::QueryTableRequest;
using QueryTableResponse = dp::service::query::QueryTableResponse;
using QueryPvMetadataRequest = dp::service::query::QueryPvMetadataRequest;
using QueryPvMetadataResponse = dp::service::query::QueryPvMetadataResponse;
using QueryProvidersRequest = dp::service::query::QueryProvidersRequest;
using QueryProvidersResponse = dp::service::query::QueryProvidersResponse;
using QueryProviderMetadataRequest = dp::service::query::QueryProviderMetadataRequest;
using QueryProviderMetadataResponse = dp::service::query::QueryProviderMetadataResponse;

// Forward declarations
Timestamp makeTimestamp(uint64_t epochSeconds, uint64_t nanoseconds);

// Configuration structure
struct QueryClientConfig {
    std::string server_address = "localhost:50052";
    int connection_timeout_seconds = 5;
    int max_message_size_mb = 64;
    bool enable_spatial_queries = true;
    std::string dictionaries_path = "config/dictionaries/";
    
    static QueryClientConfig fromConfigFile(const std::string& config_path);
    static QueryClientConfig fromJson(const nlohmann::json& config);
};

// Spatial query parameters
struct SpatialQueryParams {
    std::string beam_path;
    std::string area;
    std::vector<std::string> areas;
    std::string device_class;
    std::vector<double> z_range; // [min_z, max_z]
    int area_sequence_min = -1;
    int area_sequence_max = -1;
    
    bool isEmpty() const {
        return beam_path.empty() && area.empty() && areas.empty() && 
               device_class.empty() && z_range.empty() && 
               area_sequence_min == -1 && area_sequence_max == -1;
    }
};

// Time range helper
struct TimeRange {
    uint64_t start_epoch_sec;
    uint64_t start_nano;
    uint64_t end_epoch_sec;
    uint64_t end_nano;
    
    TimeRange(uint64_t start_sec, uint64_t start_ns, uint64_t end_sec, uint64_t end_ns)
        : start_epoch_sec(start_sec), start_nano(start_ns), 
          end_epoch_sec(end_sec), end_nano(end_ns) {}
    
    Timestamp getStartTimestamp() const;
    Timestamp getEndTimestamp() const;
};

// Query result wrapper
struct QueryResult {
    bool success = false;
    std::string error_message;
    size_t total_buckets = 0;
    size_t total_data_points = 0;
    double processing_time_seconds = 0.0;
    std::vector<std::string> pv_names_found;
    std::vector<QueryDataResponse> responses;
    
    void addPvName(const std::string& pv_name) {
        if (std::find(pv_names_found.begin(), pv_names_found.end(), pv_name) == pv_names_found.end()) {
            pv_names_found.push_back(pv_name);
        }
    }
    
    size_t getTotalDataPoints() const {
        size_t total = 0;
        for (const auto& response : responses) {
            if (response.has_querydata()) {
                for (const auto& bucket : response.querydata().databuckets()) {
                    if (bucket.has_datacolumn()) {
                        total += bucket.datacolumn().datavalues_size();
                    }
                }
            }
        }
        return total;
    }
};

// Forward declaration
class SpatialQueryEngine;

// Progress callback for streaming queries
using QueryProgressCallback = std::function<void(size_t buckets_processed, const std::string& current_pv)>;

// Main QueryClient class
class QueryClient {
public:
    explicit QueryClient(const QueryClientConfig& config);
    explicit QueryClient(const std::string& server_address = "localhost:50052");
    explicit QueryClient(const std::string& config_path, bool is_config_file);
    
    ~QueryClient();

    // Basic query methods (existing API)
    QueryDataResponse queryData(const QueryDataRequest& request);
    QueryTableResponse queryTable(const QueryTableRequest& request);
    QueryPvMetadataResponse queryPvMetadata(const QueryPvMetadataRequest& request);
    QueryProvidersResponse queryProviders(const QueryProvidersRequest& request);
    QueryProviderMetadataResponse queryProviderMetadata(const QueryProviderMetadataRequest& request);
    std::vector<QueryDataResponse> queryDataStream(const QueryDataRequest& request);

    // Spatial-aware query methods (new enhanced API)
    QueryResult queryByArea(const std::string& area, const TimeRange& time_range, 
                           const std::vector<std::string>& pv_patterns = {});
    
    QueryResult queryByBeamPath(const std::string& beam_path, const TimeRange& time_range,
                                const std::vector<std::string>& pv_patterns = {});
    
    QueryResult queryByDeviceClass(const std::string& device_class, const TimeRange& time_range,
                                  const std::vector<std::string>& pv_patterns = {});
    
    QueryResult queryByZRange(double min_z, double max_z, const TimeRange& time_range,
                             const std::vector<std::string>& pv_patterns = {});
    
    QueryResult queryBySequenceRange(const std::string& beam_path, int min_sequence, int max_sequence,
                                    const TimeRange& time_range, const std::vector<std::string>& pv_patterns = {});
    
    // Advanced spatial query with multiple criteria
    QueryResult spatialQuery(const SpatialQueryParams& spatial_params, const TimeRange& time_range,
                            const std::vector<std::string>& pv_patterns = {});

    // PV discovery methods
    std::vector<std::string> findPvsByArea(const std::string& area, const std::string& pattern = ".*");
    std::vector<std::string> findPvsByBeamPath(const std::string& beam_path, const std::string& pattern = ".*");
    std::vector<std::string> findPvsByDeviceClass(const std::string& device_class, const std::string& pattern = ".*");
    
    // Metadata queries with spatial context
    std::vector<std::string> getAvailableAreas();
    std::vector<std::string> getAvailableBeamPaths();
    std::vector<std::string> getAvailableDeviceClasses();
    std::vector<std::string> getAreasInBeamPath(const std::string& beam_path);
    
    // Configuration and monitoring
    const QueryClientConfig& getConfig() const { return config_; }
    void updateConfig(const QueryClientConfig& config);
    void setProgressCallback(QueryProgressCallback callback) { progress_callback_ = callback; }
    
    // Spatial query control
    void enableSpatialQueries(bool enable = true);
    bool isSpatialQueriesEnabled() const;
    
    // Connection management
    bool isConnected() const;
    void reconnect();

private:
    QueryClientConfig config_;
    std::unique_ptr<dp::service::query::DpQueryService::Stub> stub_;
    std::unique_ptr<SpatialQueryEngine> spatial_engine_;
    
    QueryProgressCallback progress_callback_;
    bool spatial_queries_enabled_;
    mutable std::string last_error_;
    
    // Internal methods
    void initializeConnection();
    void initializeSpatialEngine();
    std::vector<std::string> expandPvPatterns(const std::vector<std::string>& patterns, 
                                             const SpatialQueryParams& spatial_params);
    QueryDataRequest createQueryRequest(const std::vector<std::string>& pv_names, 
                                       const TimeRange& time_range, bool use_serialized = false);
    QueryResult processQueryResponses(const std::vector<QueryDataResponse>& responses, 
                                     double processing_time);
    void notifyProgress(size_t buckets_processed, const std::string& current_pv) const;
};

// Spatial Query Engine for PV discovery and filtering
class SpatialQueryEngine {
public:
    explicit SpatialQueryEngine(const std::string& dictionaries_path);
    
    bool loadDictionaries();
    std::vector<std::string> findPvsByArea(const std::string& area, const std::string& pattern = ".*") const;
    std::vector<std::string> findPvsByBeamPath(const std::string& beam_path, const std::string& pattern = ".*") const;
    std::vector<std::string> findPvsByDeviceClass(const std::string& device_class, const std::string& pattern = ".*") const;
    std::vector<std::string> findPvsBySpatialParams(const SpatialQueryParams& params, const std::string& pattern = ".*") const;
    
    std::vector<std::string> getAvailableAreas() const;
    std::vector<std::string> getAvailableBeamPaths() const;
    std::vector<std::string> getAvailableDeviceClasses() const;
    std::vector<std::string> getAreasInBeamPath(const std::string& beam_path) const;
    
    bool isLoaded() const { return dictionaries_loaded_; }

private:
    std::string dictionaries_path_;
    bool dictionaries_loaded_ = false;
    
    // Dictionary data structures
    nlohmann::json beamline_boundaries_;
    nlohmann::json device_classifications_;
    nlohmann::json coordinate_systems_;
    nlohmann::json epics_records_;
    
    // Internal methods
    std::vector<std::string> generatePvPatterns(const std::string& area, const std::string& device_pattern = ".*") const;
    std::vector<std::string> getDevicesInArea(const std::string& area) const;
    std::vector<std::string> getDevicesByClass(const std::string& device_class) const;
    bool matchesZRange(const std::string& area, double min_z, double max_z) const;
    bool matchesSequenceRange(const std::string& area, const std::string& beam_path, 
                             int min_sequence, int max_sequence) const;
};

// Helper function declarations
Timestamp makeTimestamp(uint64_t epochSeconds, uint64_t nanoseconds);
QueryDataRequest makeQueryDataRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, 
                                     const Timestamp& endTime, bool useSerializedDataColumns = false);
QueryTableRequest makeQueryTableRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, 
                                       const Timestamp& endTime, 
                                       dp::service::query::QueryTableRequest::TableResultFormat format);
QueryPvMetadataRequest makeQueryPvMetadataRequest(const std::vector<std::string>& pvNames);
QueryPvMetadataRequest makeQueryPvMetadataRequestWithPattern(const std::string& pattern);
QueryProvidersRequest makeQueryProvidersRequest(const std::string& textSearch = "");
QueryProviderMetadataRequest makeQueryProviderMetadataRequest(const std::string& providerId);

// Utility functions for common query patterns
namespace QueryUtils {
    TimeRange createTimeRange(uint64_t start_epoch_sec, uint64_t end_epoch_sec, 
                             uint64_t start_nano = 0, uint64_t end_nano = 0);
    TimeRange createTimeRangeFromDuration(uint64_t start_epoch_sec, uint64_t duration_seconds,
                                         uint64_t start_nano = 0);
    uint64_t getCurrentEpochSeconds();
    TimeRange createRecentTimeRange(uint64_t duration_seconds); // Query last N seconds
    
    SpatialQueryParams createAreaQuery(const std::string& area);
    SpatialQueryParams createBeamPathQuery(const std::string& beam_path);
    SpatialQueryParams createDeviceClassQuery(const std::string& device_class);
    SpatialQueryParams createZRangeQuery(double min_z, double max_z);
    SpatialQueryParams createSequenceQuery(const std::string& beam_path, int min_seq, int max_seq);
    
    // Combine multiple spatial criteria
    SpatialQueryParams combineSpatialParams(const SpatialQueryParams& base, const SpatialQueryParams& additional);
}

#endif // QUERY_CLIENT_HPP
