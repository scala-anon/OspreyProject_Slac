#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "parsers/data_decoder.hpp"

void printUsage(const std::string& program_name) {
    std::cout << "LCLS Data Decoder - Convert MongoDB bytes to readable format\n";
    std::cout << "Usage: " << program_name << " [OPTIONS] COMMAND [ARGS...]\n\n";
    
    std::cout << "OPTIONS:\n";
    std::cout << "  -s, --server HOST:PORT     Data platform server address (default: localhost:50052)\n";
    std::cout << "  -h, --help                 Show this help message\n\n";
    
    std::cout << "COMMANDS:\n";
    std::cout << "  discover [PATTERN]         List available PVs matching pattern (default: .*)\n";
    std::cout << "  decode PV1,PV2,...         Decode specific PVs\n";
    std::cout << "  pattern REGEX              Decode PVs matching regex pattern\n";
    std::cout << "  test                       Test connection to data platform\n";
    std::cout << "  raw PV1,PV2,...            Show raw query response (debug mode)\n\n";
    
    std::cout << "TIME RANGE OPTIONS (for decode/pattern/raw commands):\n";
    std::cout << "  --start TIME               Start time (format: 'YYYY-MM-DD HH:MM:SS' or Unix timestamp)\n";
    std::cout << "  --end TIME                 End time (format: 'YYYY-MM-DD HH:MM:SS' or Unix timestamp)\n";
    std::cout << "  --last DURATION            Last N time (e.g., '1h', '30m', '2d')\n";
    std::cout << "  --range HOURS              Query last N hours (default: 1)\n\n";
    
    std::cout << "OUTPUT OPTIONS:\n";
    std::cout << "  --csv FILE                 Export to CSV file\n";
    std::cout << "  --json FILE                Export to JSON file\n";
    std::cout << "  --max-points N             Max data points to display (default: 10)\n";
    std::cout << "  --stats-only               Show only statistics, no raw data\n";
    std::cout << "  --no-serialized            Don't use serialized data columns\n";
    std::cout << "  --quiet                    Minimal output\n\n";
    
    std::cout << "EXAMPLES:\n";
    std::cout << "  # Discover all BPM PVs\n";
    std::cout << "  " << program_name << " discover \".*BPM.*\"\n\n";
    
    std::cout << "  # Decode specific PVs for last hour\n";
    std::cout << "  " << program_name << " decode BPMS_DMPH_502_TMITBR,BPMS_LTUH_250_XBR --last 1h\n\n";
    
    std::cout << "  # Decode all TMIT PVs and export to CSV\n";
    std::cout << "  " << program_name << " pattern \".*TMIT.*\" --range 2 --csv tmit_data.csv\n\n";
    
    std::cout << "  # Decode with specific time range\n";
    std::cout << "  " << program_name << " pattern \".*BPM.*\" --start \"2025-07-11 10:00:00\" --end \"2025-07-11 12:00:00\"\n\n";
    
    std::cout << "  # Show statistics only for large dataset\n";
    std::cout << "  " << program_name << " pattern \".*KLYS.*\" --range 6 --stats-only\n\n";
    
    std::cout << "  # Debug mode - show raw protobuf response\n";
    std::cout << "  " << program_name << " raw BPMS_DMPH_502_TMITBR --last 30m\n\n";
}

void printExamples() {
    std::cout << "\nCommon PV Patterns for LCLS-II:\n";
    std::cout << "  .*BPM.*           - All Beam Position Monitors\n";
    std::cout << "  .*TMIT.*          - All charge/transmission monitors\n";
    std::cout << "  .*KLYS.*          - All klystron/RF systems\n";
    std::cout << "  .*LTUH.*          - HXR transport line\n";
    std::cout << "  .*DMPH.*          - HXR dump line\n";
    std::cout << "  .*LI20.*          - Sector 20 devices\n";
    std::cout << "  .*_XBR$           - X-axis BPM readbacks\n";
    std::cout << "  .*_YBR$           - Y-axis BPM readbacks\n";
    std::cout << "  .*AMPL.*          - Amplitude measurements\n";
    std::cout << "  .*PHAS.*          - Phase measurements\n";
}

struct CommandArgs {
    std::string server_address = "localhost:50052";
    std::string command;
    std::vector<std::string> command_args;
    
    std::string start_time;
    std::string end_time;
    std::string last_duration;
    int range_hours = 1;
    
    std::string csv_file;
    std::string json_file;
    int max_points = 10;
    bool stats_only = false;
    bool use_serialized = true;
    bool quiet = false;
    bool show_help = false;
    bool show_examples = false;
};

CommandArgs parseArgs(int argc, char* argv[]) {
    CommandArgs args;
    std::vector<std::string> arguments(argv + 1, argv + argc);
    
    for (size_t i = 0; i < arguments.size(); ++i) {
        const std::string& arg = arguments[i];
        
        if (arg == "-h" || arg == "--help") {
            args.show_help = true;
        } else if (arg == "--examples") {
            args.show_examples = true;
        } else if (arg == "-s" || arg == "--server") {
            if (i + 1 < arguments.size()) {
                args.server_address = arguments[++i];
            }
        } else if (arg == "--start") {
            if (i + 1 < arguments.size()) {
                args.start_time = arguments[++i];
            }
        } else if (arg == "--end") {
            if (i + 1 < arguments.size()) {
                args.end_time = arguments[++i];
            }
        } else if (arg == "--last") {
            if (i + 1 < arguments.size()) {
                args.last_duration = arguments[++i];
            }
        } else if (arg == "--range") {
            if (i + 1 < arguments.size()) {
                args.range_hours = std::stoi(arguments[++i]);
            }
        } else if (arg == "--csv") {
            if (i + 1 < arguments.size()) {
                args.csv_file = arguments[++i];
            }
        } else if (arg == "--json") {
            if (i + 1 < arguments.size()) {
                args.json_file = arguments[++i];
            }
        } else if (arg == "--max-points") {
            if (i + 1 < arguments.size()) {
                args.max_points = std::stoi(arguments[++i]);
            }
        } else if (arg == "--stats-only") {
            args.stats_only = true;
        } else if (arg == "--no-serialized") {
            args.use_serialized = false;
        } else if (arg == "--quiet") {
            args.quiet = true;
        } else if (args.command.empty()) {
            // First non-option argument is the command
            args.command = arg;
        } else {
            // Remaining arguments are command args
            args.command_args.push_back(arg);
        }
    }
    
    return args;
}

std::pair<uint64_t, uint64_t> calculateTimeRange(const CommandArgs& args) {
    uint64_t now = TimeUtils::getCurrentUnixTime();
    
    if (!args.start_time.empty() && !args.end_time.empty()) {
        // Explicit time range
        uint64_t start = TimeUtils::parseTimeString(args.start_time);
        uint64_t end = TimeUtils::parseTimeString(args.end_time);
        return {start, end};
    } else if (!args.last_duration.empty()) {
        // Relative duration
        uint64_t start = TimeUtils::getRelativeTime(args.last_duration);
        return {start, now};
    } else {
        // Default: last N hours
        uint64_t start = now - (args.range_hours * 3600);
        return {start, now};
    }
}

void printRawResponse(const QueryDataResponse& response) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "RAW QUERY RESPONSE DEBUG" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << "Response time: " << TimeUtils::formatUnixTime(response.responsetime().epochseconds()) << std::endl;
    
    if (response.has_exceptionalresult()) {
        std::cout << "EXCEPTIONAL RESULT - Query failed" << std::endl;
        // Add more details about the exceptional result if needed
    } else if (response.has_querydata()) {
        const auto& query_data = response.querydata();
        std::cout << "SUCCESS - Got query data" << std::endl;
        std::cout << "Number of data buckets: " << query_data.databuckets_size() << std::endl;
        
        for (int i = 0; i < query_data.databuckets_size(); ++i) {
            const auto& bucket = query_data.databuckets(i);
            std::cout << "\nBucket " << i << ":" << std::endl;
            
            if (bucket.has_datatimestamps()) {
                const auto& ts = bucket.datatimestamps();
                if (ts.value_case() == DataTimestamps::kSamplingClock) {
                    std::cout << "  Timestamps: SamplingClock" << std::endl;
                    std::cout << "  Start time: " << TimeUtils::formatUnixTime(ts.samplingclock().starttime().epochseconds()) << std::endl;
                    std::cout << "  Sample period: " << ts.samplingclock().periodnanos() << " ns" << std::endl;
                    std::cout << "  Num samples: " << ts.samplingclock().count() << std::endl;
                } else if (ts.value_case() == DataTimestamps::kTimestampList) {
                    std::cout << "  Timestamps: TimestampsList" << std::endl;
                    std::cout << "  Num timestamps: " << ts.timestamplist().timestamps_size() << std::endl;
                }
            }
            
            if (bucket.has_datacolumn()) {
                const auto& col = bucket.datacolumn();
                std::cout << "  DataColumn: " << col.name() << std::endl;
                std::cout << "  Num values: " << col.datavalues_size() << std::endl;
            } else if (bucket.has_serializeddatacolumn()) {
                std::cout << "  SerializedDataColumn: <serialized data>" << std::endl;
                // TODO: Add actual field name when we know the correct one
            }
            
            std::cout << "  Tags: " << bucket.tags_size() << std::endl;
            std::cout << "  Attributes: " << bucket.attributes_size() << std::endl;
        }
    } else {
        std::cout << "UNKNOWN RESPONSE TYPE" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    auto args = parseArgs(argc, argv);
    
    if (args.show_help || argc == 1) {
        printUsage(argv[0]);
        return 0;
    }
    
    if (args.show_examples) {
        printExamples();
        return 0;
    }
    
    if (args.command.empty()) {
        std::cerr << "Error: No command specified. Use --help for usage information." << std::endl;
        return 1;
    }
    
    // Initialize data decoder
    try {
        DataDecoder decoder(args.server_address);
        
        if (!args.quiet) {
            std::cout << "Connecting to data platform at " << args.server_address << "..." << std::endl;
        }
        
        // Execute command
        if (args.command == "test") {
            bool connected = decoder.testConnection();
            return connected ? 0 : 1;
            
        } else if (args.command == "discover") {
            std::string pattern = args.command_args.empty() ? ".*" : args.command_args[0];
            auto pv_names = decoder.discoverPVs(pattern);
            
            if (!args.quiet) {
                std::cout << "\nFound " << pv_names.size() << " PVs matching pattern: " << pattern << std::endl;
            }
            
            for (const auto& pv : pv_names) {
                std::cout << pv << std::endl;
            }
            
            return 0;
            
        } else if (args.command == "decode") {
            if (args.command_args.empty()) {
                std::cerr << "Error: No PV names specified for decode command" << std::endl;
                return 1;
            }
            
            // Split comma-separated PV names
            std::vector<std::string> pv_names;
            std::string pv_list = args.command_args[0];
            std::replace(pv_list.begin(), pv_list.end(), ',', ' ');
            std::istringstream iss(pv_list);
            std::string pv;
            while (iss >> pv) {
                pv_names.push_back(pv);
            }
            
            auto [start_time, end_time] = calculateTimeRange(args);
            auto decoded_data = decoder.queryAndDecode(pv_names, start_time, end_time, args.use_serialized);
            
            if (decoded_data.empty()) {
                std::cout << "No data retrieved" << std::endl;
                return 1;
            }
            
            // Output results
            if (!args.stats_only && !args.quiet) {
                decoder.printDecodedData(decoded_data, args.max_points, false);
            }
            
            if (args.stats_only || !args.quiet) {
                decoder.printStatistics(decoded_data);
            }
            
            // Export if requested
            if (!args.csv_file.empty()) {
                decoder.exportToCSV(decoded_data, args.csv_file);
            }
            
            if (!args.json_file.empty()) {
                decoder.exportToJSON(decoded_data, args.json_file);
            }
            
            return 0;
            
        } else if (args.command == "pattern") {
            if (args.command_args.empty()) {
                std::cerr << "Error: No pattern specified for pattern command" << std::endl;
                return 1;
            }
            
            std::string pattern = args.command_args[0];
            auto [start_time, end_time] = calculateTimeRange(args);
            auto decoded_data = decoder.queryAndDecodeByPattern(pattern, start_time, end_time, args.use_serialized);
            
            if (decoded_data.empty()) {
                std::cout << "No data retrieved" << std::endl;
                return 1;
            }
            
            // Output results
            if (!args.stats_only && !args.quiet) {
                decoder.printDecodedData(decoded_data, args.max_points, false);
            }
            
            if (args.stats_only || !args.quiet) {
                decoder.printStatistics(decoded_data);
            }
            
            // Export if requested
            if (!args.csv_file.empty()) {
                decoder.exportToCSV(decoded_data, args.csv_file);
            }
            
            if (!args.json_file.empty()) {
                decoder.exportToJSON(decoded_data, args.json_file);
            }
            
            return 0;
            
        } else if (args.command == "raw") {
            if (args.command_args.empty()) {
                std::cerr << "Error: No PV names specified for raw command" << std::endl;
                return 1;
            }
            
            // Split comma-separated PV names
            std::vector<std::string> pv_names;
            std::string pv_list = args.command_args[0];
            std::replace(pv_list.begin(), pv_list.end(), ',', ' ');
            std::istringstream iss(pv_list);
            std::string pv;
            while (iss >> pv) {
                pv_names.push_back(pv);
            }
            
            auto [start_time, end_time] = calculateTimeRange(args);
            auto response = decoder.getRawQueryResponse(pv_names, start_time, end_time, args.use_serialized);
            
            printRawResponse(response);
            return 0;
            
        } else {
            std::cerr << "Error: Unknown command '" << args.command << "'" << std::endl;
            std::cerr << "Use --help for available commands" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
