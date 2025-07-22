#ifndef INGEST_CLIENT_HPP
#define INGEST_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "common.pb.h"
#include "ingestion.pb.h"
#include "ingestion.grpc.pb.h"
#include "spatial_analyzer.hpp"

using Timestamp = ::Timestamp;
using Attribute = ::Attribute;
using EventMetadata = ::EventMetadata;
using SamplingClock = ::SamplingClock;
using DataValue = ::DataValue;
using DataColumn = ::DataColumn;
using IngestDataRequest = dp::service::ingestion::IngestDataRequest;
using RegisterProviderRequest = dp::service::ingestion::RegisterProviderRequest;
using RegisterProviderResponse = dp::service::ingestion::RegisterProviderResponse;

// Configuration structure
struct IngestClientConfig {
    std::string server_address = "localhost:50051";
    int connection_timeout_seconds = 5;
    int max_message_size_mb = 64;
    bool enable_spatial_enrichment = true;
    std::string dictionaries_path = "config/dictionaries/";
    size_t default_batch_size = 100;
    size_t max_batch_size = 1000;
    bool streaming_preferred = true;
    int retry_attempts = 3;
    int retry_delay_ms = 1000;
    bool enable_progress_monitoring = true;

    static IngestClientConfig fromConfigFile(const std::string& config_path);
    static IngestClientConfig fromJson(const nlohmann::json& config);
};

// Callback types for progress monitoring
using ProgressCallback = std::function<void(size_t processed, size_t total, size_t successful)>;
using ErrorCallback = std::function<void(const std::string& error, const std::string& context)>;

// Spatial context structure
struct SpatialContext {
    std::string beam_path;
    std::string area;
    std::vector<double> z_coordinates;
    int area_sequence_index = -1;
    std::string sector;
    std::string device_class;
    std::string area_description;
    std::string device_type;
    std::string device_location;
    std::string device_attribute;
    std::string full_pv_name;

    bool isValid() const {
        return !beam_path.empty() && !area.empty() && area_sequence_index >= 0;
    }

    std::vector<double> getDeviceCoordinates() const {
        return z_coordinates;
    }

    std::string getLocationId() const {
        if (!area.empty() && !device_location.empty()) {
            return area + "_" + device_location;
        }
        return area;
    }
};

// Result structures
struct IngestionResult {
    bool success = false;
    size_t total_requests = 0;
    size_t successful_requests = 0;
    size_t failed_requests = 0;
    std::vector<std::string> error_messages;
    std::string provider_id;
    double processing_time_seconds = 0.0;

    double getSuccessRate() const {
        return total_requests > 0 ? static_cast<double>(successful_requests) / total_requests : 0.0;
    }
};

class IngestClient {
public:
    explicit IngestClient(const IngestClientConfig& config);
    explicit IngestClient(const std::string& config_file_path);
    ~IngestClient();

    // Provider registration
    RegisterProviderResponse registerProvider(const RegisterProviderRequest& request);
    RegisterProviderResponse registerProvider(const std::string& name,
                                             const std::vector<Attribute>& attributes,
                                             const std::vector<std::string>& tags);

    // Data ingestion methods
    std::string ingestData(const IngestDataRequest& request);
    IngestionResult ingestBatch(const std::vector<IngestDataRequest>& requests, const std::string& provider_id);

    // Configuration and callbacks
    void setProgressCallback(ProgressCallback callback) { progress_callback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { error_callback_ = callback; }
    const IngestClientConfig& getConfig() const { return config_; }
    void updateConfig(const IngestClientConfig& config);

    // Spatial enrichment control
    void enableSpatialEnrichment(bool enable = true);
    bool isSpatialEnrichmentEnabled() const;
    SpatialContext enrichPvName(const std::string& pv_name) const;

    // Connection management
    bool isConnected() const;
    void reconnect();

private:
    IngestClientConfig config_;
    std::unique_ptr<dp::service::ingestion::DpIngestionService::Stub> stub_;
    std::unique_ptr<SpatialAnalyzer> spatial_analyzer_;
    
    ProgressCallback progress_callback_;
    ErrorCallback error_callback_;

    bool spatial_enrichment_enabled_;

    // Internal methods
    void initializeConnection();
    void initializeSpatialAnalyzer();
    
    // Spatial enrichment with caching
    IngestDataRequest enrichRequest(const IngestDataRequest& request) const;
    std::vector<IngestDataRequest> enrichRequestsBatch(const std::vector<IngestDataRequest>& requests) const;
    
    // Bulk operation helpers  
    std::string sendBatchToServerStream(const std::vector<IngestDataRequest>& batch);
    std::vector<std::vector<IngestDataRequest>> chunkRequestsToVector(const std::vector<IngestDataRequest>& requests,
                                                                     size_t chunk_size) const;
    
    // Monitoring and callbacks
    void notifyProgress(size_t processed, size_t total, size_t successful) const;
    void notifyError(const std::string& error, const std::string& context) const;
};

// Helper function declarations
Timestamp makeTimeStamp(uint64_t epoch, uint64_t nano);
Attribute makeAttribute(const std::string& name, const std::string& value);
EventMetadata makeEventMetadata(const std::string& desc, uint64_t startEpoch, uint64_t startNano,
                               uint64_t endEpoch, uint64_t endNano);
SamplingClock makeSamplingClock(uint64_t epoch, uint64_t nano, uint64_t periodNanos, uint32_t count);
DataValue makeDataValueWithSInt32(int val);
DataValue makeDataValueWithUInt64(uint64_t val);
DataValue makeDataValueWithDouble(double val);
DataValue makeDataValueWithTimestamp(uint64_t sec, uint64_t nano);
DataColumn makeDataColumn(const std::string& name, const std::vector<DataValue>& values);
IngestDataRequest makeIngestDataRequest(const std::string& providerId, const std::string& clientRequestId,
                                       const std::vector<Attribute>& attributes, const std::vector<std::string>& tags,
                                       const EventMetadata& metadata, const SamplingClock& samplingClock,
                                       const std::vector<DataColumn>& dataColumns);

// Utility functions
namespace IngestUtils {
    uint64_t getCurrentEpochSeconds();
    uint64_t getCurrentEpochNanos();
    std::string generateRequestId(const std::string& prefix = "req");
}

#endif // INGEST_CLIENT_HPP