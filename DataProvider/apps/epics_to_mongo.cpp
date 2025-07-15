#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include "parsers/epics_archiver.hpp"
#include "clients/ingest_client.hpp"

struct CommandArgs {
    std::string command;
    std::string pv_name;
    std::string start_time;
    std::string end_time;
    bool verbose = false;
    std::string mldp_server = "localhost:50051";
};

void printUsage(const char* program_name) {
    std::cout << "=== EPICS Archiver to MongoDB Integration ===" << std::endl;
    std::cout << "Fetches EPICS archiver data and ingests to MongoDB via MLDP" << std::endl;
    std::cout << "Usage: " << program_name << " <command> [options]" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Commands:" << std::endl;
    std::cout << "  test                                    - Test EPICS archiver connection" << std::endl;
    std::cout << "  fetch <pv_name> <time_range>           - Fetch data (no ingestion)" << std::endl;
    std::cout << "  ingest <pv_name> <time_range>          - Fetch data and ingest to MLDP" << std::endl;
    std::cout << std::endl;

    std::cout << "Time Range Options:" << std::endl;
    std::cout << "  today                                   - From start of today to now" << std::endl;
    std::cout << "  yesterday                               - All of yesterday (24 hours)" << std::endl;
    std::cout << "  \"YYYY-MM-DDTHH:MM:SS.000Z\"             - ISO 8601 format" << std::endl;
    std::cout << "  \"start_time\" \"end_time\"                - Two separate ISO timestamps" << std::endl;

    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  # Test connection" << std::endl;
    std::cout << "  " << program_name << " test" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "  # Fetch data for yesterday" << std::endl;
    std::cout << "  " << program_name << " fetch IOC:BSY0:BP01:MEM_FREE yesterday" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "  # Ingest data for yesterday" << std::endl;
    std::cout << "  " << program_name << " ingest IOC:BSY0:BP01:MEM_FREE yesterday" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "  # Fetch data for today" << std::endl;
    std::cout << "  " << program_name << " fetch IOC:BSY0:BP01:MEM_FREE today" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "  # Fetch with specific time range" << std::endl;
    std::cout << "  " << program_name << " fetch BPMS:LI21:233:X \"2025-07-14T10:00:00.000Z\" \"2025-07-14T12:00:00.000Z\"" << std::endl;

    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --verbose, -v                          - Enable verbose output" << std::endl;
    std::cout << "  --mldp-server=ADDRESS                  - MLDP server address (default: localhost:50051)" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: This is a working implementation with JSON parsing" << std::endl;
}

CommandArgs parseArgs(int argc, char* argv[]) {
    CommandArgs args;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "test") {
            args.command = "test";
        } else if (arg == "fetch") {
            args.command = "fetch";
        } else if (arg == "ingest") {
            args.command = "ingest";
        } else if (arg == "--verbose" || arg == "-v") {
            args.verbose = true;
        } else if (arg.find("--mldp-server=") == 0) {
            args.mldp_server = arg.substr(14);
        } else if (args.pv_name.empty()) {
            args.pv_name = arg;
        } else if (args.start_time.empty()) {
            args.start_time = arg;
        } else if (args.end_time.empty()) {
            args.end_time = arg;
        }
    }
    
    // Handle single time argument - expand keywords to proper ranges
    if (!args.start_time.empty() && args.end_time.empty()) {
        if (args.start_time == "today") {
            // Today: from start of today to now
            args.end_time = "now";
        } else if (args.start_time == "yesterday") {
            // Yesterday: from start of yesterday to end of yesterday
            auto now = std::chrono::system_clock::now();
            auto yesterday = now - std::chrono::hours(24);
            auto yesterday_start = yesterday; // yesterday is already 24 hours ago

            auto yesterday_start_t = std::chrono::system_clock::to_time_t(yesterday_start);
            auto yesterday_end_t = std::chrono::system_clock::to_time_t(yesterday);

            std::tm gmt_start = *std::gmtime(&yesterday_start_t);
            std::tm gmt_end = *std::gmtime(&yesterday_end_t);

            // Set to start of day for start time
            gmt_start.tm_hour = 0;
            gmt_start.tm_min = 0;
            gmt_start.tm_sec = 0;

            // Set to end of day for end time
            gmt_end.tm_hour = 23;
            gmt_end.tm_min = 59;
            gmt_end.tm_sec = 59;

            char start_buf[32], end_buf[32];
            std::strftime(start_buf, sizeof(start_buf), "%Y-%m-%dT%H:%M:%S.000Z", &gmt_start);
            std::strftime(end_buf, sizeof(end_buf), "%Y-%m-%dT%H:%M:%S.000Z", &gmt_end);

            args.start_time = start_buf;
            args.end_time = end_buf;
        } else {
            // For specific ISO time, default end to now
            args.end_time = "now";
        }
    }
    
    return args;
}

bool validateArgs(const CommandArgs& args) {
    if (args.command.empty()) {
        std::cerr << "Error: No command specified." << std::endl;
        return false;
    }
    
    if (args.command == "fetch" || args.command == "ingest") {
        if (args.pv_name.empty()) {
            std::cerr << "Error: PV name required for " << args.command << " command." << std::endl;
            return false;
        }
        if (args.start_time.empty()) {
            std::cerr << "Error: Time range required for " << args.command << " command." << std::endl;
            return false;
        }
    }
    
    return true;
}

int runTestCommand(const CommandArgs& args) {
    std::cout << "\n=== EPICS Archiver Connection Test ===" << std::endl;
    
    EPICSArchiver archiver("http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json", args.verbose);
    
    if (archiver.testConnection()) {
        std::cout << "✅ EPICS archiver connection successful" << std::endl;
        return 0;
    } else {
        std::cout << "❌ EPICS archiver connection failed" << std::endl;
        return 1;
    }
}

int runFetchCommand(const CommandArgs& args) {
    std::cout << "\n=== EPICS Data Fetch (No Ingestion) ===" << std::endl;
    
    EPICSArchiver archiver("http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json", args.verbose);
    
    EPICSTimeSeries series = archiver.fetchPVData(args.pv_name, args.start_time, args.end_time);
    
    if (series.empty()) {
        std::cout << "No data retrieved for " << args.pv_name << std::endl;
        return 1;
    }

    std::cout << "Successfully fetched data for " << args.pv_name << std::endl;
    archiver.printDataStatistics(series);

    return 0;
}

int runIngestCommand(const CommandArgs& args) {
    std::cout << "\n=== EPICS Data Ingestion to MLDP ===" << std::endl;
    
    // Step 1: Fetch data from EPICS archiver
    EPICSArchiver archiver("http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json", args.verbose);
    
    std::cout << "Fetching data for PV: " << args.pv_name << std::endl;
    std::cout << "Time range: " << args.start_time << " to " << args.end_time << std::endl;
    std::cout << "MLDP server: " << args.mldp_server << std::endl;
    
    EPICSTimeSeries series = archiver.fetchPVData(args.pv_name, args.start_time, args.end_time);
    
    if (series.empty()) {
        std::cout << "No data retrieved for " << args.pv_name << std::endl;
        return 1;
    }
    
    std::cout << "Retrieved " << series.total_points() << " data points" << std::endl;
    
    // Step 2: Connect to MLDP and register provider
    try {
        IngestClient client(args.mldp_server);
        
        // Create provider registration (following h5_to_dp pattern)
        std::vector<Attribute> provider_attributes;
        provider_attributes.push_back(makeAttribute("source", "EPICS_Archiver"));
        provider_attributes.push_back(makeAttribute("pv_name", args.pv_name));
        provider_attributes.push_back(makeAttribute("data_points", std::to_string(series.total_points())));
        provider_attributes.push_back(makeAttribute("units", series.metadata.units));
        provider_attributes.push_back(makeAttribute("precision", series.metadata.precision));
        provider_attributes.push_back(makeAttribute("archiver_url", "http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json"));
        
        uint64_t nowSec = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
        uint64_t nowNano = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
            
        auto regReq = makeRegisterProviderRequest("EPICS_Archiver_Provider", provider_attributes, nowSec, nowNano);
        auto regResp = client.sendRegisterProvider(regReq);
        
        if (!regResp.has_registrationresult()) {
            std::cerr << "Failed to register with MLDP" << std::endl;
            if (regResp.has_exceptionalresult()) {
                std::cerr << "Error: " << regResp.exceptionalresult().message() << std::endl;
            }
            return 1;
        }
        
        std::string providerId = regResp.registrationresult().providerid();
        std::cout << "Successfully registered as provider: " << providerId << std::endl;
        
        // Step 3: Convert EPICS data to MLDP format (following h5_to_dp pattern)
        std::cout << "Converting EPICS data to MLDP format..." << std::endl;
        
        // Create data values
        std::vector<DataValue> data_values;
        data_values.reserve(series.data_points.size());
        
        for (const auto& point : series.data_points) {
            data_values.push_back(makeDataValueWithDouble(point.value));
        }
        
        // Create sampling clock
        uint64_t period_nanos = 1000000000; // Default 1 second
        if (series.data_points.size() > 1) {
            uint64_t total_duration = series.end_time() - series.start_time();
            if (total_duration > 0) {
                period_nanos = (total_duration * 1000000000ULL) / (series.data_points.size() - 1);
            }
        }
        
        auto sampling_clock = makeSamplingClock(series.start_time(), 0, period_nanos, 
                                              static_cast<uint32_t>(series.data_points.size()));
        
        // Create data column
        auto data_column = makeDataColumn(args.pv_name, data_values);
        
        // Create event metadata
        auto event_metadata = makeEventMetadata(
            "EPICS Archiver Data: " + args.pv_name,
            series.start_time(), 0,
            series.end_time(), 0
        );
        
        // Create enhanced attributes
        std::vector<Attribute> attributes;
        attributes.push_back(makeAttribute("source", "EPICS_Archiver"));
        attributes.push_back(makeAttribute("pv_name", args.pv_name));
        attributes.push_back(makeAttribute("units", series.metadata.units));
        attributes.push_back(makeAttribute("precision", series.metadata.precision));
        attributes.push_back(makeAttribute("time_range", args.start_time + " to " + args.end_time));
        attributes.push_back(makeAttribute("sample_count", std::to_string(series.total_points())));
        
        // Create tags
        std::vector<std::string> tags;
        tags.push_back("epics_archiver");
        tags.push_back("pv_data");
        tags.push_back("time_series");
        
        // Create request ID (clean PV name for MongoDB)
        std::string requestId = "epics_" + args.pv_name + "_" + std::to_string(nowSec);
        std::replace(requestId.begin(), requestId.end(), ':', '_');
        
        // Create ingestion request
        auto ingestRequest = makeIngestDataRequest(providerId, requestId, attributes, tags, 
                                                  event_metadata, sampling_clock, {data_column});
        
        // Step 4: Send to MLDP (following h5_to_dp pattern)
        std::cout << "Sending " << series.total_points() << " data points to MLDP..." << std::endl;
        std::string result = client.ingestData(ingestRequest);
        
        if (result.find("Success") != std::string::npos) {
            std::cout << "Successfully ingested " << series.total_points() 
                      << " data points for " << args.pv_name << std::endl;
            std::cout << "   Provider ID: " << providerId << std::endl;
            std::cout << "   Request ID: " << requestId << std::endl;
            std::cout << "   Time range: " << epics_time_utils::formatTimestamp(series.start_time(), 0)
                      << " to " << epics_time_utils::formatTimestamp(series.end_time(), 0) << std::endl;
            std::cout << "   Data is now available in MongoDB for querying" << std::endl;
            return 0;
        } else {
            std::cerr << "Ingestion failed: " << result << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "MLDP connection error: " << e.what() << std::endl;
        std::cerr << "Make sure MLDP is running on " << args.mldp_server << std::endl;
        return 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    CommandArgs args = parseArgs(argc, argv);

    if (!validateArgs(args)) {
        std::cout << "\nUse '" << argv[0] << " --help' for usage information." << std::endl;
        return 1;
    }

    if (args.command == "test") {
        return runTestCommand(args);
    } else if (args.command == "fetch") {
        return runFetchCommand(args);
    } else if (args.command == "ingest") {
        return runIngestCommand(args);
    } else {
        std::cerr << "Error: Unknown command '" << args.command << "'" << std::endl;
        std::cerr << "Use '" << argv[0] << " --help' for usage information." << std::endl;
        return 1;
    }
}
