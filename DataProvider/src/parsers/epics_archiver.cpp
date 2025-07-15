#include "epics_archiver.hpp"
#include "clients/ingest_client.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <thread>
#include <numeric>

EPICSArchiver::EPICSArchiver(const std::string& archiver_url, bool verbose)
    : archiver_url_(archiver_url), temp_file_prefix_("epics_temp"), verbose_(verbose) {
    
    if (verbose_) {
        std::cout << "EPICS Archiver initialized with URL: " << archiver_url_ << std::endl;
    }
}

bool EPICSArchiver::testConnection() const {
    std::cout << "Testing EPICS Archiver connection..." << std::endl;
    std::cout << "Archiver URL: " << archiver_url_ << std::endl;
    
    std::string test_pv = "IOC:BSY0:BP01:MEM_FREE";
    std::string end_time = getCurrentISOTime();
    std::string start_time = getYesterdayISOTime();
    
    std::string test_url = buildQueryURL(test_pv, start_time, end_time);
    std::string temp_file = generateTempFileName();
    
    std::cout << "Test query URL: " << test_url << std::endl;
    
    std::string curl_command = "curl --noproxy \"*\" --silent --max-time 30 \"" + test_url + "\" -o " + temp_file;
    
    if (verbose_) {
        std::cout << "Executing: " << curl_command << std::endl;
    }
    
    bool success = executeCommand(curl_command);
    
    if (success) {
        std::string response = readFileContents(temp_file);
        
        if (response.empty()) {
            std::cout << "❌ Empty response from archiver" << std::endl;
            cleanupTempFile(temp_file);
            return false;
        }
        
        if (response.find("\"meta\"") != std::string::npos || response.find("\"data\"") != std::string::npos) {
            std::cout << "✅ Connection successful!" << std::endl;
            std::cout << "Response size: " << response.length() << " bytes" << std::endl;
            
            size_t first_newline = response.find('\n');
            std::string first_line = (first_newline != std::string::npos) ? 
                                   response.substr(0, first_newline) : response.substr(0, 100);
            std::cout << "Sample response: " << first_line << "..." << std::endl;
            
            cleanupTempFile(temp_file);
            return true;
        } else {
            std::cout << "❌ Invalid response format" << std::endl;
            std::cout << "Response: " << response.substr(0, 200) << "..." << std::endl;
        }
    } else {
        std::cout << "❌ Failed to connect to archiver" << std::endl;
    }
    
    cleanupTempFile(temp_file);
    return false;
}

EPICSTimeSeries EPICSArchiver::fetchPVData(const std::string& pv_name,
                                          const std::string& start_time,
                                          const std::string& end_time) const {
    EPICSTimeSeries series;
    
    if (verbose_) {
        std::cout << "Fetching data for PV: " << pv_name << std::endl;
        std::cout << "Time range: " << start_time << " to " << end_time << std::endl;
    }
    
    // Expand time keywords
    std::string actual_start = expandTimeKeyword(start_time);
    std::string actual_end = expandTimeKeyword(end_time);
    
    // Build query URL
    std::string query_url = buildQueryURL(pv_name, actual_start, actual_end);
    std::string temp_file = generateTempFileName();
    
    // Execute curl command
    std::string curl_command = "curl --noproxy \"*\" --silent --max-time 60 \"" + query_url + "\" -o " + temp_file;
    
    if (verbose_) {
        std::cout << "Query URL: " << query_url << std::endl;
        std::cout << "Executing: " << curl_command << std::endl;
    }
    
    if (!executeCommand(curl_command)) {
        std::cerr << "Failed to execute curl command for PV: " << pv_name << std::endl;
        cleanupTempFile(temp_file);
        return series;
    }
    
    // Read and parse response
    std::string response = readFileContents(temp_file);
    cleanupTempFile(temp_file);
    
    if (response.empty()) {
        std::cerr << "Empty response for PV: " << pv_name << std::endl;
        return series;
    }
    
    if (verbose_) {
        std::cout << "Raw response length: " << response.length() << " bytes" << std::endl;
        std::cout << "First 200 chars: " << response.substr(0, 200) << std::endl;
    }
    
    // Parse the JSON response (EPICS archiver format)
    try {
        // Set the PV name in metadata
        series.metadata.pv_name = pv_name;
        
        // Parse metadata section: "meta": { "name": "...", "EGU": "...", "PREC": "..." }
        size_t meta_start = response.find("\"meta\"");
        if (meta_start != std::string::npos) {
            size_t meta_brace_start = response.find("{", meta_start);
            size_t meta_brace_end = response.find("}", meta_brace_start);
            if (meta_brace_start != std::string::npos && meta_brace_end != std::string::npos) {
                std::string meta_section = response.substr(meta_brace_start, meta_brace_end - meta_brace_start + 1);
                
                // Extract EGU (units)
                std::string egu = extractJsonValue(meta_section, "EGU");
                if (!egu.empty()) {
                    series.metadata.units = egu;
                }
                
                // Extract PREC (precision)
                std::string prec = extractJsonValue(meta_section, "PREC");
                if (!prec.empty()) {
                    series.metadata.precision = prec;
                }
                
                if (verbose_) {
                    std::cout << "Parsed metadata - Units: " << series.metadata.units 
                              << ", Precision: " << series.metadata.precision << std::endl;
                }
            }
        }
        
        // Parse data section: "data": [ { "secs": ..., "val": ..., "nanos": ..., "severity": ..., "status": ... }, ... ]
        size_t data_start = response.find("\"data\"");
        if (data_start != std::string::npos) {
            size_t array_start = response.find("[", data_start);
            size_t array_end = response.rfind("]");
            
            if (array_start != std::string::npos && array_end != std::string::npos) {
                std::string data_section = response.substr(array_start + 1, array_end - array_start - 1);
                
                if (verbose_) {
                    std::cout << "Data section length: " << data_section.length() << " chars" << std::endl;
                }
                
                // Parse individual data points
                size_t pos = 0;
                while (pos < data_section.length()) {
                    size_t obj_start = data_section.find("{", pos);
                    if (obj_start == std::string::npos) break;
                    
                    size_t obj_end = data_section.find("}", obj_start);
                    if (obj_end == std::string::npos) break;
                    
                    std::string data_obj = data_section.substr(obj_start, obj_end - obj_start + 1);
                    EPICSDataPoint point = parseDataPoint(data_obj);
                    
                    // Only add valid points
                    if (point.seconds > 0) {
                        series.data_points.push_back(point);
                    }
                    
                    pos = obj_end + 1;
                }
            }
        }
        
        if (verbose_) {
            std::cout << "Successfully parsed " << series.data_points.size() 
                      << " data points for " << pv_name << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON response for PV " << pv_name << ": " << e.what() << std::endl;
    }
    
    return series;
}

std::vector<EPICSTimeSeries> EPICSArchiver::fetchMultiplePVs(const std::vector<std::string>& pv_names,
                                                            const std::string& start_time,
                                                            const std::string& end_time) const {
    std::vector<EPICSTimeSeries> results;
    
    std::cout << "Fetching data for " << pv_names.size() << " PVs..." << std::endl;
    
    for (size_t i = 0; i < pv_names.size(); ++i) {
        std::cout << "Progress: " << (i + 1) << "/" << pv_names.size() 
                  << " - Fetching " << pv_names[i] << std::endl;
        
        EPICSTimeSeries series = fetchPVData(pv_names[i], start_time, end_time);
        
        if (!series.empty()) {
            results.push_back(std::move(series));
        } else {
            std::cerr << "Warning: No data retrieved for PV: " << pv_names[i] << std::endl;
        }
        
        // Small delay to be nice to the archiver
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Successfully fetched data for " << results.size() << "/" << pv_names.size() << " PVs" << std::endl;
    return results;
}

// WORKING JSON PARSING METHODS
std::string EPICSArchiver::extractJsonValue(const std::string& json, const std::string& key) const {
    // Look for "key": "value" or "key": value pattern
    std::string search_pattern = "\"" + key + "\"";
    size_t key_pos = json.find(search_pattern);
    
    if (key_pos == std::string::npos) return "";
    
    size_t colon_pos = json.find(":", key_pos);
    if (colon_pos == std::string::npos) return "";
    
    // Skip whitespace after colon
    size_t value_start = colon_pos + 1;
    while (value_start < json.length() && (json[value_start] == ' ' || json[value_start] == '\t')) {
        value_start++;
    }
    
    if (value_start >= json.length()) return "";
    
    std::string value;
    
    if (json[value_start] == '"') {
        // String value - find closing quote
        value_start++; // Skip opening quote
        size_t value_end = json.find('"', value_start);
        if (value_end != std::string::npos) {
            value = json.substr(value_start, value_end - value_start);
        }
    } else {
        // Numeric value - find next comma, } or whitespace
        size_t value_end = value_start;
        while (value_end < json.length() && 
               json[value_end] != ',' && 
               json[value_end] != '}' && 
               json[value_end] != ' ' && 
               json[value_end] != '\t' &&
               json[value_end] != '\n') {
            value_end++;
        }
        value = json.substr(value_start, value_end - value_start);
        
        // Trim any trailing whitespace
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
            value.pop_back();
        }
    }
    
    return value;
}

EPICSDataPoint EPICSArchiver::parseDataPoint(const std::string& json_object) const {
    EPICSDataPoint point;
    
    try {
        std::string secs_str = extractJsonValue(json_object, "secs");
        std::string nanos_str = extractJsonValue(json_object, "nanos");
        std::string val_str = extractJsonValue(json_object, "val");
        std::string sev_str = extractJsonValue(json_object, "severity");
        std::string stat_str = extractJsonValue(json_object, "status");
        
        if (!secs_str.empty()) {
            point.seconds = std::stoull(secs_str);
        }
        if (!nanos_str.empty()) {
            point.nanoseconds = std::stoull(nanos_str);
        }
        if (!val_str.empty()) {
            point.value = std::stod(val_str);
        }
        if (!sev_str.empty()) {
            point.severity = std::stoi(sev_str);
        }
        if (!stat_str.empty()) {
            point.status = std::stoi(stat_str);
        }
        
        if (verbose_) {
            std::cout << "Parsed point: secs=" << point.seconds 
                      << ", val=" << point.value 
                      << ", nanos=" << point.nanoseconds << std::endl;
        }
        
    } catch (const std::exception& e) {
        if (verbose_) {
            std::cerr << "Warning: Error parsing data point: " << e.what() << std::endl;
            std::cerr << "JSON object: " << json_object << std::endl;
        }
    }
    
    return point;
}

// Placeholder implementations for methods we don't need yet
bool EPICSArchiver::ingestToMongoDB(const EPICSTimeSeries& series,
                                   IngestClient& ingest_client,
                                   const std::string& provider_id) const {
    std::cout << "TODO: Complete ingestToMongoDB implementation" << std::endl;
    return false;
}

bool EPICSArchiver::ingestMultipleToMongoDB(const std::vector<EPICSTimeSeries>& series_vector,
                                           IngestClient& ingest_client,
                                           const std::string& provider_id,
                                           bool use_streaming) const {
    std::cout << "TODO: Complete ingestMultipleToMongoDB implementation" << std::endl;
    return false;
}

std::vector<std::string> EPICSArchiver::parsePVList(const std::string& pv_list) {
    std::vector<std::string> pv_names;
    std::stringstream ss(pv_list);
    std::string pv;
    
    while (std::getline(ss, pv, ',')) {
        // Trim whitespace
        pv.erase(0, pv.find_first_not_of(" \t"));
        pv.erase(pv.find_last_not_of(" \t") + 1);
        
        if (!pv.empty()) {
            pv_names.push_back(pv);
        }
    }
    
    return pv_names;
}

bool EPICSArchiver::isValidPVName(const std::string& pv_name) {
    if (pv_name.empty()) return false;
    std::regex pv_pattern(R"([A-Za-z0-9:_\-\.]+)");
    return std::regex_match(pv_name, pv_pattern);
}

void EPICSArchiver::printDataStatistics(const EPICSTimeSeries& series) const {
    std::cout << "\n=== Data Statistics for " << series.metadata.pv_name << " ===" << std::endl;
    std::cout << "Total points: " << series.total_points() << std::endl;
    std::cout << "Units: " << series.metadata.units << std::endl;
    std::cout << "Precision: " << series.metadata.precision << std::endl;
    
    if (!series.empty()) {
        std::cout << "Time range: " << epics_time_utils::formatTimestamp(series.start_time(), 0)
                  << " to " << epics_time_utils::formatTimestamp(series.end_time(), 0) << std::endl;
        
        // Calculate value statistics
        std::vector<double> values;
        for (const auto& point : series.data_points) {
            values.push_back(point.value);
        }
        
        if (!values.empty()) {
            auto minmax = std::minmax_element(values.begin(), values.end());
            double sum = std::accumulate(values.begin(), values.end(), 0.0);
            double mean = sum / values.size();
            
            std::cout << "Value range: " << *minmax.first << " to " << *minmax.second << std::endl;
            std::cout << "Mean value: " << mean << std::endl;
            
            // Show first few data points
            std::cout << "\nFirst few data points:" << std::endl;
            for (size_t i = 0; i < std::min(size_t(5), series.data_points.size()); ++i) {
                const auto& point = series.data_points[i];
                std::cout << "  " << i+1 << ". Time: " << point.seconds 
                          << ", Value: " << point.value 
                          << ", Status: " << point.status << std::endl;
            }
        }
    }
}

void EPICSArchiver::printMultipleDataStatistics(const std::vector<EPICSTimeSeries>& series_vector) const {
    std::cout << "\n=== Multiple PV Statistics ===" << std::endl;
    std::cout << "Total PVs: " << series_vector.size() << std::endl;
    
    size_t total_points = 0;
    for (const auto& series : series_vector) {
        total_points += series.total_points();
    }
    std::cout << "Total data points: " << total_points << std::endl;
    
    for (const auto& series : series_vector) {
        if (!series.empty()) {
            std::cout << "  " << series.metadata.pv_name << ": " << series.total_points() << " points" << std::endl;
        }
    }
}

// Placeholder implementations for unused methods
std::vector<std::string> EPICSArchiver::extractJsonArray(const std::string& json, const std::string& array_name) const {
    return std::vector<std::string>();
}

EPICSMetadata EPICSArchiver::parseMetadata(const std::string& json_object) const {
    return EPICSMetadata();
}

// Helper method implementations
std::string EPICSArchiver::urlEncode(const std::string& value) const {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    
    for (char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    
    return escaped.str();
}

std::string EPICSArchiver::buildQueryURL(const std::string& pv_name,
                                        const std::string& start_time,
                                        const std::string& end_time) const {
    std::ostringstream url;
    url << archiver_url_ << "?";
    url << "pv=" << urlEncode(pv_name);
    url << "&from=" << urlEncode(start_time);
    url << "&to=" << urlEncode(end_time);
    
    return url.str();
}

std::string EPICSArchiver::generateTempFileName() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream filename;
    filename << temp_file_prefix_ << "_" << time_t << ".json";
    
    return filename.str();
}

bool EPICSArchiver::executeCommand(const std::string& command) const {
    int result = std::system(command.c_str());
    return result == 0;
}

std::string EPICSArchiver::readFileContents(const std::string& filename) const {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    file.close();
    
    return content.str();
}

void EPICSArchiver::cleanupTempFile(const std::string& filename) const {
    try {
        std::filesystem::remove(filename);
    } catch (const std::exception& e) {
        if (verbose_) {
            std::cerr << "Warning: Failed to remove temp file " << filename << ": " << e.what() << std::endl;
        }
    }
}

std::string EPICSArchiver::getCurrentISOTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto gmt = std::gmtime(&time_t);
    
    std::ostringstream iso_time;
    iso_time << std::put_time(gmt, "%Y-%m-%dT%H:%M:%S.000Z");
    
    return iso_time.str();
}

std::string EPICSArchiver::getYesterdayISOTime() const {
    auto now = std::chrono::system_clock::now();
    auto yesterday = now - std::chrono::hours(24);
    auto time_t = std::chrono::system_clock::to_time_t(yesterday);
    auto gmt = std::gmtime(&time_t);
    
    std::ostringstream iso_time;
    iso_time << std::put_time(gmt, "%Y-%m-%dT%H:%M:%S.000Z");
    
    return iso_time.str();
}

std::string EPICSArchiver::expandTimeKeyword(const std::string& time_spec) const {
    if (time_spec == "today") {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto local = std::localtime(&time_t);
        
        // Start of today
        local->tm_hour = 0;
        local->tm_min = 0;
        local->tm_sec = 0;
        
        auto start_of_day = std::mktime(local);
        auto gmt = std::gmtime(&start_of_day);
        
        std::ostringstream iso_time;
        iso_time << std::put_time(gmt, "%Y-%m-%dT%H:%M:%S.000Z");
        
        return iso_time.str();
    } else if (time_spec == "yesterday") {
        return getYesterdayISOTime();
    } else if (time_spec == "now") {
        return getCurrentISOTime();
    }
    
    // Return as-is for ISO format strings
    return time_spec;
}

// Time utilities namespace implementation
namespace epics_time_utils {
    uint64_t epicsToMLDPTimestamp(uint64_t seconds, uint64_t nanoseconds) {
        return seconds * 1000000000ULL + nanoseconds;
    }
    
    std::string formatTimestamp(uint64_t seconds, uint64_t nanoseconds) {
        auto time_t = static_cast<std::time_t>(seconds);
        auto gmt = std::gmtime(&time_t);
        
        std::ostringstream formatted;
        formatted << std::put_time(gmt, "%Y-%m-%d %H:%M:%S");
        
        if (nanoseconds > 0) {
            formatted << "." << std::setfill('0') << std::setw(9) << nanoseconds;
        }
        
        formatted << " UTC";
        return formatted.str();
    }
    
    std::string getCurrentTimeISO() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto gmt = std::gmtime(&time_t);
        
        std::ostringstream iso_time;
        iso_time << std::put_time(gmt, "%Y-%m-%dT%H:%M:%S.000Z");
        
        return iso_time.str();
    }
    
    std::string parseTimeKeyword(const std::string& keyword) {
        if (keyword == "today" || keyword == "now") {
            return getCurrentTimeISO();
        } else if (keyword == "yesterday") {
            auto now = std::chrono::system_clock::now();
            auto yesterday = now - std::chrono::hours(24);
            auto time_t = std::chrono::system_clock::to_time_t(yesterday);
            auto gmt = std::gmtime(&time_t);
            
            std::ostringstream iso_time;
            iso_time << std::put_time(gmt, "%Y-%m-%dT%H:%M:%S.000Z");
            
            return iso_time.str();
        }
        
        return keyword;
    }
}
