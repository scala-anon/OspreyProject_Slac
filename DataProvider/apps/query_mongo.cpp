#include "query_client.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <map>
#include <set>

// Convert DDMMYYYY to epoch seconds
uint64_t parseDateDDMMYYYY(const std::string& date_str) {
    if (date_str.length() != 8) return 0;
    
    std::string dd = date_str.substr(0, 2);
    std::string mm = date_str.substr(2, 2);
    std::string yyyy = date_str.substr(4, 4);
    
    std::string converted = mm + "-" + dd + "-" + yyyy;
    
    std::tm tm = {};
    std::istringstream ss(converted);
    ss >> std::get_time(&tm, "%m-%d-%Y");
    if (ss.fail()) return 0;
    
    return std::mktime(&tm);
}

// Extract double value from DataValue
double getDoubleFromDataValue(const DataValue& dv) {
    switch (dv.value_case()) {
        case DataValue::kDoubleValue:
            return dv.doublevalue();
        case DataValue::kFloatValue:
            return static_cast<double>(dv.floatvalue());
        case DataValue::kUintValue:
            return static_cast<double>(dv.uintvalue());
        case DataValue::kUlongValue:
            return static_cast<double>(dv.ulongvalue());
        case DataValue::kIntValue:
            return static_cast<double>(dv.intvalue());
        case DataValue::kLongValue:
            return static_cast<double>(dv.longvalue());
        default:
            return std::numeric_limits<double>::quiet_NaN();
    }
}

// Calculate statistics
struct Statistics {
    double mean = 0.0, median = 0.0, std_dev = 0.0, min_val = 0.0, max_val = 0.0;
    size_t count = 0, nan_count = 0;
};

Statistics calculateStats(const std::vector<double>& values) {
    Statistics stats;
    std::vector<double> valid_values;
    
    for (double val : values) {
        if (std::isnan(val)) {
            stats.nan_count++;
        } else {
            valid_values.push_back(val);
        }
    }
    
    stats.count = valid_values.size();
    if (stats.count == 0) return stats;
    
    // Mean
    stats.mean = std::accumulate(valid_values.begin(), valid_values.end(), 0.0) / stats.count;
    
    // Min/Max
    auto minmax = std::minmax_element(valid_values.begin(), valid_values.end());
    stats.min_val = *minmax.first;
    stats.max_val = *minmax.second;
    
    // Median
    std::vector<double> sorted_values = valid_values;
    std::sort(sorted_values.begin(), sorted_values.end());
    size_t mid = sorted_values.size() / 2;
    stats.median = (sorted_values.size() % 2 == 0) ? 
        (sorted_values[mid-1] + sorted_values[mid]) / 2.0 : sorted_values[mid];
    
    // Standard deviation
    double variance = 0.0;
    for (double val : valid_values) {
        variance += (val - stats.mean) * (val - stats.mean);
    }
    stats.std_dev = std::sqrt(variance / stats.count);
    
    return stats;
}

// Get PV names from various sources
std::vector<std::string> getPvNames(const std::string& pv, const std::string& pvs, const std::string& pattern, QueryClient& client) {
    std::vector<std::string> pv_names;
    
    if (!pv.empty()) {
        pv_names.push_back(pv);
    } else if (!pvs.empty()) {
        std::istringstream ss(pvs);
        std::string name;
        while (std::getline(ss, name, ',')) {
            if (!name.empty()) pv_names.push_back(name);
        }
    } else if (!pattern.empty()) {
        auto request = makeQueryPvMetadataRequestWithPattern(pattern);
        auto response = client.queryPvMetadata(request);
        if (response.has_metadataresult()) {
            for (const auto& pv_info : response.metadataresult().pvinfos()) {
                pv_names.push_back(pv_info.pvname());
            }
        }
    }
    
    return pv_names;
}

// Function implementations
void listPvs(QueryClient& client) {
    auto request = makeQueryPvMetadataRequestWithPattern(".*");
    auto response = client.queryPvMetadata(request);
    
    if (response.has_metadataresult()) {
        for (const auto& pv_info : response.metadataresult().pvinfos()) {
            std::cout << pv_info.pvname() << std::endl;
        }
    }
}

void listPvsTimerange(QueryClient& client, const std::string& start_date, const std::string& end_date) {
    uint64_t start_epoch = parseDateDDMMYYYY(start_date);
    uint64_t end_epoch = parseDateDDMMYYYY(end_date);
    
    auto request = makeQueryPvMetadataRequestWithPattern(".*");
    auto response = client.queryPvMetadata(request);
    
    if (response.has_metadataresult()) {
        for (const auto& pv_info : response.metadataresult().pvinfos()) {
            uint64_t first_data = pv_info.firstdatatimestamp().epochseconds();
            uint64_t last_data = pv_info.lastdatatimestamp().epochseconds();
            
            if (first_data <= end_epoch && last_data >= start_epoch) {
                std::cout << pv_info.pvname() << std::endl;
            }
        }
    }
}

void patternSearch(QueryClient& client, const std::string& pattern) {
    auto request = makeQueryPvMetadataRequestWithPattern(pattern);
    auto response = client.queryPvMetadata(request);
    
    if (response.has_metadataresult()) {
        for (const auto& pv_info : response.metadataresult().pvinfos()) {
            std::cout << pv_info.pvname() << std::endl;
        }
    }
}

void listNans(QueryClient& client, const std::string& start_date, const std::string& end_date) {
    auto request = makeQueryPvMetadataRequestWithPattern(".*");
    auto response = client.queryPvMetadata(request);
    
    if (!response.has_metadataresult()) return;
    
    uint64_t start_epoch = start_date.empty() ? 0 : parseDateDDMMYYYY(start_date);
    uint64_t end_epoch = end_date.empty() ? UINT64_MAX : parseDateDDMMYYYY(end_date);
    
    for (const auto& pv_info : response.metadataresult().pvinfos()) {
        std::vector<std::string> pv_names = {pv_info.pvname()};
        auto data_request = makeQueryDataRequest(pv_names, 
            makeTimestamp(start_epoch, 0), makeTimestamp(end_epoch, 0), false);
        
        auto data_responses = client.queryDataStream(data_request);
        bool has_nans = false;
        
        for (const auto& response : data_responses) {
            if (response.has_querydata()) {
                for (const auto& bucket : response.querydata().databuckets()) {
                    if (bucket.has_datacolumn()) {
                        for (const auto& data_value : bucket.datacolumn().datavalues()) {
                            double val = getDoubleFromDataValue(data_value);
                            if (std::isnan(val)) {
                                has_nans = true;
                                break;
                            }
                        }
                    }
                    if (has_nans) break;
                }
            }
            if (has_nans) break;
        }
        
        if (has_nans) {
            std::cout << pv_info.pvname() << std::endl;
        }
    }
}

void countTimestamps(QueryClient& client, const std::string& pv, const std::string& pvs, 
                    const std::string& pattern, const std::string& start_date, const std::string& end_date) {
    auto pv_names = getPvNames(pv, pvs, pattern, client);
    
    uint64_t start_epoch, end_epoch;
    
    if (start_date.empty() && end_date.empty()) {
        auto now = std::time(nullptr);
        end_epoch = static_cast<uint64_t>(now);
        start_epoch = end_epoch - (30 * 24 * 3600);
    } else {
        start_epoch = start_date.empty() ? 0 : parseDateDDMMYYYY(start_date);
        end_epoch = end_date.empty() ? static_cast<uint64_t>(std::time(nullptr)) : parseDateDDMMYYYY(end_date);
    }
    
    size_t total_timestamps = 0;
    
    for (const auto& pv_name : pv_names) {
        auto request = makeQueryDataRequest({pv_name}, 
            makeTimestamp(start_epoch, 0), makeTimestamp(end_epoch, 0), false);
        auto responses = client.queryDataStream(request);
        
        size_t pv_timestamps = 0;
        for (const auto& response : responses) {
            if (response.has_querydata()) {
                for (const auto& bucket : response.querydata().databuckets()) {
                    if (bucket.has_datacolumn()) {
                        pv_timestamps += bucket.datacolumn().datavalues_size();
                    } else if (bucket.has_serializeddatacolumn()) {
                        if (bucket.has_datatimestamps()) {
                            const auto& timestamps = bucket.datatimestamps();
                            if (timestamps.has_samplingclock()) {
                                pv_timestamps += timestamps.samplingclock().count();
                            } else if (timestamps.has_timestamplist()) {
                                pv_timestamps += timestamps.timestamplist().timestamps_size();
                            }
                        }
                    }
                }
            }
        }
        
        std::cout << pv_name << ": " << pv_timestamps << std::endl;
        total_timestamps += pv_timestamps;
    }
    
    if (pv_names.size() > 1) {
        std::cout << "Total: " << total_timestamps << std::endl;
    }
}

void statistics(QueryClient& client, const std::string& pv, const std::string& pvs, 
               const std::string& pattern, const std::string& start_date, const std::string& end_date) {
    auto pv_names = getPvNames(pv, pvs, pattern, client);
    
    uint64_t start_epoch, end_epoch;
    
    if (start_date.empty() && end_date.empty()) {
        auto now = std::time(nullptr);
        end_epoch = static_cast<uint64_t>(now);
        start_epoch = end_epoch - (30 * 24 * 3600);
    } else {
        start_epoch = start_date.empty() ? 0 : parseDateDDMMYYYY(start_date);
        end_epoch = end_date.empty() ? static_cast<uint64_t>(std::time(nullptr)) : parseDateDDMMYYYY(end_date);
    }
    
    for (const auto& pv_name : pv_names) {
        auto request = makeQueryDataRequest({pv_name}, 
            makeTimestamp(start_epoch, 0), makeTimestamp(end_epoch, 0), false);
        auto responses = client.queryDataStream(request);
        
        std::vector<double> all_values;
        for (const auto& response : responses) {
            if (response.has_querydata()) {
                for (const auto& bucket : response.querydata().databuckets()) {
                    if (bucket.has_datacolumn()) {
                        for (const auto& data_value : bucket.datacolumn().datavalues()) {
                            all_values.push_back(getDoubleFromDataValue(data_value));
                        }
                    }
                }
            }
        }
        
        if (!all_values.empty()) {
            auto stats = calculateStats(all_values);
            std::cout << pv_name << ":\n"
                      << "  Count: " << stats.count << "\n"
                      << "  NaNs: " << stats.nan_count << "\n"
                      << "  Mean: " << stats.mean << "\n"
                      << "  Median: " << stats.median << "\n"
                      << "  Std Dev: " << stats.std_dev << "\n"
                      << "  Min: " << stats.min_val << "\n"
                      << "  Max: " << stats.max_val << std::endl;
        } else {
            std::cout << pv_name << ": No data" << std::endl;
        }
    }
}

void pvInfo(QueryClient& client, const std::string& pv, const std::string& pvs, const std::string& pattern) {
    auto pv_names = getPvNames(pv, pvs, pattern, client);
    
    auto request = makeQueryPvMetadataRequest(pv_names);
    auto response = client.queryPvMetadata(request);
    
    if (response.has_metadataresult()) {
        for (const auto& pv_info : response.metadataresult().pvinfos()) {
            std::cout << pv_info.pvname() << ":\n"
                      << "  Data Type: " << pv_info.lastbucketdatatype() << "\n"
                      << "  Total Timestamps: " << pv_info.lastbucketsamplecount() << "\n"
                      << "  Buckets: " << pv_info.numbuckets() << "\n"
                      << "  First Data: " << pv_info.firstdatatimestamp().epochseconds() << "\n"
                      << "  Last Data: " << pv_info.lastdatatimestamp().epochseconds() << std::endl;
            
            auto data_request = makeQueryDataRequest({pv_info.pvname()}, 
                pv_info.lastdatatimestamp(), pv_info.lastdatatimestamp(), false);
            auto data_responses = client.queryDataStream(data_request);
            
            for (const auto& data_response : data_responses) {
                if (data_response.has_querydata()) {
                    for (const auto& bucket : data_response.querydata().databuckets()) {
                        if (bucket.attributes_size() > 0) {
                            std::cout << "  Attributes:\n";
                            for (const auto& attr : bucket.attributes()) {
                                std::cout << "    " << attr.name() << ": " << attr.value() << "\n";
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}

void timeCoverage(QueryClient& client, const std::string& pv, const std::string& pvs, const std::string& pattern) {
    auto pv_names = getPvNames(pv, pvs, pattern, client);
    
    auto request = makeQueryPvMetadataRequest(pv_names);
    auto response = client.queryPvMetadata(request);
    
    if (response.has_metadataresult()) {
        for (const auto& pv_info : response.metadataresult().pvinfos()) {
            uint64_t first_time = pv_info.firstdatatimestamp().epochseconds();
            uint64_t last_time = pv_info.lastdatatimestamp().epochseconds();
            uint64_t total_duration = last_time - first_time;
            
            std::cout << pv_info.pvname() << ":\n"
                      << "  First: " << first_time << "\n"
                      << "  Last: " << last_time << "\n"
                      << "  Duration: " << total_duration << " seconds\n"
                      << "  Buckets: " << pv_info.numbuckets() << std::endl;
        }
    }
}

void countNans(QueryClient& client, const std::string& pv, const std::string& pvs, 
              const std::string& pattern, const std::string& start_date, const std::string& end_date) {
    auto pv_names = getPvNames(pv, pvs, pattern, client);
    
    uint64_t start_epoch, end_epoch;
    
    if (start_date.empty() && end_date.empty()) {
        auto now = std::time(nullptr);
        end_epoch = static_cast<uint64_t>(now);
        start_epoch = end_epoch - (30 * 24 * 3600);
    } else {
        start_epoch = start_date.empty() ? 0 : parseDateDDMMYYYY(start_date);
        end_epoch = end_date.empty() ? static_cast<uint64_t>(std::time(nullptr)) : parseDateDDMMYYYY(end_date);
    }
    
    size_t total_nans = 0;
    
    for (const auto& pv_name : pv_names) {
        auto request = makeQueryDataRequest({pv_name}, 
            makeTimestamp(start_epoch, 0), makeTimestamp(end_epoch, 0), false);
        auto responses = client.queryDataStream(request);
        
        size_t pv_nans = 0;
        for (const auto& response : responses) {
            if (response.has_querydata()) {
                for (const auto& bucket : response.querydata().databuckets()) {
                    if (bucket.has_datacolumn()) {
                        for (const auto& data_value : bucket.datacolumn().datavalues()) {
                            double val = getDoubleFromDataValue(data_value);
                            if (std::isnan(val)) pv_nans++;
                        }
                    }
                }
            }
        }
        
        std::cout << pv_name << ": " << pv_nans << std::endl;
        total_nans += pv_nans;
    }
    
    if (pv_names.size() > 1) {
        std::cout << "Total: " << total_nans << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    
    std::string operation = argv[1];
    std::string server = "localhost:50052";
    std::string pv, pvs, pattern, start_date, end_date;
    
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.find("--server=") == 0) server = arg.substr(9);
        else if (arg.find("--pv=") == 0) pv = arg.substr(5);
        else if (arg.find("--pvs=") == 0) pvs = arg.substr(6);
        else if (arg.find("--pattern=") == 0) pattern = arg.substr(10);
        else if (arg.find("--start=") == 0) start_date = arg.substr(8);
        else if (arg.find("--end=") == 0) end_date = arg.substr(6);
    }
    
    try {
        QueryClient client(server);
        
        if (operation == "list-pvs") {
            listPvs(client);
        } else if (operation == "list-pvs-timerange") {
            listPvsTimerange(client, start_date, end_date);
        } else if (operation == "pattern-search") {
            patternSearch(client, pattern);
        } else if (operation == "list-nans") {
            listNans(client, start_date, end_date);
        } else if (operation == "count-timestamps") {
            countTimestamps(client, pv, pvs, pattern, start_date, end_date);
        } else if (operation == "statistics") {
            statistics(client, pv, pvs, pattern, start_date, end_date);
        } else if (operation == "pv-info") {
            pvInfo(client, pv, pvs, pattern);
        } else if (operation == "time-coverage") {
            timeCoverage(client, pv, pvs, pattern);
        } else if (operation == "count-nans") {
            countNans(client, pv, pvs, pattern, start_date, end_date);
        }
        
        return 0;
    } catch (const std::exception& e) {
        return 1;
    }
}