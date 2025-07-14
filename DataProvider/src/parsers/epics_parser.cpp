#include "epics_parser.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <regex>
#include <ctime>
#include <iomanip>
#include <thread>
#include <future>
#include <fstream>
#include <numeric>
#include <cmath>

// Include JSON parsing (assumes nlohmann/json is available)
#ifdef HAVE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#else
#error "nlohmann/json is required for EPICS parser"
#endif

// =============================================================================
// EPICSTimeRange Implementation
// =============================================================================

EPICSTimeRange::EPICSTimeRange(uint64_t start_epoch, uint64_t end_epoch)
    : from_epoch(start_epoch), to_epoch(end_epoch) {
    from_iso = EPICSTimeUtils::epochToISO8601(start_epoch);
    to_iso = EPICSTimeUtils::epochToISO8601(end_epoch);
}

EPICSTimeRange::EPICSTimeRange(const std::string& from, const std::string& to)
    : from_iso(from), to_iso(to) {
    from_epoch = EPICSTimeUtils::iso8601ToEpoch(from);
    to_epoch = EPICSTimeUtils::iso8601ToEpoch(to);
}

// =============================================================================
// EPICSSignalData Implementation
// =============================================================================

EPICSSignalData::EPICSSignalData(const std::string& pv_name, const EPICSTimeRange& range)
    : time_range(range), sample_count(0), min_value(0), max_value(0), avg_value(0),
      start_time_sec(0), end_time_sec(0), query_status("pending") {
    info.pv_name = pv_name;
}

void EPICSSignalData::calculateStats() {
    if (data_points.empty()) {
        sample_count = 0;
        min_value = max_value = avg_value = 0.0;
        query_status = "no_data";
        return;
    }
    
    sample_count = data_points.size();
    start_time_sec = data_points.front().seconds;
    end_time_sec = data_points.back().seconds;
    
    // Extract valid (finite) values
    std::vector<double> valid_values;
    for (const auto& point : data_points) {
        if (std::isfinite(point.value)) {
            valid_values.push_back(point.value);
        }
    }
    
    if (valid_values.empty()) {
        min_value = max_value = avg_value = std::numeric_limits<double>::quiet_NaN();
        query_status = "invalid_data";
        return;
    }
    
    auto minmax = std::minmax_element(valid_values.begin(), valid_values.end());
    min_value = *minmax.first;
    max_value = *minmax.second;
    avg_value = std::accumulate(valid_values.begin(), valid_values.end(), 0.0) / valid_values.size();
    query_status = "success";
}

// =============================================================================
// EPICSParser Implementation
// =============================================================================

EPICSParser::EPICSParser(const std::string& management_url, const std::string& retrieval_url)
    : management_url_(management_url), retrieval_url_(retrieval_url), 
      timeout_(30), verbose_(false) {
    
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle_ = curl_easy_init();
    
    if (!curl_handle_) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    // Set common CURL options
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT, timeout_);
    curl_easy_setopt(curl_handle_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle_, CURLOPT_NOPROXY, "*"); // CRITICAL: Bypass proxy
    
    if (verbose_) {
        curl_easy_setopt(curl_handle_, CURLOPT_VERBOSE, 1L);
    }
    
    std::cout << "EPICS Parser initialized:" << std::endl;
    std::cout << "  Management URL: " << management_url_ << std::endl;
    std::cout << "  Retrieval URL: " << retrieval_url_ << std::endl;
}

EPICSParser::~EPICSParser() {
    if (curl_handle_) {
        curl_easy_cleanup(curl_handle_);
    }
    curl_global_cleanup();
}

// === PV DISCOVERY METHODS ===

std::vector<std::string> EPICSParser::discoverPVs(const std::string& regex_pattern, size_t limit) const {
    std::string url = buildDiscoveryURL(regex_pattern, limit);
    
    if (verbose_) {
        std::cout << "Discovering PVs with URL: " << url << std::endl;
    }
    
    HTTPResponse response = httpGet(url);
    
    if (!response.isSuccess()) {
        handleHTTPError(response, "PV discovery");
        return {};
    }
    
    try {
        return parseDiscoveryResponse(response.data);
    } catch (const std::exception& e) {
        handleJSONError(e.what(), "PV discovery parsing");
        return {};
    }
}

std::vector<std::string> EPICSParser::discoverByDevice(const std::string& device_pattern, size_t limit) const {
    // Convert device pattern to EPICS regex: DEVICE:.*
    std::string epics_pattern = device_pattern + ":.*";
    return discoverPVs(epics_pattern, limit);
}

bool EPICSParser::testConnection() const {
    try {
        std::cout << "Testing EPICS archiver connection..." << std::endl;
        
        // Test management interface
        std::string test_url = management_url_ + "getVersion";
        HTTPResponse response = httpGet(test_url);
        
        if (!response.isSuccess()) {
            std::cerr << "Management interface test failed: " << response.error_message << std::endl;
            return false;
        }
        
        std::cout << "✓ Management interface accessible" << std::endl;
        
        // Test discovery with simple pattern
        auto test_pvs = discoverPVs("IOC.*", 5);
        
        if (test_pvs.empty()) {
            std::cerr << "Discovery test failed: no PVs found" << std::endl;
            return false;
        }
        
        std::cout << "✓ Discovery working - found " << test_pvs.size() << " test PVs" << std::endl;
        
        // Test data retrieval
        if (!test_pvs.empty()) {
            EPICSTimeRange recent_range = EPICSTimeUtils::createRecentTimeRange(1); // Last hour
            auto test_data = queryPV(test_pvs[0], recent_range);
            
            if (test_data.query_status == "success") {
                std::cout << "✓ Data retrieval working - got " << test_data.sample_count << " samples" << std::endl;
            } else {
                std::cout << "⚠ Data retrieval limited - status: " << test_data.query_status << std::endl;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Connection test failed: " << e.what() << std::endl;
        return false;
    }
}

// === DATA QUERY METHODS ===

EPICSSignalData EPICSParser::queryPV(const std::string& pv_name, const EPICSTimeRange& time_range) const {
    EPICSSignalData signal_data(pv_name, time_range);
    signal_data.info = parsePVName(pv_name);
    
    std::string url = buildDataURL(pv_name, time_range);
    
    if (verbose_) {
        std::cout << "Querying PV: " << pv_name << " from " << time_range.from_iso 
                  << " to " << time_range.to_iso << std::endl;
    }
    
    HTTPResponse response = httpGet(url);
    
    if (!response.isSuccess()) {
        handleHTTPError(response, "PV data query for " + pv_name);
        signal_data.query_status = "http_error";
        return signal_data;
    }
    
    try {
        signal_data = parseDataResponse(response.data, pv_name, time_range);
        signal_data.calculateStats();
        
        if (verbose_ && signal_data.query_status == "success") {
            std::cout << "  Retrieved " << signal_data.sample_count << " samples" << std::endl;
        }
        
    } catch (const std::exception& e) {
        handleJSONError(e.what(), "PV data parsing for " + pv_name);
        signal_data.query_status = "parse_error";
    }
    
    return signal_data;
}

std::vector<EPICSSignalData> EPICSParser::queryMultiplePVs(const std::vector<std::string>& pv_names,
                                                          const EPICSTimeRange& time_range,
                                                          bool parallel) const {
    std::vector<EPICSSignalData> results;
    
    if (parallel && pv_names.size() > 1) {
        // Parallel queries using futures
        std::vector<std::future<EPICSSignalData>> futures;
        
        for (const auto& pv_name : pv_names) {
            futures.push_back(std::async(std::launch::async, [this, pv_name, time_range]() {
                return queryPV(pv_name, time_range);
            }));
        }
        
        // Collect results
        for (auto& future : futures) {
            results.push_back(future.get());
        }
        
    } else {
        // Sequential queries
        for (const auto& pv_name : pv_names) {
            results.push_back(queryPV(pv_name, time_range));
        }
    }
    
    return results;
}

std::vector<EPICSSignalData> EPICSParser::queryByPattern(const std::string& pattern,
                                                        const EPICSTimeRange& time_range,
                                                        size_t limit) const {
    // First discover PVs
    auto pv_names = discoverPVs(pattern, limit);
    
    if (pv_names.empty()) {
        std::cout << "No PVs found matching pattern: " << pattern << std::endl;
        return {};
    }
    
    std::cout << "Found " << pv_names.size() << " PVs matching pattern: " << pattern << std::endl;
    
    // Then query data
    return queryMultiplePVs(pv_names, time_range, true);
}

// === HTTP REQUEST METHODS ===

HTTPResponse EPICSParser::httpGet(const std::string& url) const {
    HTTPResponse response;
    
    curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &response.data);
    
    CURLcode res = curl_easy_perform(curl_handle_);
    
    if (res != CURLE_OK) {
        response.error_message = curl_easy_strerror(res);
        response.response_code = 0;
    } else {
        curl_easy_getinfo(curl_handle_, CURLINFO_RESPONSE_CODE, &response.response_code);
    }
    
    return response;
}

std::string EPICSParser::buildDiscoveryURL(const std::string& pattern, size_t limit) const {
    std::ostringstream url;
    url << management_url_ << "getMatchingPVsForThisAppliance?regex=" << pattern << "&limit=" << limit;
    return url.str();
}

std::string EPICSParser::buildDataURL(const std::string& pv_name, const EPICSTimeRange& range) const {
    std::ostringstream url;
    url << retrieval_url_ << "?pv=" << pv_name 
        << "&from=" << range.from_iso 
        << "&to=" << range.to_iso;
    return url.str();
}

// === JSON PARSING METHODS ===

std::vector<std::string> EPICSParser::parseDiscoveryResponse(const std::string& json_response) const {
    std::vector<std::string> pv_names;
    
    try {
        json j = json::parse(json_response);
        
        if (j.is_array()) {
            for (const auto& item : j) {
                if (item.is_string()) {
                    pv_names.push_back(item.get<std::string>());
                }
            }
        }
        
    } catch (const json::exception& e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }
    
    return pv_names;
}

EPICSSignalData EPICSParser::parseDataResponse(const std::string& json_response, 
                                              const std::string& pv_name,
                                              const EPICSTimeRange& time_range) const {
    EPICSSignalData signal_data(pv_name, time_range);
    signal_data.info = parsePVName(pv_name);
    
    try {
        json j = json::parse(json_response);
        
        // Parse metadata
        if (j.contains("meta")) {
            const auto& meta = j["meta"];
            if (meta.contains("PREC")) {
                signal_data.info.precision = meta["PREC"].get<std::string>();
            }
        }
        
        // Parse data points
        if (j.contains("data") && j["data"].is_array()) {
            for (const auto& data_point : j["data"]) {
                uint64_t secs = data_point.value("secs", 0ULL);
                uint64_t nanos = data_point.value("nanos", 0ULL);
                double val = data_point.value("val", 0.0);
                uint16_t severity = data_point.value("severity", 0);
                uint16_t status = data_point.value("status", 0);
                
                signal_data.data_points.emplace_back(secs, nanos, val, severity, status);
            }
        }
        
        signal_data.query_status = signal_data.data_points.empty() ? "no_data" : "success";
        
    } catch (const json::exception& e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }
    
    return signal_data;
}

// === DATA PROCESSING METHODS ===

EPICSSignalInfo EPICSParser::parsePVName(const std::string& pv_name) const {
    EPICSSignalInfo info;
    info.pv_name = pv_name;
    
    // Parse EPICS naming convention: DEVICE:AREA:LOCATION:ATTRIBUTE
    std::regex pattern(R"(([A-Z]+):([A-Z0-9]+):([A-Z0-9]+):([A-Z0-9_]+))");
    std::smatch matches;
    
    if (std::regex_match(pv_name, matches, pattern)) {
        info.device = matches[1].str();
        info.area = matches[2].str();
        info.location = matches[3].str();
        info.attribute = matches[4].str();
        
        info.signal_type = inferSignalType(info.device, info.attribute);
        info.units = inferUnits(info.attribute);
        
    } else {
        // Handle non-standard naming
        info.device = "unknown";
        info.area = "unknown";
        info.location = "unknown";
        info.attribute = pv_name;
        info.signal_type = "unknown";
        info.units = "unknown";
    }
    
    return info;
}

std::string EPICSParser::inferSignalType(const std::string& device, const std::string& attribute) const {
    // Status signals
    if (attribute.find("STAT") != std::string::npos || 
        attribute.find("FLT") != std::string::npos ||
        attribute.find("HEARTBEAT") != std::string::npos) {
        return "status";
    }
    
    // Control signals
    if (attribute.find("CTRL") != std::string::npos ||
        attribute.find("DES") != std::string::npos ||
        attribute.find("SET") != std::string::npos) {
        return "control";
    }
    
    // Measurement signals
    if (attribute.find("ACT") != std::string::npos ||
        attribute.find("RBV") != std::string::npos ||
        attribute.find("RATE") != std::string::npos) {
        return "measurement";
    }
    
    // Device-specific inference
    if (device == "IOC") return "system";
    if (device == "BPMS") return "measurement";
    if (device == "KLYS") return "control";
    
    return "control"; // Default for EPICS
}

std::string EPICSParser::inferUnits(const std::string& attribute) const {
    // Rate measurements
    if (attribute.find("RATE") != std::string::npos) return "Hz";
    
    // Power measurements
    if (attribute.find("POW") != std::string::npos) return "MW";
    
    // Phase measurements
    if (attribute.find("PHAS") != std::string::npos) return "deg";
    
    // Amplitude measurements
    if (attribute.find("AMPL") != std::string::npos) return "MV/m";
    
    // Position measurements
    if (attribute == "X" || attribute == "Y") return "mm";
    
    // Status/control signals typically dimensionless
    if (attribute.find("STAT") != std::string::npos ||
        attribute.find("CTRL") != std::string::npos) return "1";
    
    return "unknown";
}

// === UTILITY METHODS ===

void EPICSParser::printDiscoveryStats(const std::vector<std::string>& pv_names) const {
    std::cout << "\n=== EPICS Discovery Statistics ===" << std::endl;
    std::cout << "Total PVs found: " << pv_names.size() << std::endl;
    
    // Count by device type
    std::map<std::string, size_t> device_counts;
    for (const auto& pv : pv_names) {
        auto info = parsePVName(pv);
        device_counts[info.device]++;
    }
    
    std::cout << "\nDevice breakdown:" << std::endl;
    for (const auto& [device, count] : device_counts) {
        double percentage = (double)count / pv_names.size() * 100.0;
        std::cout << "  " << device << ": " << count << " (" << std::fixed 
                  << std::setprecision(1) << percentage << "%)" << std::endl;
    }
}

void EPICSParser::printDataStats(const std::vector<EPICSSignalData>& signals) const {
    std::cout << "\n=== EPICS Data Statistics ===" << std::endl;
    std::cout << "Signals queried: " << signals.size() << std::endl;
    
    size_t successful = 0;
    size_t total_points = 0;
    
    for (const auto& signal : signals) {
        if (signal.query_status == "success") {
            successful++;
            total_points += signal.sample_count;
        }
    }
    
    std::cout << "Successful queries: " << successful << "/" << signals.size() << std::endl;
    std::cout << "Total data points: " << total_points << std::endl;
    
    if (successful > 0) {
        std::cout << "Average points per signal: " << (total_points / successful) << std::endl;
    }
}

// === CURL CALLBACK ===

size_t EPICSParser::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* response_data = static_cast<std::string*>(userp);
    response_data->append(static_cast<char*>(contents), realsize);
    return realsize;
}

// === ERROR HANDLING ===

void EPICSParser::handleHTTPError(const HTTPResponse& response, const std::string& context) const {
    std::cerr << "HTTP Error in " << context << ": ";
    if (response.response_code == 0) {
        std::cerr << "Connection failed - " << response.error_message << std::endl;
    } else {
        std::cerr << "HTTP " << response.response_code << std::endl;
    }
    
    if (verbose_ && !response.data.empty()) {
        std::cerr << "Response data: " << response.data.substr(0, 200) << "..." << std::endl;
    }
}

void EPICSParser::handleJSONError(const std::string& error, const std::string& context) const {
    std::cerr << "JSON Error in " << context << ": " << error << std::endl;
}

// =============================================================================
// Time Utilities Implementation
// =============================================================================

namespace EPICSTimeUtils {
    uint64_t getCurrentEpochTime() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    std::string epochToISO8601(uint64_t epoch_time) {
        std::time_t time = static_cast<std::time_t>(epoch_time);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%S.000Z");
        return oss.str();
    }
    
    uint64_t iso8601ToEpoch(const std::string& iso_time) {
        // Simplified ISO 8601 parsing
        std::tm tm = {};
        std::istringstream ss(iso_time);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        return static_cast<uint64_t>(std::mktime(&tm));
    }
    
    uint64_t getHoursAgo(uint64_t hours) {
        return getCurrentEpochTime() - (hours * 3600);
    }
    
    EPICSTimeRange createRecentTimeRange(uint64_t hours_back) {
        uint64_t now = getCurrentEpochTime();
        uint64_t start = now - (hours_back * 3600);
        return EPICSTimeRange(start, now);
    }
    
    EPICSTimeRange createTimeRange(uint64_t start_epoch, uint64_t end_epoch) {
        return EPICSTimeRange(start_epoch, end_epoch);
    }
}

// =============================================================================
// EPICS Patterns Implementation
// =============================================================================

namespace EPICSPatterns {
    const std::vector<std::pair<std::string, std::string>> DEVICE_PATTERNS = {
        {"BPMS:.*", "Beam Position Monitors"},
        {"KLYS:.*", "Klystron Systems"},
        {"ACCL:.*", "Accelerator Controls"},
        {"IOC:.*", "Input/Output Controllers"},
        {"MPLN:.*", "Multi-purpose Link Networks"},
        {"TPR:.*", "Timing Pattern Receivers"},
        {"STPR:.*", "Stepper Motors"},
        {"CAMR:.*", "Camera Systems"},
        {"VALVE:.*", "Vacuum Valves"}
    };
    
    const std::vector<std::pair<std::string, std::string>> SIGNAL_PATTERNS = {
        {".*HEARTBEAT.*", "System Heartbeat Signals"},
        {".*_STAT.*", "Status Indicators"},
        {".*_FLT.*", "Fault Signals"},
        {".*_CTRL.*", "Control Signals"},
        {".*_RATE.*", "Rate Measurements"},
        {".*INTLK.*", "Interlock Systems"}
    };
    
    std::vector<std::string> getDevicePatterns() {
        std::vector<std::string> patterns;
        for (const auto& [pattern, desc] : DEVICE_PATTERNS) {
            patterns.push_back(pattern);
        }
        return patterns;
    }
    
    bool isValidEPICSPVName(const std::string& pv_name) {
        // Basic EPICS PV name validation
        std::regex pattern(R"([A-Z][A-Z0-9]*:[A-Z0-9]+:[A-Z0-9]+:[A-Z0-9_]+)");
        return std::regex_match(pv_name, pattern);
    }
}
