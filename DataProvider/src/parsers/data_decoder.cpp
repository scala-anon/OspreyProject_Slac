#include "parsers/data_decoder.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <regex>
#include <ctime>
#include <stdexcept>
#include <cmath>
#include <limits>

void DecodedPVSeries::calculateStats() {
    if (data_points.empty()) {
        total_points = 0;
        min_value = max_value = avg_value = 0.0;
        return;
    }
    
    total_points = data_points.size();
    
    // Collect only valid (non-NaN, finite) values
    std::vector<double> valid_values;
    valid_values.reserve(data_points.size());
    
    for (const auto& point : data_points) {
        if (std::isfinite(point.value) && !std::isnan(point.value)) {
            valid_values.push_back(point.value);
        }
    }
    
    if (valid_values.empty()) {
        // All values are NaN/infinite
        min_value = max_value = avg_value = std::numeric_limits<double>::quiet_NaN();
        return;
    }
    
    // Calculate min/max from valid values only
    auto minmax = std::minmax_element(valid_values.begin(), valid_values.end());
    min_value = *minmax.first;
    max_value = *minmax.second;
    
    // Calculate average from valid values only
    double sum = std::accumulate(valid_values.begin(), valid_values.end(), 0.0);
    avg_value = sum / valid_values.size();
}

std::vector<DecodedDataPoint> DecodedPVSeries::getDataInRange(
    std::chrono::system_clock::time_point start,
    std::chrono::system_clock::time_point end) const {
    
    std::vector<DecodedDataPoint> result;
    
    for (const auto& point : data_points) {
        if (point.timestamp >= start && point.timestamp <= end) {
            result.push_back(point);
        }
    }
    
    return result;
}

DataDecoder::DataDecoder(const std::string& server_address)
    : server_address_(server_address) {
    
    query_client_ = std::make_unique<QueryClient>(server_address);
    
    std::cout << "Warning: Using default gRPC message size limits. "
              << "For large datasets, you may need to update QueryClient to use larger limits." << std::endl;
}

std::vector<DecodedPVSeries> DataDecoder::queryAndDecode(
    const std::vector<std::string>& pv_names,
    uint64_t start_time,
    uint64_t end_time,
    bool use_serialized) {
    
    std::cout << "Querying " << pv_names.size() << " PVs from " 
              << TimeUtils::formatUnixTime(start_time) << " to " 
              << TimeUtils::formatUnixTime(end_time) << std::endl;
    
    // Create timestamps
    Timestamp begin_ts = makeTimestamp(start_time, 0);
    Timestamp end_ts = makeTimestamp(end_time, 0);
    
    // Create query request
    QueryDataRequest request = makeQueryDataRequest(pv_names, begin_ts, end_ts, use_serialized);
    
    try {
        // Execute query
        QueryDataResponse response = query_client_->queryData(request);
        
        // Decode the response
        return decodeQueryResponse(response);
        
    } catch (const std::exception& e) {
        std::cerr << "Query failed: " << e.what() << std::endl;
        return {};
    }
}

std::vector<DecodedPVSeries> DataDecoder::queryAndDecodeByPattern(
    const std::string& pattern,
    uint64_t start_time,
    uint64_t end_time,
    bool use_serialized) {
    
    // First discover PVs matching pattern
    auto pv_names = discoverPVs(pattern);
    
    if (pv_names.empty()) {
        std::cout << "No PVs found matching pattern: " << pattern << std::endl;
        return {};
    }
    
    std::cout << "Found " << pv_names.size() << " PVs matching pattern: " << pattern << std::endl;
    
    // Query and decode the discovered PVs
    return queryAndDecode(pv_names, start_time, end_time, use_serialized);
}

std::vector<std::string> DataDecoder::discoverPVs(const std::string& pattern) const {
    try {
        // Create metadata query request
        QueryPvMetadataRequest request = makeQueryPvMetadataRequestWithPattern(pattern);
        
        // Execute metadata query
        QueryPvMetadataResponse response = query_client_->queryPvMetadata(request);
        
        std::vector<std::string> pv_names;
        
        // Check if we got a successful response
        if (response.has_metadataresult()) {
            const auto& metadata_result = response.metadataresult();
            
            for (const auto& pv_info : metadata_result.pvinfos()) {
                pv_names.push_back(pv_info.pvname());
            }
        } else if (response.has_exceptionalresult()) {
            std::cerr << "Metadata query failed with exceptional result" << std::endl;
        }
        
        return pv_names;
        
    } catch (const std::exception& e) {
        std::cerr << "PV discovery failed: " << e.what() << std::endl;
        return {};
    }
}

std::vector<DecodedPVSeries> DataDecoder::decodeQueryResponse(const QueryDataResponse& response) const {
    std::vector<DecodedPVSeries> decoded_series;
    
    // Check if response contains data
    if (!response.has_querydata()) {
        if (response.has_exceptionalresult()) {
            std::cerr << "Query returned exceptional result" << std::endl;
        } else {
            std::cerr << "Query response has no data" << std::endl;
        }
        return decoded_series;
    }
    
    const auto& query_data = response.querydata();
    
    std::cout << "Decoding " << query_data.databuckets_size() << " data buckets..." << std::endl;
    
    // Process each data bucket
    for (const auto& bucket : query_data.databuckets()) {
        // Decode timestamps
        auto timestamps = decodeTimestamps(bucket);
        
        // Decode data values
        std::vector<double> values;
        std::string pv_name = "Unknown_PV";
        
        if (bucket.has_datacolumn()) {
            values = decodeDataValues(bucket.datacolumn());
            pv_name = bucket.datacolumn().name();
        } else if (bucket.has_serializeddatacolumn()) {
            values = decodeSerializedDataColumn(bucket.serializeddatacolumn());
            pv_name = "SerializedPV_" + std::to_string(decoded_series.size());
        }
        
        // Create decoded series
        DecodedPVSeries series(pv_name);
        
        // Combine timestamps and values
        size_t min_size = std::min(timestamps.size(), values.size());
        for (size_t i = 0; i < min_size; ++i) {
            series.data_points.emplace_back(timestamps[i], values[i]);
        }
        
        // Calculate statistics
        series.calculateStats();
        
        decoded_series.push_back(std::move(series));
    }
    
    std::cout << "Successfully decoded " << decoded_series.size() << " PV series" << std::endl;
    return decoded_series;
}

std::chrono::system_clock::time_point DataDecoder::convertTimestamp(const Timestamp& ts) const {
    auto duration = std::chrono::seconds(ts.epochseconds()) + 
                   std::chrono::nanoseconds(ts.nanoseconds());
    return std::chrono::system_clock::time_point(duration);
}

std::vector<std::chrono::system_clock::time_point> DataDecoder::decodeTimestamps(
    const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket) const {
    
    std::vector<std::chrono::system_clock::time_point> timestamps;
    
    if (!bucket.has_datatimestamps()) {
        std::cerr << "Bucket has no timestamp information" << std::endl;
        return timestamps;
    }
    
    const auto& data_timestamps = bucket.datatimestamps();
    
    if (data_timestamps.value_case() == DataTimestamps::kSamplingClock) {
        // Decode sampling clock (regular intervals)
        const auto& sampling_clock = data_timestamps.samplingclock();
        
        auto start_time = convertTimestamp(sampling_clock.starttime());
        uint64_t period_nanos = sampling_clock.periodnanos();  // Matches: uint64 periodNanos = 2;
        uint32_t num_samples = sampling_clock.count();         // Matches: uint32 count = 3;
        
        for (uint32_t i = 0; i < num_samples; ++i) {
            auto sample_time = start_time + std::chrono::nanoseconds(i * period_nanos);
            timestamps.push_back(sample_time);
        }
        
    } else if (data_timestamps.value_case() == DataTimestamps::kTimestampList) {
        // Decode explicit timestamp list
        const auto& timestamps_list = data_timestamps.timestamplist();
        
        for (const auto& ts : timestamps_list.timestamps()) {
            timestamps.push_back(convertTimestamp(ts));
        }
    }
    
    return timestamps;
}

std::vector<double> DataDecoder::decodeDataValues(const DataColumn& column) const {
    std::vector<double> values;
    
    // Handle different data types based on DataValue oneof using value_case()
    for (const auto& data_value : column.datavalues()) {
        switch (data_value.value_case()) {
            case DataValue::kStringValue:
                try {
                    values.push_back(std::stod(data_value.stringvalue()));
                } catch (...) {
                    values.push_back(0.0);  // Default for non-numeric strings
                }
                break;
            case DataValue::kBooleanValue:
                values.push_back(data_value.booleanvalue() ? 1.0 : 0.0);
                break;
            case DataValue::kUintValue:
                values.push_back(static_cast<double>(data_value.uintvalue()));
                break;
            case DataValue::kUlongValue:
                values.push_back(static_cast<double>(data_value.ulongvalue()));
                break;
            case DataValue::kIntValue:
                values.push_back(static_cast<double>(data_value.intvalue()));
                break;
            case DataValue::kDoubleValue:
                values.push_back(data_value.doublevalue());
                break;
            case DataValue::kFloatValue:
                values.push_back(static_cast<double>(data_value.floatvalue()));
                break;
            default:
                // Handle unknown or unset values
                values.push_back(0.0);
                break;
        }
    }
    
    return values;
}

std::vector<double> DataDecoder::decodeSerializedDataColumn(const SerializedDataColumn& serialized_column) const {
    // TODO: Implement serialized data column decoding
    // This would involve deserializing the byte data back to a DataColumn
    // For now, return empty vector
    std::cerr << "Serialized data column decoding not yet implemented" << std::endl;
    return {};
}

void DataDecoder::printDecodedData(const std::vector<DecodedPVSeries>& decoded_data,
                                  size_t max_points,
                                  bool show_stats) const {
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "DECODED PV DATA SUMMARY" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    for (const auto& series : decoded_data) {
        std::cout << "\nPV: " << series.pv_name << std::endl;
        std::cout << "Total Points: " << series.total_points << std::endl;
        
        if (series.total_points > 0) {
           std::cout << "Value Range: " << std::scientific << std::setprecision(3) 
                << series.min_value << " to " << series.max_value << std::endl;
            std::cout << "Average: " << std::scientific << std::setprecision(3) << series.avg_value << std::endl;
        }
        
        std::cout << "\nSample Data Points (showing up to " << max_points << "):" << std::endl;
        std::cout << std::left << std::setw(25) << "Timestamp" 
                  << std::setw(15) << "Value" 
                  << std::setw(10) << "Status" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        size_t points_to_show = std::min(max_points, series.data_points.size());
        for (size_t i = 0; i < points_to_show; ++i) {
            const auto& point = series.data_points[i];
            std::cout << std::left << std::setw(25) << formatTimestamp(point.timestamp)
                      << std::setw(15) << std::fixed << std::setprecision(3) << point.value
                      << std::setw(10) << point.status << std::endl;
        }
        
        if (series.data_points.size() > max_points) {
            std::cout << "... and " << (series.data_points.size() - max_points) 
                      << " more points" << std::endl;
        }
        
        std::cout << std::string(80, '-') << std::endl;
    }
    
    if (show_stats) {
        printStatistics(decoded_data);
    }
}

bool DataDecoder::exportToCSV(const std::vector<DecodedPVSeries>& decoded_data,
                             const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }
        
        // Write header
        file << "PV_Name,Timestamp,Unix_Timestamp,Value,Status,Severity" << std::endl;
        
        // Write data
        for (const auto& series : decoded_data) {
            for (const auto& point : series.data_points) {
                file << series.pv_name << ","
                     << formatTimestamp(point.timestamp) << ","
                     << TimeUtils::toUnixTime(point.timestamp) << ","
                     << std::fixed << std::setprecision(6) << point.value << ","
                     << point.status << ","
                     << point.severity << std::endl;
            }
        }
        
        file.close();
        std::cout << "Data exported to CSV: " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error exporting to CSV: " << e.what() << std::endl;
        return false;
    }
}

bool DataDecoder::exportToJSON(const std::vector<DecodedPVSeries>& decoded_data,
                              const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }
        
        file << "{\n  \"decoded_pv_data\": [\n";
        
        for (size_t i = 0; i < decoded_data.size(); ++i) {
            const auto& series = decoded_data[i];
            
            file << "    {\n";
            file << "      \"pv_name\": \"" << series.pv_name << "\",\n";
            file << "      \"total_points\": " << series.total_points << ",\n";
            file << "      \"statistics\": {\n";
            file << "        \"min_value\": " << series.min_value << ",\n";
            file << "        \"max_value\": " << series.max_value << ",\n";
            file << "        \"avg_value\": " << series.avg_value << "\n";
            file << "      },\n";
            file << "      \"data_points\": [\n";
            
            for (size_t j = 0; j < series.data_points.size(); ++j) {
                const auto& point = series.data_points[j];
                file << "        {\n";
                file << "          \"timestamp\": \"" << formatTimestamp(point.timestamp) << "\",\n";
                file << "          \"unix_timestamp\": " << TimeUtils::toUnixTime(point.timestamp) << ",\n";
                file << "          \"value\": " << point.value << ",\n";
                file << "          \"status\": \"" << point.status << "\",\n";
                file << "          \"severity\": " << point.severity << "\n";
                file << "        }";
                if (j < series.data_points.size() - 1) file << ",";
                file << "\n";
            }
            
            file << "      ]\n";
            file << "    }";
            if (i < decoded_data.size() - 1) file << ",";
            file << "\n";
        }
        
        file << "  ]\n}\n";
        file.close();
        
        std::cout << "Data exported to JSON: " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error exporting to JSON: " << e.what() << std::endl;
        return false;
    }
}

void DataDecoder::printStatistics(const std::vector<DecodedPVSeries>& decoded_data) const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "STATISTICAL SUMMARY" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    size_t total_points = 0;
    for (const auto& series : decoded_data) {
        total_points += series.total_points;
    }
    
    std::cout << "Total PVs: " << decoded_data.size() << std::endl;
    std::cout << "Total Data Points: " << total_points << std::endl;
    std::cout << "Average Points per PV: "
              << (decoded_data.empty() ? 0 : total_points / decoded_data.size()) << std::endl;
    
    std::cout << "\nPer-PV Statistics:" << std::endl;
    std::cout << std::left << std::setw(25) << "PV Name"
              << std::setw(10) << "Points"
              << std::setw(15) << "Min Value"
              << std::setw(15) << "Max Value"
              << std::setw(15) << "Avg Value" << std::endl;
    std::cout << std::string(85, '-') << std::endl;
    
    for (const auto& series : decoded_data) {
        std::cout << std::left << std::setw(25) << series.pv_name.substr(0, 24)
                  << std::setw(10) << series.total_points
                  << std::setw(15) << std::scientific << std::setprecision(2) << series.min_value
                  << std::setw(15) << series.max_value
                  << std::setw(15) << series.avg_value << std::endl;
    }
}

bool DataDecoder::testConnection() const {
    try {
        // Try a simple metadata query to test connection
        auto pv_names = discoverPVs(".*BPM.*");
        std::cout << "Connection test successful - found " << pv_names.size() 
                  << " BPM PVs" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Connection test failed: " << e.what() << std::endl;
        return false;
    }
}

QueryDataResponse DataDecoder::getRawQueryResponse(
    const std::vector<std::string>& pv_names,
    uint64_t start_time,
    uint64_t end_time,
    bool use_serialized) const {
    
    Timestamp begin_ts = makeTimestamp(start_time, 0);
    Timestamp end_ts = makeTimestamp(end_time, 0);
    QueryDataRequest request = makeQueryDataRequest(pv_names, begin_ts, end_ts, use_serialized);
    
    return query_client_->queryData(request);
}

// Helper methods
std::string DataDecoder::formatTimestamp(const std::chrono::system_clock::time_point& tp) const {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string DataDecoder::formatDuration(std::chrono::seconds duration) const {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration % std::chrono::hours(1));
    auto seconds = duration % std::chrono::minutes(1);
    
    std::ostringstream oss;
    if (hours.count() > 0) {
        oss << hours.count() << "h ";
    }
    if (minutes.count() > 0) {
        oss << minutes.count() << "m ";
    }
    oss << seconds.count() << "s";
    return oss.str();
}


uint64_t TimeUtils::getCurrentUnixTime() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::chrono::system_clock::time_point TimeUtils::fromUnixTime(uint64_t unix_time) {
    return std::chrono::system_clock::from_time_t(unix_time);
}

uint64_t TimeUtils::toUnixTime(const std::chrono::system_clock::time_point& tp) {
    return std::chrono::duration_cast<std::chrono::seconds>(
        tp.time_since_epoch()).count();
}

uint64_t TimeUtils::parseTimeString(const std::string& time_str) {
    // Try parsing as Unix timestamp first
    try {
        return std::stoull(time_str);
    } catch (...) {
        // Not a number, try parsing as formatted string
    }
    
    // Try parsing as "YYYY-MM-DD HH:MM:SS"
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    if (!ss.fail()) {
        return static_cast<uint64_t>(std::mktime(&tm));
    }
    
    // Default to current time if parsing fails
    std::cerr << "Failed to parse time string: " << time_str << ", using current time" << std::endl;
    return getCurrentUnixTime();
}

uint64_t TimeUtils::getRelativeTime(const std::string& relative_str) {
    uint64_t now = getCurrentUnixTime();
    
    // Parse strings like "1h", "30m", "2d", etc.
    std::regex pattern(R"((\d+)([smhd]))");
    std::smatch match;
    
    if (std::regex_match(relative_str, match, pattern)) {
        int value = std::stoi(match[1]);
        char unit = match[2].str()[0];
        
        switch (unit) {
            case 's': return now - value;
            case 'm': return now - (value * 60);
            case 'h': return now - (value * 3600);
            case 'd': return now - (value * 86400);
        }
    }
    
    return now;
}

std::string TimeUtils::formatUnixTime(uint64_t unix_time) {
    auto time_t = static_cast<std::time_t>(unix_time);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
