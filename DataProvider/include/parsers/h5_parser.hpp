#ifndef H5_PARSER_HPP
#define H5_PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <memory>
#include <H5Cpp.h>

// File metadata extracted from naming convention
struct H5FileMetadata {
    std::string origin;               // "CU"
    std::string pathway;              // "HXR"
    std::string date;                 // "20250623"
    std::string time;                 // "195500"
    std::string project;              // "CoAD"
    std::string full_path;
    uint64_t file_timestamp_seconds = 0;
    bool valid_timestamp = false;
};

// Signal name parsing result
struct SignalInfo {
    std::string device;               // "KLYS", "BPMS"
    std::string device_area;          // "LI23", "DMPH"
    std::string device_location;      // "31", "502"
    std::string device_attribute;     // "AMPL", "TMITBR", "PHAS_FASTBR"
    std::string full_name;            // "KLYS_LI23_31_AMPL"
    std::string label;                // From H5 attribute: "KLYS:LI23:31:AMPL"
    std::string matlab_class;         // From H5 attribute: "double"
    std::string units = "unknown";    // Inferred: "MV/m"
    std::string signal_type = "measurement"; // Inferred: "amplitude"
};

// Timestamp data structure (shared across signals from same file)
struct TimestampData {
    std::vector<uint64_t> seconds;    // secondsPastEpoch array
    std::vector<uint64_t> nanoseconds; // nanoseconds array
    uint64_t period_nanos = 1000000000; // Calculated sampling period
    bool is_regular_sampling = false; // True if regular sampling detected
    size_t count = 0;                 // Number of timestamps
    uint64_t start_time_sec = 0;      // First timestamp seconds
    uint64_t start_time_nano = 0;     // First timestamp nanoseconds
    uint64_t end_time_sec = 0;        // Last timestamp seconds
    uint64_t end_time_nano = 0;       // Last timestamp nanoseconds
};

// Complete signal data
struct SignalData {
    SignalInfo info;                  // Parsed signal metadata
    std::vector<double> values;       // Signal values
    std::shared_ptr<TimestampData> timestamps; // Shared timestamp reference
    H5FileMetadata file_metadata;     // Source file metadata
    bool spatial_enrichment_ready = false; // Flag for spatial enrichment
};

// Main parser class
class H5Parser {
public:
    explicit H5Parser(const std::string& h5_directory_path);
    virtual ~H5Parser();

    // Spatial enrichment control
    void enableSpatialEnrichment(bool enable = true);
    bool isSpatialEnrichmentEnabled() const { return spatial_enrichment_enabled_; }

    // Main parsing functions
    bool parseDirectory();
    virtual bool parseFile(const std::string& filepath);

    // Data access
    std::vector<SignalData> getAllSignals() const;
    std::vector<SignalData> getSignalsByDevice(const std::string& device) const;
    std::vector<SignalData> getSignalsByDeviceArea(const std::string& device_area) const;
    std::vector<SignalData> getSignalsByDeviceAttribute(const std::string& device_attribute) const;
    std::vector<SignalData> getSignalsByProject(const std::string& project) const;

    // Individual signal lookup
    SignalData* findSignal(const std::string& signal_name);
    const SignalData* findSignal(const std::string& signal_name) const;

    // Statistics
    size_t getTotalSignals() const { return parsed_signals_.size(); }
    size_t getTotalFiles() const { return file_timestamps_.size(); }

    // File metadata access
    std::vector<H5FileMetadata> getFileMetadata() const;

protected:
    std::string h5_directory_;
    std::vector<SignalData> parsed_signals_;
    std::map<std::string, std::shared_ptr<TimestampData>> file_timestamps_;
    bool spatial_enrichment_enabled_;

    // File discovery and validation
    std::vector<std::string> discoverH5Files() const;
    bool matchesNamingConvention(const std::string& filepath) const;

    // Filename parsing
    H5FileMetadata parseFilename(const std::string& filepath) const;
    uint64_t parseTimestamp(const std::string& date, const std::string& time) const;

    // H5 file processing
    std::shared_ptr<TimestampData> extractTimestamps(H5::H5File& file) const;
    std::vector<std::string> getSignalDatasets(H5::H5File& file) const;

    // Signal processing
    SignalInfo parseSignalName(const std::string& signal_name) const;
    SignalData processSignal(H5::H5File& file,
                           const std::string& signal_name,
                           std::shared_ptr<TimestampData> timestamps,
                           const H5FileMetadata& file_metadata) const;

    // H5 attribute reading
    std::string readStringAttribute(H5::DataSet& dataset, const std::string& attr_name) const;

    // Timestamp analysis
    uint64_t calculatePeriodNanos(const std::vector<uint64_t>& seconds,
                                 const std::vector<uint64_t>& nanoseconds) const;
    bool checkRegularSampling(const std::vector<uint64_t>& seconds,
                             const std::vector<uint64_t>& nanoseconds,
                             uint64_t expected_period) const;

    // Unit and type inference
    std::string inferUnits(const std::string& device_attribute) const;
    std::string inferSignalType(const std::string& device_attribute) const;

    // Validation
    bool validateDataConsistency(const std::vector<double>& values,
                               const TimestampData& timestamps,
                               const std::string& signal_name) const;
};

#endif // H5_PARSER_HPP