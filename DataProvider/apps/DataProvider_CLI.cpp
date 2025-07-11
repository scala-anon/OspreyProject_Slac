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

// Include your existing client headers
#include "clients/ingest_client.hpp"
#include "clients/query_client.hpp"
#include "parsers/h5_parser.hpp"
#include "parsers/data_decoder.hpp"

class DataProviderCLI
{
private:
    std::map<std::string, std::string> config_;

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
        std::cout << "DATA PROVIDER CLI TOOLKIT\n";
        std::cout << "=======================================\n\n";
        std::cout << "USAGE:\n";
        std::cout << "  DataProvider_CLI <command> [options]\n\n";
        std::cout << "COMMANDS:\n";
        std::cout << "  discover [pattern]      Find PVs matching pattern\n";
        std::cout << "  view <pv> [time_range]  Quick data view\n";
        std::cout << "  export <pvs> --ml       ML export\n";
        std::cout << "  plot <pv> [time_range]  Quick plot\n";
        std::cout << "  ingest <path>           Ingest H5 data\n";
        std::cout << "  decode <pv>             Decode MongoDB data\n";
        std::cout << "  monitor <pv>            Real-time monitoring\n";
        std::cout << "  analyze <pvs>           Data analysis\n";
        std::cout << "  tools [tool]            Show tools help (decode/export/ingest)\n";
        std::cout << "  examples [command]      Show examples (general or command-specific)\n";
        std::cout << "  help                    Show this help\n\n";
        std::cout << "QUICK START:\n";
        std::cout << "  DataProvider_CLI examples           # See all examples\n";
        std::cout << "  DataProvider_CLI examples discover  # Examples for discover command\n";
        std::cout << "  DataProvider_CLI discover \".*BPM.*\" # Find all BPM PVs\n\n";
        std::cout << "OUTPUT LOCATIONS:\n";
        std::cout << "  CSV files:     build/exports/csv/\n";
        std::cout << "  JSON files:    build/exports/json/\n";
        std::cout << "  NumPy files:   build/exports/numpy/\n";
        std::cout << "  PyTorch data:  build/exports/pytorch/\n";
        std::cout << "  HDF5 files:    build/exports/hdf5/\n";
    }

    void showGeneralExamples()
    {
        std::cout << "LCLS DATA PROVIDER CLI - COMPLETE EXAMPLES\n";
        std::cout << "==========================================\n\n";
        
        std::cout << "=== IMPORTANT: HISTORICAL DATA TIMESTAMPS ===\n";
        std::cout << "Your MongoDB contains historical data from June 23, 2025.\n";
        std::cout << "Use these timestamps for queries:\n";
        std::cout << "  --start 1750690485 --end 1750706894 --no-serialized\n";
        std::cout << "  (June 23, 2025 14:54:45 to 19:28:14 UTC)\n\n";
        
        std::cout << "=== DISCOVERY EXAMPLES ===\n";
        std::cout << "# Find all available PVs\n";
        std::cout << "DataProvider_CLI discover\n\n";
        
        std::cout << "# Find all BPM (Beam Position Monitor) PVs\n";
        std::cout << "DataProvider_CLI discover \".*BPM.*\"\n\n";
        
        std::cout << "# Find all charge monitors (TMIT)\n";
        std::cout << "DataProvider_CLI discover \".*TMIT.*\"\n\n";
        
        std::cout << "# Find all PVs in sector LI20\n";
        std::cout << "DataProvider_CLI discover \".*LI20.*\"\n\n";
        
        std::cout << "# Find X-axis measurements\n";
        std::cout << "DataProvider_CLI discover \".*_X.*\"\n\n";
        
        std::cout << "=== VIEWING DATA EXAMPLES ===\n";
        std::cout << "# Quick view of a PV (using historical data range)\n";
        std::cout << "DataProvider_CLI view BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n\n";
        
        std::cout << "# View with shorter time range (30 minutes)\n";
        std::cout << "DataProvider_CLI view KLYS_LI23_31_AMPL --start 1750690485 --end 1750692285 --no-serialized\n\n";
        
        std::cout << "=== DATA DECODING EXAMPLES ===\n";
        std::cout << "# Decode a single PV (with historical timestamps)\n";
        std::cout << "DataProvider_CLI decode BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n\n";
        
        std::cout << "# Decode multiple PVs\n";
        std::cout << "DataProvider_CLI decode BPMS_DMPH_502_TMITBR,KLYS_LI23_31_AMPL --start 1750690485 --end 1750706894 --no-serialized\n\n";
        
        std::cout << "# Decode and export to CSV\n";
        std::cout << "DataProvider_CLI decode BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized --csv bpm_data.csv\n";
        std::cout << "# Output: build/exports/csv/bpm_data.csv\n\n";
        
        std::cout << "# Decode all TMIT PVs and export to JSON\n";
        std::cout << "DataProvider_CLI decode \".*TMIT.*\" --start 1750690485 --end 1750706894 --no-serialized --json tmit_analysis.json\n";
        std::cout << "# Output: build/exports/json/tmit_analysis.json\n\n";
        
        std::cout << "=== ML/NUMPY EXPORT EXAMPLES ===\n";
        std::cout << "# Export all available PVs for ML (uses historical data automatically)\n";
        std::cout << "DataProvider_CLI export discover\n";
        std::cout << "# Output: build/exports/numpy/\n\n";
        
        std::cout << "# Export specific PV pattern for PyTorch\n";
        std::cout << "DataProvider_CLI export export-pattern \".*BPM.*\"\n\n";
        
        std::cout << "# Export specific PVs by name\n";
        std::cout << "DataProvider_CLI export export-pvs BPMS_DMPH_502_TMITBR,KLYS_LI23_31_AMPL\n\n";
        
        std::cout << "# Export for time series ML (LSTM)\n";
        std::cout << "DataProvider_CLI export export-timeseries \".*BPM.*\" 100\n\n";
        
        std::cout << "=== DATA INGESTION EXAMPLES ===\n";
        std::cout << "# Ingest H5 files from directory\n";
        std::cout << "DataProvider_CLI ingest /data/h5_files/\n\n";
        
        std::cout << "# Ingest with specific project filter\n";
        std::cout << "DataProvider_CLI ingest /data/h5_files/ --project CoAD\n\n";
        
        std::cout << "# Ingest with device filter\n";
        std::cout << "DataProvider_CLI ingest /data/h5_files/ --device BPMS\n\n";
        
        std::cout << "# Dry run (parse only, don't ingest)\n";
        std::cout << "DataProvider_CLI ingest /data/h5_files/ --local-only\n\n";
        
        std::cout << "=== MONITORING EXAMPLES ===\n";
        std::cout << "# Real-time monitoring (Note: uses historical data timestamps)\n";
        std::cout << "DataProvider_CLI monitor BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n";
        std::cout << "# Press Ctrl+C to stop\n\n";
        
        std::cout << "=== ANALYSIS EXAMPLES ===\n";
        std::cout << "# Analyze multiple PVs (with correct timestamps)\n";
        std::cout << "DataProvider_CLI analyze \"BPMS_DMPH_502_TMITBR,BPMS_LTUH_250_XBR\" --start 1750690485 --end 1750706894 --no-serialized\n\n";
        
        std::cout << "=== TOOL ACCESS EXAMPLES ===\n";
        std::cout << "# Show all available tools\n";
        std::cout << "DataProvider_CLI tools\n\n";
        
        std::cout << "# Get help for specific tool\n";
        std::cout << "DataProvider_CLI tools decode\n";
        std::cout << "DataProvider_CLI tools export\n";
        std::cout << "DataProvider_CLI tools ingest\n\n";
        
        std::cout << "=== COMMON LCLS PV PATTERNS ===\n";
        std::cout << ".*BPM.*           # All Beam Position Monitors\n";
        std::cout << ".*TMIT.*          # All charge/transmission monitors\n";
        std::cout << ".*KLYS.*          # All klystron/RF systems\n";
        std::cout << ".*LTUH.*          # HXR transport line\n";
        std::cout << ".*DMPH.*          # HXR dump line\n";
        std::cout << ".*LI20.*          # Sector 20 devices\n";
        std::cout << ".*_XBR$           # X-axis BPM readbacks\n";
        std::cout << ".*_YBR$           # Y-axis BPM readbacks\n";
        std::cout << ".*AMPL.*          # Amplitude measurements\n";
        std::cout << ".*PHAS.*          # Phase measurements\n\n";
        
        std::cout << "=== WORKFLOW EXAMPLES ===\n";
        std::cout << "# Complete workflow: Ingest → Discover → Export\n";
        std::cout << "1. DataProvider_CLI ingest /data/h5_files/\n";
        std::cout << "2. DataProvider_CLI discover \".*BPM.*\"\n";
        std::cout << "3. DataProvider_CLI export export-pattern \".*BPM.*\"\n\n";
        
        std::cout << "# Analysis workflow: Discover → View → Decode → Export (with correct timestamps)\n";
        std::cout << "1. DataProvider_CLI discover \".*TMIT.*\"\n";
        std::cout << "2. DataProvider_CLI view BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n";
        std::cout << "3. DataProvider_CLI decode BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized --csv results.csv\n";
        std::cout << "4. DataProvider_CLI export export-pvs BPMS_DMPH_502_TMITBR\n\n";
        
        std::cout << "=== HISTORICAL DATA INFO ===\n";
        std::cout << "Your MongoDB contains data from: June 23, 2025\n";
        std::cout << "Start time: 1750690485 (14:54:45 UTC)\n";
        std::cout << "End time:   1750706894 (19:28:14 UTC)\n";
        std::cout << "Duration:   ~4.5 hours of LCLS data\n";
        std::cout << "PVs:        1600+ process variables\n\n";
        
        std::cout << "Use --no-serialized flag to avoid serialization issues.\n";
        std::cout << "Always specify --start and --end for data queries.\n\n";
    }

    void showCommandExamples(const std::string& command)
    {
        if (command == "discover") {
            std::cout << "DISCOVER COMMAND EXAMPLES\n";
            std::cout << "========================\n\n";
            std::cout << "Basic discovery:\n";
            std::cout << "  DataProvider_CLI discover                    # All PVs\n";
            std::cout << "  DataProvider_CLI discover \".*\"              # All PVs (explicit)\n\n";
            
            std::cout << "Device-based patterns:\n";
            std::cout << "  DataProvider_CLI discover \".*BPM.*\"         # Beam Position Monitors\n";
            std::cout << "  DataProvider_CLI discover \".*KLYS.*\"        # Klystrons\n";
            std::cout << "  DataProvider_CLI discover \".*ACCL.*\"        # Accelerator components\n";
            std::cout << "  DataProvider_CLI discover \".*BEND.*\"        # Bending magnets\n";
            std::cout << "  DataProvider_CLI discover \".*QUAD.*\"        # Quadrupole magnets\n\n";
            
            std::cout << "Location-based patterns:\n";
            std::cout << "  DataProvider_CLI discover \".*LI20.*\"        # Sector 20\n";
            std::cout << "  DataProvider_CLI discover \".*DMPH.*\"        # HXR dump line\n";
            std::cout << "  DataProvider_CLI discover \".*LTUH.*\"        # HXR transport\n";
            std::cout << "  DataProvider_CLI discover \".*GUNB.*\"        # Gun/Buncher area\n\n";
            
            std::cout << "Measurement-based patterns:\n";
            std::cout << "  DataProvider_CLI discover \".*TMIT.*\"        # Charge monitors\n";
            std::cout << "  DataProvider_CLI discover \".*_X.*\"          # X-axis measurements\n";
            std::cout << "  DataProvider_CLI discover \".*_Y.*\"          # Y-axis measurements\n";
            std::cout << "  DataProvider_CLI discover \".*AMPL.*\"        # Amplitude measurements\n";
            std::cout << "  DataProvider_CLI discover \".*PHAS.*\"        # Phase measurements\n\n";
            
        } else if (command == "view") {
            std::cout << "VIEW COMMAND EXAMPLES\n";
            std::cout << "====================\n\n";
            std::cout << "Basic viewing:\n";
            std::cout << "  DataProvider_CLI view BPMS_DMPH_502_TMITBR           # Last hour (default)\n";
            std::cout << "  DataProvider_CLI view KLYS_LI23_31_AMPL              # Last hour\n\n";
            
            std::cout << "Time range options:\n";
            std::cout << "  DataProvider_CLI view BPMS_DMPH_502_TMITBR --last 30m    # Last 30 minutes\n";
            std::cout << "  DataProvider_CLI view BPMS_DMPH_502_TMITBR --last 2h     # Last 2 hours\n";
            std::cout << "  DataProvider_CLI view BPMS_DMPH_502_TMITBR --last 1d     # Last day\n\n";
            
        } else if (command == "decode") {
            std::cout << "DECODE COMMAND EXAMPLES\n";
            std::cout << "======================\n\n";
            std::cout << "Basic decoding:\n";
            std::cout << "  DataProvider_CLI decode BPMS_DMPH_502_TMITBR         # Display decoded data\n";
            std::cout << "  DataProvider_CLI decode KLYS_LI23_31_AMPL            # RF amplitude data\n\n";
            
            std::cout << "Multiple PVs:\n";
            std::cout << "  DataProvider_CLI decode BPMS_DMPH_502_TMITBR,KLYS_LI23_31_AMPL\n";
            std::cout << "  DataProvider_CLI decode \"PV1,PV2,PV3\"               # Quote for safety\n\n";
            
            std::cout << "Export options:\n";
            std::cout << "  DataProvider_CLI decode BPMS_DMPH_502_TMITBR --csv bpm_data.csv\n";
            std::cout << "  DataProvider_CLI decode KLYS_LI23_31_AMPL --json rf_data.json\n";
            std::cout << "  DataProvider_CLI decode \".*TMIT.*\" --csv all_tmit.csv --json all_tmit.json\n\n";
            
            std::cout << "Output locations:\n";
            std::cout << "  CSV files:  build/exports/csv/\n";
            std::cout << "  JSON files: build/exports/json/\n\n";
            
        } else if (command == "export") {
            std::cout << "EXPORT COMMAND EXAMPLES\n";
            std::cout << "======================\n\n";
            std::cout << "Discovery export:\n";
            std::cout << "  DataProvider_CLI export discover                    # Export all found PVs\n\n";
            
            std::cout << "Pattern-based export:\n";
            std::cout << "  DataProvider_CLI export export-pattern \".*BPM.*\"   # All BPM data\n";
            std::cout << "  DataProvider_CLI export export-pattern \".*TMIT.*\"  # All charge monitors\n";
            std::cout << "  DataProvider_CLI export export-pattern \".*LI20.*\"  # Sector 20 data\n\n";
            
            std::cout << "Specific PVs export:\n";
            std::cout << "  DataProvider_CLI export export-pvs BPMS_DMPH_502_TMITBR\n";
            std::cout << "  DataProvider_CLI export export-pvs PV1,PV2,PV3\n\n";
            
            std::cout << "ML-specific exports:\n";
            std::cout << "  DataProvider_CLI export export-timeseries \".*BPM.*\" 50   # LSTM sequences\n";
            std::cout << "  DataProvider_CLI export export-timeseries \".*BPM.*\" 100  # Longer sequences\n";
            std::cout << "  DataProvider_CLI export export-dataset \".*GCC.*\" gas_chambers\n\n";
            
            std::cout << "Output: build/exports/numpy/\n";
            std::cout << "  - <name>_data.npy         # Main data matrix\n";
            std::cout << "  - <name>_timestamps.npy   # Timestamps\n";
            std::cout << "  - <name>_loader.py        # PyTorch loader\n\n";
            
        } else if (command == "ingest") {
            std::cout << "INGEST COMMAND EXAMPLES\n";
            std::cout << "======================\n\n";
            std::cout << "Basic ingestion:\n";
            std::cout << "  DataProvider_CLI ingest /path/to/h5/files/           # All H5 files\n";
            std::cout << "  DataProvider_CLI ingest /data/lcls_data/             # Production data\n\n";
            
            std::cout << "Filtered ingestion:\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --project CoAD     # Specific project\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --device BPMS      # Only BPM devices\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --device-area LI20 # Sector 20 only\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --max-signals 100  # Limit signals\n\n";
            
            std::cout << "Ingestion modes:\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --streaming         # Use streaming\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --batch-size 20     # Custom batch size\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --local-only        # Parse only (no MongoDB)\n\n";
            
            std::cout << "Show available filters:\n";
            std::cout << "  DataProvider_CLI ingest /data/h5/ --show-filters      # See filter options\n\n";
            
        } else if (command == "monitor") {
            std::cout << "MONITOR COMMAND EXAMPLES\n";
            std::cout << "=======================\n\n";
            std::cout << "Basic monitoring (using historical data range):\n";
            std::cout << "  DataProvider_CLI monitor BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n";
            std::cout << "  DataProvider_CLI monitor KLYS_LI23_31_AMPL --start 1750690485 --end 1750706894 --no-serialized\n\n";
            
            std::cout << "Note: Press Ctrl+C to stop monitoring\n";
            std::cout << "Output format: [HH:MM:SS] - PV_NAME: value\n";
            std::cout << "IMPORTANT: Use historical timestamps since data is from June 23, 2025\n\n";
            
        } else if (command == "analyze") {
            std::cout << "ANALYZE COMMAND EXAMPLES\n";
            std::cout << "=======================\n\n";
            std::cout << "Single PV analysis (with historical timestamps):\n";
            std::cout << "  DataProvider_CLI analyze BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized\n\n";
            
            std::cout << "Multiple PV analysis:\n";
            std::cout << "  DataProvider_CLI analyze \"BPMS_DMPH_502_TMITBR,KLYS_LI23_31_AMPL\" --start 1750690485 --end 1750706894 --no-serialized\n";
            std::cout << "  DataProvider_CLI analyze \"PV1,PV2,PV3\" --start 1750690485 --end 1750706894 --no-serialized\n\n";
            
            std::cout << "Output includes:\n";
            std::cout << "  - Data point counts\n";
            std::cout << "  - Min/Max/Mean values\n";
            std::cout << "  - Value ranges\n";
            std::cout << "  - Statistical summary\n\n";
            
            std::cout << "REMEMBER: Your data is historical (June 23, 2025)\n";
            std::cout << "Always use: --start 1750690485 --end 1750706894 --no-serialized\n\n";
            
        } else if (command == "tools") {
            std::cout << "TOOLS COMMAND EXAMPLES\n";
            std::cout << "=====================\n\n";
            std::cout << "Show all tools:\n";
            std::cout << "  DataProvider_CLI tools                              # List all tools\n\n";
            
            std::cout << "Get tool-specific help:\n";
            std::cout << "  DataProvider_CLI tools decode                       # data_decoder help\n";
            std::cout << "  DataProvider_CLI tools export                       # mongo_to_npy help\n";
            std::cout << "  DataProvider_CLI tools ingest                       # h5_to_dp help\n\n";
            
            std::cout << "Available tools:\n";
            std::cout << "  - data_decoder: MongoDB data decoder and viewer\n";
            std::cout << "  - mongo_to_npy: Export MongoDB data to NumPy/PyTorch format\n";
            std::cout << "  - h5_to_dp: Ingest HDF5 files into MongoDB via MLDP\n\n";
            
        } else {
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
        if (args.empty() || args[0] == "help" || args[0] == "--help")
        {
            showHelp();
            return 0;
        }

        std::string command = args[0];

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
            return runIngest(args);
        }
        else if (command == "decode")
        {
            return runDecode(args);
        }
        else if (command == "monitor")
        {
            return runMonitor(args);
        }
        else if (command == "analyze")
        {
            return runAnalyze(args);
        }
        else if (command == "tools")
        {
            return runTools(args);
        }
        else
        {
            std::cerr << "Unknown command: " << command << std::endl;
            std::cerr << "Use 'DataProvider_CLI help' for usage information." << std::endl;
            return 1;
        }
    }

private:
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
        std::cout << "Running PV discovery..." << std::endl;

        try
        {
            QueryClient queryClient(config_["query_server"]);
            std::string pattern = args.size() > 1 ? args[1] : ".*";

            std::cout << "Searching for PVs matching pattern: " << pattern << std::endl;

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
                std::cout << "\nFound " << result.pvinfos_size() << " PVs matching pattern: " << pattern << std::endl;

                for (int i = 0; i < result.pvinfos_size(); ++i)
                {
                    const auto &pv_info = result.pvinfos(i);
                    std::cout << "  " << (i + 1) << ". " << pv_info.pvname()
                              << " (buckets: " << pv_info.numbuckets()
                              << ", type: " << pv_info.lastbucketdatatype() << ")" << std::endl;
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

        std::cout << "Viewing data for PV: " << pv << std::endl;
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
                std::cerr << "Query failed: " << response.exceptionalresult().message() << std::endl;
                return 1;
            }

            if (response.has_querydata())
            {
                const auto &query_data = response.querydata();
                std::cout << "\nQuery successful! Found " << query_data.databuckets_size() << " data buckets" << std::endl;

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
        std::cout << "Running data export..." << std::endl;

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

    int runIngest(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            std::cerr << "Usage: DataProvider_CLI ingest <path>" << std::endl;
            return 1;
        }

        std::string path = args[1];
        std::cout << "Ingesting data from: " << path << std::endl;

        // Build command for h5_to_dp with correct path
        std::string cmd = getToolPath("h5_to_dp");
        cmd += " " + path;
        for (size_t i = 2; i < args.size(); ++i)
        {
            cmd += " " + args[i];
        }

        std::cout << "Executing: " << cmd << std::endl;
        int result = system(cmd.c_str());

        if (result == 0)
        {
            std::cout << "Ingestion completed successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Ingestion failed with code: " << result << std::endl;
        }

        return result;
    }

    int runDecode(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            std::cerr << "Usage: DataProvider_CLI decode <pv> [options]" << std::endl;
            std::cerr << "  Options: --start <timestamp> --end <timestamp> --no-serialized --csv <file> --json <file>" << std::endl;
            std::cerr << "  Example: DataProvider_CLI decode BPMS_DMPH_502_TMITBR --start 1750690485 --end 1750706894 --no-serialized --csv data.csv" << std::endl;
            return 1;
        }

        // Build command for data_decoder with correct path
        std::string cmd = getToolPath("data_decoder");
        cmd += " decode";

        // Parse arguments for timestamps and options
        uint64_t start_time = 1750690485;  // Default to historical data start
        uint64_t end_time = 1750706894;    // Default to historical data end
        bool use_serialized = true;
        bool has_custom_timestamps = false;

        // First pass: check for custom timestamps and flags
        for (size_t i = 1; i < args.size(); ++i)
        {
            std::string arg = args[i];
            if (arg == "--start" && i + 1 < args.size())
            {
                start_time = std::stoull(args[i + 1]);
                has_custom_timestamps = true;
                i++;
            }
            else if (arg == "--end" && i + 1 < args.size())
            {
                end_time = std::stoull(args[i + 1]);
                has_custom_timestamps = true;
                i++;
            }
            else if (arg == "--no-serialized")
            {
                use_serialized = false;
            }
        }

        // Add timestamps to command
        cmd += " --start " + std::to_string(start_time);
        cmd += " --end " + std::to_string(end_time);
        if (!use_serialized)
        {
            cmd += " --no-serialized";
        }

        // Process arguments and handle file outputs
        for (size_t i = 1; i < args.size(); ++i)
        {
            std::string arg = args[i];

            // Skip timestamp arguments (already processed)
            if (arg == "--start" || arg == "--end")
            {
                i++; // Skip the value too
                continue;
            }
            if (arg == "--no-serialized")
            {
                continue; // Already added
            }

            // Handle CSV output
            if (arg == "--csv" && i + 1 < args.size())
            {
                std::string filename = args[i + 1];
                std::string csv_path = getExportPath("csv");
                cmd += " --csv " + csv_path + "/" + filename;
                i++; // Skip the filename argument
            }
            // Handle JSON output
            else if (arg == "--json" && i + 1 < args.size())
            {
                std::string filename = args[i + 1];
                std::string json_path = getExportPath("json");
                cmd += " --json " + json_path + "/" + filename;
                i++; // Skip the filename argument
            }
            else
            {
                cmd += " " + arg;
            }
        }

        std::cout << "Executing decode with historical timestamps..." << std::endl;
        std::cout << "Time range: " << start_time << " to " << end_time << " (June 23, 2025)" << std::endl;
        std::cout << "Command: " << cmd << std::endl;

        int result = system(cmd.c_str());

        if (result == 0)
        {
            std::cout << "Decode completed successfully!" << std::endl;

            // Only show export paths if files were actually exported
            bool exported_files = false;
            for (size_t i = 1; i < args.size(); ++i)
            {
                if (args[i] == "--csv" || args[i] == "--json")
                {
                    exported_files = true;
                    break;
                }
            }

            if (exported_files)
            {
                std::cout << "Files saved to: " << getExportPath("csv") << " or " << getExportPath("json") << std::endl;
            }
        }
        else
        {
            std::cerr << "Decode failed with code: " << result << std::endl;
        }

        return result;
    }

    int runMonitor(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            std::cerr << "Usage: DataProvider_CLI monitor <pv>" << std::endl;
            return 1;
        }

        std::string pv = args[1];
        std::cout << "Starting real-time monitoring for PV: " << pv << std::endl;
        std::cout << "Press Ctrl+C to stop monitoring..." << std::endl;

        try
        {
            QueryClient queryClient(config_["query_server"]);

            // Monitor loop - query every 5 seconds
            while (true)
            {
                uint64_t now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
                uint64_t start_time = now - 10; // Last 10 seconds

                auto begin_time = makeTimestamp(start_time, 0);
                auto end_time = makeTimestamp(now, 0);

                std::vector<std::string> pv_names = {pv};
                auto request = makeQueryDataRequest(pv_names, begin_time, end_time, false);
                auto response = queryClient.queryData(request);

                if (response.has_querydata())
                {
                    const auto &query_data = response.querydata();
                    if (query_data.databuckets_size() > 0)
                    {
                        const auto &bucket = query_data.databuckets(0);
                        if (bucket.has_datacolumn())
                        {
                            const auto &column = bucket.datacolumn();
                            if (column.datavalues_size() > 0)
                            {
                                const auto &latest_value = column.datavalues(column.datavalues_size() - 1);

                                auto t = std::time(nullptr);
                                auto tm = *std::localtime(&t);
                                std::cout << std::put_time(&tm, "%H:%M:%S") << " - " << pv << ": ";

                                // Use value_case() instead of has_* methods
                                switch (latest_value.value_case())
                                {
                                case DataValue::kDoubleValue:
                                    std::cout << latest_value.doublevalue();
                                    break;
                                case DataValue::kFloatValue:
                                    std::cout << latest_value.floatvalue();
                                    break;
                                case DataValue::kLongValue:
                                    std::cout << latest_value.longvalue();
                                    break;
                                case DataValue::kIntValue:
                                    std::cout << latest_value.intvalue();
                                    break;
                                case DataValue::kUlongValue:
                                    std::cout << latest_value.ulongvalue();
                                    break;
                                case DataValue::kUintValue:
                                    std::cout << latest_value.uintvalue();
                                    break;
                                case DataValue::kStringValue:
                                    std::cout << latest_value.stringvalue();
                                    break;
                                default:
                                    std::cout << "unknown_type";
                                    break;
                                }
                                std::cout << std::endl;
                            }
                        }
                    }
                    else
                    {
                        std::cout << "No recent data for " << pv << std::endl;
                    }
                }
                else
                {
                    std::cout << "Query failed for " << pv << std::endl;
                }

                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Monitor error: " << e.what() << std::endl;
            return 1;
        }

        return 0;
    }

    int runTools(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            // Show list of all available tools
            std::cout << "AVAILABLE UNDERLYING TOOLS\n";
            std::cout << "==========================\n\n";

            std::cout << "DECODE TOOLS:\n";
            std::cout << "  data_decoder           MongoDB data decoder and viewer\n";
            std::cout << "                         Usage: DataProvider_CLI tools decode\n\n";

            std::cout << "EXPORT TOOLS:\n";
            std::cout << "  mongo_to_npy           Export MongoDB data to NumPy/PyTorch format\n";
            std::cout << "                         Usage: DataProvider_CLI tools export\n\n";

            std::cout << "INGESTION TOOLS:\n";
            std::cout << "  h5_to_dp               Ingest HDF5 files into MongoDB via MLDP\n";
            std::cout << "                         Usage: DataProvider_CLI tools ingest\n\n";

            std::cout << "USAGE:\n";
            std::cout << "  DataProvider_CLI tools <tool_name>    Show detailed help for specific tool\n";
            std::cout << "  DataProvider_CLI tools                Show this tools overview\n\n";

            std::cout << "EXAMPLES:\n";
            std::cout << "  DataProvider_CLI tools decode         # Show data_decoder help\n";
            std::cout << "  DataProvider_CLI tools export         # Show mongo_to_npy help\n";
            std::cout << "  DataProvider_CLI tools ingest         # Show h5_to_dp help\n";

            return 0;
        }

        std::string tool = args[1];

        if (tool == "decode")
        {
            std::cout << "=== DATA_DECODER TOOL HELP ===\n";
            std::string cmd = getToolPath("data_decoder");
            cmd += " --help";
            std::cout << "Executing: " << cmd << "\n\n";
            return system(cmd.c_str());
        }
        else if (tool == "export")
        {
            std::cout << "=== MONGO_TO_NPY TOOL HELP ===\n";
            std::string cmd = getToolPath("mongo_to_npy");
            // mongo_to_npy shows help when called without args
            std::cout << "Executing: " << cmd << "\n\n";
            return system(cmd.c_str());
        }
        else if (tool == "ingest")
        {
            std::cout << "=== H5_TO_DP TOOL HELP ===\n";
            std::string cmd = getToolPath("h5_to_dp");
            // h5_to_dp shows help when called without args
            std::cout << "Executing: " << cmd << "\n\n";
            return system(cmd.c_str());
        }
        else
        {
            std::cerr << "Unknown tool: " << tool << std::endl;
            std::cerr << "Available tools: decode, export, ingest" << std::endl;
            std::cerr << "Use 'DataProvider_CLI tools' to see all available tools." << std::endl;
            return 1;
        }
    }

    int runAnalyze(const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            std::cerr << "Usage: DataProvider_CLI analyze <pvs> [options]" << std::endl;
            std::cerr << "  Options: --start <timestamp> --end <timestamp> --no-serialized" << std::endl;
            std::cerr << "  Example: DataProvider_CLI analyze \"BPMS_DMPH_502_TMITBR,BPMS_LTUH_250_XBR\" --start 1750690485 --end 1750706894 --no-serialized" << std::endl;
            return 1;
        }

        std::string pv_list = args[1];

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

        std::cout << "Running data analysis for PVs: " << pv_list << std::endl;
        std::cout << "Time range: " << start_time << " to " << end_time << " (June 23, 2025)" << std::endl;
        std::cout << "Serialized: " << (use_serialized ? "yes" : "no") << std::endl;

        try
        {
            QueryClient queryClient(config_["query_server"]);

            // Parse comma-separated PVs
            std::vector<std::string> pv_names;
            std::stringstream ss(pv_list);
            std::string pv;
            while (std::getline(ss, pv, ','))
            {
                // Trim whitespace
                pv.erase(0, pv.find_first_not_of(" \t"));
                pv.erase(pv.find_last_not_of(" \t") + 1);
                if (!pv.empty())
                {
                    pv_names.push_back(pv);
                }
            }

            if (pv_names.empty())
            {
                std::cerr << "No valid PV names found" << std::endl;
                return 1;
            }

            // Use the parsed timestamps
            auto begin_time = makeTimestamp(start_time, 0);
            auto end_time_ts = makeTimestamp(end_time, 0);

            auto request = makeQueryDataRequest(pv_names, begin_time, end_time_ts, use_serialized);
            auto response = queryClient.queryData(request);

            if (response.has_exceptionalresult())
            {
                std::cerr << "Analysis query failed: " << response.exceptionalresult().message() << std::endl;
                return 1;
            }

            if (response.has_querydata())
            {
                const auto &query_data = response.querydata();
                std::cout << "\n=== Data Analysis Results ===" << std::endl;
                std::cout << "Time range: " << start_time << " to " << end_time << std::endl;
                std::cout << "PVs analyzed: " << pv_names.size() << std::endl;
                std::cout << "Data buckets found: " << query_data.databuckets_size() << std::endl;

                for (int i = 0; i < query_data.databuckets_size(); ++i)
                {
                    const auto &bucket = query_data.databuckets(i);
                    if (bucket.has_datacolumn())
                    {
                        const auto &column = bucket.datacolumn();

                        std::cout << "\n--- " << column.name() << " ---" << std::endl;
                        std::cout << "  Data points: " << column.datavalues_size() << std::endl;

                        // Calculate basic statistics - FIXED TO USE value_case()
                        if (column.datavalues_size() > 0)
                        {
                            std::vector<double> values;
                            for (const auto &val : column.datavalues())
                            {
                                switch (val.value_case())
                                {
                                case DataValue::kDoubleValue:
                                    values.push_back(val.doublevalue());
                                    break;
                                case DataValue::kFloatValue:
                                    values.push_back(val.floatvalue());
                                    break;
                                case DataValue::kLongValue:
                                    values.push_back(val.longvalue());
                                    break;
                                case DataValue::kIntValue:
                                    values.push_back(val.intvalue());
                                    break;
                                case DataValue::kUlongValue:
                                    values.push_back(val.ulongvalue());
                                    break;
                                case DataValue::kUintValue:
                                    values.push_back(val.uintvalue());
                                    break;
                                default:
                                    // Skip non-numeric values
                                    break;
                                }
                            }

                            if (!values.empty())
                            {
                                // Filter out NaN and infinite values for statistics
                                std::vector<double> valid_values;
                                size_t invalid_count = 0;
                                
                                for (double val : values) {
                                    if (std::isfinite(val) && !std::isnan(val)) {
                                        valid_values.push_back(val);
                                    } else {
                                        invalid_count++;
                                    }
                                }
                                
                                std::cout << "  Total values: " << values.size() << std::endl;
                                if (invalid_count > 0) {
                                    std::cout << "  Invalid values (NaN/Inf): " << invalid_count << std::endl;
                                    std::cout << "  Valid values: " << valid_values.size() << std::endl;
                                }
                                
                                if (!valid_values.empty()) {
                                    auto minmax = std::minmax_element(valid_values.begin(), valid_values.end());
                                    double sum = std::accumulate(valid_values.begin(), valid_values.end(), 0.0);
                                    double mean = sum / valid_values.size();
                                    
                                    // Calculate standard deviation
                                    double sq_sum = 0.0;
                                    for (double val : valid_values) {
                                        sq_sum += (val - mean) * (val - mean);
                                    }
                                    double std_dev = std::sqrt(sq_sum / valid_values.size());

                                    std::cout << "  Min: " << std::scientific << std::setprecision(3) << *minmax.first << std::endl;
                                    std::cout << "  Max: " << std::scientific << std::setprecision(3) << *minmax.second << std::endl;
                                    std::cout << "  Mean: " << std::scientific << std::setprecision(3) << mean << std::endl;
                                    std::cout << "  Std Dev: " << std::scientific << std::setprecision(3) << std_dev << std::endl;
                                    std::cout << "  Range: " << std::scientific << std::setprecision(3) << (*minmax.second - *minmax.first) << std::endl;
                                    
                                    // Show data quality
                                    double valid_percentage = (double)valid_values.size() / values.size() * 100.0;
                                    std::cout << "  Data Quality: " << std::fixed << std::setprecision(1) << valid_percentage << "% valid" << std::endl;
                                } else {
                                    std::cout << "  ERROR: All values are NaN or infinite!" << std::endl;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                std::cout << "No data returned from query" << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Analysis error: " << e.what() << std::endl;
            return 1;
        }

        return 0;
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