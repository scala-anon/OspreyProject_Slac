#ifndef H5_PARSER_HPP
#define H5_PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include "H5Cpp.h"

// Single PV measurement at a specific time
struct PVMeasurement {
    uint64_t timestamp_seconds;      // Unix epoch seconds
    uint64_t timestamp_nanoseconds;  // Nanosecond precision
    double value;                    // PV value (could be int, double, etc.)
    std::string status;              // Optional: "GOOD", "BAD", "DISCONNECTED"
    double severity;                 // Optional: alarm severity
};

// Complete time-series for one PV
struct PVTimeSeries {
    std::string pv_name;             // e.g., "S01-GCC-01", "S01A-BPM-X"
    std::string description;         // Human readable description
    std::string units;               // "mbar", "mm", "volts", etc.
    std::string data_type;           // "DOUBLE", "LONG", "STRING"
    
    // Time-series data
    std::vector<PVMeasurement> measurements;
    
    // Metadata
    uint64_t start_time_seconds;     // First measurement time
    uint64_t end_time_seconds;       // Last measurement time
    double sample_rate_hz;           // Sampling frequency
    size_t total_samples;            // Number of measurements
    
    // File info
    std::string source_file;         // Which H5 file this came from
    std::string dataset_path;        // Path within H5 file
};

// Collection of all PV data from multiple H5 files
struct PVDataCollection {
    std::vector<PVTimeSeries> pv_series;        // All PV time-series
    std::map<std::string, size_t> pv_index;     // Quick lookup: PV name -> index
    
    // Global metadata
    uint64_t earliest_time;          // Earliest timestamp across all PVs
    uint64_t latest_time;            // Latest timestamp across all PVs
    std::vector<std::string> source_files;  // List of H5 files processed
    
    // Helper functions
    void addPVSeries(const PVTimeSeries& series);
    PVTimeSeries* findPV(const std::string& pv_name);
    std::vector<std::string> getAllPVNames() const;
    size_t getTotalMeasurements() const;
};

// For MLDP ingestion - correlated data at same timestamps
struct CorrelatedPVData {
    uint64_t timestamp_seconds;
    uint64_t timestamp_nanoseconds;
    std::map<std::string, double> pv_values;    // PV_name -> value
    std::map<std::string, std::string> pv_status; // PV_name -> status
};

class H5Parser {
public:
    explicit H5Parser(const std::string& h5_path);
    ~H5Parser();
    
    // Main parsing functions
    bool parseDirectory();
    bool parseFile(const std::string& filename);
    
    // Data access
    const PVDataCollection& getPVData() const { return pv_data_; }
    std::vector<CorrelatedPVData> getCorrelatedData(uint64_t start_time, uint64_t end_time) const;
    
    // MLDP integration helpers
    std::vector<std::string> getAllPVNames() const;
    PVTimeSeries* getPVSeries(const std::string& pv_name);
    
    // Statistics
    void printSummary() const;
    
private:
    std::string h5_path_;
    PVDataCollection pv_data_;
    
    // H5 parsing helpers
    bool parsePVDataset(H5::H5File& file, const std::string& dataset_name);
    bool parseTimeseriesGroup(H5::Group& group, const std::string& group_name);
    uint64_t convertToUnixTime(const std::string& timestamp_str);
    
    // Data organization
    void correlatePVData();
    void validateTimestamps();
};

#endif // H5_PARSER_HPP
