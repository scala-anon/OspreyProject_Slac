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

class DataProviderCLI
{
private:
    std::map<std::string, std::string> config_;

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
 ║                      DataProvider Client CLI                     ║
 ║                            Version 2.0                           ║
 ║                                                                  ║
 ╚══════════════════════════════════════════════════════════════════╝

)" << std::endl;
    }

public:
    DataProviderCLI()
    {
        // Load default configuration
        config_["ingest_server"] = "localhost:50051";
        config_["query_server"] = "localhost:50052";
        config_["annotation_server"] = "localhost:50053";
    }

    void showHelp()
    {
        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                              USAGE                                │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "  DataProvider_CLI <command> [options]\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                         H5 DATA COMMANDS                          │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "  discover [pattern]         Find H5 PVs matching pattern\n";
        std::cout << "  view <pv> [time_range]     Quick H5 data view\n";
        std::cout << "  export <pvs> --ml          ML export from MongoDB\n";
        std::cout << "  ingest <path>              Ingest H5 data files\n";
        std::cout << "  decode <pv>                Decode MongoDB data\n";
        std::cout << "  monitor <pv>               Monitor H5 data\n";
        std::cout << "  analyze <pvs>              H5 data analysis\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                        UTILITY COMMANDS                           │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "  tools [tool]               Show tools help (decode/export/ingest)\n";
        std::cout << "  examples [command]         Show examples (general or command-specific)\n";
        std::cout << "  help                       Show this help\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                        QUICK START EXAMPLES                       │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "  DataProvider_CLI discover \".*BPM.*\"\n";
        std::cout << "  DataProvider_CLI view BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n";
        std::cout << "  DataProvider_CLI export export-pattern \".*TMIT.*\"\n";
        std::cout << "  DataProvider_CLI ingest /data/h5_files/ --project CoAD\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                        OUTPUT LOCATIONS                           │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "  CSV files:                 build/exports/csv/\n";
        std::cout << "  JSON files:                build/exports/json/\n";
        std::cout << "  NumPy files:               build/exports/numpy/\n";
        std::cout << "  PyTorch data:              build/exports/pytorch/\n";
        std::cout << "  HDF5 files:                build/exports/hdf5/\n\n";
    }

    void showGeneralExamples()
    {
        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                    LCLS DATA PROVIDER CLI EXAMPLES                │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n\n";

        std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                    IMPORTANT: HISTORICAL DATA                     ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
        std::cout << "Your MongoDB contains historical data from June 23, 2025.\n";
        std::cout << "Use these timestamps for queries:\n";
        std::cout << "  --start 1750690485 --end 1750706894 --no-serialized\n";
        std::cout << "  (June 23, 2025 14:54:45 to 19:28:14 UTC)\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                     H5 DATA DISCOVERY                             │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "# Find all available H5 PVs\n";
        std::cout << "DataProvider_CLI discover\n\n";

        std::cout << "# Find all BPM (Beam Position Monitor) PVs\n";
        std::cout << "DataProvider_CLI discover \".*BPM.*\"\n\n";

        std::cout << "# Find all charge monitors (TMIT)\n";
        std::cout << "DataProvider_CLI discover \".*TMIT.*\"\n\n";

        std::cout << "# Find all PVs in sector LI20\n";
        std::cout << "DataProvider_CLI discover \".*LI20.*\"\n\n";

        std::cout << "# Find X-axis measurements\n";
        std::cout << "DataProvider_CLI discover \".*_X.*\"\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                      H5 DATA VIEWING                              │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "# Quick view of a PV (using historical data range)\n";
        std::cout << "DataProvider_CLI view BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n\n";

        std::cout << "# View with shorter time range (30 minutes)\n";
        std::cout << "DataProvider_CLI view KLYS_LI23_31_AMPL --start 1750690485 --end 1750692285 --no-serialized\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                      H5 DATA DECODING                             │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "# Decode a single PV (with historical timestamps)\n";
        std::cout << "DataProvider_CLI decode BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n\n";

        std::cout << "# Decode multiple PVs\n";
        std::cout << "DataProvider_CLI decode BPMS_DMPH_502_TMITBR,KLYS_LI23_31_AMPL --start 1750690485 --end 1750706894 --no-serialized\n\n";

        std::cout << "# Decode and export to CSV\n";
        std::cout << "DataProvider_CLI decode BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized --csv bpm_data.csv\n";
        std::cout << "# Output: build/exports/csv/bpm_data.csv\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                      ML/NUMPY EXPORT                              │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "# Export all available PVs for ML (uses historical data automatically)\n";
        std::cout << "DataProvider_CLI export discover\n";
        std::cout << "# Output: build/exports/numpy/\n\n";

        std::cout << "# Export specific PV pattern for PyTorch\n";
        std::cout << "DataProvider_CLI export export-pattern \".*BPM.*\"\n\n";

        std::cout << "# Export specific PVs by name\n";
        std::cout << "DataProvider_CLI export export-pvs BPMS_DMPH_502_TMITBR,KLYS_LI23_31_AMPL\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                      H5 DATA INGESTION                            │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "# Ingest H5 files from directory\n";
        std::cout << "DataProvider_CLI ingest /data/h5_files/\n\n";

        std::cout << "# Ingest with specific project filter\n";
        std::cout << "DataProvider_CLI ingest /data/h5_files/ --project CoAD\n\n";

        std::cout << "# Ingest with device filter\n";
        std::cout << "DataProvider_CLI ingest /data/h5_files/ --device BPMS\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                    COMMON LCLS PV PATTERNS                        │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "H5 Physics Data Patterns:\n";
        std::cout << "  .*BPM.*                    Beam Position Monitors (physics measurements)\n";
        std::cout << "  .*TMIT.*                   Charge/transmission monitors\n";
        std::cout << "  .*KLYS.*                   Klystron/RF systems (measurements)\n";
        std::cout << "  .*LTUH.*                   HXR transport line\n";
        std::cout << "  .*DMPH.*                   HXR dump line\n";
        std::cout << "  .*LI20.*                   Sector 20 devices\n";
        std::cout << "  .*_XBR$                    X-axis BPM readbacks\n";
        std::cout << "  .*_YBR$                    Y-axis BPM readbacks\n";
        std::cout << "  .*AMPL.*                   Amplitude measurements\n";
        std::cout << "  .*PHAS.*                   Phase measurements\n\n";

        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                      HISTORICAL DATA INFO                         │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "Your MongoDB contains data from:     June 23, 2025\n";
        std::cout << "Start time:                          1750690485 (14:54:45 UTC)\n";
        std::cout << "End time:                            1750706894 (19:28:14 UTC)\n";
        std::cout << "Duration:                            ~4.5 hours of LCLS data\n";
        std::cout << "PVs:                                 1600+ process variables\n\n";

        std::cout << "Use --no-serialized flag to avoid serialization issues.\n";
        std::cout << "Always specify --start and --end for data queries.\n\n";
    }

    void showCommandExamples(const std::string& command)
    {
        if (command == "discover") {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                       DISCOVER COMMAND EXAMPLES                   │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "H5 Data Discovery:\n";
            std::cout << "  DataProvider_CLI discover                     # All H5 PVs\n";
            std::cout << "  DataProvider_CLI discover \".*\"               # All H5 PVs (explicit)\n";
            std::cout << "  DataProvider_CLI discover \".*BPM.*\"          # Beam Position Monitors\n";
            std::cout << "  DataProvider_CLI discover \".*KLYS.*\"         # Klystrons\n";
            std::cout << "  DataProvider_CLI discover \".*TMIT.*\"         # Charge monitors\n\n";

        } else if (command == "view") {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                         VIEW COMMAND EXAMPLES                     │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "H5 Data Viewing:\n";
            std::cout << "  DataProvider_CLI view BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n";
            std::cout << "  DataProvider_CLI view KLYS_LI23_31_AMPL --start 1750690485 --end 1750706894 --no-serialized\n\n";

        } else if (command == "ingest") {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                        INGEST COMMAND EXAMPLES                    │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "H5 Data Ingestion:\n";
            std::cout << "  DataProvider_CLI ingest /path/to/h5/files/            # All H5 files\n";
            std::cout << "  DataProvider_CLI ingest /data/lcls_data/              # Production data\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --project CoAD      # Specific project\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --device BPMS       # Only BPM devices\n\n";

        } else if (command == "decode") {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                        DECODE COMMAND EXAMPLES                    │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "H5 Data Decoding:\n";
            std::cout << "  DataProvider_CLI decode BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n";
            std::cout << "  DataProvider_CLI decode KLYS_LI23_31_AMPL --start 1750690485 --end 1750706894 --no-serialized --csv output.csv\n\n";

        } else if (command == "export") {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                        EXPORT COMMAND EXAMPLES                    │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "ML/NumPy Export:\n";
            std::cout << "  DataProvider_CLI export discover                      # Export all available PVs\n";
            std::cout << "  DataProvider_CLI export export-pattern \".*BPM.*\"     # Export BPM data\n";
            std::cout << "  DataProvider_CLI export export-pvs PV1,PV2,PV3       # Export specific PVs\n\n";

        } else if (command == "monitor") {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                       MONITOR COMMAND EXAMPLES                    │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "H5 Data Monitoring:\n";
            std::cout << "  DataProvider_CLI monitor BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n\n";

        } else if (command == "analyze") {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                       ANALYZE COMMAND EXAMPLES                    │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "H5 Data Analysis:\n";
            std::cout << "  DataProvider_CLI analyze discover                     # Analyze all PVs\n";
            std::cout << "  DataProvider_CLI analyze export-pattern \".*BPM.*\"    # Analyze BPM data\n\n";

        } else if (command == "tools") {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                         TOOLS COMMAND EXAMPLES                    │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "Available Tools:\n";
            std::cout << "  DataProvider_CLI tools h5_to_dp                       # H5 ingestion tool help\n";
            std::cout << "  DataProvider_CLI tools data_decoder                   # Data decoder help\n";
            std::cout << "  DataProvider_CLI tools mongo_to_npy                   # NumPy export help\n\n";

        } else {
            std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                          UNKNOWN COMMAND                          │\n";
            std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
            std::cout << "UNKNOWN COMMAND: " << command << "\n\n";
            std::cout << "Available commands for examples:\n";
            std::cout << "  discover, view, decode, export, ingest, monitor, analyze, tools\n\n";
            std::cout << "Usage: DataProvider_CLI examples [command]\n";
            std::cout << "   or: DataProvider_CLI examples              # Show all examples\n\n";
        }
    }

    int runExamples(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            // Show all examples
            showGeneralExamples();
            return 0;
        }

        std::string command = args[1];
        showCommandExamples(command);
        return 0;
    }

    int runCommand(const std::vector<std::string> &args)
    {
        if (args.empty())
        {
            // Show banner only when called without arguments
            showBanner();
            showHelp();
            return 0;
        }
        
        if (args[0] == "help" || args[0] == "--help")
        {
            showHelp();
            return 0;
        }

        std::string command = args[0];

        // H5 data commands
        if (command == "examples")
        {
            return runExamples(args);
        }
        else if (command == "discover")
        {
            return runDiscover(args);
        }
        else if (command == "view")
        {
            return runView(args);
        }
        else if (command == "export")
        {
            return runExport(args);
        }
        else if (command == "ingest")
        {
            return runExternalTool("h5_to_dp", args);
        }
        else if (command == "decode")
        {
            return runExternalTool("data_decoder", args);
        }
        else if (command == "monitor")
        {
            return runMonitor(args);
        }
        else if (command == "analyze")
        {
            return runExternalTool("mongo_to_npy", args);
        }
        else if (command == "tools")
        {
            return runToolsHelp(args);
        }
        else
        {
            std::cerr << "\nUnknown command: " << command << std::endl;
            std::cerr << "Use 'DataProvider_CLI help' for usage information." << std::endl;
            return 1;
        }
    }

private:
    int runExternalTool(const std::string& tool_name, const std::vector<std::string>& args) {
        std::string cmd = getToolPath(tool_name);
        for (size_t i = 1; i < args.size(); ++i) {
            cmd += " " + args[i];
        }
        std::cout << "Executing: " << cmd << std::endl;
        return system(cmd.c_str());
    }

    int runToolsHelp(const std::vector<std::string>& args) {
        std::cout << "┌────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│                           AVAILABLE TOOLS                         │\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
        std::cout << "  h5_to_dp        HDF5 data ingestion\n";
        std::cout << "  data_decoder    Data decoding and export\n";
        std::cout << "  mongo_to_npy    MongoDB to NumPy export\n\n";
        return 0;
    }

    std::string getToolPath(const std::string &tool_name)
    {
        // Get the directory where this CLI executable is located
        std::filesystem::path cli_path = std::filesystem::canonical("/proc/self/exe");
        std::filesystem::path cli_dir = cli_path.parent_path();

        // Tools are in the same directory as the CLI
        return cli_dir / tool_name;
    }

    std::string getExportPath(const std::string &format)
    {
        // Get the directory where this CLI executable is located
        std::filesystem::path cli_path = std::filesystem::canonical("/proc/self/exe");
        std::filesystem::path cli_dir = cli_path.parent_path();

        // Create exports directory structure
        std::filesystem::path exports_dir = cli_dir / "exports" / format;

        // Create directories if they don't exist
        std::filesystem::create_directories(exports_dir);

        return exports_dir.string();
    }

    int runDiscover(const std::vector<std::string> &args)
    {
        std::cout << "Running H5 PV discovery..." << std::endl;

        try
        {
            QueryClient queryClient(config_["query_server"]);
            std::string pattern = args.size() > 1 ? args[1] : ".*";

            std::cout << "Searching for H5 PVs matching pattern: " << pattern << std::endl;

            // Use the query client to find PVs
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
                std::cout << "\nFound " << result.pvinfos_size() << " H5 PVs matching pattern: " << pattern << std::endl;
                std::cout << "┌─────┬──────────────────────────────────────┬─────────┬──────────────┐\n";
                std::cout << "│ No. │                PV Name               │ Buckets │     Type     │\n";
                std::cout << "├─────┼──────────────────────────────────────┼─────────┼──────────────┤\n";

                for (int i = 0; i < result.pvinfos_size(); ++i)
                {
                    const auto &pv_info = result.pvinfos(i);
                    std::cout << "│ " << std::setw(3) << (i + 1) << " │ " 
                              << std::setw(36) << std::left << pv_info.pvname() << " │ "
                              << std::setw(7) << std::right << pv_info.numbuckets() << " │ "
                              << std::setw(12) << std::left << pv_info.lastbucketdatatype() << " │\n";
                }
                std::cout << "└─────┴──────────────────────────────────────┴─────────┴──────────────┘\n";
            }

            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Discovery error: " << e.what() << std::endl;
            return 1;
        }
    }

    int runView(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            std::cerr << "Usage: DataProvider_CLI view <pv> [options]" << std::endl;
            std::cerr << "  Options: --start <timestamp> --end <timestamp> --no-serialized" << std::endl;
            std::cerr << "  Example: DataProvider_CLI view BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized" << std::endl;
            return 1;
        }

        std::string pv = args[1];

        // Parse arguments for timestamps and options
        uint64_t start_time = 1750690485;  // Default to historical data start
        uint64_t end_time = 1750706894;    // Default to historical data end
        bool use_serialized = true;

        for (size_t i = 2; i < args.size(); ++i)
        {
            if (args[i] == "--start" && i + 1 < args.size())
            {
                start_time = std::stoull(args[i + 1]);
                i++;
            }
            else if (args[i] == "--end" && i + 1 < args.size())
            {
                end_time = std::stoull(args[i + 1]);
                i++;
            }
            else if (args[i] == "--no-serialized")
            {
                use_serialized = false;
            }
        }

        std::cout << "Viewing H5 data for PV: " << pv << std::endl;
        std::cout << "Time range: " << start_time << " to " << end_time
                  << " (June 23, 2025)" << std::endl;
        std::cout << "Serialized: " << (use_serialized ? "yes" : "no") << std::endl;

        try
        {
            QueryClient queryClient(config_["query_server"]);

            // Create timestamps
            auto begin_time = makeTimestamp(start_time, 0);
            auto end_time_ts = makeTimestamp(end_time, 0);

            // Query data for the PV
            std::vector<std::string> pv_names = {pv};
            auto request = makeQueryDataRequest(pv_names, begin_time, end_time_ts, use_serialized);
            auto response = queryClient.queryData(request);

            if (response.has_exceptionalresult())
            {
                std::cerr << "❌ Query failed: " << response.exceptionalresult().message() << std::endl;
                return 1;
            }

            if (response.has_querydata())
            {
                const auto &query_data = response.querydata();
                std::cout << "\n✅ Query successful! Found " << query_data.databuckets_size() << " data buckets" << std::endl;

                for (int i = 0; i < query_data.databuckets_size(); ++i)
                {
                    const auto &bucket = query_data.databuckets(i);
                    std::cout << "\nBucket " << (i + 1) << ":" << std::endl;

                    if (bucket.has_datacolumn())
                    {
                        const auto &column = bucket.datacolumn();
                        std::cout << "  PV: " << column.name() << std::endl;
                        std::cout << "  Data points: " << column.datavalues_size() << std::endl;

                        // Show first few values
                        int max_display = std::min(5, column.datavalues_size());
                        std::cout << "  Sample values: ";
                        for (int j = 0; j < max_display; ++j)
                        {
                            const auto &value = column.datavalues(j);
                            switch (value.value_case())
                            {
                            case DataValue::kDoubleValue:
                                std::cout << value.doublevalue() << " ";
                                break;
                            case DataValue::kFloatValue:
                                std::cout << value.floatvalue() << " ";
                                break;
                            case DataValue::kLongValue:
                                std::cout << value.longvalue() << " ";
                                break;
                            case DataValue::kIntValue:
                                std::cout << value.intvalue() << " ";
                                break;
                            default:
                                std::cout << "? ";
                                break;
                            }
                        }
                        if (column.datavalues_size() > max_display)
                        {
                            std::cout << "... (" << (column.datavalues_size() - max_display) << " more)";
                        }
                        std::cout << std::endl;
                    }

                    if (bucket.has_datatimestamps())
                    {
                        const auto &timestamps = bucket.datatimestamps();
                        if (timestamps.has_samplingclock())
                        {
                            const auto &clock = timestamps.samplingclock();
                            std::cout << "  Sampling: " << clock.count() << " samples, "
                                      << (clock.periodnanos() / 1000000.0) << " ms period" << std::endl;
                        }
                    }
                }
            }

            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "View error: " << e.what() << std::endl;
            return 1;
        }
    }

    int runExport(const std::vector<std::string> &args)
    {
        std::cout << "Running ML/NumPy data export..." << std::endl;

        // Build command for mongo_to_npy with correct path
        std::string cmd = getToolPath("mongo_to_npy");

        // Process arguments and handle directory changes for mongo_to_npy
        std::vector<std::string> processed_args;
        for (size_t i = 1; i < args.size(); ++i)
        {
            processed_args.push_back(args[i]);
        }

        // Change to appropriate export directory before running mongo_to_npy
        std::string numpy_path = getExportPath("numpy");
        std::string original_cmd = cmd;
        for (const auto &arg : processed_args)
        {
            original_cmd += " " + arg;
        }

        // Run mongo_to_npy from the numpy export directory
        cmd = "cd " + numpy_path + " && " + original_cmd;

        std::cout << "Executing: " << cmd << std::endl;
        std::cout << "Output will be saved to: " << numpy_path << std::endl;

        int result = system(cmd.c_str());

        if (result == 0)
        {
            std::cout << "Export completed successfully!" << std::endl;
            std::cout << "Files saved to: " << numpy_path << std::endl;
        }
        else
        {
            std::cerr << "Export failed with code: " << result << std::endl;
        }

        return result;
    }

    int runMonitor(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            std::cerr << "Usage: DataProvider_CLI monitor <pv_pattern>" << std::endl;
            std::cerr << "  Example: DataProvider_CLI monitor BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized" << std::endl;
            return 1;
        }

        std::string pattern = args[1];
        std::cout << "Starting H5 data monitoring for pattern: " << pattern << std::endl;
        std::cout << "This will display data updates..." << std::endl;
        std::cout << "Press Ctrl+C to stop monitoring..." << std::endl;

        // For H5 monitoring, we can periodically query the database
        // This is a simplified implementation
        while (true) {
            try {
                std::cout << "\n--- Monitoring Update: " << getCurrentTimeString() << " ---" << std::endl;
                
                // Run a view command to show current data
                std::vector<std::string> view_args = {"view", pattern};
                // Add any additional args from the original command
                for (size_t i = 2; i < args.size(); ++i) {
                    view_args.push_back(args[i]);
                }
                
                runView(view_args);
                
                // Wait 10 seconds before next update
                std::this_thread::sleep_for(std::chrono::seconds(10));
                
            } catch (const std::exception& e) {
                std::cerr << "Monitor error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }

        return 0;
    }

    std::string getCurrentTimeString() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

int main(int argc, char *argv[])
{
    try
    {
        // Convert arguments to vector for easier handling
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i)
        {
            args.push_back(argv[i]);
        }

        DataProviderCLI cli;
        return cli.runCommand(args);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
