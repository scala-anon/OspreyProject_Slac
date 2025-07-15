#ifndef EPICS_ARCHIVER_HPP
#define EPICS_ARCHIVER_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

// Forward declaration for MLDP integration
class IngestClient;

/**
 * @brief Structure to hold a single EPICS data point
 */
struct EPICSDataPoint {
    uint64_t seconds;        // Unix timestamp seconds
    uint64_t nanoseconds;    // Nanoseconds part
    double value;            // Data value
    int severity;            // EPICS severity
    int status;              // EPICS status
    
    EPICSDataPoint() : seconds(0), nanoseconds(0), value(0.0), severity(0), status(0) {}
    EPICSDataPoint(uint64_t secs, uint64_t nanos, double val, int sev = 0, int stat = 0)
        : seconds(secs), nanoseconds(nanos), value(val), severity(sev), status(stat) {}
};

/**
 * @brief Structure to hold EPICS PV metadata
 */
struct EPICSMetadata {
    std::string pv_name;     // Process Variable name
    std::string units;       // Engineering Units (EGU)
    std::string precision;   // Display precision (PREC)
    
    EPICSMetadata() = default;
    EPICSMetadata(const std::string& name, const std::string& egu = "", const std::string& prec = "")
        : pv_name(name), units(egu), precision(prec) {}
};

/**
 * @brief Complete EPICS time series data
 */
struct EPICSTimeSeries {
    EPICSMetadata metadata;
    std::vector<EPICSDataPoint> data_points;
    
    // Statistics
    size_t total_points() const { return data_points.size(); }
    bool empty() const { return data_points.empty(); }
    
    // Time range
    uint64_t start_time() const { 
        return data_points.empty() ? 0 : data_points.front().seconds; 
    }
    uint64_t end_time() const { 
        return data_points.empty() ? 0 : data_points.back().seconds; 
    }
};

/**
 * @brief Main EPICS Archiver client class
 */
class EPICSArchiver {
private:
    std::string archiver_url_;
    std::string temp_file_prefix_;
    bool verbose_;
    
    // JSON parsing helpers (lightweight, no external dependencies)
    std::string extractJsonValue(const std::string& json, const std::string& key) const;
    std::vector<std::string> extractJsonArray(const std::string& json, const std::string& array_name) const;
    EPICSDataPoint parseDataPoint(const std::string& json_object) const;
    EPICSMetadata parseMetadata(const std::string& json_object) const;
    
    // URL and file utilities
    std::string urlEncode(const std::string& value) const;
    std::string buildQueryURL(const std::string& pv_name, 
                             const std::string& start_time, 
                             const std::string& end_time) const;
    std::string generateTempFileName() const;
    bool executeCommand(const std::string& command) const;
    std::string readFileContents(const std::string& filename) const;
    void cleanupTempFile(const std::string& filename) const;
    
    // Time utilities
    std::string getCurrentISOTime() const;
    std::string getYesterdayISOTime() const;
    std::string expandTimeKeyword(const std::string& time_spec) const;
    
public:
    /**
     * @brief Constructor
     * @param archiver_url Base URL for EPICS archiver (default: LCLS archiver)
     * @param verbose Enable verbose output
     */
    explicit EPICSArchiver(const std::string& archiver_url = "http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json",
                          bool verbose = false);
    
    /**
     * @brief Test connection to EPICS archiver
     * @return true if connection successful
     */
    bool testConnection() const;
    
    /**
     * @brief Fetch data for a single PV
     * @param pv_name Process Variable name (e.g., "IOC:BSY0:BP01:MEM_FREE")
     * @param start_time Start time in ISO format or keyword ("today", "yesterday")
     * @param end_time End time in ISO format or keyword
     * @return EPICSTimeSeries with fetched data
     */
    EPICSTimeSeries fetchPVData(const std::string& pv_name,
                               const std::string& start_time,
                               const std::string& end_time) const;
    
    /**
     * @brief Fetch data for multiple PVs
     * @param pv_names Vector of PV names
     * @param start_time Start time
     * @param end_time End time
     * @return Vector of EPICSTimeSeries
     */
    std::vector<EPICSTimeSeries> fetchMultiplePVs(const std::vector<std::string>& pv_names,
                                                  const std::string& start_time,
                                                  const std::string& end_time) const;
    
    /**
     * @brief Ingest EPICS data to MongoDB via MLDP
     * @param series EPICS time series data
     * @param ingest_client MLDP ingest client
     * @param provider_id MongoDB provider ID
     * @return true if ingestion successful
     */
    bool ingestToMongoDB(const EPICSTimeSeries& series,
                        IngestClient& ingest_client,
                        const std::string& provider_id) const;
    
    /**
     * @brief Ingest multiple PV data series to MongoDB
     * @param series_vector Vector of EPICS time series
     * @param ingest_client MLDP ingest client
     * @param provider_id MongoDB provider ID
     * @param use_streaming Use streaming ingestion for better performance
     * @return true if ingestion successful
     */
    bool ingestMultipleToMongoDB(const std::vector<EPICSTimeSeries>& series_vector,
                                IngestClient& ingest_client,
                                const std::string& provider_id,
                                bool use_streaming = true) const;
    
    /**
     * @brief Parse comma-separated PV list
     * @param pv_list Comma-separated PV names
     * @return Vector of individual PV names
     */
    static std::vector<std::string> parsePVList(const std::string& pv_list);
    
    /**
     * @brief Validate PV name format
     * @param pv_name PV name to validate
     * @return true if valid format
     */
    static bool isValidPVName(const std::string& pv_name);
    
    /**
     * @brief Print statistics for fetched data
     * @param series EPICS time series
     */
    void printDataStatistics(const EPICSTimeSeries& series) const;
    
    /**
     * @brief Print statistics for multiple series
     * @param series_vector Vector of time series
     */
    void printMultipleDataStatistics(const std::vector<EPICSTimeSeries>& series_vector) const;
    
    /**
     * @brief Set verbose mode
     * @param verbose Enable/disable verbose output
     */
    void setVerbose(bool verbose) { verbose_ = verbose; }
};

/**
 * @brief Utility functions for time conversion and formatting
 */
namespace epics_time_utils {
    /**
     * @brief Convert EPICS timestamp to MLDP timestamp format
     * @param seconds Unix seconds
     * @param nanoseconds Nanoseconds
     * @return Combined nanoseconds since epoch
     */
    uint64_t epicsToMLDPTimestamp(uint64_t seconds, uint64_t nanoseconds);
    
    /**
     * @brief Format timestamp for display
     * @param seconds Unix seconds
     * @param nanoseconds Nanoseconds
     * @return Formatted time string
     */
    std::string formatTimestamp(uint64_t seconds, uint64_t nanoseconds);
    
    /**
     * @brief Get current time in ISO 8601 format for EPICS archiver
     * @return ISO formatted time string
     */
    std::string getCurrentTimeISO();
    
    /**
     * @brief Parse relative time keywords
     * @param keyword Time keyword ("today", "yesterday", "1h", "24h")
     * @return ISO formatted time string
     */
    std::string parseTimeKeyword(const std::string& keyword);
}

#endif // EPICS_ARCHIVER_HPP
