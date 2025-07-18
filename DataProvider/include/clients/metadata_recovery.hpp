#ifndef METADATA_RECOVERY_HPP
#define METADATA_RECOVERY_HPP

#include "query_client.hpp"
#include <map>
#include <set>
#include <regex>
#include <fstream>
#include <algorithm>

// Structure to hold discovered metadata
struct DiscoveredPvMetadata {
    std::string pv_name;
    std::string last_bucket_id;
    int32_t data_type_case = 0;
    std::string data_type_name;
    int32_t timestamps_case = 0;
    std::string timestamps_type;
    uint32_t sample_count = 0;
    uint64_t sample_period_nanos = 0;
    Timestamp first_data_timestamp;
    Timestamp last_data_timestamp;
    int32_t num_buckets = 0;

    // Derived/inferred metadata
    std::string area;
    std::string device_type;
    std::string signal_type;
    std::string beam_path;

    bool isValid() const {
        return !pv_name.empty() && num_buckets > 0;
    }

    bool hasRecentData(uint64_t cutoff_epoch_seconds) const {
        return last_data_timestamp.epochseconds() >= cutoff_epoch_seconds;
    }
};

// Structure for provider metadata
struct ProviderInfo {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> tags;
    std::vector<std::pair<std::string, std::string>> attributes;
    std::vector<std::string> pv_names;
    Timestamp first_bucket_time;
    Timestamp last_bucket_time;
    int32_t num_buckets = 0;
};

// Main metadata recovery class
class MetadataRecoveryEngine {
private:
    QueryClient& query_client_;
    std::map<std::string, DiscoveredPvMetadata> discovered_metadata_;
    std::map<std::string, ProviderInfo> provider_info_;
    std::vector<std::string> missing_pvs_;
    std::vector<std::string> inactive_pvs_;

    // Configuration
    uint64_t recent_data_cutoff_hours_ = 24;
    size_t batch_size_ = 1000;
    bool enable_pattern_discovery_ = true;

public:
    explicit MetadataRecoveryEngine(QueryClient& client) : query_client_(client) {}

    // Main recovery methods
    bool discoverAllAvailablePvs();
    bool validateExistingMetadata(const std::vector<std::string>& known_pvs);
    bool recoverMissingMetadata(const std::vector<std::string>& missing_pvs);

    // Pattern-based discovery
    std::vector<std::string> discoverPvsByPattern(const std::string& pattern);
    std::vector<std::string> discoverPvsByArea(const std::string& area);
    std::vector<std::string> discoverPvsByDeviceType(const std::string& device_type);

    // Provider-based discovery
    bool discoverAllProviders();
    std::vector<std::string> getPvsFromProvider(const std::string& provider_id);

    // Metadata enrichment
    bool enrichMetadataFromNaming();
    bool inferSpatialInformation();
    bool categorizeByDeviceType();

    // Export/Import functionality
    bool exportDiscoveredMetadata(const std::string& output_file, const std::string& format = "json");
    bool importExistingMetadata(const std::string& input_file);

    // Validation and health checks
    std::vector<std::string> findInactivePvs(uint64_t cutoff_hours = 24);
    std::vector<std::string> findPvsWithInconsistentData();
    std::map<std::string, std::vector<std::string>> groupPvsByProvider();

    // Getters
    const std::map<std::string, DiscoveredPvMetadata>& getDiscoveredMetadata() const {
        return discovered_metadata_;
    }
    const std::vector<std::string>& getMissingPvs() const { return missing_pvs_; }
    const std::vector<std::string>& getInactivePvs() const { return inactive_pvs_; }

    // Configuration
    void setRecentDataCutoff(uint64_t hours) { recent_data_cutoff_hours_ = hours; }
    void setBatchSize(size_t size) { batch_size_ = size; }
    void enablePatternDiscovery(bool enable) { enable_pattern_discovery_ = enable; }

private:
    // Internal helper methods
    void processPvMetadataResponse(const QueryPvMetadataResponse& response);
    void processProviderResponse(const QueryProvidersResponse& response);
    void processProviderMetadata(const QueryProviderMetadataResponse& response);

    // Metadata inference
    std::string inferAreaFromPvName(const std::string& pv_name);
    std::string inferDeviceTypeFromPvName(const std::string& pv_name);
    std::string inferSignalTypeFromPvName(const std::string& pv_name);
    std::string inferBeamPathFromArea(const std::string& area);

    // Batch processing
    void processPvBatch(const std::vector<std::string>& pv_batch);
    std::vector<std::vector<std::string>> createBatches(const std::vector<std::string>& pvs);
};

#endif // METADATA_RECOVERY_HPP
