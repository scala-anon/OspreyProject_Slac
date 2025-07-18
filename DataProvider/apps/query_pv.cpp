#include "query_client.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <cmath>

struct PvStatistics {
    double mean = 0.0, median = 0.0, std_dev = 0.0, min_val = 0.0, max_val = 0.0;
    size_t total_points = 0, valid_points = 0, nan_count = 0;
    double valid_percentage = 0.0;
    
    void calculate(const std::vector<double>& values) {
        total_points = values.size();
        std::vector<double> valid_values;
        
        for (double v : values) {
            if (std::isfinite(v)) valid_values.push_back(v);
        }
        
        valid_points = valid_values.size();
        nan_count = total_points - valid_points;
        valid_percentage = total_points > 0 ? (double)valid_points / total_points * 100.0 : 0.0;
        
        if (valid_values.empty()) {
            mean = median = std_dev = min_val = max_val = std::numeric_limits<double>::quiet_NaN();
            return;
        }
        
        std::sort(valid_values.begin(), valid_values.end());
        min_val = valid_values.front();
        max_val = valid_values.back();
        median = valid_values[valid_values.size() / 2];
        
        mean = std::accumulate(valid_values.begin(), valid_values.end(), 0.0) / valid_values.size();
        
        double sq_sum = 0.0;
        for (double v : valid_values) sq_sum += (v - mean) * (v - mean);
        std_dev = std::sqrt(sq_sum / valid_values.size());
    }
};

uint64_t parseDate(const std::string& date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%m-%d-%Y");
    if (ss.fail()) throw std::runtime_error("Invalid date format. Use MM-DD-YYYY");
    return std::mktime(&tm);
}

TimeRange createTimeRange(const std::string& date_str, int hours = 24) {
    uint64_t start_epoch = parseDate(date_str);
    return TimeRange(start_epoch, 0, start_epoch + (hours * 3600), 0);
}

std::vector<double> extractDataFromBucket(const dp::service::query::QueryDataResponse::QueryData::DataBucket& bucket) {
    std::vector<double> values;
    
    if (bucket.has_datacolumn()) {
        const auto& data_column = bucket.datacolumn();
        for (const auto& data_value : data_column.datavalues()) {
            switch (data_value.value_case()) {
                case DataValue::kDoubleValue:
                    values.push_back(data_value.doublevalue());
                    break;
                case DataValue::kFloatValue:
                    values.push_back(static_cast<double>(data_value.floatvalue()));
                    break;
                case DataValue::kIntValue:
                    values.push_back(static_cast<double>(data_value.intvalue()));
                    break;
                case DataValue::kLongValue:
                    values.push_back(static_cast<double>(data_value.longvalue()));
                    break;
                default:
                    values.push_back(std::numeric_limits<double>::quiet_NaN());
                    break;
            }
        }
    }
    return values;
}

std::vector<double> extractAllDataValues(const QueryDataResponse& response) {
    std::vector<double> all_values;
    
    if (response.has_querydata()) {
        for (const auto& bucket : response.querydata().databuckets()) {
            auto bucket_values = extractDataFromBucket(bucket);
            all_values.insert(all_values.end(), bucket_values.begin(), bucket_values.end());
        }
    }
    return all_values;
}

void printStatistics(const std::string& pv_name, const PvStatistics& stats) {
    std::cout << "PV: " << pv_name << std::endl;
    std::cout << "  Data Points: " << stats.total_points << std::endl;
    std::cout << "  Valid: " << stats.valid_points << " (" << std::fixed << std::setprecision(1) 
              << stats.valid_percentage << "%)" << std::endl;
    std::cout << "  NaN Count: " << stats.nan_count << std::endl;
    
    if (stats.valid_points > 0) {
        std::cout << "  Mean: " << std::setprecision(3) << stats.mean << std::endl;
        std::cout << "  Median: " << stats.median << std::endl;
        std::cout << "  Std Dev: " << stats.std_dev << std::endl;
        std::cout << "  Min: " << stats.min_val << std::endl;
        std::cout << "  Max: " << stats.max_val << std::endl;
    } else {
        std::cout << "  No valid data" << std::endl;
    }
    std::cout << std::endl;
}

void printValues(const std::string& pv_name, const std::vector<double>& values, size_t max_show = 10) {
    std::cout << "PV: " << pv_name << " (" << values.size() << " points)" << std::endl;
    size_t show_count = std::min(values.size(), max_show);
    for (size_t i = 0; i < show_count; ++i) {
        std::cout << "  [" << i << "] " << std::setprecision(6) << values[i] << std::endl;
    }
    if (values.size() > max_show) {
        std::cout << "  ... (" << (values.size() - max_show) << " more)" << std::endl;
    }
    std::cout << std::endl;
}

std::vector<std::string> findMatchingPvs(QueryClient& client, const std::string& pattern) {
    try {
        auto request = makeQueryPvMetadataRequestWithPattern(pattern);
        auto response = client.queryPvMetadata(request);
        
        std::vector<std::string> pv_names;
        if (response.has_metadataresult()) {
            for (const auto& pv_info : response.metadataresult().pvinfos()) {
                pv_names.push_back(pv_info.pvname());
            }
        }
        return pv_names;
    } catch (const std::exception& e) {
        std::cerr << "Error finding PVs: " << e.what() << std::endl;
        return {};
    }
}

void printUsage(const std::string& program_name) {
    std::cout << "PV Query Tool\n";
    std::cout << "USAGE: " << program_name << " [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  --pv=NAME                     Query single PV\n";
    std::cout << "  --pvs=NAME1,NAME2,...         Query multiple PVs (comma-separated)\n";
    std::cout << "  --pattern=REGEX               Query PVs matching regex pattern\n";
    std::cout << "  --date=MM-DD-YYYY             Date to query (default: today)\n";
    std::cout << "  --hours=N                     Hours from date (default: 24)\n";
    std::cout << "  --show-values[=N]             Print data values (max N, default 10)\n";
    std::cout << "  --stats-only                  Show only statistics\n";
    std::cout << "  --server=ADDRESS              Server address (default: localhost:50052)\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  " << program_name << " --pv=KLYS_LI20_61_AMPL --date=07-16-2025\n";
    std::cout << "  " << program_name << " --pattern=\"BPMS.*\" --date=07-16-2025 --hours=1\n";
    std::cout << "  " << program_name << " --pvs=KLYS_LI20_61_AMPL,BPMS_DMPH_502_TMITBR --show-values=5\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string single_pv, pattern, date_str, server_address = "localhost:50052";
    std::vector<std::string> multiple_pvs;
    int hours = 24;
    bool show_values = false, stats_only = false;
    size_t max_values = 10;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg.find("--pv=") == 0) {
            single_pv = arg.substr(5);
        } else if (arg.find("--pvs=") == 0) {
            std::stringstream ss(arg.substr(6));
            std::string pv;
            while (std::getline(ss, pv, ',')) multiple_pvs.push_back(pv);
        } else if (arg.find("--pattern=") == 0) {
            pattern = arg.substr(10);
        } else if (arg.find("--date=") == 0) {
            date_str = arg.substr(7);
        } else if (arg.find("--hours=") == 0) {
            hours = std::stoi(arg.substr(8));
        } else if (arg == "--show-values") {
            show_values = true;
        } else if (arg.find("--show-values=") == 0) {
            show_values = true;
            max_values = std::stoul(arg.substr(14));
        } else if (arg == "--stats-only") {
            stats_only = true;
        } else if (arg.find("--server=") == 0) {
            server_address = arg.substr(9);
        }
    }
    
    // Default to today if no date specified
    if (date_str.empty()) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%m-%d-%Y");
        date_str = oss.str();
    }
    
    try {
        QueryClient client(server_address);
        TimeRange time_range = createTimeRange(date_str, hours);
        
        std::vector<std::string> pv_names;
        
        // Determine which PVs to query
        if (!single_pv.empty()) {
            pv_names.push_back(single_pv);
        } else if (!multiple_pvs.empty()) {
            pv_names = multiple_pvs;
        } else if (!pattern.empty()) {
            pv_names = findMatchingPvs(client, pattern);
            if (pv_names.empty()) {
                std::cerr << "No PVs found matching pattern: " << pattern << std::endl;
                return 1;
            }
            std::cout << "Found " << pv_names.size() << " PVs matching pattern\n\n";
        } else {
            std::cerr << "Must specify --pv, --pvs, or --pattern" << std::endl;
            return 1;
        }
        
        // Query each PV using proto-based MongoDB retrieval
        for (const auto& pv_name : pv_names) {
            try {
                // Create QueryDataRequest using proto makeQueryDataRequest (NOT serialized by default)
                auto request = makeQueryDataRequest({pv_name}, 
                                                  time_range.getStartTimestamp(), 
                                                  time_range.getEndTimestamp(), 
                                                  false); // useSerializedDataColumns = false
                
                // Use streaming query for large datasets
                auto responses = client.queryDataStream(request);
                
                if (responses.empty()) {
                    std::cout << "PV: " << pv_name << " - No data found" << std::endl;
                    continue;
                }
                
                // Check for errors in first response
                if (responses[0].has_exceptionalresult()) {
                    std::cerr << "Error querying " << pv_name << ": " 
                              << responses[0].exceptionalresult().message() << std::endl;
                    continue;
                }
                
                // Extract data from all streaming responses
                std::vector<double> values;
                for (const auto& response : responses) {
                    if (response.has_querydata()) {
                        auto response_values = extractAllDataValues(response);
                        values.insert(values.end(), response_values.begin(), response_values.end());
                    }
                }
                
                if (values.empty()) {
                    std::cout << "PV: " << pv_name << " - No data found" << std::endl;
                    continue;
                }
                
                // Calculate statistics from extracted data
                PvStatistics stats;
                stats.calculate(values);
                
                // Output based on options
                if (!stats_only) {
                    printStatistics(pv_name, stats);
                }
                
                if (show_values && !stats_only) {
                    printValues(pv_name, values, max_values);
                }
                
                if (stats_only) {
                    std::cout << pv_name << ": " << stats.valid_points << "/" << stats.total_points 
                              << " (" << std::setprecision(1) << stats.valid_percentage 
                              << "% valid), mean=" << std::setprecision(3) << stats.mean << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error querying " << pv_name << ": " << e.what() << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
