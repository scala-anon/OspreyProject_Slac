#include "query_client.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <cmath>

struct Config {
    std::string server = "localhost:50052";
    std::string operation;
    std::string pv;
    std::string pvs;
    std::string pattern;
    std::string date;
    std::string time;
    std::string end_date;
    std::string end_time;
    std::string format = "table";
    bool verbose = false;
};

uint64_t parseDateTime(const std::string& date_str, const std::string& time_str) {
    if (date_str.length() != 8 || time_str.length() != 6) {
        throw std::runtime_error("Invalid date/time format. Use date=MMDDYYYY time=HHMMSS");
    }
    
    std::string mm = date_str.substr(0, 2);
    std::string dd = date_str.substr(2, 2);
    std::string yyyy = date_str.substr(4, 4);
    std::string hh = time_str.substr(0, 2);
    std::string min = time_str.substr(2, 2);
    std::string ss = time_str.substr(4, 2);
    
    std::tm tm = {};
    tm.tm_year = std::stoi(yyyy) - 1900;
    tm.tm_mon = std::stoi(mm) - 1;
    tm.tm_mday = std::stoi(dd);
    tm.tm_hour = std::stoi(hh);
    tm.tm_min = std::stoi(min);
    tm.tm_sec = std::stoi(ss);
    
    std::time_t result = std::mktime(&tm);
    if (result == -1) {
        throw std::runtime_error("Invalid date/time values");
    }
    
    return static_cast<uint64_t>(result);
}

std::string formatTimestamp(uint64_t epoch_seconds) {
    std::time_t time = static_cast<std::time_t>(epoch_seconds);
    std::tm* tm = std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::vector<std::string> splitPvs(const std::string& pvs_str) {
    std::vector<std::string> result;
    std::istringstream ss(pvs_str);
    std::string pv;
    while (std::getline(ss, pv, ',')) {
        if (!pv.empty()) {
            result.push_back(pv);
        }
    }
    return result;
}

std::vector<std::string> getPvList(QueryClient& client, const Config& config) {
    std::vector<std::string> pv_names;
    
    if (!config.pv.empty()) {
        pv_names.push_back(config.pv);
    } else if (!config.pvs.empty()) {
        pv_names = splitPvs(config.pvs);
    } else if (!config.pattern.empty()) {
        auto metadata = client.QueryPvMetadataWithPattern(config.pattern);
        for (const auto& pv_info : metadata) {
            pv_names.push_back(pv_info.pvname());
        }
    }
    
    return pv_names;
}

double extractNumericValue(const DataValue& dv) {
    switch (dv.value_case()) {
        case DataValue::kDoubleValue: return dv.doublevalue();
        case DataValue::kFloatValue: return static_cast<double>(dv.floatvalue());
        case DataValue::kUintValue: return static_cast<double>(dv.uintvalue());
        case DataValue::kUlongValue: return static_cast<double>(dv.ulongvalue());
        case DataValue::kIntValue: return static_cast<double>(dv.intvalue());
        case DataValue::kLongValue: return static_cast<double>(dv.longvalue());
        default: return std::numeric_limits<double>::quiet_NaN();
    }
}

struct Statistics {
    double mean = 0.0;
    double median = 0.0;
    double std_dev = 0.0;
    double min_val = 0.0;
    double max_val = 0.0;
    size_t count = 0;
    size_t nan_count = 0;
};

Statistics calculateStatistics(const std::vector<double>& values) {
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
    
    stats.mean = std::accumulate(valid_values.begin(), valid_values.end(), 0.0) / stats.count;
    
    auto minmax = std::minmax_element(valid_values.begin(), valid_values.end());
    stats.min_val = *minmax.first;
    stats.max_val = *minmax.second;
    
    std::vector<double> sorted = valid_values;
    std::sort(sorted.begin(), sorted.end());
    size_t mid = sorted.size() / 2;
    stats.median = (sorted.size() % 2 == 0) ? 
        (sorted[mid-1] + sorted[mid]) / 2.0 : sorted[mid];
    
    double variance = 0.0;
    for (double val : valid_values) {
        variance += (val - stats.mean) * (val - stats.mean);
    }
    stats.std_dev = std::sqrt(variance / stats.count);
    
    return stats;
}

void executeData(QueryClient& client, const Config& config) {
    auto pv_names = getPvList(client, config);
    if (pv_names.empty()) {
        std::cerr << "No PVs specified" << std::endl;
        return;
    }
    
    std::string time_str = config.time.empty() ? "000000" : config.time;
    std::string end_time_str = config.end_time.empty() ? time_str : config.end_time;
    
    uint64_t start_time = parseDateTime(config.date, time_str);
    uint64_t end_time = config.end_date.empty() ? 
        start_time + 3600 : parseDateTime(config.end_date, end_time_str);
    
    Timestamp begin_ts = client.GetCommonClient().CreateTimestamp(start_time, 0);
    Timestamp end_ts = client.GetCommonClient().CreateTimestamp(end_time, 0);
    
    // Use tabular query to get PV names with data
    auto table = client.QueryTableColumns(begin_ts, end_ts, pv_names);
    if (!table) {
        std::cerr << "No data returned" << std::endl;
        return;
    }
    
    auto column_map = client.ColumnTableToMap(*table);
    
    if (config.format == "csv") {
        std::cout << "PV,Points,First_Value,Last_Value" << std::endl;
        for (const auto& [pv, values] : column_map) {
            if (!values.empty()) {
                std::cout << pv << "," << values.size() << ","
                          << extractNumericValue(values[0]) << ","
                          << extractNumericValue(values.back()) << std::endl;
            }
        }
    } else {
        std::cout << "Data Results:" << std::endl;
        for (const auto& [pv, values] : column_map) {
            std::cout << "PV: " << pv << std::endl;
            std::cout << "  Points: " << values.size() << std::endl;
            
            if (config.verbose && !values.empty()) {
                std::cout << "  First: " << extractNumericValue(values[0]) << std::endl;
                std::cout << "  Last: " << extractNumericValue(values.back()) << std::endl;
            }
        }
    }
}

void executeStatistics(QueryClient& client, const Config& config) {
    auto pv_names = getPvList(client, config);
    if (pv_names.empty()) {
        std::cerr << "No PVs specified" << std::endl;
        return;
    }
    
    std::string time_str = config.time.empty() ? "000000" : config.time;
    std::string end_time_str = config.end_time.empty() ? time_str : config.end_time;
    
    uint64_t start_time = parseDateTime(config.date, time_str);
    uint64_t end_time = config.end_date.empty() ? 
        start_time + 3600 : parseDateTime(config.end_date, end_time_str);
    
    Timestamp begin_ts = client.GetCommonClient().CreateTimestamp(start_time, 0);
    Timestamp end_ts = client.GetCommonClient().CreateTimestamp(end_time, 0);
    
    // Use tabular query to get PV names with data
    auto table = client.QueryTableColumns(begin_ts, end_ts, pv_names);
    if (!table) {
        std::cerr << "No data returned" << std::endl;
        return;
    }
    
    auto column_map = client.ColumnTableToMap(*table);
    
    for (const auto& [pv_name, values] : column_map) {
        if (values.empty()) {
            std::cout << pv_name << ": No data" << std::endl;
            continue;
        }
        
        std::vector<double> numeric_values;
        for (const auto& val : values) {
            numeric_values.push_back(extractNumericValue(val));
        }
        
        auto stats = calculateStatistics(numeric_values);
        
        std::cout << pv_name << ":" << std::endl;
        std::cout << "  Count: " << stats.count << std::endl;
        if (stats.nan_count > 0) {
            std::cout << "  NaNs: " << stats.nan_count << std::endl;
        }
        std::cout << "  Mean: " << std::fixed << std::setprecision(6) << stats.mean << std::endl;
        std::cout << "  Median: " << stats.median << std::endl;
        std::cout << "  Std Dev: " << stats.std_dev << std::endl;
        std::cout << "  Min: " << stats.min_val << std::endl;
        std::cout << "  Max: " << stats.max_val << std::endl;
    }
}

void executeTable(QueryClient& client, const Config& config) {
    auto pv_names = getPvList(client, config);
    if (pv_names.empty()) {
        std::cerr << "No PVs specified" << std::endl;
        return;
    }
    
    std::string time_str = config.time.empty() ? "000000" : config.time;
    std::string end_time_str = config.end_time.empty() ? time_str : config.end_time;
    
    uint64_t start_time = parseDateTime(config.date, time_str);
    uint64_t end_time = config.end_date.empty() ? 
        start_time + 3600 : parseDateTime(config.end_date, end_time_str);
    
    Timestamp begin_ts = client.GetCommonClient().CreateTimestamp(start_time, 0);
    Timestamp end_ts = client.GetCommonClient().CreateTimestamp(end_time, 0);
    
    auto table = client.QueryTableColumns(begin_ts, end_ts, pv_names);
    if (!table) {
        std::cerr << "No table data returned" << std::endl;
        return;
    }
    
    auto column_map = client.ColumnTableToMap(*table);
    
    if (config.format == "csv") {
        std::cout << "PV,Count,Min,Max,Mean" << std::endl;
        for (const auto& [pv, values] : column_map) {
            std::vector<double> numeric_values;
            for (const auto& val : values) {
                numeric_values.push_back(extractNumericValue(val));
            }
            
            auto stats = calculateStatistics(numeric_values);
            std::cout << pv << "," << stats.count << "," 
                      << stats.min_val << "," << stats.max_val << "," 
                      << stats.mean << std::endl;
        }
    } else {
        std::cout << "Table Results:" << std::endl;
        for (const auto& [pv, values] : column_map) {
            std::cout << pv << ": " << values.size() << " points" << std::endl;
        }
    }
}

void executeMetadata(QueryClient& client, const Config& config) {
    auto pv_names = getPvList(client, config);
    if (pv_names.empty()) {
        std::cerr << "No PVs specified" << std::endl;
        return;
    }
    
    auto metadata = client.QueryPvMetadata(pv_names);
    
    for (const auto& pv_info : metadata) {
        std::cout << pv_info.pvname() << ":" << std::endl;
        std::cout << "  Data Type: " << pv_info.lastbucketdatatype() << std::endl;
        std::cout << "  Total Points: " << pv_info.lastbucketsamplecount() << std::endl;
        std::cout << "  Buckets: " << pv_info.numbuckets() << std::endl;
        std::cout << "  First Data: " << formatTimestamp(pv_info.firstdatatimestamp().epochseconds()) 
                  << " (" << pv_info.firstdatatimestamp().epochseconds() << ")" << std::endl;
        std::cout << "  Last Data: " << formatTimestamp(pv_info.lastdatatimestamp().epochseconds()) 
                  << " (" << pv_info.lastdatatimestamp().epochseconds() << ")" << std::endl;
    }
}

void executeList(QueryClient& client, const Config& config) {
    std::string search_pattern = config.pattern.empty() ? ".*" : config.pattern;
    auto metadata = client.QueryPvMetadataWithPattern(search_pattern);
    
    if (metadata.empty()) {
        std::cerr << "No PVs found with pattern: " << search_pattern << std::endl;
        std::cerr << "Last error: " << client.GetLastError() << std::endl;
        return;
    }
    
    for (const auto& pv_info : metadata) {
        if (config.verbose) {
            std::cout << pv_info.pvname() 
                      << " [" << pv_info.lastbucketsamplecount() << " points, "
                      << pv_info.numbuckets() << " buckets]" << std::endl;
        } else {
            std::cout << pv_info.pvname() << std::endl;
        }
    }
}

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " OPERATION [OPTIONS]\n\n"
              << "OPERATIONS:\n"
              << "  data       - Retrieve time series data\n"
              << "  statistics - Calculate statistical summaries\n"
              << "  table      - Get tabular data view\n"
              << "  metadata   - Show PV metadata information\n"
              << "  list       - List available PVs\n\n"
              << "PV SPECIFICATION (choose one):\n"
              << "  --pv=NAME              Single PV name\n"
              << "  --pvs=NAME1,NAME2,...  Comma-separated PV list\n"
              << "  --pattern=REGEX        PV name pattern\n\n"
              << "TIME SPECIFICATION:\n"
              << "  --date=MMDDYYYY        Start date (required)\n"
              << "  --time=HHMMSS          Start time (default: 000000)\n"
              << "  --end-date=MMDDYYYY    End date (default: start date)\n"
              << "  --end-time=HHMMSS      End time (default: start time + 1 hour)\n\n"
              << "OPTIONS:\n"
              << "  --server=ADDRESS       Server address (default: localhost:50052)\n"
              << "  --format=FORMAT        Output format: table, csv (default: table)\n"
              << "  --verbose              Show detailed information\n"
              << "  --help                 Show this help\n\n"
              << "EXAMPLES:\n"
              << "  " << program << " data --pv=BPMS:LI20:2445:X --date=01152024 --time=143000\n"
              << "  " << program << " statistics --pvs=PV1,PV2 --date=01152024\n"
              << "  " << program << " list --pattern=BPMS.*:X\n";
}

Config parseArgs(int argc, char* argv[]) {
    Config config;
    
    if (argc < 2) {
        printUsage(argv[0]);
        exit(1);
    }
    
    config.operation = argv[1];
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        } else if (arg.find("--server=") == 0) {
            config.server = arg.substr(9);
        } else if (arg.find("--pv=") == 0) {
            config.pv = arg.substr(5);
        } else if (arg.find("--pvs=") == 0) {
            config.pvs = arg.substr(6);
        } else if (arg.find("--pattern=") == 0) {
            config.pattern = arg.substr(10);
        } else if (arg.find("--date=") == 0) {
            config.date = arg.substr(7);
        } else if (arg.find("--time=") == 0) {
            config.time = arg.substr(7);
        } else if (arg.find("--end-date=") == 0) {
            config.end_date = arg.substr(11);
        } else if (arg.find("--end-time=") == 0) {
            config.end_time = arg.substr(11);
        } else if (arg.find("--format=") == 0) {
            config.format = arg.substr(9);
        } else if (arg == "--verbose") {
            config.verbose = true;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            exit(1);
        }
    }
    
    return config;
}

int main(int argc, char* argv[]) {
    try {
        Config config = parseArgs(argc, argv);
        QueryClient client(config.server);
        
        // Set a longer timeout for large datasets
        client.SetDefaultTimeout(300);  // 5 minutes
        
        if (config.operation == "data") {
            executeData(client, config);
        } else if (config.operation == "statistics") {
            executeStatistics(client, config);
        } else if (config.operation == "table") {
            executeTable(client, config);
        } else if (config.operation == "metadata") {
            executeMetadata(client, config);
        } else if (config.operation == "list") {
            executeList(client, config);
        } else {
            std::cerr << "Unknown operation: " << config.operation << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
