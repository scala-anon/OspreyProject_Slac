#include "archiver_client.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <regex>

ArchiverClientConfig ArchiverClientConfig::fromConfigFile(const std::string& config_path) {
    ArchiverClientConfig config;
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            return config;
        }
        
        json j;
        file >> j;
        
        if (j.contains("archiver")) {
            const auto& archiver = j["archiver"];
            if (archiver.contains("base_url")) {
                config.base_url = archiver["base_url"];
            }
            if (archiver.contains("timeout_seconds")) {
                config.timeout_seconds = archiver["timeout_seconds"];
            }
            if (archiver.contains("fetch_metadata")) {
                config.fetch_metadata = archiver["fetch_metadata"];
            }
        }
    } catch (const std::exception&) {
        // Silent fallback to defaults
    }
    return config;
}

ArchiverClient::ArchiverClient(const ArchiverClientConfig& config) 
    : config_(config), curl_handle_(nullptr) {
    initializeCurl();
}

ArchiverClient::~ArchiverClient() {
    cleanup();
}

void ArchiverClient::initializeCurl() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle_ = curl_easy_init();
    
    if (!curl_handle_) {
        last_error_ = "Failed to initialize CURL";
        return;
    }
    
    curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT, config_.timeout_seconds);
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_handle_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
}

void ArchiverClient::cleanup() {
    if (curl_handle_) {
        curl_easy_cleanup(curl_handle_);
        curl_handle_ = nullptr;
    }
    curl_global_cleanup();
}

ArchiverResponse ArchiverClient::queryPv(const std::string& pv_name,
                                         const std::string& start_time,
                                         const std::string& end_time) {
    ArchiverResponse response;
    
    if (!curl_handle_) {
        response.error_message = "CURL not initialized";
        return response;
    }
    
    try {
        std::string url = buildUrl(pv_name, start_time, end_time);
        std::string json_data = performRequest(url);
        
        if (json_data.empty()) {
            response.error_message = "Empty response from archiver";
            return response;
        }
        
        response = parseJsonResponse(json_data);
        
    } catch (const std::exception& e) {
        response.error_message = "Error querying PV: " + std::string(e.what());
    }
    
    return response;
}

std::vector<ArchiverResponse> ArchiverClient::queryPvs(const std::vector<std::string>& pv_names,
                                                      const std::string& start_time,
                                                      const std::string& end_time) {
    std::vector<ArchiverResponse> responses;
    responses.reserve(pv_names.size());
    
    for (const auto& pv_name : pv_names) {
        auto response = queryPv(pv_name, start_time, end_time);
        responses.push_back(std::move(response));
    }
    
    return responses;
}

ArchiverResponse ArchiverClient::queryPvByDate(const std::string& pv_name,
                                              const std::string& date_str,
                                              int hours) {
    try {
        std::string start_time = dateToIsoString(date_str, 0);
        std::string end_time = dateToIsoString(date_str, hours);
        return queryPv(pv_name, start_time, end_time);
    } catch (const std::exception& e) {
        ArchiverResponse response;
        response.error_message = "Date parsing error: " + std::string(e.what());
        return response;
    }
}

std::string ArchiverClient::buildUrl(const std::string& pv_name,
                                    const std::string& start_time,
                                    const std::string& end_time) const {
    std::string encoded_pv = urlEncode(pv_name);
    std::string encoded_start = urlEncode(start_time);
    std::string encoded_end = urlEncode(end_time);
    
    std::ostringstream url;
    url << config_.base_url
        << "?pv=" << encoded_pv
        << "&from=" << encoded_start
        << "&to=" << encoded_end;
    
    if (config_.fetch_metadata) {
        url << "&fetchLatestMetadata=true";
    }
    
    return url.str();
}

std::string ArchiverClient::performRequest(const std::string& url) {
    std::string response_data;
    
    curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &response_data);
    
    CURLcode result = curl_easy_perform(curl_handle_);
    
    if (result != CURLE_OK) {
        last_error_ = "CURL request failed: " + std::string(curl_easy_strerror(result));
        return "";
    }
    
    long response_code;
    curl_easy_getinfo(curl_handle_, CURLINFO_RESPONSE_CODE, &response_code);
    
    if (response_code != 200) {
        last_error_ = "HTTP error: " + std::to_string(response_code);
        return "";
    }
    
    return response_data;
}

ArchiverResponse ArchiverClient::parseJsonResponse(const std::string& json_data) const {
    ArchiverResponse response;
    
    try {
        json parsed = json::parse(json_data);
        
        if (!parsed.is_array() || parsed.empty()) {
            response.error_message = "Invalid JSON format: expected non-empty array";
            return response;
        }
        
        const auto& pv_data = parsed[0];
        
        if (pv_data.contains("meta")) {
            response.metadata = parseMetadata(pv_data["meta"]);
        }
        
        if (pv_data.contains("data")) {
            response.data_points = parseDataPoints(pv_data["data"]);
        }
        
        response.success = true;
        
    } catch (const std::exception& e) {
        response.error_message = "JSON parsing error: " + std::string(e.what());
    }
    
    return response;
}

EpicsMetadata ArchiverClient::parseMetadata(const json& meta_json) const {
    EpicsMetadata metadata;
    
    for (auto& [key, value] : meta_json.items()) {
        if (key == "name") {
            metadata.name = value.get<std::string>();
        } else if (key.find("ENUM_") == 0) {
            std::string enum_key = key.substr(5);
            metadata.enums[enum_key] = value.get<std::string>();
        } else {
            metadata.properties[key] = value.get<std::string>();
        }
    }
    
    if (metadata.properties.find("DESC") != metadata.properties.end()) {
        metadata.description = metadata.properties["DESC"];
    }
    
    return metadata;
}

std::vector<EpicsDataPoint> ArchiverClient::parseDataPoints(const json& data_json) const {
    std::vector<EpicsDataPoint> data_points;
    
    if (!data_json.is_array()) {
        return data_points;
    }
    
    data_points.reserve(data_json.size());
    
    for (const auto& point : data_json) {
        EpicsDataPoint dp;
        
        if (point.contains("secs")) dp.secs = point["secs"].get<uint64_t>();
        if (point.contains("nanos")) dp.nanos = point["nanos"].get<uint64_t>();
        if (point.contains("val")) {
            if (point["val"].is_number()) {
                dp.value = point["val"].get<double>();
            } else {
                dp.value = std::numeric_limits<double>::quiet_NaN();
            }
        }
        if (point.contains("severity")) dp.severity = point["severity"].get<int>();
        if (point.contains("status")) dp.status = point["status"].get<int>();
        
        if (point.contains("fields")) {
            for (auto& [key, value] : point["fields"].items()) {
                dp.fields[key] = value.get<std::string>();
            }
        }
        
        data_points.push_back(std::move(dp));
    }
    
    return data_points;
}

std::string ArchiverClient::urlEncode(const std::string& input) const {
    if (!curl_handle_) return input;
    
    char* encoded = curl_easy_escape(curl_handle_, input.c_str(), input.length());
    std::string result(encoded);
    curl_free(encoded);
    return result;
}

size_t ArchiverClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string ArchiverClient::formatTimeString(uint64_t epoch_sec, uint64_t nano_sec) {
    return ArchiverUtils::epochToIsoTime(epoch_sec, nano_sec);
}

std::string ArchiverClient::dateToIsoString(const std::string& date_str, int hour_offset) {
    uint64_t epoch = parseDate(date_str);
    epoch += hour_offset * 3600;
    return ArchiverUtils::epochToIsoTime(epoch);
}

uint64_t ArchiverClient::parseDate(const std::string& date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%m-%d-%Y");
    if (ss.fail()) {
        throw std::runtime_error("Invalid date format. Use MM-DD-YYYY");
    }
    return std::mktime(&tm);
}

namespace ArchiverUtils {
    
std::string getCurrentIsoTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::gmtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S.000Z");
    return oss.str();
}

std::string epochToIsoTime(uint64_t epoch_sec, uint64_t nano_sec) {
    auto time_t = static_cast<std::time_t>(epoch_sec);
    auto tm = *std::gmtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    
    if (nano_sec > 0) {
        oss << "." << std::setfill('0') << std::setw(3) << (nano_sec / 1000000);
    } else {
        oss << ".000";
    }
    
    oss << "Z";
    return oss.str();
}

uint64_t isoTimeToEpoch(const std::string& iso_time) {
    std::tm tm = {};
    std::istringstream ss(iso_time);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return std::mktime(&tm);
}

std::string createTimeRange(const std::string& date_str, int hours) {
    uint64_t start_epoch = ArchiverClient::parseDate(date_str);
    std::string start_time = epochToIsoTime(start_epoch);
    std::string end_time = epochToIsoTime(start_epoch + (hours * 3600));
    return start_time + " to " + end_time;
}

std::string severityToString(int severity) {
    switch (severity) {
        case 0: return "NO_ALARM";
        case 1: return "MINOR";
        case 2: return "MAJOR";
        case 3: return "INVALID";
        default: return "UNKNOWN";
    }
}

std::string statusToString(int status) {
    switch (status) {
        case 0: return "NO_ALARM";
        case 1: return "READ";
        case 2: return "WRITE";
        case 3: return "HIHI";
        case 4: return "HIGH";
        case 5: return "LOLO";
        case 6: return "LOW";
        case 7: return "STATE";
        case 8: return "COS";
        case 9: return "COMM";
        case 10: return "TIMEOUT";
        case 11: return "HWLIMIT";
        case 12: return "CALC";
        case 13: return "SCAN";
        case 14: return "LINK";
        case 15: return "SOFT";
        case 16: return "BAD_SUB";
        case 17: return "UDF";
        case 18: return "DISABLE";
        case 19: return "SIMM";
        case 20: return "READ_ACCESS";
        case 21: return "WRITE_ACCESS";
        default: return "UNKNOWN";
    }
}

bool isValidEpicsValue(double value, int severity, int status) {
    return std::isfinite(value) && severity < 3;
}

}