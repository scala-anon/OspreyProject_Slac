#ifndef MONGO_TO_NPY_PARSER_HPP
#define MONGO_TO_NPY_PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cstdint>
#include "query_client.hpp"

// Forward declarations
using DataColumn = ::DataColumn;
using SerializedDataColumn = ::SerializedDataColumn;
using DataValue = ::DataValue;

// Data structures for organizing MongoDB query results
struct PVDataSeries {
    std::string pv_name;
    std::vector<double> values;
    std::vector<uint64_t> timestamps_seconds;
    std::vector<uint64_t> timestamps_nanoseconds;
    std::vector<std::string> status_values;
    
    // Metadata
    size_t sample_count;
    uint64_t start_time;
    uint64_t end_time;
    double sample_rate_hz;
    std::string data_type;
};

struct CorrelatedDataMatrix {
    std::vector<std::string> pv_names;           // Column names
    std::vector<uint64_t> timestamps;            // Row timestamps
    std::vector<std::vector<double>> data_matrix; // [time_point][pv_index]
    
    size_t num_time_points;
    size_t num_pvs;
    uint64_t start_time;
    uint64_t end_time;
};

// Export configuration for different ML use cases
struct NPYExportConfig {
    std::string output_directory = "./npy_exports";
    std::string file_prefix = "mldp_data";
    
    // Data format options
    bool export_as_float32 = true;          // vs float64 (PyTorch prefers float32)
    bool normalize_timestamps = true;       // Convert to relative time from start
    bool export_separate_timestamps = true; // Separate timestamp file
    bool export_pv_metadata = true;         // PV names, sample rates, etc.
    
    // ML-specific options
    bool create_sequences = false;          // For LSTM/RNN training
    size_t sequence_length = 50;           // Length of sequences
    size_t sequence_overlap = 10;          // Overlap between sequences
    
    bool create_sliding_windows = false;   // For anomaly detection
    size_t window_size = 100;             // Window size for features
    
    bool normalize_values = false;         // Z-score normalization
    bool fill_missing_values = true;      // Interpolate missing data
    double fill_value = 0.0;              // Default fill value
};

class MongoToNPYParser {
private:
    QueryClient& query_client_;
    NPYExportConfig config_;
    
    // NPY writing helpers
    void writeNPYHeader(std::ofstream& file, const std::string& dtype, const std::vector<size_t>& shape);
    bool writeFloat32NPY(const std::string& filename, const std::vector<float>& data, const std::vector<size_t>& shape);
    bool writeFloat64NPY(const std::string& filename, const std::vector<double>& data, const std::vector<size_t>& shape);
    bool writeUInt64NPY(const std::string& filename, const std::vector<uint64_t>& data, const std::vector<size_t>& shape);
    
    // Data processing helpers
    std::vector<PVDataSeries> extractPVSeriesFromQueryResponse(const std::vector<QueryDataResponse>& responses);
    CorrelatedDataMatrix createCorrelatedMatrix(const std::vector<PVDataSeries>& pv_series);
    std::vector<float> normalizeValues(const std::vector<double>& values);
    std::vector<float> createSequences(const CorrelatedDataMatrix& matrix);
    std::vector<float> createSlidingWindows(const CorrelatedDataMatrix& matrix);
    
    // NEW: Debug and extraction helpers
    void debugQueryResponse(const std::vector<QueryDataResponse>& responses);
    void extractTimestampsFromBucket(const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket,
                                     std::vector<uint64_t>& timestamps_sec,
                                     std::vector<uint64_t>& timestamps_nano);
    double extractNumericValue(const DataValue& value);
    void calculateSeriesMetadata(PVDataSeries& series);
    bool extractFromRegularDataColumn(const DataColumn& column, 
                                      const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket, 
                                      PVDataSeries& series);
    bool extractFromSerializedDataColumn(const SerializedDataColumn& serialized,
                                         const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket,
                                         PVDataSeries& series);
    
    // Metadata helpers
    bool writeMetadataJSON(const std::vector<PVDataSeries>& pv_series, const std::string& filename);
    bool writePVNamesFile(const std::vector<std::string>& pv_names, const std::string& filename);
    
public:
    MongoToNPYParser(QueryClient& client, const NPYExportConfig& config = NPYExportConfig{});
    
    // === MAIN CONVERSION METHODS ===
    
    // Convert query results to NPY format
    bool convertQueryResultsToNPY(const std::vector<QueryDataResponse>& query_responses, 
                                   const std::string& output_name);
    
    // Query specific PVs and convert to NPY
    bool queryAndExportPVs(const std::vector<std::string>& pv_names,
                          uint64_t start_time, uint64_t end_time,
                          const std::string& output_name);
    
    // Query by pattern and convert to NPY  
    bool queryPatternAndExportToNPY(const std::string& pv_pattern,
                                    uint64_t start_time, uint64_t end_time,
                                    const std::string& output_name);
    
    // === ML-SPECIFIC EXPORT METHODS ===
    
    // Export for time series prediction (LSTM/RNN)
    bool exportForTimeSeriesML(const std::vector<std::string>& pv_names,
                               uint64_t start_time, uint64_t end_time,
                               const std::string& model_name = "timeseries");
    
    // Export for anomaly detection (sliding windows)
    bool exportForAnomalyDetection(const std::vector<std::string>& pv_names,
                                   uint64_t start_time, uint64_t end_time,
                                   const std::string& model_name = "anomaly");
    
    // Export correlation matrix for analysis
    bool exportCorrelationMatrix(const std::vector<std::string>& pv_names,
                                 uint64_t start_time, uint64_t end_time,
                                 const std::string& analysis_name = "correlation");
    
    // Export raw data matrix (most common)
    bool exportRawDataMatrix(const std::vector<std::string>& pv_names,
                            uint64_t start_time, uint64_t end_time,
                            const std::string& dataset_name = "dataset");
    
    // === UTILITY METHODS ===
    
    // Find available PVs from MongoDB
    std::vector<std::string> findAvailablePVs(const std::string& pattern = ".*");
    
    // Get time range for available data
    std::pair<uint64_t, uint64_t> getDataTimeRange(const std::vector<std::string>& pv_names);
    
    // Validate PVs exist in MongoDB
    bool validatePVsExist(const std::vector<std::string>& pv_names);
    
    // Get sample statistics
    void printDataStatistics(const std::vector<PVDataSeries>& pv_series);
    
    // Configuration
    void setConfig(const NPYExportConfig& config) { config_ = config; }
    NPYExportConfig getConfig() const { return config_; }
    
    // === BATCH PROCESSING ===
    
    // Process multiple time windows
    bool exportTimeWindows(const std::vector<std::string>& pv_names,
                          uint64_t start_time, uint64_t end_time,
                          uint64_t window_duration, uint64_t window_step,
                          const std::string& batch_name = "batch");
    
    // Process multiple PV groups
    bool exportPVGroups(const std::map<std::string, std::vector<std::string>>& pv_groups,
                       uint64_t start_time, uint64_t end_time);
};

// === PYTORCH INTEGRATION HELPERS ===
namespace pytorch_integration {
    
    // Generate PyTorch DataLoader script
    void generatePyTorchLoader(const std::string& npy_directory,
                              const NPYExportConfig& config,
                              const std::string& script_name = "pytorch_loader.py");
    
    // Generate training script template
    void generateMLTrainingTemplate(const std::string& npy_directory,
                                   const std::string& model_type,  // "lstm", "cnn", "transformer"
                                   const std::string& script_name);
    
    // Create dataset info file
    void createDatasetInfo(const std::string& npy_directory,
                          const std::vector<std::string>& pv_names,
                          const NPYExportConfig& config);
}

#endif // MONGO_TO_NPY_PARSER_HPP
