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
        std::cout << "ENHANCED LCLS DATA PLATFORM CLI TOOLKIT\n";
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
        std::cout << "  help                    Show this help\n\n";
        std::cout << "EXAMPLES:\n";
        std::cout << "  DataProvider_CLI discover \".*BPM.*\"\n";
        std::cout << "  DataProvider_CLI view BPMS_DMPH_502_TMITBR --last 1h\n";
        std::cout << "  DataProvider_CLI export \".*TMIT.*\" --pytorch\n";
        std::cout << "  DataProvider_CLI ingest /data/h5_files/ --project CoAD\n";
        std::cout << "  DataProvider_CLI decode PV_NAME --csv data.csv    # Saves to exports/csv/\n";
        std::cout << "  DataProvider_CLI export discover                  # Saves to exports/numpy/\n";
        std::cout << "  DataProvider_CLI tools                    # List all tools\n";
        std::cout << "  DataProvider_CLI tools decode             # Show data_decoder help\n\n";
        std::cout << "OUTPUT LOCATIONS:\n";
        std::cout << "  CSV files:     build/exports/csv/\n";
        std::cout << "  JSON files:    build/exports/json/\n";
        std::cout << "  NumPy files:   build/exports/numpy/\n";
        std::cout << "  PyTorch data:  build/exports/pytorch/\n";
        std::cout << "  HDF5 files:    build/exports/hdf5/\n";
    }

    int runCommand(const std::vector<std::string> &args)
    {
        if (args.empty() || args[0] == "help" || args[0] == "--help")
        {
            showHelp();
            return 0;
        }

        std::string command = args[0];

        if (command == "discover")
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
            std::cerr << "Usage: DataProvider_CLI view <pv> [time_range]" << std::endl;
            return 1;
        }

        std::string pv = args[1];
        std::string timeRange = args.size() > 2 ? args[2] : "last 1h";

        std::cout << "Viewing data for PV: " << pv << " (" << timeRange << ")" << std::endl;

        try
        {
            QueryClient queryClient(config_["query_server"]);

            // Calculate time range (simplified - use last hour for now)
            uint64_t now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
            uint64_t start_time = now - 3600; // 1 hour ago
            uint64_t end_time = now;

            // Create timestamps
            auto begin_time = makeTimestamp(start_time, 0);
            auto end_time_ts = makeTimestamp(end_time, 0);

            // Query data for the PV
            std::vector<std::string> pv_names = {pv};
            auto request = makeQueryDataRequest(pv_names, begin_time, end_time_ts, false);
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
            std::cerr << "Usage: DataProvider_CLI decode <pv>" << std::endl;
            return 1;
        }

        // Build command for data_decoder with correct path
        std::string cmd = getToolPath("data_decoder");
        cmd += " decode";

        // Process arguments and handle file outputs
        for (size_t i = 1; i < args.size(); ++i)
        {
            std::string arg = args[i];

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

        std::cout << "Executing: " << cmd << std::endl;
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
            std::cerr << "Usage: DataProvider_CLI analyze <pvs>" << std::endl;
            std::cerr << "  Example: DataProvider_CLI analyze \"BPMS_DMPH_502_TMITBR,BPMS_LTUH_250_XBR\"" << std::endl;
            return 1;
        }

        std::string pv_list = args[1];
        std::cout << "Running data analysis for PVs: " << pv_list << std::endl;

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

            // Get last hour of data
            uint64_t now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
            uint64_t start_time = now - 3600; // 1 hour ago

            auto begin_time = makeTimestamp(start_time, 0);
            auto end_time = makeTimestamp(now, 0);

            auto request = makeQueryDataRequest(pv_names, begin_time, end_time, false);
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
                std::cout << "Time range: Last 1 hour" << std::endl;
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
                                auto minmax = std::minmax_element(values.begin(), values.end());
                                double sum = std::accumulate(values.begin(), values.end(), 0.0);
                                double mean = sum / values.size();

                                std::cout << "  Min: " << *minmax.first << std::endl;
                                std::cout << "  Max: " << *minmax.second << std::endl;
                                std::cout << "  Mean: " << mean << std::endl;
                                std::cout << "  Range: " << (*minmax.second - *minmax.first) << std::endl;
                            }
                        }
                    }
                }
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
