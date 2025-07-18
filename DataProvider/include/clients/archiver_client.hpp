#ifndef ARCHIVER_CLIENT_HPP
#define ARCHIVER_CLIENT_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

// EPICS data point structure
struct EpicsDataPoint {
    uint64_t secs;
    uint64_t nanos;
    double value;
    int severity;
    int status;
    std::map<std::string, std::string> fields;
};

// EPICS metadata structure
struct EpicsMetadata {
    std::string name;
    std::string description;
    std::map<std::string, std::string> enums;
    std::map<std::string, std::string> properties; // PREC, DRVH, etc.
};

// Archiver response structure
struct ArchiverResponse {
    EpicsMetadata metadata;
    std::vector<EpicsDataPoint> data_points;
    bool success = false;
    std::string error_message;
};

// Configuration for archiver client
struct ArchiverClientConfig {
    std::string base_url = "http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json";
    int timeout_seconds = 30;
    bool fetch_metadata = true;
    
    static ArchiverClientConfig fromConfigFile(const std::string& config_path);
};

// Main archiver client class
class ArchiverClient {
public:
    explicit ArchiverClient(const ArchiverClientConfig& config = ArchiverClientConfig{});
    ~ArchiverClient();
    
    // Single PV query
    ArchiverResponse queryPv(const std::string& pv_name, 
                            const std::string& start_time,
                            const std::string& end_time);
    
    // Multiple PV query
    std::vector<ArchiverResponse> queryPvs(const std::vector<std::string>& pv_names,
                                          const std::string& start_time, 
                                          const std::string& end_time);
    
    // Query with date strings (MM-DD-YYYY format)
    ArchiverResponse queryPvByDate(const std::string& pv_name,
                                  const std::string& date_str,
                                  int hours = 24);
    
    // Utility methods
    bool isConnected() const { return curl_handle_ != nullptr; }
    const std::string& getLastError() const { return last_error_; }
    
    // Static utility methods
    static std::string formatTimeString(uint64_t epoch_sec, uint64_t nano_sec = 0);
    static std::string dateToIsoString(const std::string& date_str, int hour_offset = 0);
    static uint64_t parseDate(const std::string& date_str);

private:
    ArchiverClientConfig config_;
    CURL* curl_handle_;
    mutable std::string last_error_;
    
    // Internal methods
    void initializeCurl();
    void cleanup();
    std::string buildUrl(const std::string& pv_name, 
                        const std::string& start_time,
                        const std::string& end_time) const;
    std::string performRequest(const std::string& url);
    ArchiverResponse parseJsonResponse(const std::string& json_data) const;
    EpicsMetadata parseMetadata(const json& meta_json) const;
    std::vector<EpicsDataPoint> parseDataPoints(const json& data_json) const;
    std::string urlEncode(const std::string& input) const;
    
    // Curl callback
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* response);
};

// Utility functions for time conversion
namespace ArchiverUtils {
    std::string getCurrentIsoTime();
    std::string epochToIsoTime(uint64_t epoch_sec, uint64_t nano_sec = 0);
    uint64_t isoTimeToEpoch(const std::string& iso_time);
    std::string createTimeRange(const std::string& date_str, int hours = 24);
    
    // EPICS-specific utilities
    std::string severityToString(int severity);
    std::string statusToString(int status);
    bool isValidEpicsValue(double value, int severity, int status);
}

#endif // ARCHIVER_CLIENT_HPP
