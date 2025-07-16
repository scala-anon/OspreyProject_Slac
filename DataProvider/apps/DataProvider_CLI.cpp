#include <regex>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <thread>
#include <iomanip>
#include <ctime>
#include <numeric>
#include <regex>

// Include your existing client headers
#include "clients/ingest_client.hpp"
#include "clients/query_client.hpp"
#include "parsers/h5_parser.hpp"
#include "parsers/data_decoder.hpp"

class UnifiedDataProviderCLI
{
private:
    std::map<std::string, std::string> config_;
    std::string server_address_;

    void showBanner()
    {
        std::cout << R"(
 ╔══════════════════════════════════════════════════════════════════╗
 ║                                                                  ║
 ║  ██████╗  █████╗ ████████╗ █████╗ ██████╗ ██████╗  ██████╗ ██╗   ║
 ║  ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗██╔══██╗██╔══██╗██╔═══██╗██║   ║
 ║  ██║  ██║███████║   ██║   ███████║██████╔╝██████╔╝██║   ██║██║   ║
 ║  ██║  ██║██╔══██║   ██║   ██╔══██║██╔═══╝ ██╔══██╗██║   ██║╚═╝   ║
 ║  ██████╔╝██║  ██║   ██║   ██║  ██║██║     ██║  ██║╚██████╔╝██╗   ║
 ║  ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝   ║
 ║                                                                  ║
 ║       ██████╗██╗     ██╗███████╗███╗   ██╗████████╗              ║
 ║      ██╔════╝██║     ██║██╔════╝████╗  ██║╚══██╔══╝              ║
 ║      ██║     ██║     ██║█████╗  ██╔██╗ ██║   ██║                 ║
 ║      ██║     ██║     ██║██╔══╝  ██║╚██╗██║   ██║                 ║
 ║      ╚██████╗███████╗██║███████╗██║ ╚████║   ██║                 ║
 ║       ╚═════╝╚══════╝╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝                 ║
 ║                                                                  ║
 ║                  Unified DataProvider Client CLI                 ║
 ║                          Version 3.0                             ║
 ║               Single Interface for All Data Operations           ║
 ║                                                                  ║
 ╚══════════════════════════════════════════════════════════════════╝

)" << std::endl;
    }

    // Helper function to check if string ends with suffix (C++11 compatible)
    bool stringEndsWith(const std::string& str, const std::string& suffix) {
        if (suffix.length() > str.length()) return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }

    std::string getCurrentTimeString() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    void parseGlobalOptions(const std::vector<std::string>& args) {
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--server" && i + 1 < args.size()) {
                server_address_ = args[i + 1];
            }
        }
    }

    std::string getToolPath(const std::string &tool_name)
    {
        std::filesystem::path cli_path = std::filesystem::canonical("/proc/self/exe");
        std::filesystem::path cli_dir = cli_path.parent_path();
        return cli_dir / tool_name;
    }

    int runExternalTool(const std::string& tool_name, const std::vector<std::string>& args) {
        std::string cmd = getToolPath(tool_name);
        
        for (size_t i = 0; i < args.size(); ++i) {
            cmd += " \"" + args[i] + "\"";
        }
        
        std::cout << "DELEGATING TO BACKEND TOOL\n";
        std::cout << "Executing: " << tool_name << "\n\n";
        
        return system(cmd.c_str());
    }

public:
    UnifiedDataProviderCLI()
    {
        // Load default configuration
        config_["ingest_server"] = "localhost:50051";
        config_["query_server"] = "localhost:50052";
        config_["annotation_server"] = "localhost:50053";
        server_address_ = config_["query_server"];
    }

    void showHelp()
    {
        std::cout << "USAGE: DataProvider_CLI <command> [options]\n\n";

        std::cout << "COMMANDS:\n";
        std::cout << "  ingest <h5_directory>      Ingest H5 files into MongoDB\n";
        std::cout << "  discover [pattern]         Find available PVs in the database\n";
        std::cout << "  query <pvs>                Query and decode PV data\n";
        std::cout << "  export <target>            Export data for ML/analysis\n";
        std::cout << "  monitor <pv>               Monitor PV data in real-time\n";
        std::cout << "  status                     Check system status\n\n";

        std::cout << "COMMON OPTIONS:\n";
        std::cout << "  --server ADDRESS           Server address (default: localhost:50052)\n";
        std::cout << "  --start TIME               Start time (Unix timestamp or ISO format)\n";
        std::cout << "  --end TIME                 End time (Unix timestamp or ISO format)\n";
        std::cout << "  --last DURATION            Last N duration (e.g., '2h', '30m', '1d')\n";
        std::cout << "  --full-range               Use complete available time range\n";
        std::cout << "  --format FORMAT            Output format: csv, json, numpy, pytorch\n";
        std::cout << "  --output PATH              Output path for exports\n";
        std::cout << "  --max-points N             Limit displayed points (default: 10)\n";
        std::cout << "  --quiet                    Minimal output\n";
        std::cout << "  --help                     Show this help\n\n";

        std::cout << "EXPERT MODE:\n";
        std::cout << "  tools h5_to_dp              Access H5 ingestion tool directly\n";
        std::cout << "  tools data_decoder          Access data decoder tool directly\n";
        std::cout << "  tools mongo_to_npy          Access ML export tool directly\n\n";
    }

    void showCommandHelp(const std::string& command)
    {
        if (command == "ingest") {
            std::cout << "INGEST COMMAND - Convert H5 files to MongoDB\n\n";
            std::cout << "USAGE: DataProvider_CLI ingest <h5_directory> [OPTIONS]\n\n";
            std::cout << "OPTIONS:\n";
            std::cout << "  --project NAME             Filter by project (e.g., CoAD)\n";
            std::cout << "  --device TYPE              Filter by device type (e.g., BPMS, KLYS)\n";
            std::cout << "  --device-area AREA         Filter by device area (e.g., LI23, DMPH)\n";
            std::cout << "  --streaming                Use streaming ingestion (recommended)\n";
            std::cout << "  --batch-size N             Streaming batch size (default: 10)\n";
            std::cout << "  --max-signals N            Limit number of signals\n";
            std::cout << "  --local-only               Parse only, don't ingest\n";
            std::cout << "  --show-filters             Show available filter options\n\n";
            std::cout << "EXAMPLES:\n";
            std::cout << "  DataProvider_CLI ingest /data/h5_files/ --streaming\n";
            std::cout << "  DataProvider_CLI ingest /data/lcls/ --project=CoAD --device=BPMS\n\n";

        } else if (command == "discover") {
            std::cout << "DISCOVER COMMAND - Find available PVs in database\n\n";
            std::cout << "USAGE: DataProvider_CLI discover [PATTERN/TYPE] [OPTIONS]\n\n";
            std::cout << "PATTERN/TYPE:\n";
            std::cout << "  <empty>                    Show all available PVs\n";
            std::cout << "  bpm                        All Beam Position Monitors\n";
            std::cout << "  klys                       All Klystron/RF systems\n";
            std::cout << "  tmit                       All charge/transmission monitors\n";
            std::cout << "  \"regex_pattern\"            Custom regex pattern\n\n";
            std::cout << "OPTIONS:\n";
            std::cout << "  --count-only               Show only count, not PV names\n";
            std::cout << "  --details                  Show PV metadata\n";
            std::cout << "  --group-by TYPE            Group results by device type\n\n";
            std::cout << "EXAMPLES:\n";
            std::cout << "  DataProvider_CLI discover\n";
            std::cout << "  DataProvider_CLI discover bpm\n";
            std::cout << "  DataProvider_CLI discover \".*LI20.*\" --details\n\n";

        } else if (command == "query") {
            std::cout << "QUERY COMMAND - Query and decode PV data\n\n";
            std::cout << "USAGE: DataProvider_CLI query <PVS/PATTERN> [OPTIONS]\n\n";
            std::cout << "PVS/PATTERN:\n";
            std::cout << "  PV1,PV2,PV3                Comma-separated PV names\n";
            std::cout << "  \"regex_pattern\"            Regex pattern matching PVs\n";
            std::cout << "  BPMS, KLYS, TMIT            Device type shortcuts\n\n";
            std::cout << "TIME OPTIONS:\n";
            std::cout << "  --start TIME               Start time (Unix or ISO format)\n";
            std::cout << "  --end TIME                 End time (Unix or ISO format)\n";
            std::cout << "  --last DURATION            Last N duration (2h, 30m, 1d)\n";
            std::cout << "  --range HOURS              Last N hours (default: 1)\n";
            std::cout << "  --full-range               Use complete available time range\n\n";
            std::cout << "OUTPUT OPTIONS:\n";
            std::cout << "  --format FORMAT            csv, json, table (default: table)\n";
            std::cout << "  --output PATH              Save to file\n";
            std::cout << "  --max-points N             Limit displayed points (default: 10)\n";
            std::cout << "  --stats-only               Show only statistics\n\n";
            std::cout << "EXAMPLES:\n";
            std::cout << "  DataProvider_CLI query BPMS_DMPH_502_TMITBR --last 1h\n";
            std::cout << "  DataProvider_CLI query \".*BPM.*\" --start 1750690485 --end 1750706894\n";
            std::cout << "  DataProvider_CLI query BPMS --full-range --max-points 5\n";
            std::cout << "  DataProvider_CLI query TMIT --last 6h --stats-only\n\n";

        } else if (command == "export") {
            std::cout << "EXPORT COMMAND - Export data for ML and analysis\n\n";
            std::cout << "USAGE: DataProvider_CLI export <TARGET> [OPTIONS]\n\n";
            std::cout << "TARGET:\n";
            std::cout << "  all                        Export all available PVs\n";
            std::cout << "  \"regex_pattern\"            Export PVs matching pattern\n";
            std::cout << "  PV1,PV2,PV3                Export specific PVs\n\n";
            std::cout << "FORMAT OPTIONS:\n";
            std::cout << "  --format FORMAT            numpy, pytorch, csv, json\n";
            std::cout << "  --timeseries               Format for LSTM/RNN training\n";
            std::cout << "  --sequence-length N        Sequence length for timeseries\n";
            std::cout << "  --full-range               Use complete available time range\n\n";
            std::cout << "EXAMPLES:\n";
            std::cout << "  DataProvider_CLI export \".*BPM.*\" --format=numpy\n";
            std::cout << "  DataProvider_CLI export BPMS --full-range --format=csv\n";
            std::cout << "  DataProvider_CLI export all --format=pytorch --name=full_dataset\n\n";

        } else {
            std::cout << "No help available for command: " << command << std::endl;
            std::cout << "Use 'DataProvider_CLI help' for general help.\n";
        }
    }

    void showWorkflows()
    {
        std::cout << "COMMON PHYSICIST WORKFLOWS\n\n";

        std::cout << "WORKFLOW 1: QUICK DATA EXPLORATION\n";
        std::cout << "Fast analysis of existing data:\n\n";
        std::cout << "1. Find what's available:\n";
        std::cout << "   DataProvider_CLI discover bpm --count-only\n\n";
        std::cout << "2. Get all BPM data:\n";
        std::cout << "   DataProvider_CLI query BPMS --full-range --stats-only\n\n";
        std::cout << "3. Quick sample for analysis:\n";
        std::cout << "   DataProvider_CLI query \".*TMIT.*\" --last 2h --max-points 100\n\n";

        std::cout << "WORKFLOW 2: COMPLETE DATASET EXPORT\n";
        std::cout << "Export full datasets for detailed analysis:\n\n";
        std::cout << "1. Export all BPM data:\n";
        std::cout << "   DataProvider_CLI export BPMS --full-range --format=csv\n\n";
        std::cout << "2. Export for machine learning:\n";
        std::cout << "   DataProvider_CLI export \".*KLYS.*\" --full-range --format=numpy\n\n";
        std::cout << "3. Time series for ML training:\n";
        std::cout << "   DataProvider_CLI export \".*BPM.*\" --timeseries --sequence-length=100\n\n";

        std::cout << "WORKFLOW 3: NEW DATA INGESTION\n";
        std::cout << "Processing new H5 files from accelerator:\n\n";
        std::cout << "1. Ingest new data:\n";
        std::cout << "   DataProvider_CLI ingest /data/h5_files/ --streaming\n\n";
        std::cout << "2. Verify ingestion:\n";
        std::cout << "   DataProvider_CLI discover --group-by device\n\n";
        std::cout << "3. Test query new data:\n";
        std::cout << "   DataProvider_CLI query \".*BPM.*\" --last 1h --max-points 3\n\n";

        std::cout << "WORKFLOW 4: REAL-TIME MONITORING\n";
        std::cout << "Monitor live accelerator data:\n\n";
        std::cout << "1. Monitor specific PV:\n";
        std::cout << "   DataProvider_CLI monitor BPMS_DMPH_502_TMITBR\n\n";
        std::cout << "2. Check recent trends:\n";
        std::cout << "   DataProvider_CLI query BPMS --last 30m --stats-only\n\n";
        std::cout << "3. Export recent data:\n";
        std::cout << "   DataProvider_CLI query \".*TMIT.*\" --last 1h --format=csv\n\n";
    }

    int runStatus(const std::vector<std::string>& args) {
        std::cout << "SYSTEM STATUS\n\n";

        bool all_good = true;

        // Test connections
        try {
            QueryClient queryClient(config_["query_server"]);
            std::cout << "Query server (" << config_["query_server"] << "): Connected\n";
        } catch (const std::exception& e) {
            std::cout << "Query server (" << config_["query_server"] << "): FAILED - " << e.what() << "\n";
            all_good = false;
        }

        try {
            IngestClient ingestClient(config_["ingest_server"]);
            std::cout << "Ingest server (" << config_["ingest_server"] << "): Connected\n";
        } catch (const std::exception& e) {
            std::cout << "Ingest server (" << config_["ingest_server"] << "): FAILED - " << e.what() << "\n";
            all_good = false;
        }

        // Check tool availability
        std::vector<std::string> tools = {"h5_to_dp", "data_decoder", "mongo_to_npy"};
        for (const auto& tool : tools) {
            std::string path = getToolPath(tool);
            if (std::filesystem::exists(path)) {
                std::cout << "Tool " << tool << ": Available\n";
            } else {
                std::cout << "Tool " << tool << ": NOT FOUND at " << path << "\n";
                all_good = false;
            }
        }

        std::cout << "\nOverall status: " << (all_good ? "All systems operational" : "Issues detected") << "\n";
        return all_good ? 0 : 1;
    }

    int runDebug(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Debug commands:\n";
            std::cout << "  connection    Test server connections\n";
            std::cout << "  query <pv>    Show raw query response\n";
            std::cout << "  memory        Show memory usage\n";
            return 1;
        }

        std::string debug_cmd = args[1];

        if (debug_cmd == "connection") {
            return runStatus(args);
        } else if (debug_cmd == "query" && args.size() > 2) {
            // Delegate to data_decoder raw command
            std::vector<std::string> decoder_args = {"raw", args[2], "--server", server_address_};
            return runExternalTool("data_decoder", decoder_args);
        } else if (debug_cmd == "memory") {
            std::cout << "Memory debugging not implemented yet.\n";
            return 0;
        } else {
            std::cerr << "Unknown debug command: " << debug_cmd << std::endl;
            return 1;
        }
    }

    int runToolsAccess(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "EXPERT TOOLS\n\n";
            std::cout << "Available tools:\n";
            std::cout << "  h5_to_dp        H5 data ingestion tool\n";
            std::cout << "  data_decoder    Data query and decoding tool\n";
            std::cout << "  mongo_to_npy    MongoDB to NumPy/PyTorch export tool\n\n";
            std::cout << "Usage: DataProvider_CLI tools <tool_name> [tool_args...]\n";
            std::cout << "   or: DataProvider_CLI tools <tool_name> --help\n";
            return 0;
        }

        std::string tool_name = args[1];
        
        // Validate tool exists
        std::vector<std::string> valid_tools = {"h5_to_dp", "data_decoder", "mongo_to_npy"};
        if (std::find(valid_tools.begin(), valid_tools.end(), tool_name) == valid_tools.end()) {
            std::cerr << "Unknown tool: " << tool_name << std::endl;
            return 1;
        }

        // Pass remaining args to the tool
        std::vector<std::string> tool_args(args.begin() + 2, args.end());
        
        return runExternalTool(tool_name, tool_args);
    }

    int runIngest(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cerr << "Usage: DataProvider_CLI ingest <h5_directory> [options]" << std::endl;
            std::cerr << "Use 'DataProvider_CLI ingest --help' for detailed help." << std::endl;
            return 1;
        }

        std::cout << "H5 DATA INGESTION\n";
        std::cout << "Ingesting H5 files into MongoDB...\n\n";

        // Transform user-friendly options to h5_to_dp options
        std::vector<std::string> h5_args;
        h5_args.push_back(args[1]); // h5_directory

        // Parse and transform options
        for (size_t i = 2; i < args.size(); ++i) {
            const std::string& arg = args[i];
            
            if (arg == "--help") {
                showCommandHelp("ingest");
                return 0;
            }
            
            // Pass through compatible options
            if (arg.find("--project=") == 0 || 
                arg.find("--device=") == 0 || 
                arg.find("--device-area=") == 0 ||
                arg.find("--batch-size=") == 0 ||
                arg.find("--max-signals=") == 0 ||
                arg == "--streaming" ||
                arg == "--local-only" ||
                arg == "--show-filters") {
                h5_args.push_back(arg);
            }
            // Transform server option
            else if (arg == "--server" && i + 1 < args.size()) {
                h5_args.push_back("--mldp-server=" + args[i + 1]);
                i++; // skip next arg
            }
        }

        // Add default streaming if not specified
        if (std::find(h5_args.begin(), h5_args.end(), "--streaming") == h5_args.end() &&
            std::find(h5_args.begin(), h5_args.end(), "--local-only") == h5_args.end()) {
            h5_args.push_back("--streaming");
        }

        return runExternalTool("h5_to_dp", h5_args);
    }

    int runDiscover(const std::vector<std::string>& args) {
        std::cout << "PV DISCOVERY\n\n";

        // Handle user-friendly patterns
        std::string pattern = ".*";
        bool count_only = false;
        bool details = false;
        std::string group_by = "";

        for (size_t i = 1; i < args.size(); ++i) {
            const std::string& arg = args[i];
            
            if (arg == "--help") {
                showCommandHelp("discover");
                return 0;
            } else if (arg == "--count-only") {
                count_only = true;
            } else if (arg == "--details") {
                details = true;
            } else if (arg.find("--group-by=") == 0) {
                group_by = arg.substr(11);
            } else if (arg == "bmp" || arg == "bpm") {
                pattern = ".*BPM.*";
            } else if (arg == "klys" || arg == "klystron") {
                pattern = ".*KLYS.*";
            } else if (arg == "tmit" || arg == "charge") {
                pattern = ".*TMIT.*";
            } else if (arg[0] != '-') {
                pattern = arg;
            }
        }

        std::cout << "Searching for PVs with pattern: " << pattern << "\n";

        // Use built-in discovery (lightweight for common cases)
        try
        {
            QueryClient queryClient(server_address_);
            auto request = makeQueryPvMetadataRequestWithPattern(pattern);
            auto response = queryClient.queryPvMetadata(request);

            if (response.has_exceptionalresult())
            {
                std::cerr << "PV discovery failed: " << response.exceptionalresult().message() << std::endl;
                return 1;
            }

            if (response.has_metadataresult())
            {
                const auto &result = response.metadataresult();
                std::cout << "\nFound " << result.pvinfos_size() << " PVs matching pattern: " << pattern << "\n";
                
                if (count_only) {
                    return 0;
                }

                if (group_by.empty()) {
                    std::cout << "\nNo.  PV Name                             Buckets  Type\n";
                    std::cout << "---  -----------------------------------  -------  ------------\n";

                    for (int i = 0; i < result.pvinfos_size(); ++i)
                    {
                        const auto &pv_info = result.pvinfos(i);
                        std::cout << std::setw(3) << (i + 1) << "  "
                                  << std::setw(35) << std::left << pv_info.pvname() << "  "
                                  << std::setw(7) << std::right << pv_info.numbuckets() << "  "
                                  << std::setw(12) << std::left << pv_info.lastbucketdatatype() << "\n";
                    }
                } else {
                    // Group by device type (simple grouping)
                    std::map<std::string, std::vector<std::string>> groups;
                    for (int i = 0; i < result.pvinfos_size(); ++i) {
                        const auto &pv_info = result.pvinfos(i);
                        std::string pv_name = pv_info.pvname();
                        
                        // Simple device type extraction
                        std::string device_type = "OTHER";
                        if (pv_name.find("BPM") != std::string::npos) device_type = "BPM";
                        else if (pv_name.find("KLYS") != std::string::npos) device_type = "KLYS";
                        else if (pv_name.find("TMIT") != std::string::npos) device_type = "TMIT";
                        else if (pv_name.find("QUAD") != std::string::npos) device_type = "QUAD";
                        
                        groups[device_type].push_back(pv_name);
                    }

                    for (const auto& [device_type, pvs] : groups) {
                        std::cout << "\n" << device_type << " (" << pvs.size() << " PVs):\n";
                        for (size_t i = 0; i < pvs.size() && i < 5; ++i) {
                            std::cout << "  " << pvs[i] << "\n";
                        }
                        if (pvs.size() > 5) {
                            std::cout << "  ... and " << (pvs.size() - 5) << " more\n";
                        }
                    }
                }
            }

            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Discovery error: " << e.what() << std::endl;
            return 1;
        }
    }

    int runQuery(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cerr << "Usage: DataProvider_CLI query <pvs/pattern> [options]" << std::endl;
            std::cerr << "Use 'DataProvider_CLI query --help' for detailed help." << std::endl;
            return 1;
        }

        std::cout << "PV DATA QUERY\n\n";

        // Transform unified options to data_decoder options
        std::vector<std::string> decoder_args;

        std::string target = args[1];

        // Better logic for determining command type
        bool is_regex_pattern = target.find(".*") != std::string::npos ||
                               target.find("[") != std::string::npos ||
                               target.find("^") != std::string::npos ||
                               target.find("$") != std::string::npos;

        bool is_comma_separated = target.find(",") != std::string::npos;

        // Check if it looks like a device type (short, all caps, no special chars)
        bool looks_like_device_type = target.length() <= 10 &&
                                     target.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ_") == std::string::npos &&
                                     !is_comma_separated;

        if (is_regex_pattern) {
            // It's a regex pattern - use pattern command to get data
            decoder_args.push_back("pattern");
            decoder_args.push_back(target);
        } else if (is_comma_separated || target.find("_") != std::string::npos) {
            // It's comma-separated PVs or looks like a full PV name (contains underscore)
            decoder_args.push_back("decode");
            decoder_args.push_back(target);
        } else if (looks_like_device_type) {
            // It's a device type like "BPMS", "KLYS", etc. - use pattern to get data
            decoder_args.push_back("pattern");
            decoder_args.push_back(".*" + target + ".*");
        } else {
            // Default to decode for full PV names
            decoder_args.push_back("decode");
            decoder_args.push_back(target);
        }

        // Transform common options
        for (size_t i = 2; i < args.size(); ++i) {
            const std::string& arg = args[i];

            if (arg == "--help") {
                showCommandHelp("query");
                return 0;
            }

            // Handle options that take values
            if (arg == "--start" || arg == "--end" || arg == "--range" || arg == "--max-points") {
                decoder_args.push_back(arg);
                if (i + 1 < args.size()) {
                    decoder_args.push_back(args[i + 1]);
                    i++; // Skip the next argument since we just processed it
                }
            }
            // Handle options that start with -- and contain =
            else if (arg.find("--") == 0 && arg.find("=") != std::string::npos) {
                if (arg.find("--format=") == 0) {
                    std::string format = arg.substr(9);
                    if (format == "csv") {
                        decoder_args.push_back("--csv");
                        decoder_args.push_back("query_output.csv");
                    } else if (format == "json") {
                        decoder_args.push_back("--json");
                        decoder_args.push_back("query_output.json");
                    }
                } else if (arg.find("--output=") == 0) {
                    std::string output = arg.substr(9);
                    if (stringEndsWith(output, ".csv")) {
                        decoder_args.push_back("--csv");
                        decoder_args.push_back(output);
                    } else if (stringEndsWith(output, ".json")) {
                        decoder_args.push_back("--json");
                        decoder_args.push_back(output);
                    }
                } else {
                    // Pass through other --option=value arguments as-is
                    decoder_args.push_back(arg);
                }
            }
            // Handle options that are just flags (no values)
            else if (arg == "--stats-only" || arg == "--no-serialized" || arg == "--quiet" || arg == "--full-range") {
                decoder_args.push_back(arg);
            }
            // Handle --last which takes a value
            else if (arg == "--last") {
                decoder_args.push_back(arg);
                if (i + 1 < args.size()) {
                    decoder_args.push_back(args[i + 1]);
                    i++; // Skip the next argument
                }
            }
        }

        // Add server option
        decoder_args.push_back("--server");
        decoder_args.push_back(server_address_);

        // Add --no-serialized by default to avoid UTF-8 issues
        if (std::find(decoder_args.begin(), decoder_args.end(), "--no-serialized") == decoder_args.end()) {
            decoder_args.push_back("--no-serialized");
        }

        return runExternalTool("data_decoder", decoder_args);
    }

    int runExport(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cerr << "Usage: DataProvider_CLI export <target> [options]" << std::endl;
            std::cerr << "Use 'DataProvider_CLI export --help' for detailed help." << std::endl;
            return 1;
        }

        std::cout << "ML DATA EXPORT\n";
        std::cout << "Exporting data for ML/analysis...\n\n";

        // Transform unified options to mongo_to_npy options
        std::vector<std::string> npy_args;
        
        std::string target = args[1];
        
        // Map user-friendly targets to mongo_to_npy commands
        if (target == "all") {
            npy_args.push_back("export-all");
        } else if (target.find(",") != std::string::npos) {
            // Comma-separated PVs
            npy_args.push_back("export-pvs");
            npy_args.push_back(target);
        } else if (target.find(".*") != std::string::npos) {
            // Pattern
            npy_args.push_back("export-pattern");
            npy_args.push_back(target);
        } else {
            // Assume named dataset
            npy_args.push_back("export-dataset");
            npy_args.push_back("\".*\"");  // Pattern for all
            npy_args.push_back(target);   // Dataset name
        }

        // Parse additional options
        for (size_t i = 2; i < args.size(); ++i) {
            const std::string& arg = args[i];
            
            if (arg == "--help") {
                showCommandHelp("export");
                return 0;
            }
            
            if (arg == "--timeseries") {
                // Switch to timeseries export
                npy_args[0] = "export-timeseries";
            } else if (arg.find("--sequence-length=") == 0) {
                std::string seq_len = arg.substr(18);
                if (npy_args[0] == "export-timeseries") {
                    npy_args.push_back(seq_len);
                }
            } else if (arg.find("--name=") == 0) {
                std::string name = arg.substr(7);
                if (npy_args[0] != "export-dataset") {
                    // Convert to dataset export
                    std::string old_target = npy_args.size() > 1 ? npy_args[1] : "\".*\"";
                    npy_args.clear();
                    npy_args.push_back("export-dataset");
                    npy_args.push_back(old_target);
                    npy_args.push_back(name);
                }
            }
            // Other options are handled by mongo_to_npy internally
        }

        return runExternalTool("mongo_to_npy", npy_args);
    }

    int runMonitor(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            std::cerr << "Usage: DataProvider_CLI monitor <pv_pattern> [options]" << std::endl;
            return 1;
        }

        std::string pattern = args[1];
        std::cout << "REAL-TIME MONITORING\n";
        std::cout << "Monitoring PV: " << pattern << "\n";
        std::cout << "Press Ctrl+C to stop...\n\n";

        // Simple monitoring implementation
        while (true) {
            try {
                std::cout << "--- Update: " << getCurrentTimeString() << " ---\n";

                // Use the direct query implementation
                QueryClient queryClient(server_address_);
                
                // For monitoring, get recent data (last 5 minutes)
                uint64_t now = std::chrono::system_clock::now().time_since_epoch().count() / 1000000000;
                uint64_t start_time = now - 300; // 5 minutes ago

                auto begin_time = makeTimestamp(start_time, 0);
                auto end_time_ts = makeTimestamp(now, 0);

                std::vector<std::string> pv_names = {pattern};
                auto request = makeQueryDataRequest(pv_names, begin_time, end_time_ts, true);
                auto response = queryClient.queryData(request);

                if (response.has_querydata()) {
                    const auto &query_data = response.querydata();
                    std::cout << "Data buckets: " << query_data.databuckets_size() << "\n";
                    
                    for (int i = 0; i < std::min(1, query_data.databuckets_size()); ++i) {
                        const auto &bucket = query_data.databuckets(i);
                        if (bucket.has_datacolumn()) {
                            const auto &column = bucket.datacolumn();
                            std::cout << "PV: " << column.name() 
                                      << ", Points: " << column.datavalues_size() << "\n";
                            
                            // Show latest value
                            if (column.datavalues_size() > 0) {
                                const auto &latest = column.datavalues(column.datavalues_size() - 1);
                                std::cout << "Latest value: ";
                                switch (latest.value_case()) {
                                    case DataValue::kDoubleValue:
                                        std::cout << latest.doublevalue();
                                        break;
                                    case DataValue::kFloatValue:
                                        std::cout << latest.floatvalue();
                                        break;
                                    case DataValue::kLongValue:
                                        std::cout << latest.longvalue();
                                        break;
                                    case DataValue::kIntValue:
                                        std::cout << latest.intvalue();
                                        break;
                                    default:
                                        std::cout << "?";
                                        break;
                                }
                                std::cout << "\n";
                            }
                        }
                    }
                } else {
                    std::cout << "No data available\n";
                }

                std::this_thread::sleep_for(std::chrono::seconds(10));

            } catch (const std::exception& e) {
                std::cerr << "Monitor error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }

        return 0;
    }

    int runExamples(const std::vector<std::string> &args)
    {
        std::cout << "DATAPROVIDER CLI EXAMPLES\n\n";

        std::cout << "BASIC EXAMPLES\n\n";

        std::cout << "1. Ingest H5 data\n";
        std::cout << "   DataProvider_CLI ingest /data/h5_files/ --project=CoAD\n\n";

        std::cout << "2. Discover available data\n";
        std::cout << "   DataProvider_CLI discover bpm\n";
        std::cout << "   DataProvider_CLI discover \".*TMIT.*\"\n\n";

        std::cout << "3. Query specific data\n";
        std::cout << "   DataProvider_CLI query BPMS_DMPH_502_TMITBR --last 1h\n";
        std::cout << "   DataProvider_CLI query \".*BPM.*\" --start 1750690485 --end 1750706894\n\n";

        std::cout << "4. Export for ML\n";
        std::cout << "   DataProvider_CLI export \".*BPM.*\" --format=numpy\n";
        std::cout << "   DataProvider_CLI export all --format=pytorch --name=full_dataset\n\n";

        std::cout << "5. Real-time monitoring\n";
        std::cout << "   DataProvider_CLI monitor BPMS_DMPH_502_TMITBR\n\n";

        std::cout << "ADVANCED EXAMPLES\n\n";

        std::cout << "Time series ML export:\n";
        std::cout << "   DataProvider_CLI export \".*BPM.*\" --timeseries --sequence-length=100\n\n";

        std::cout << "Multiple format export:\n";
        std::cout << "   DataProvider_CLI query \".*KLYS.*\" --format=csv --output=klys_data.csv\n";
        std::cout << "   DataProvider_CLI export \".*KLYS.*\" --format=numpy --name=klys_ml\n\n";

        std::cout << "Filtered ingestion:\n";
        std::cout << "   DataProvider_CLI ingest /data/recent/ --device=BPMS --device-area=DMPH\n\n";

        std::cout << "Expert tool access:\n";
        std::cout << "   DataProvider_CLI tools data_decoder pattern \".*BPM.*\" --stats-only\n";
        std::cout << "   DataProvider_CLI tools mongo_to_npy discover\n\n";

        return 0;
    }

    int runCommand(const std::vector<std::string> &args)
    {
        if (args.empty())
        {
            showBanner();
            showHelp();
            return 0;
        }

        std::string command = args[0];

        // Global options parsing
        parseGlobalOptions(args);

        // Help system
        if (command == "help" || command == "--help")
        {
            if (args.size() > 1) {
                showCommandHelp(args[1]);
            } else {
                showHelp();
            }
            return 0;
        }

        if (command == "examples")
        {
            return runExamples(args);
        }

        if (command == "workflows")
        {
            showWorkflows();
            return 0;
        }

        // System commands
        if (command == "status")
        {
            return runStatus(args);
        }

        if (command == "debug")
        {
            return runDebug(args);
        }

        if (command == "tools")
        {
            return runToolsAccess(args);
        }

        // Core workflow commands
        if (command == "ingest")
        {
            return runIngest(args);
        }
        else if (command == "discover")
        {
            return runDiscover(args);
        }
        else if (command == "query")
        {
            return runQuery(args);
        }
        else if (command == "export")
        {
            return runExport(args);
        }
        else if (command == "monitor")
        {
            return runMonitor(args);
        }
        else
        {
            std::cerr << "\nUnknown command: " << command << std::endl;
            std::cerr << "Use 'DataProvider_CLI help' for usage information." << std::endl;
            return 1;
        }
    }
};

int main(int argc, char *argv[])
{
    try
    {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i)
        {
            args.push_back(argv[i]);
        }

        UnifiedDataProviderCLI cli;
        return cli.runCommand(args);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
