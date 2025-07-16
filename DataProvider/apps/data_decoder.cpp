#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "parsers/data_decoder.hpp"

void printUsage(const std::string& program_name) {
    std::cout << R"(
 ╔══════════════════════════════════════════════════════════════════╗
 ║                                                                  ║
 ║  ██████╗  █████╗ ████████╗ █████╗     ██████╗ ███████╗ ██████╗   ║
 ║  ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗    ██╔══██╗██╔════╝██╔════╝   ║
 ║  ██║  ██║███████║   ██║   ███████║    ██║  ██║█████╗  ██║        ║
 ║  ██║  ██║██╔══██║   ██║   ██╔══██║    ██║  ██║██╔══╝  ██║        ║
 ║  ██████╔╝██║  ██║   ██║   ██║  ██║    ██████╔╝███████╗╚██████╗   ║
 ║  ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝    ╚═════╝ ╚══════╝ ╚═════╝   ║
 ║                                                                  ║
 ║                    LCLS Data Decoder Tool                        ║
 ║                Convert MongoDB bytes to readable format          ║
 ║                            Version 2.0                           ║
 ║                                                                  ║
 ╚══════════════════════════════════════════════════════════════════╝

)" << std::endl;

    std::cout << "USAGE: " << program_name << " [OPTIONS] COMMAND [ARGS...]\n\n";

    std::cout << "OPTIONS:\n";
    std::cout << "  -s, --server HOST:PORT     Data platform server address\n";
    std::cout << "                             (default: localhost:50052)\n";
    std::cout << "  -h, --help                 Show this help message\n\n";

    std::cout << "COMMANDS:\n";
    std::cout << "  discover [PATTERN]         List available PVs matching pattern\n";
    std::cout << "                             (default pattern: .*)\n";
    std::cout << "  decode PV1,PV2,...         Decode specific PVs\n";
    std::cout << "  pattern REGEX              Decode PVs matching regex pattern\n";
    std::cout << "  test                       Test connection to data platform\n";
    std::cout << "  raw PV1,PV2,...            Show raw query response (debug mode)\n\n";

    std::cout << "TIME RANGE OPTIONS:\n";
    std::cout << "  --start TIME               Start time (YYYY-MM-DD HH:MM:SS or Unix)\n";
    std::cout << "  --end TIME                 End time (YYYY-MM-DD HH:MM:SS or Unix)\n";
    std::cout << "  --last DURATION            Last N time (e.g., '1h', '30m', '2d')\n";
    std::cout << "  --range HOURS              Query last N hours (default: 1)\n";
    std::cout << "  --full-range               Use full available time range for pattern\n\n";

    std::cout << "OUTPUT OPTIONS:\n";
    std::cout << "  --csv FILE                 Export to CSV file\n";
    std::cout << "  --json FILE                Export to JSON file\n";
    std::cout << "  --max-points N             Max data points to display (default: 10)\n";
    std::cout << "  --stats-only               Show only statistics, no raw data\n";
    std::cout << "  --no-serialized            Don't use serialized data columns\n";
    std::cout << "  --quiet                    Minimal output\n\n";
}

void printExamples() {
    std::cout << "COMMON LCLS-II PV PATTERNS:\n";
    std::cout << "  .*BPM.*              All Beam Position Monitors\n";
    std::cout << "  .*TMIT.*             All charge/transmission monitors\n";
    std::cout << "  .*KLYS.*             All klystron/RF systems\n";
    std::cout << "  .*LTUH.*             HXR transport line\n";
    std::cout << "  .*DMPH.*             HXR dump line\n";
    std::cout << "  .*LI20.*             Sector 20 devices\n";
    std::cout << "  .*_XBR$              X-axis BPM readbacks\n";
    std::cout << "  .*_YBR$              Y-axis BPM readbacks\n";
    std::cout << "  .*AMPL.*             Amplitude measurements\n";
    std::cout << "  .*PHAS.*             Phase measurements\n\n";
}

struct CommandArgs {
    std::string server_address = "localhost:50052";
    std::string command;
    std::vector<std::string> command_args;

    std::string start_time;
    std::string end_time;
    std::string last_duration;
    int range_hours = 1;
    bool full_range = false;

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
        } else if (arg == "--full-range") {
            args.full_range = true;
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

std::pair<uint64_t, uint64_t> calculateTimeRange(const CommandArgs& args, DataDecoder& decoder, const std::string& pattern = "") {
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
    } else if (args.full_range && !pattern.empty()) {
        // NEW: Use full available time range for pattern
        std::cout << "Determining full time range for pattern: " << pattern << std::endl;
        
        try {
            // Get metadata for all PVs matching pattern
            auto request = makeQueryPvMetadataRequestWithPattern(pattern);
            auto response = decoder.queryPvMetadata(request);
            
            if (response.has_metadataresult()) {
                const auto& result = response.metadataresult();
                
                if (result.pvinfos_size() == 0) {
                    std::cout << "No PVs found for pattern, using default range" << std::endl;
                    uint64_t start = now - (args.range_hours * 3600);
                    return {start, now};
                }
                
                uint64_t earliest_start = UINT64_MAX;
                uint64_t latest_end = 0;
                
                for (int i = 0; i < result.pvinfos_size(); ++i) {
                    const auto& pv_info = result.pvinfos(i);
                    
                    if (pv_info.has_firstdatatimestamp()) {
                        uint64_t pv_start = pv_info.firstdatatimestamp().epochseconds();
                        earliest_start = std::min(earliest_start, pv_start);
                    }
                    
                    if (pv_info.has_lastdatatimestamp()) {
                        uint64_t pv_end = pv_info.lastdatatimestamp().epochseconds();
                        latest_end = std::max(latest_end, pv_end);
                    }
                }
                
                if (earliest_start != UINT64_MAX && latest_end != 0) {
                    std::cout << "Full time range: " << TimeUtils::formatUnixTime(earliest_start) 
                              << " to " << TimeUtils::formatUnixTime(latest_end) << std::endl;
                    std::cout << "Total duration: " << ((latest_end - earliest_start) / 3600) << " hours" << std::endl;
                    return {earliest_start, latest_end};
                } else {
                    std::cout << "Could not determine full range, using default" << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error determining full range: " << e.what() << std::endl;
        }
        
        // Fallback to default range
        uint64_t start = now - (args.range_hours * 3600);
        return {start, now};
    } else {
        // Default: last N hours
        uint64_t start = now - (args.range_hours * 3600);
        return {start, now};
    }
}

void printRawResponse(const QueryDataResponse& response) {
    std::cout << "\nRAW QUERY RESPONSE DEBUG\n";
    std::cout << "Response time: " << TimeUtils::formatUnixTime(response.responsetime().epochseconds()) << std::endl;

    if (response.has_exceptionalresult()) {
        std::cout << "\nEXCEPTIONAL RESULT - Query failed\n";
        // Add more details about the exceptional result if needed
    } else if (response.has_querydata()) {
        const auto& query_data = response.querydata();
        std::cout << "\nSUCCESS - Got query data\n";
        std::cout << "Number of data buckets: " << query_data.databuckets_size() << std::endl;

        for (int i = 0; i < query_data.databuckets_size(); ++i) {
            const auto& bucket = query_data.databuckets(i);
            std::cout << "\nBucket " << std::setw(3) << i << ":\n";

            if (bucket.has_datatimestamps()) {
                const auto& ts = bucket.datatimestamps();
                if (ts.value_case() == DataTimestamps::kSamplingClock) {
                    std::cout << "  Timestamps: SamplingClock\n";
                    std::cout << "  Start time: " << TimeUtils::formatUnixTime(ts.samplingclock().starttime().epochseconds()) << std::endl;
                    std::cout << "  Sample period: " << ts.samplingclock().periodnanos() << " ns\n";
                    std::cout << "  Num samples: " << ts.samplingclock().count() << std::endl;
                } else if (ts.value_case() == DataTimestamps::kTimestampList) {
                    std::cout << "  Timestamps: TimestampsList\n";
                    std::cout << "  Num timestamps: " << ts.timestamplist().timestamps_size() << std::endl;
                }
            }

            if (bucket.has_datacolumn()) {
                const auto& col = bucket.datacolumn();
                std::cout << "  DataColumn: " << col.name() << std::endl;
                std::cout << "  Num values: " << col.datavalues_size() << std::endl;
            } else if (bucket.has_serializeddatacolumn()) {
                std::cout << "  SerializedDataColumn: <serialized data>\n";
            }

            std::cout << "  Tags: " << bucket.tags_size() << std::endl;
            std::cout << "  Attributes: " << bucket.attributes_size() << std::endl;
        }
    } else {
        std::cout << "\nUNKNOWN RESPONSE TYPE\n";
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
        std::cerr << "\nError: No command specified. Use --help for usage information." << std::endl;
        return 1;
    }

    // Initialize data decoder
    try {
        DataDecoder decoder(args.server_address);

        if (!args.quiet) {
            std::cout << "\nConnecting to data platform at " << args.server_address << "..." << std::endl;
        }

        // Execute command
        if (args.command == "test") {
            std::cout << "\nCONNECTION TEST\n";
            bool connected = decoder.testConnection();
            if (connected) {
                std::cout << "Connection successful!\n";
            } else {
                std::cout << "Connection failed!\n";
            }
            return connected ? 0 : 1;

        } else if (args.command == "discover") {
            std::string pattern = args.command_args.empty() ? ".*" : args.command_args[0];
            auto pv_names = decoder.discoverPVs(pattern);

            std::cout << "\nPV DISCOVERY\n";

            if (!args.quiet) {
                std::cout << "Pattern: " << pattern << std::endl;
                std::cout << "Found " << pv_names.size() << " PVs\n\n";
            }

            if (!pv_names.empty()) {
                std::cout << "No.  PV Name\n";
                for (size_t i = 0; i < pv_names.size(); ++i) {
                    std::cout << std::setw(3) << (i + 1) << "  " << pv_names[i] << std::endl;
                }
            } else {
                std::cout << "No PVs found matching pattern: " << pattern << std::endl;
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

            std::cout << "\nPV DECODING\n";

            auto [start_time, end_time] = calculateTimeRange(args, decoder);
            std::cout << "Time range: " << TimeUtils::formatUnixTime(start_time)
                      << " to " << TimeUtils::formatUnixTime(end_time) << std::endl;
            std::cout << "PVs to decode: " << pv_names.size() << std::endl;

            auto decoded_data = decoder.queryAndDecode(pv_names, start_time, end_time, args.use_serialized);

            if (decoded_data.empty()) {
                std::cout << "\nNo data retrieved" << std::endl;
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
                std::cout << "\nExporting to CSV: " << args.csv_file << std::endl;
                decoder.exportToCSV(decoded_data, args.csv_file);
            }

            if (!args.json_file.empty()) {
                std::cout << "\nExporting to JSON: " << args.json_file << std::endl;
                decoder.exportToJSON(decoded_data, args.json_file);
            }

            return 0;

        } else if (args.command == "pattern") {
            if (args.command_args.empty()) {
                std::cerr << "Error: No pattern specified for pattern command" << std::endl;
                return 1;
            }

            std::string pattern = args.command_args[0];

            std::cout << "\nPATTERN DECODING\n";

            // NEW: Calculate time range with pattern support for full range
            auto [start_time, end_time] = calculateTimeRange(args, decoder, pattern);
            std::cout << "Pattern: " << pattern << std::endl;
            std::cout << "Time range: " << TimeUtils::formatUnixTime(start_time)
                      << " to " << TimeUtils::formatUnixTime(end_time) << std::endl;

            // Use streaming for pattern queries (designed for large datasets)
            auto decoded_data = decoder.queryAndDecodeByPatternStreaming(pattern, start_time, end_time, args.use_serialized);

            if (decoded_data.empty()) {
                std::cout << "\nNo data retrieved" << std::endl;
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
                std::cout << "\nExporting to CSV: " << args.csv_file << std::endl;
                decoder.exportToCSV(decoded_data, args.csv_file);
            }

            if (!args.json_file.empty()) {
                std::cout << "\nExporting to JSON: " << args.json_file << std::endl;
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

            auto [start_time, end_time] = calculateTimeRange(args, decoder);
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
