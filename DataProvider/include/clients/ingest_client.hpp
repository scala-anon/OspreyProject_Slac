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

using Timestamp = ::Timestamp;
using Attribute = ::Attribute;
using EventMetadata = ::EventMetadata;
using SamplingClock = ::SamplingClock;
using DataValue = ::DataValue;
using DataColumn = ::DataColumn;
using IngestDataRequest = dp::service::ingestion::IngestDataRequest;
using RegisterProviderRequest = dp::service::ingestion::RegisterProviderRequest;
using RegisterProviderResponse = dp::service::ingestion::RegisterProviderResponse;

// Forward declarations
class SpatialEnrichmentEngine;

// Configuration structure
struct IngestClientConfig {
    std::string server_address = "localhost:50051";
    int connection_timeout_seconds = 5;
    int max_message_size_mb = 64;
    bool enable_spatial_enrichment = true;
    std::string dictionaries_path = "config/dictionaries/";
    size_t default_batch_size = 10;
    size_t max_batch_size = 100;
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
    std::string beam_path;              // e.g., "CU_HXR"
    std::string area;                   // e.g., "DMPH"
    std::vector<double> z_coordinates;  // e.g., [3718.483, 3734.407]
    int area_sequence_index = -1;       // e.g., 17
    std::string sector;                 // e.g., "92H"
    std::string device_class;           // e.g., "beam_position_monitor"
    std::string area_description;       // e.g., "HXR post-undulator line"
    
    // New fields for complete PV parsing
    std::string device_type;            // e.g., "BPMS"
    std::string device_location;        // e.g., "502"
    std::string device_attribute;       // e.g., "TMITBR"
    std::string full_pv_name;          // e.g., "BPMS:DMPH:502:TMITBR"
    
    bool isValid() const {
        return !beam_path.empty() && !area.empty() && area_sequence_index >= 0;
    }
    
    // Get specific device coordinates if available
    std::vector<double> getDeviceCoordinates() const {
        // If we have specific device location coordinates, return those
        // Otherwise return area range coordinates
        return z_coordinates;
    }
    
    // Generate location-specific identifier
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
        return total_requests > 0 ? (double)successful_requests / total_requests : 0.0;
    }
};

// Main IngestClient class
class IngestClient {
public:
    explicit IngestClient(const IngestClientConfig& config);
    explicit IngestClient(const std::string& server_address = "localhost:50051");
    explicit IngestClient(const std::string& config_path, bool is_config_file);
    
    ~IngestClient();

    // Provider management
    RegisterProviderResponse registerProvider(const RegisterProviderRequest& request);
    RegisterProviderResponse registerProvider(const std::string& name, 
                                            const std::vector<Attribute>& attributes = {},
                                            const std::vector<std::string>& tags = {});

    // Core ingestion methods
    std::string ingestData(const IngestDataRequest& request);
    std::string ingestDataStream(const std::vector<IngestDataRequest>& requests);
    
    // Batch ingestion with spatial enrichment
    IngestionResult ingestBatch(const std::vector<IngestDataRequest>& requests,
                               const std::string& provider_id = "");
    
    // Spatial-aware ingestion (main new feature)
    IngestionResult ingestWithSpatialEnrichment(const std::vector<IngestDataRequest>& requests,
                                               const std::string& provider_id = "");
    
    // Progress monitoring
    void setProgressCallback(ProgressCallback callback) { progress_callback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { error_callback_ = callback; }
    
    // Configuration
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
    std::unique_ptr<SpatialEnrichmentEngine> spatial_engine_;
    
    ProgressCallback progress_callback_;
    ErrorCallback error_callback_;
    
    bool spatial_enrichment_enabled_;
    mutable std::string last_error_;
    
    // Internal methods
    void initializeConnection();
    void initializeSpatialEngine();
    IngestDataRequest enrichRequest(const IngestDataRequest& request) const;
    std::vector<std::vector<IngestDataRequest>> chunkRequestsToVector(const std::vector<IngestDataRequest>& requests, 
                                                                     size_t chunk_size) const;
    void notifyProgress(size_t processed, size_t total, size_t successful) const;
    void notifyError(const std::string& error, const std::string& context) const;
};

// Spatial Enrichment Engine
class SpatialEnrichmentEngine {
public:
    explicit SpatialEnrichmentEngine(const std::string& dictionaries_path);
    
    bool loadDictionaries();
    SpatialContext enrichPvName(const std::string& pv_name) const;
    std::vector<Attribute> generateSpatialAttributes(const SpatialContext& context) const;
    std::vector<std::string> generateSpatialTags(const SpatialContext& context) const;
    
    bool isLoaded() const { return dictionaries_loaded_; }

private:
    std::string dictionaries_path_;
    bool dictionaries_loaded_ = false;
    
    // Dictionary data structures
    nlohmann::json beamline_boundaries_;
    nlohmann::json device_classifications_;
    nlohmann::json coordinate_systems_;
    nlohmann::json epics_records_;
    
    // Internal parsing methods
    std::string extractDevice(const std::string& pv_name) const;
    std::string extractDeviceArea(const std::string& pv_name) const;
    std::string extractDeviceLocation(const std::string& pv_name) const;
    std::string extractDeviceAttribute(const std::string& pv_name) const;
    std::string extractSector(const std::string& area, const std::string& location) const;
    std::string findBeamPath(const std::string& device_area) const;
    std::vector<double> getZCoordinates(const std::string& area, const std::string& location) const;
    int getAreaSequenceIndex(const std::string& area, const std::string& beam_path) const;
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
RegisterProviderRequest makeRegisterProviderRequest(const std::string& providerName, 
                                                   const std::vector<Attribute>& attributes, 
                                                   uint64_t epoch, uint64_t nano);

// Utility functions for common patterns
namespace IngestUtils {
    uint64_t getCurrentEpochSeconds();
    uint64_t getCurrentEpochNanos();
    std::string generateRequestId(const std::string& prefix = "req");
    std::vector<Attribute> mergeAttributes(const std::vector<Attribute>& base, 
                                          const std::vector<Attribute>& additional);
    std::vector<std::string> mergeTags(const std::vector<std::string>& base, 
                                      const std::vector<std::string>& additional);
}

#endif // INGEST_CLIENT_HPP
