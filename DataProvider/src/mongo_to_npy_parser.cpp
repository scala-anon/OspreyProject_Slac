#include "mongo_to_npy_parser.hpp"
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <fstream>
#include <set>

MongoToNPYParser::MongoToNPYParser(QueryClient& client, const NPYExportConfig& config)
    : query_client_(client), config_(config) {
    
    // Create output directory
    std::filesystem::create_directories(config_.output_directory);
    std::cout << "NPY export directory: " << config_.output_directory << std::endl;
}

// === NPY FORMAT WRITING ===
void MongoToNPYParser::writeNPYHeader(std::ofstream& file, const std::string& dtype, const std::vector<size_t>& shape) {
    // NPY format: magic + version + header_len + header + data
    file.write("\x93NUMPY", 6);  // Magic string
    file.write("\x01\x00", 2);   // Version 1.0
    
    // Build header dictionary
    std::ostringstream header_stream;
    header_stream << "{'descr': '" << dtype << "', 'fortran_order': False, 'shape': (";
    
    for (size_t i = 0; i < shape.size(); i++) {
        header_stream << shape[i];
        if (i < shape.size() - 1) header_stream << ", ";
    }
    if (shape.size() == 1) header_stream << ",";  // Python tuple syntax
    header_stream << "), }";
    
    std::string header = header_stream.str();
    
    // Pad to 64-byte boundary
    size_t total_header_size = header.length() + 10;  // +10 for magic, version, len
    size_t padding = (64 - (total_header_size % 64)) % 64;
    header += std::string(padding, ' ') + '\n';
    
    // Write header length (little endian)
    uint16_t header_len = static_cast<uint16_t>(header.length());
    file.write(reinterpret_cast<const char*>(&header_len), sizeof(header_len));
    
    // Write header
    file.write(header.c_str(), header.length());
}

bool MongoToNPYParser::writeFloat32NPY(const std::string& filename, const std::vector<float>& data, const std::vector<size_t>& shape) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot create file: " << filename << std::endl;
        return false;
    }
    
    writeNPYHeader(file, "<f4", shape);  // Little endian float32
    file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(float));
    file.close();
    
    std::cout << "Exported " << data.size() << " float32 values to " << filename << std::endl;
    return true;
}

bool MongoToNPYParser::writeFloat64NPY(const std::string& filename, const std::vector<double>& data, const std::vector<size_t>& shape) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot create file: " << filename << std::endl;
        return false;
    }
    
    writeNPYHeader(file, "<f8", shape);  // Little endian float64
    file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(double));
    file.close();
    
    std::cout << "Exported " << data.size() << " float64 values to " << filename << std::endl;
    return true;
}

bool MongoToNPYParser::writeUInt64NPY(const std::string& filename, const std::vector<uint64_t>& data, const std::vector<size_t>& shape) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot create file: " << filename << std::endl;
        return false;
    }
    
    writeNPYHeader(file, "<u8", shape);  // Little endian uint64
    file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint64_t));
    file.close();
    
    std::cout << "Exported " << data.size() << " uint64 timestamps to " << filename << std::endl;
    return true;
}

// === DEBUG METHOD ===
void MongoToNPYParser::debugQueryResponse(const std::vector<QueryDataResponse>& responses) {
    std::cout << "\n=== DEBUG: Query Response Analysis ===" << std::endl;
    std::cout << "Number of responses: " << responses.size() << std::endl;
    
    for (size_t i = 0; i < responses.size(); i++) {
        const auto& response = responses[i];
        std::cout << "\nResponse " << i << ":" << std::endl;
        
        if (response.has_querydata()) {
            const auto& querydata = response.querydata();
            std::cout << "  Has querydata: YES" << std::endl;
            std::cout << "  Number of data buckets: " << querydata.databuckets_size() << std::endl;
            
            for (int j = 0; j < querydata.databuckets_size(); j++) {
                const auto& bucket = querydata.databuckets(j);
                std::cout << "    Bucket " << j << ":" << std::endl;
                
                if (bucket.has_datacolumn()) {
                    const auto& column = bucket.datacolumn();
                    std::cout << "      Has datacolumn: YES" << std::endl;
                    std::cout << "      Column name: " << column.name() << std::endl;
                    std::cout << "      Number of data values: " << column.datavalues_size() << std::endl;
                } else if (bucket.has_serializeddatacolumn()) {
                    const auto& serialized = bucket.serializeddatacolumn();
                    std::cout << "      Has SERIALIZED datacolumn: YES" << std::endl;
                    std::cout << "      Column name: " << serialized.columnname() << std::endl;
                    std::cout << "      Serialized data size: " << serialized.serializeddata().size() << " bytes" << std::endl;
                } else {
                    std::cout << "      Has datacolumn: NO" << std::endl;
                    std::cout << "      Has serializeddatacolumn: NO" << std::endl;
                }
                
                if (bucket.has_datatimestamps()) {
                    std::cout << "      Has timestamps: YES" << std::endl;
                    if (bucket.datatimestamps().has_samplingclock()) {
                        const auto& clock = bucket.datatimestamps().samplingclock();
                        std::cout << "        Sampling clock - count: " << clock.count() << std::endl;
                    }
                } else {
                    std::cout << "      Has timestamps: NO" << std::endl;
                }
            }
        } else if (response.has_exceptionalresult()) {
            std::cout << "  Has exceptionalresult: " << response.exceptionalresult().message() << std::endl;
        } else {
            std::cout << "  Has querydata: NO" << std::endl;
            std::cout << "  Has exceptionalresult: NO" << std::endl;
        }
    }
    std::cout << "=== END DEBUG ===" << std::endl;
}

// === HELPER METHODS ===
void MongoToNPYParser::extractTimestampsFromBucket(const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket,
                                                   std::vector<uint64_t>& timestamps_sec,
                                                   std::vector<uint64_t>& timestamps_nano) {
    if (bucket.has_datatimestamps()) {
        if (bucket.datatimestamps().has_timestamplist()) {
            // Explicit timestamp list
            for (const auto& ts : bucket.datatimestamps().timestamplist().timestamps()) {
                timestamps_sec.push_back(ts.epochseconds());
                timestamps_nano.push_back(ts.nanoseconds());
            }
        } else if (bucket.datatimestamps().has_samplingclock()) {
            // Generate from sampling clock
            const auto& clock = bucket.datatimestamps().samplingclock();
            uint64_t start_sec = clock.starttime().epochseconds();
            uint64_t start_nano = clock.starttime().nanoseconds();
            uint64_t period_nano = clock.periodnanos();
            
            for (uint32_t i = 0; i < clock.count(); ++i) {
                uint64_t total_nano = start_nano + i * period_nano;
                timestamps_sec.push_back(start_sec + total_nano / 1000000000);
                timestamps_nano.push_back(total_nano % 1000000000);
            }
        }
    }
}

double MongoToNPYParser::extractNumericValue(const DataValue& value) {
    switch (value.value_case()) {
        case DataValue::kDoubleValue:
            return value.doublevalue();
        case DataValue::kLongValue:
            return static_cast<double>(value.longvalue());
        case DataValue::kIntValue:
            return static_cast<double>(value.intvalue());
        case DataValue::kFloatValue:
            return static_cast<double>(value.floatvalue());
        case DataValue::kUlongValue:
            return static_cast<double>(value.ulongvalue());
        case DataValue::kUintValue:
            return static_cast<double>(value.uintvalue());
        default:
            return 0.0;
    }
}

void MongoToNPYParser::calculateSeriesMetadata(PVDataSeries& series) {
    series.sample_count = series.values.size();
    if (series.sample_count > 0) {
        series.start_time = series.timestamps_seconds.front();
        series.end_time = series.timestamps_seconds.back();
        
        if (series.sample_count > 1) {
            double duration = static_cast<double>(series.end_time - series.start_time);
            series.sample_rate_hz = (series.sample_count - 1) / duration;
        } else {
            series.sample_rate_hz = 0.0;
        }
    }
}

bool MongoToNPYParser::extractFromRegularDataColumn(const DataColumn& column, 
                                                    const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket, 
                                                    PVDataSeries& series) {
    series.data_type = "DOUBLE";
    
    // Extract timestamps from bucket
    std::vector<uint64_t> timestamps_sec;
    std::vector<uint64_t> timestamps_nano;
    extractTimestampsFromBucket(bucket, timestamps_sec, timestamps_nano);
    
    // Extract values
    for (int i = 0; i < column.datavalues_size(); ++i) {
        const auto& value = column.datavalues(i);
        
        // Extract numeric value
        double numeric_value = extractNumericValue(value);
        series.values.push_back(numeric_value);
        
        // Timestamps
        if (i < timestamps_sec.size()) {
            series.timestamps_seconds.push_back(timestamps_sec[i]);
            series.timestamps_nanoseconds.push_back(i < timestamps_nano.size() ? timestamps_nano[i] : 0);
        }
        
        // Status
        std::string status = "GOOD";
        if (value.has_valuestatus()) {
            status = value.valuestatus().message();
        }
        series.status_values.push_back(status);
    }
    
    calculateSeriesMetadata(series);
    return !series.values.empty();
}

bool MongoToNPYParser::extractFromSerializedDataColumn(const SerializedDataColumn& serialized,
                                                       const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket,
                                                       PVDataSeries& series) {
    try {
        // Deserialize the data column from bytes
        DataColumn column;
        if (!column.ParseFromString(serialized.serializeddata())) {
            std::cerr << "Failed to deserialize data column for " << series.pv_name << std::endl;
            return false;
        }
        
        std::cout << "    Successfully deserialized " << column.datavalues_size() << " data values" << std::endl;
        
        // Now process as regular data column
        return extractFromRegularDataColumn(column, bucket, series);
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing serialized data column: " << e.what() << std::endl;
        return false;
    }
}

// === DATA EXTRACTION FROM MONGODB QUERY RESULTS ===
std::vector<PVDataSeries> MongoToNPYParser::extractPVSeriesFromQueryResponse(const std::vector<QueryDataResponse>& responses) {
    std::vector<PVDataSeries> pv_series;
    
    std::cout << "Extracting PV data from " << responses.size() << " MongoDB query responses..." << std::endl;
    
    for (const auto& response : responses) {
        if (!response.has_querydata()) {
            std::cout << "Response has no querydata" << std::endl;
            continue;
        }
        
        const auto& querydata = response.querydata();
        std::cout << "Processing response with " << querydata.databuckets_size() << " data buckets" << std::endl;
        
        for (const auto& bucket : querydata.databuckets()) {
            PVDataSeries series;
            
            // Handle both regular and serialized data columns
            bool has_data = false;
            
            if (bucket.has_datacolumn()) {
                std::cout << "  Processing regular data column..." << std::endl;
                const auto& column = bucket.datacolumn();
                series.pv_name = column.name();
                has_data = extractFromRegularDataColumn(column, bucket, series);
                
            } else if (bucket.has_serializeddatacolumn()) {
                std::cout << "  Processing SERIALIZED data column..." << std::endl;
                const auto& serialized = bucket.serializeddatacolumn();
                series.pv_name = serialized.columnname();
                has_data = extractFromSerializedDataColumn(serialized, bucket, series);
                
            } else {
                std::cout << "  Bucket has no data column (regular or serialized)" << std::endl;
                continue;
            }
            
            if (has_data && !series.values.empty()) {
                std::cout << "    Extracted " << series.values.size() << " values for " << series.pv_name << std::endl;
                pv_series.push_back(series);
            } else {
                std::cout << "    No data extracted for " << series.pv_name << std::endl;
            }
        }
    }
    
    std::cout << "✅ Extracted " << pv_series.size() << " PV time series from MongoDB" << std::endl;
    return pv_series;
}

CorrelatedDataMatrix MongoToNPYParser::createCorrelatedMatrix(const std::vector<PVDataSeries>& pv_series) {
    CorrelatedDataMatrix matrix;
    
    if (pv_series.empty()) return matrix;
    
    std::cout << "Creating correlated data matrix..." << std::endl;
    
    // Collect all unique timestamps
    std::set<uint64_t> unique_timestamps;
    for (const auto& series : pv_series) {
        for (uint64_t ts : series.timestamps_seconds) {
            unique_timestamps.insert(ts);
        }
    }
    
    matrix.timestamps.assign(unique_timestamps.begin(), unique_timestamps.end());
    matrix.num_time_points = matrix.timestamps.size();
    matrix.num_pvs = pv_series.size();
    
    // Get PV names
    for (const auto& series : pv_series) {
        matrix.pv_names.push_back(series.pv_name);
    }
    
    // Initialize data matrix
    matrix.data_matrix.resize(matrix.num_time_points);
    for (auto& row : matrix.data_matrix) {
        row.resize(matrix.num_pvs, config_.fill_value);  // Fill with default value
    }
    
    // Fill matrix with values
    for (size_t pv_idx = 0; pv_idx < pv_series.size(); ++pv_idx) {
        const auto& series = pv_series[pv_idx];
        
        for (size_t val_idx = 0; val_idx < series.values.size(); ++val_idx) {
            uint64_t timestamp = series.timestamps_seconds[val_idx];
            
            // Find timestamp index in matrix
            auto it = std::lower_bound(matrix.timestamps.begin(), matrix.timestamps.end(), timestamp);
            if (it != matrix.timestamps.end() && *it == timestamp) {
                size_t time_idx = std::distance(matrix.timestamps.begin(), it);
                matrix.data_matrix[time_idx][pv_idx] = series.values[val_idx];
            }
        }
    }
    
    // Set time range
    if (!matrix.timestamps.empty()) {
        matrix.start_time = matrix.timestamps.front();
        matrix.end_time = matrix.timestamps.back();
    }
    
    std::cout << "Created " << matrix.num_time_points << "×" << matrix.num_pvs << " correlated data matrix" << std::endl;
    return matrix;
}

// === MAIN EXPORT METHODS ===
bool MongoToNPYParser::queryAndExportPVs(const std::vector<std::string>& pv_names,
                                         uint64_t start_time, uint64_t end_time,
                                         const std::string& output_name) {
    try {
        std::cout << "Querying " << pv_names.size() << " PVs from MongoDB..." << std::endl;
        std::cout << "Time range: " << start_time << " to " << end_time << std::endl;
        
        // Query data from MongoDB via MLDP
        Timestamp begin_ts = makeTimestamp(start_time, 0);
        Timestamp end_ts = makeTimestamp(end_time, 0);
        
        // Try both serialized and non-serialized data requests
        std::vector<QueryDataResponse> responses;
        
        // First, try with serialized data columns (likely what your MLDP returns)
        std::cout << "Trying query with serialized data columns..." << std::endl;
        auto request_serialized = makeQueryDataRequest(pv_names, begin_ts, end_ts, true);  // true = serialized
        
        try {
            responses = query_client_.queryDataStream(request_serialized);
            std::cout << "Serialized query successful, got " << responses.size() << " responses" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Serialized query failed: " << e.what() << std::endl;
            responses.clear();
        }
        
        // If serialized failed, try regular data columns
        if (responses.empty()) {
            std::cout << "Trying query with regular data columns..." << std::endl;
            auto request_regular = makeQueryDataRequest(pv_names, begin_ts, end_ts, false);  // false = regular
            
            try {
                responses = query_client_.queryDataStream(request_regular);
                std::cout << "Regular query successful, got " << responses.size() << " responses" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Regular query also failed: " << e.what() << std::endl;
                return false;
            }
        }
        
        if (responses.empty()) {
            std::cerr << "No data returned from MongoDB query" << std::endl;
            return false;
        }
        
        // Debug the response structure
        debugQueryResponse(responses);
        
        return convertQueryResultsToNPY(responses, output_name);
        
    } catch (const std::exception& e) {
        std::cerr << "Query and export failed: " << e.what() << std::endl;
        return false;
    }
}

bool MongoToNPYParser::convertQueryResultsToNPY(const std::vector<QueryDataResponse>& query_responses,
                                                const std::string& output_name) {
    try {
        // Extract PV series from MongoDB query results
        auto pv_series = extractPVSeriesFromQueryResponse(query_responses);
        if (pv_series.empty()) {
            std::cerr << "No PV data extracted from query responses" << std::endl;
            return false;
        }
        
        // Print statistics
        printDataStatistics(pv_series);
        
        // Create correlated data matrix
        auto matrix = createCorrelatedMatrix(pv_series);
        if (matrix.data_matrix.empty()) {
            std::cerr << "Failed to create data matrix" << std::endl;
            return false;
        }
        
        // Convert to flat vector for NPY export
        std::vector<float> flat_data;
        flat_data.reserve(matrix.num_time_points * matrix.num_pvs);
        
        for (const auto& row : matrix.data_matrix) {
            for (double val : row) {
                flat_data.push_back(config_.export_as_float32 ? static_cast<float>(val) : val);
            }
        }
        
        // Export data matrix
        std::string data_file = config_.output_directory + "/" + output_name + "_data.npy";
        bool success = false;
        
        if (config_.export_as_float32) {
            success = writeFloat32NPY(data_file, flat_data, {matrix.num_time_points, matrix.num_pvs});
        } else {
            std::vector<double> double_data(flat_data.begin(), flat_data.end());
            success = writeFloat64NPY(data_file, double_data, {matrix.num_time_points, matrix.num_pvs});
        }
        
        if (!success) return false;
        
        // Export timestamps if requested
        if (config_.export_separate_timestamps) {
            std::vector<uint64_t> timestamps = matrix.timestamps;
            if (config_.normalize_timestamps && !timestamps.empty()) {
                uint64_t start_time = timestamps[0];
                for (auto& ts : timestamps) {
                    ts -= start_time;
                }
            }
            
            std::string timestamp_file = config_.output_directory + "/" + output_name + "_timestamps.npy";
            writeUInt64NPY(timestamp_file, timestamps, {timestamps.size()});
        }
        
        // Export PV names and metadata
        if (config_.export_pv_metadata) {
            writePVNamesFile(matrix.pv_names, config_.output_directory + "/" + output_name + "_pv_names.txt");
            writeMetadataJSON(pv_series, config_.output_directory + "/" + output_name + "_metadata.json");
        }
        
        // Generate PyTorch loader
        pytorch_integration::generatePyTorchLoader(config_.output_directory, config_, output_name + "_loader.py");
        
        std::cout << "MongoDB to NPY conversion complete!" << std::endl;
        std::cout << "   Data shape: [" << matrix.num_time_points << ", " << matrix.num_pvs << "]" << std::endl;
        std::cout << "   Files in: " << config_.output_directory << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Conversion failed: " << e.what() << std::endl;
        return false;
    }
}

// === ML-SPECIFIC EXPORTS ===
bool MongoToNPYParser::exportForTimeSeriesML(const std::vector<std::string>& pv_names,
                                             uint64_t start_time, uint64_t end_time,
                                             const std::string& model_name) {
    std::cout << "Exporting for Time Series ML (LSTM/RNN training)..." << std::endl;
    
    // Temporarily enable sequence creation
    NPYExportConfig original_config = config_;
    config_.create_sequences = true;
    
    bool success = queryAndExportPVs(pv_names, start_time, end_time, model_name + "_timeseries");
    
    // Restore original config
    config_ = original_config;
    
    if (success) {
        // Generate ML training template
        pytorch_integration::generateMLTrainingTemplate(config_.output_directory, "lstm", model_name + "_train.py");
    }
    
    return success;
}

bool MongoToNPYParser::exportRawDataMatrix(const std::vector<std::string>& pv_names,
                                           uint64_t start_time, uint64_t end_time,
                                           const std::string& dataset_name) {
    std::cout << "Exporting raw data matrix for general ML..." << std::endl;
    return queryAndExportPVs(pv_names, start_time, end_time, dataset_name + "_raw");
}

// === UTILITY METHODS ===
std::vector<std::string> MongoToNPYParser::findAvailablePVs(const std::string& pattern) {
    std::vector<std::string> pv_names;
    
    try {
        // Try metadata query first (ideal approach)
        auto metadataRequest = makeQueryPvMetadataRequestWithPattern(pattern);
        auto metadataResponse = query_client_.queryPvMetadata(metadataRequest);
        
        if (metadataResponse.has_metadataresult()) {
            auto pvInfos = metadataResponse.metadataresult().pvinfos();
            std::cout << "Found " << pvInfos.size() << " PVs using metadata query" << std::endl;
            
            for (const auto& pvInfo : pvInfos) {
                pv_names.push_back(pvInfo.pvname());
            }
            
            return pv_names;
        }
    } catch (const std::exception& e) {
        std::cout << "Metadata query not available, using data query fallback..." << std::endl;
    }
    
    // Fallback: Test known PVs with actual data time range
    std::vector<std::string> test_pv_names = {
        "ACCL_IN20_300_L0A_ACUHBR",
        "ACCL_IN20_300_L0A_PCUHBR", 
        "BEND_DMPH_395_I0_BACTCUHBR",
        "BLD_SYS0_500_ANG_XCUHBR",
        "BLD_SYS0_500_POS_XCUHBR",
        "BPMS_BSYH_445_XCUHBR",
        "BPMS_BSYH_445_YCUHBR",
        "BPMS_CLTH_140_XCUHBR",
        "BLEN_LI21_265_AIMAXCUHBR",
        "XPP_SB2_IPM_01_SUMCUHBR"  // Add one from your latest data
    };
    
    std::cout << "Testing " << test_pv_names.size() << " known PV names with data queries..." << std::endl;
    
    // ✅ FIXED: Use your actual data time range from MongoDB
    uint64_t test_start_time = 1750690485;  // Your actual earliest data
    uint64_t test_end_time = 1750706894;    // Your actual latest data
    
    std::cout << "Using time range: " << test_start_time << " to " << test_end_time 
              << " (June 23, 2025 14:54-19:28 UTC)" << std::endl;
    
    for (const auto& test_pv : test_pv_names) {
        try {
            Timestamp begin_ts = makeTimestamp(test_start_time, 0);
            Timestamp end_ts = makeTimestamp(test_end_time, 0);
            auto request = makeQueryDataRequest({test_pv}, begin_ts, end_ts, false);
            
            auto response = query_client_.queryData(request);
            
            if (response.has_querydata() && response.querydata().databuckets_size() > 0) {
                pv_names.push_back(test_pv);
                std::cout << "  ✓ Found data for: " << test_pv << std::endl;
            } else {
                std::cout << "  - No data for: " << test_pv << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "  ✗ Query failed for: " << test_pv << " (" << e.what() << ")" << std::endl;
        }
    }
    
    if (pv_names.empty()) {
        std::cout << "\nStill no PVs found. This suggests a deeper issue." << std::endl;
        std::cout << "Your MongoDB shows 1600+ PVs, but queries are not returning data." << std::endl;
        std::cout << "\nTry using specific PV names from your MongoDB:" << std::endl;
        std::cout << "  ./mongo_to_npy export-pvs \"ACCL_IN20_300_L0A_ACUHBR\"" << std::endl;
    } else {
        std::cout << "\n🎉 Found " << pv_names.size() << " working PVs!" << std::endl;
        std::cout << "Note: This is a small sample. Your MongoDB has 1600+ PVs available." << std::endl;
    }
    
    return pv_names;
}

bool MongoToNPYParser::validatePVsExist(const std::vector<std::string>& pv_names) {
    try {
        auto request = makeQueryPvMetadataRequest(pv_names);
        auto response = query_client_.queryPvMetadata(request);
        
        if (response.has_metadataresult()) {
            size_t found_count = response.metadataresult().pvinfos_size();
            if (found_count == pv_names.size()) {
                std::cout << "All " << pv_names.size() << " PVs validated successfully" << std::endl;
                return true;
            } else {
                std::cout << "Only " << found_count << " of " << pv_names.size() << " PVs found" << std::endl;
                
                // Show which PVs were found
                std::set<std::string> found_pvs;
                for (const auto& pv : response.metadataresult().pvinfos()) {
                    found_pvs.insert(pv.pvname());
                }
                
                std::cout << "Missing PVs:" << std::endl;
                for (const auto& pv_name : pv_names) {
                    if (found_pvs.find(pv_name) == found_pvs.end()) {
                        std::cout << "  - " << pv_name << std::endl;
                    }
                }
                return false;
            }
        } else {
            std::cout << "No PV metadata returned" << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "PV validation failed: " << e.what() << std::endl;
        std::cout << "Trying alternative validation..." << std::endl;
        
        // Alternative: try a simple data query to see if PVs exist
        try {
            // Use your actual data time range instead of current time
            uint64_t startTime = 1750690485;  // Your actual data start
            uint64_t endTime = 1750706894;    // Your actual data end
            
            Timestamp begin_ts = makeTimestamp(startTime, 0);
            Timestamp end_ts = makeTimestamp(endTime, 0);
            auto dataRequest = makeQueryDataRequest(pv_names, begin_ts, end_ts, false);
            
            auto dataResponse = query_client_.queryData(dataRequest);
            
            if (dataResponse.has_querydata()) {
                std::cout << "Data query successful - PVs likely exist" << std::endl;
                return true;
            } else {
                std::cout << "No data returned - PVs may not exist or have no data in the specified time range" << std::endl;
                return false;
            }
        } catch (const std::exception& e2) {
            std::cerr << "Alternative validation also failed: " << e2.what() << std::endl;
            return false;
        }
    }
}

void MongoToNPYParser::printDataStatistics(const std::vector<PVDataSeries>& pv_series) {
    std::cout << "\nDATA STATISTICS:" << std::endl;
    std::cout << "   PVs: " << pv_series.size() << std::endl;
    
    if (!pv_series.empty()) {
        size_t total_samples = 0;
        uint64_t earliest = UINT64_MAX, latest = 0;
        
        for (const auto& series : pv_series) {
            total_samples += series.sample_count;
            if (series.start_time < earliest) earliest = series.start_time;
            if (series.end_time > latest) latest = series.end_time;
        }
        
        std::cout << "   Total samples: " << total_samples << std::endl;
        std::cout << "   Time range: " << earliest << " to " << latest << " (" << (latest - earliest) << " seconds)" << std::endl;
        std::cout << "   Avg samples per PV: " << (total_samples / pv_series.size()) << std::endl;
    }
    std::cout << std::endl;
}

bool MongoToNPYParser::writeMetadataJSON(const std::vector<PVDataSeries>& pv_series, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "{\n";
    file << "  \"export_info\": {\n";
    file << "    \"timestamp\": " << std::time(nullptr) << ",\n";
    file << "    \"num_pvs\": " << pv_series.size() << ",\n";
    file << "    \"source\": \"MongoDB_via_MLDP\",\n";
    file << "    \"format\": \"numpy_npy\"\n";
    file << "  },\n";
    file << "  \"pvs\": [\n";
    
    for (size_t i = 0; i < pv_series.size(); ++i) {
        const auto& series = pv_series[i];
        file << "    {\n";
        file << "      \"name\": \"" << series.pv_name << "\",\n";
        file << "      \"sample_count\": " << series.sample_count << ",\n";
        file << "      \"sample_rate_hz\": " << series.sample_rate_hz << ",\n";
        file << "      \"start_time\": " << series.start_time << ",\n";
        file << "      \"end_time\": " << series.end_time << ",\n";
        file << "      \"data_type\": \"" << series.data_type << "\"\n";
        file << "    }";
        if (i < pv_series.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    file.close();
    
    return true;
}

bool MongoToNPYParser::writePVNamesFile(const std::vector<std::string>& pv_names, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    for (const auto& name : pv_names) {
        file << name << "\n";
    }
    
    file.close();
    return true;
}

// === PYTORCH INTEGRATION ===
namespace pytorch_integration {
    void generatePyTorchLoader(const std::string& npy_directory,
                              const NPYExportConfig& config,
                              const std::string& script_name) {
        std::ofstream script(npy_directory + "/" + script_name);
        
        script << R"(#!/usr/bin/env python3
"""
Auto-generated PyTorch loader for MongoDB/MLDP exported NPY data
"""
import numpy as np
import torch
import json
from pathlib import Path

def load_mldp_npy_data(data_dir, data_name="dataset"):
    """Load NPY data exported from MongoDB via MLDP"""
    data_dir = Path(data_dir)
    
    # Load main data matrix
    data_file = data_dir / f"{data_name}_data.npy"
    if not data_file.exists():
        raise FileNotFoundError(f"Data file not found: {data_file}")
    
    data = np.load(data_file)  # Shape: [time_points, num_pvs]
    print(f"Loaded data shape: {data.shape}")
    
    # Load timestamps if available
    timestamps = None
    timestamp_file = data_dir / f"{data_name}_timestamps.npy"
    if timestamp_file.exists():
        timestamps = np.load(timestamp_file)
        print(f"Loaded {len(timestamps)} timestamps")
    
    # Load PV names
    pv_names = []
    pv_names_file = data_dir / f"{data_name}_pv_names.txt"
    if pv_names_file.exists():
        with open(pv_names_file, 'r') as f:
            pv_names = [line.strip() for line in f]
        print(f"Loaded {len(pv_names)} PV names")
    
    # Load metadata
    metadata = {}
    metadata_file = data_dir / f"{data_name}_metadata.json"
    if metadata_file.exists():
        with open(metadata_file, 'r') as f:
            metadata = json.load(f)
        print(f"Loaded metadata for {metadata.get('export_info', {}).get('num_pvs', 0)} PVs")
    
    return {
        'data': torch.FloatTensor(data),
        'timestamps': torch.LongTensor(timestamps) if timestamps is not None else None,
        'pv_names': pv_names,
        'metadata': metadata
    }

class MLDPDataset(torch.utils.data.Dataset):
    """PyTorch Dataset for MLDP exported data"""
    
    def __init__(self, data_dir, data_name="dataset", sequence_length=None):
        self.data_dict = load_mldp_npy_data(data_dir, data_name)
        self.data = self.data_dict['data']
        self.timestamps = self.data_dict['timestamps']
        self.pv_names = self.data_dict['pv_names']
        self.sequence_length = sequence_length
        
    def __len__(self):
        if self.sequence_length:
            return len(self.data) - self.sequence_length + 1
        return len(self.data)
    
    def __getitem__(self, idx):
        if self.sequence_length:
            # Return sequence for LSTM/RNN
            data_seq = self.data[idx:idx+self.sequence_length]
            result = {'data': data_seq}
            
            if self.timestamps is not None:
                result['timestamps'] = self.timestamps[idx:idx+self.sequence_length]
                
            return result
        else:
            # Return single time point
            result = {'data': self.data[idx]}
            
            if self.timestamps is not None:
                result['timestamp'] = self.timestamps[idx]
                
            return result

# Example usage
if __name__ == "__main__":
    # Load data directly
    data_dict = load_mldp_npy_data(".", "dataset_raw")
    print(f"Data tensor shape: {data_dict['data'].shape}")
    print(f"PV names: {data_dict['pv_names'][:5]}...")  # First 5 PVs
    
    # Use with DataLoader
    dataset = MLDPDataset(".", "dataset_raw")
    dataloader = torch.utils.data.DataLoader(dataset, batch_size=32, shuffle=True)
    
    for batch in dataloader:
        print(f"Batch data shape: {batch['data'].shape}")
        break
)";
        
        script.close();
    }
    
    void generateMLTrainingTemplate(const std::string& npy_directory,
                                   const std::string& model_type,
                                   const std::string& script_name) {
        // Implementation for generating training templates
        std::ofstream script(npy_directory + "/" + script_name);
        
        script << "# TODO: ML training template for " << model_type << std::endl;
        script << "# This would generate LSTM, CNN, or Transformer training code" << std::endl;
        
        script.close();
    }
}
