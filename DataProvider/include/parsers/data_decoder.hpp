#ifndef DATA_DECODER_HPP
#define DATA_DECODER_HPP

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <iostream>
#include <iomanip>
#include <map>
#include <grpcpp/grpcpp.h>

// Include our query client and protobuf definitions
#include "clients/query_client.hpp"
#include "common.pb.h"
#include "query.pb.h"

/**
 * @brief Structure to hold a decoded data point with timestamp and value
 */
struct DecodedDataPoint {
    std::chrono::system_clock::time_point timestamp;
    double value;
    std::string status;
    uint32_t severity;

    DecodedDataPoint(std::chrono::system_clock::time_point ts, double val,
                     const std::string& stat = "OK", uint32_t sev = 0)
        : timestamp(ts), value(val), status(stat), severity(sev) {}
};

/**
 * @brief Structure to hold decoded time series data for a single PV
 */
struct DecodedPVSeries {
    std::string pv_name;
    std::string description;
    std::string units;
    std::vector<DecodedDataPoint> data_points;

    // Statistics
    double min_value = 0.0;
    double max_value = 0.0;
    double avg_value = 0.0;
    size_t total_points = 0;

    // Constructor
    explicit DecodedPVSeries(const std::string& name) : pv_name(name) {}

    // Calculate basic statistics
    void calculateStats();

    // Get data in time range
    std::vector<DecodedDataPoint> getDataInRange(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end) const;
};

/**
 * @brief Main class for decoding MongoDB DataColumn bytes into readable format
 */
class DataDecoder {
private:
    std::unique_ptr<QueryClient> query_client_;
    std::string server_address_;

    // Helper methods for decoding
    std::chrono::system_clock::time_point convertTimestamp(const Timestamp& ts) const;
    std::vector<std::chrono::system_clock::time_point> decodeTimestamps(
        const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket) const;
    std::vector<double> decodeDataValues(const DataColumn& column) const;
    std::vector<double> decodeSerializedDataColumn(const SerializedDataColumn& serialized_column) const;

    // Format helpers
    std::string formatTimestamp(const std::chrono::system_clock::time_point& tp) const;
    std::string formatDuration(std::chrono::seconds duration) const;

    // NEW: Streaming support methods
    /**
     * @brief Process multiple streaming responses into decoded series
     * @param responses Vector of streaming query responses
     * @return Vector of decoded PV series  
     */
    std::vector<DecodedPVSeries> processStreamingResponses(
        const std::vector<QueryDataResponse>& responses) const;

public:
    /**
     * @brief Constructor
     * @param server_address gRPC server address (default: localhost:50052)
     */
    explicit DataDecoder(const std::string& server_address = "localhost:50052");

    /**
     * @brief Destructor
     */
    ~DataDecoder() = default;

    /**
     * @brief Query and decode PV data for specific PVs
     * @param pv_names List of PV names to query
     * @param start_time Start time (Unix timestamp)
     * @param end_time End time (Unix timestamp)
     * @param use_serialized Use serialized data columns for performance
     * @return Vector of decoded PV series
     */
    std::vector<DecodedPVSeries> queryAndDecode(
        const std::vector<std::string>& pv_names,
        uint64_t start_time,
        uint64_t end_time,
        bool use_serialized = true);

    /**
     * @brief Query and decode PV data using pattern matching
     * @param pattern Regex pattern for PV names
     * @param start_time Start time (Unix timestamp)
     * @param end_time End time (Unix timestamp)
     * @param use_serialized Use serialized data columns for performance
     * @return Vector of decoded PV series
     */
    std::vector<DecodedPVSeries> queryAndDecodeByPattern(
        const std::string& pattern,
        uint64_t start_time,
        uint64_t end_time,
        bool use_serialized = true);

    /**
     * @brief Query and decode PV data using streaming API (for large pattern datasets)
     * @param pattern Regex pattern for PV names
     * @param start_time Start time (Unix timestamp)
     * @param end_time End time (Unix timestamp)
     * @param use_serialized Use serialized data columns for performance
     * @return Vector of decoded PV series
     */
    std::vector<DecodedPVSeries> queryAndDecodeByPatternStreaming(
        const std::string& pattern,
        uint64_t start_time,
        uint64_t end_time,
        bool use_serialized = true);

    /**
     * @brief Query and decode using streaming API (for large datasets)
     * @param pv_names List of PV names to query
     * @param start_time Start time (Unix timestamp)
     * @param end_time End time (Unix timestamp)
     * @param use_serialized Use serialized data columns for performance
     * @return Vector of decoded PV series
     */
    std::vector<DecodedPVSeries> queryAndDecodeStreaming(
        const std::vector<std::string>& pv_names,
        uint64_t start_time,
        uint64_t end_time,
        bool use_serialized = true);

    /**
     * @brief Discover available PVs matching pattern
     * @param pattern Regex pattern for PV names (default: ".*")
     * @return Vector of PV names
     */
    std::vector<std::string> discoverPVs(const std::string& pattern = ".*") const;

    /**
     * @brief Query PV metadata for pattern (used for time range discovery)
     * @param request QueryPvMetadataRequest
     * @return QueryPvMetadataResponse
     */
    QueryPvMetadataResponse queryPvMetadata(const QueryPvMetadataRequest& request) const;

    /**
     * @brief Decode a single QueryDataResponse into readable format
     * @param response Query response from MongoDB
     * @return Vector of decoded PV series
     */
    std::vector<DecodedPVSeries> decodeQueryResponse(const QueryDataResponse& response) const;

    /**
     * @brief Print formatted decoded data to console
     * @param decoded_data Vector of decoded PV series
     * @param max_points Maximum points to display per PV (default: 10)
     * @param show_stats Show statistics summary (default: true)
     */
    void printDecodedData(const std::vector<DecodedPVSeries>& decoded_data,
                         size_t max_points = 10,
                         bool show_stats = true) const;

    /**
     * @brief Export decoded data to CSV format
     * @param decoded_data Vector of decoded PV series
     * @param filename Output CSV filename
     * @return true if successful
     */
    bool exportToCSV(const std::vector<DecodedPVSeries>& decoded_data,
                     const std::string& filename) const;

    /**
     * @brief Export decoded data to JSON format
     * @param decoded_data Vector of decoded PV series
     * @param filename Output JSON filename
     * @return true if successful
     */
    bool exportToJSON(const std::vector<DecodedPVSeries>& decoded_data,
                      const std::string& filename) const;

    /**
     * @brief Print summary statistics for decoded data
     * @param decoded_data Vector of decoded PV series
     */
    void printStatistics(const std::vector<DecodedPVSeries>& decoded_data) const;

    /**
     * @brief Test connection to data platform
     * @return true if connection successful
     */
    bool testConnection() const;

    /**
     * @brief Get raw query response for debugging
     * @param pv_names List of PV names
     * @param start_time Start time (Unix timestamp)
     * @param end_time End time (Unix timestamp)
     * @param use_serialized Use serialized data columns
     * @return Raw QueryDataResponse
     */
    QueryDataResponse getRawQueryResponse(
        const std::vector<std::string>& pv_names,
        uint64_t start_time,
        uint64_t end_time,
        bool use_serialized = true) const;
};

/**
 * @brief Utility class for time operations
 */
class TimeUtils {
public:
    /**
     * @brief Get current Unix timestamp
     * @return Current Unix timestamp
     */
    static uint64_t getCurrentUnixTime();

    /**
     * @brief Convert Unix timestamp to system_clock time_point
     * @param unix_time Unix timestamp
     * @return system_clock time_point
     */
    static std::chrono::system_clock::time_point fromUnixTime(uint64_t unix_time);

    /**
     * @brief Convert system_clock time_point to Unix timestamp
     * @param tp system_clock time_point
     * @return Unix timestamp
     */
    static uint64_t toUnixTime(const std::chrono::system_clock::time_point& tp);

    /**
     * @brief Parse time string to Unix timestamp
     * @param time_str Time string (e.g., "2025-07-11 10:30:00" or Unix timestamp)
     * @return Unix timestamp
     */
    static uint64_t parseTimeString(const std::string& time_str);

    /**
     * @brief Get Unix timestamp for relative time (e.g., "1h ago")
     * @param relative_str Relative time string (e.g., "1h", "30m", "2d")
     * @return Unix timestamp
     */
    static uint64_t getRelativeTime(const std::string& relative_str);

    /**
     * @brief Format Unix timestamp as readable string
     * @param unix_time Unix timestamp
     * @return Formatted time string
     */
    static std::string formatUnixTime(uint64_t unix_time);
};

#endif // DATA_DECODER_HPP
