#ifndef EPICS_PARSER_HPP
#define EPICS_PARSER_HPP

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cstdint>
#include <chrono>
#include <curl/curl.h>

// Reuse existing structures from h5_parser.hpp
struct EPICSDataPoint {
    uint64_t seconds;           // secs from EPICS response
    uint64_t nanoseconds;       // nanos from EPICS response
    double value;               // val from EPICS response
    uint16_t severity;          // severity from EPICS response
    uint16_t status;            // status from EPICS response
    
    EPICSDataPoint(uint64_t s, uint64_t ns, double v, uint16_t sev = 0, uint16_t stat = 0)
        : seconds(s), nanoseconds(ns), value(v), severity(sev), status(stat) {}
};

// EPICS-specific signal information
struct EPICSSignalInfo {
    std::string pv_name;            // Full EPICS PV name: "DEVICE:AREA:LOCATION:ATTRIBUTE"
    std::string device;             // "BPMS", "KLYS", "ACCL", etc.
    std::string area;               // "DMPH", "LI23", "BSY0", etc.
    std::string location;           // "502", "31", "MP01", etc.
    std::string attribute;          // "X", "AMPL", "PC_RATE", etc.
    std::string signal_type;        // "control", "status", "measurement"
    std::string units;              // Inferred from attribute
    std::string precision;          // PREC from EPICS meta
    
    EPICSSignalInfo() : signal_type("control"), units("unknown"), precision("0") {}
};

// EPICS time range specification
struct EPICSTimeRange {
    std::string from_iso;           // ISO 8601 format: "2025-07-13T11:34:31.000Z"
    std::string to_iso;             // ISO 8601 format: "2025-07-14T11:34:31.000Z"
    uint64_t from_epoch;            // Unix timestamp
    uint64_t to_epoch;              // Unix timestamp
    
    EPICSTimeRange(uint64_t start_epoch, uint64_t end_epoch);
    EPICSTimeRange(const std::string& from, const std::string& to);
};

// Complete EPICS signal data (compatible with H5 SignalData)
struct EPICSSignalData {
    EPICSSignalInfo info;                    // EPICS-specific metadata
    std::vector<EPICSDataPoint> data_points; // Time series data
    EPICSTimeRange time_range;               // Query time range
    
    // Statistics
    size_t sample_count;
    double min_value, max_value, avg_value;
    uint64_t start_time_sec, end_time_sec;
    std::string query_status;                // "success", "no_data", "error"
    
    EPICSSignalData(const std::string& pv_name, const EPICSTimeRange& range);
    void calculateStats();
    
    // Convert to H5-compatible SignalData for MLDP ingestion
    struct SignalData toSignalData() const;
};

// HTTP response structure
struct HTTPResponse {
    std::string data;
    long response_code;
    std::string error_message;
    
    HTTPResponse() : response_code(0) {}
    bool isSuccess() const { return response_code == 200; }
};

// Main EPICS archiver parser class
class EPICSParser {
public:
    // Constructor with EPICS archiver configuration
    explicit EPICSParser(const std::string& management_url = "http://lcls-archapp01:17665/mgmt/bpl/",
                        const std::string& retrieval_url = "http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json");
    ~EPICSParser();
    
    // === PV DISCOVERY METHODS ===
    
    // Discover PVs matching regex pattern
    std::vector<std::string> discoverPVs(const std::string& regex_pattern = ".*", 
                                         size_t limit = 1000) const;
    
    // Discover PVs by device type
    std::vector<std::string> discoverByDevice(const std::string& device_pattern, 
                                             size_t limit = 1000) const;
    
    // Get all available appliances
    std::vector<std::string> getAvailableAppliances() const;
    
    // Test connection to EPICS archiver
    bool testConnection() const;
    
    // === DATA QUERY METHODS ===
    
    // Query single PV data
    EPICSSignalData queryPV(const std::string& pv_name, 
                           const EPICSTimeRange& time_range) const;
    
    // Query multiple PVs (parallel requests)
    std::vector<EPICSSignalData> queryMultiplePVs(const std::vector<std::string>& pv_names,
                                                  const EPICSTimeRange& time_range,
                                                  bool parallel = true) const;
    
    // Query PVs by pattern (discover + query)
    std::vector<EPICSSignalData> queryByPattern(const std::string& pattern,
                                               const EPICSTimeRange& time_range,
                                               size_t limit = 100) const;
    
    // === BATCH PROCESSING METHODS ===
    
    // Query data in time windows (for large time ranges)
    std::vector<EPICSSignalData> queryTimeWindows(const std::vector<std::string>& pv_names,
                                                  uint64_t start_epoch, uint64_t end_epoch,
                                                  uint64_t window_hours = 24) const;
    
    // Query recent data (last N hours)
    std::vector<EPICSSignalData> queryRecentData(const std::vector<std::string>& pv_names,
                                                 uint64_t hours_back = 24) const;
    
    // === UTILITY METHODS ===
    
    // Print discovery statistics
    void printDiscoveryStats(const std::vector<std::string>& pv_names) const;
    
    // Print data statistics
    void printDataStats(const std::vector<EPICSSignalData>& signals) const;
    
    // Export PV list to file
    bool exportPVList(const std::vector<std::string>& pv_names, 
                     const std::string& filename) const;
    
    // Configuration
    void setRetrievalURL(const std::string& url) { retrieval_url_ = url; }
    void setManagementURL(const std::string& url) { management_url_ = url; }
    void setTimeout(long timeout_seconds) { timeout_ = timeout_seconds; }
    void setVerbose(bool verbose) { verbose_ = verbose; }
    
private:
    std::string management_url_;
    std::string retrieval_url_;
    long timeout_;
    bool verbose_;
    
    // HTTP client state
    CURL* curl_handle_;
    
    // === HTTP REQUEST METHODS ===
    
    // Generic HTTP GET request
    HTTPResponse httpGet(const std::string& url) const;
    
    // Build discovery URL
    std::string buildDiscoveryURL(const std::string& pattern, size_t limit) const;
    
    // Build data query URL
    std::string buildDataURL(const std::string& pv_name, const EPICSTimeRange& range) const;
    
    // === JSON PARSING METHODS ===
    
    // Parse PV discovery response
    std::vector<std::string> parseDiscoveryResponse(const std::string& json_response) const;
    
    // Parse single PV data response
    EPICSSignalData parseDataResponse(const std::string& json_response, 
                                     const std::string& pv_name,
                                     const EPICSTimeRange& time_range) const;
    
    // Parse EPICS metadata
    EPICSSignalInfo parseSignalInfo(const std::string& pv_name, 
                                   const std::string& json_meta) const;
    
    // === DATA PROCESSING METHODS ===
    
    // Parse EPICS PV name components
    EPICSSignalInfo parsePVName(const std::string& pv_name) const;
    
    // Infer signal properties from PV name
    std::string inferSignalType(const std::string& device, const std::string& attribute) const;
    std::string inferUnits(const std::string& attribute) const;
    
    // Validate data consistency
    bool validateEPICSData(const EPICSSignalData& signal) const;
    
    // === ERROR HANDLING ===
    
    // Handle HTTP errors
    void handleHTTPError(const HTTPResponse& response, const std::string& context) const;
    
    // Handle JSON parsing errors
    void handleJSONError(const std::string& error, const std::string& context) const;
    
    // === CURL CALLBACK ===
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

// === UTILITY FUNCTIONS ===

// Time conversion utilities
namespace EPICSTimeUtils {
    // Current Unix timestamp
    uint64_t getCurrentEpochTime();
    
    // Convert Unix timestamp to ISO 8601
    std::string epochToISO8601(uint64_t epoch_time);
    
    // Convert ISO 8601 to Unix timestamp  
    uint64_t iso8601ToEpoch(const std::string& iso_time);
    
    // Get time N hours ago
    uint64_t getHoursAgo(uint64_t hours);
    
    // Create EPICSTimeRange for last N hours
    EPICSTimeRange createRecentTimeRange(uint64_t hours_back);
    
    // Create EPICSTimeRange from epoch times
    EPICSTimeRange createTimeRange(uint64_t start_epoch, uint64_t end_epoch);
}

// EPICS-specific patterns for LCLS
namespace EPICSPatterns {
    // Common device patterns
    extern const std::vector<std::pair<std::string, std::string>> DEVICE_PATTERNS;
    
    // Signal type patterns
    extern const std::vector<std::pair<std::string, std::string>> SIGNAL_PATTERNS;
    
    // Get predefined patterns
    std::vector<std::string> getDevicePatterns();
    std::vector<std::string> getStatusPatterns();
    std::vector<std::string> getControlPatterns();
    
    // Validate PV name format
    bool isValidEPICSPVName(const std::string& pv_name);
}

#endif // EPICS_PARSER_HPP
