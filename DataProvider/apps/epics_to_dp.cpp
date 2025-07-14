#include "epics_parser.hpp"
#include "ingest_client.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <fstream>
#include <set>
#include <algorithm>

// Memory monitoring helper (reuse from h5_to_dp.cpp)
struct MemoryInfo {
    size_t virtual_memory_kb = 0;
    size_t resident_memory_kb = 0;
    size_t peak_memory_kb = 0;
};

MemoryInfo getCurrentMemoryUsage() {
    MemoryInfo info;
    try {
        std::ifstream status_file("/proc/self/status");
        std::string line;
        while (std::getline(status_file, line)) {
            if (line.find("VmSize:") == 0) {
                sscanf(line.c_str(), "VmSize: %zu kB", &info.virtual_memory_kb);
            } else if (line.find("VmRSS:") == 0) {
                sscanf(line.c_str(), "VmRSS: %zu kB", &info.resident_memory_kb);
            } else if (line.find("VmPeak:") == 0) {
                sscanf(line.c_str(), "VmPeak: %zu kB", &info.peak_memory_kb);
            }
        }
    } catch (const std::exception& e) {
        // Memory monitoring is optional
    }
    return info;
}

void printMemoryUsage(const std::string& context) {
    MemoryInfo info = getCurrentMemoryUsage();
    std::cout << "[MEMORY] " << context << ": "
              << "RSS=" << (info.resident_memory_kb / 1024) << "MB, "
              << "Virtual=" << (info.virtual_memory_kb / 1024) << "MB, "
              << "Peak=" << (info.peak_memory_kb / 1024) << "MB" << std::endl;
}

// EPICS to MLDP conversion helpers
uint64_t getCurrentEpochSeconds() {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
}

uint64_t getCurrentEpochNanos() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

// Convert EPICSSignalData to MLDP IngestDataRequest
IngestDataRequest createMLDPRequest(const EPICSSignalData& epics_signal,
                                   const std::string& providerId,
                                   const std::string& requestId) {
    
    // Create sampling clock based on EPICS data
    SamplingClock sampling_clock;
    
    if (epics_signal.data_points.size() > 1) {
        // Calculate average period from EPICS timestamps
        uint64_t first_ns = epics_signal.data_points[0].seconds * 1000000000ULL + 
                           epics_signal.data_points[0].nanoseconds;
        uint64_t last_ns = epics_signal.data_points.back().seconds * 1000000000ULL + 
                          epics_signal.data_points.back().nanoseconds;
        
        uint64_t total_duration = last_ns - first_ns;
        uint64_t avg_period = total_duration / (epics_signal.data_points.size() - 1);
        
        sampling_clock = makeSamplingClock(
            epics_signal.data_points[0].seconds,
            epics_signal.data_points[0].nanoseconds,
            avg_period,
            static_cast<uint32_t>(epics_signal.data_points.size())
        );
    } else if (epics_signal.data_points.size() == 1) {
        // Single point
        sampling_clock = makeSamplingClock(
            epics_signal.data_points[0].seconds,
            epics_signal.data_points[0].nanoseconds,
            1000000000, // 1 second default
            1
        );
    } else {
        // No data - create dummy sampling clock
        uint64_t now_sec = getCurrentEpochSeconds();
        sampling_clock = makeSamplingClock(now_sec, 0, 1000000000, 0);
    }
    
    // Create data values from EPICS data points
    std::vector<DataValue> data_values;
    data_values.reserve(epics_signal.data_points.size());
    
    for (const auto& point : epics_signal.data_points) {
        data_values.push_back(makeDataValueWithDouble(point.value));
    }
    
    // Create data column
    auto data_column = makeDataColumn(epics_signal.info.pv_name, data_values);
    
    // Create enhanced attributes for EPICS metadata
    std::vector<Attribute> attributes;
    
    // EPICS-specific metadata
    attributes.push_back(makeAttribute("epics_pv_name", epics_signal.info.pv_name));
    attributes.push_back(makeAttribute("device", epics_signal.info.device));
    attributes.push_back(makeAttribute("device_area", epics_signal.info.area));
    attributes.push_back(makeAttribute("device_location", epics_signal.info.location));
    attributes.push_back(makeAttribute("device_attribute", epics_signal.info.attribute));
    attributes.push_back(makeAttribute("signal_type", epics_signal.info.signal_type));
    attributes.push_back(makeAttribute("units", epics_signal.info.units));
    attributes.push_back(makeAttribute("precision", epics_signal.info.precision));
    
    // Query metadata
    attributes.push_back(makeAttribute("time_range_from", epics_signal.time_range.from_iso));
    attributes.push_back(makeAttribute("time_range_to", epics_signal.time_range.to_iso));
    attributes.push_back(makeAttribute("query_status", epics_signal.query_status));
    
    // Data characteristics
    attributes.push_back(makeAttribute("sample_count", std::to_string(epics_signal.sample_count)));
    if (epics_signal.sample_count > 0) {
        attributes.push_back(makeAttribute("min_value", std::to_string(epics_signal.min_value)));
        attributes.push_back(makeAttribute("max_value", std::to_string(epics_signal.max_value)));
        attributes.push_back(makeAttribute("avg_value", std::to_string(epics_signal.avg_value)));
    }
    
    // Data source identification
    attributes.push_back(makeAttribute("data_source", "epics_archiver"));
    attributes.push_back(makeAttribute("archiver_url", "lcls-archapp01"));
    
    // Create tags
    std::vector<std::string> tags;
    tags.push_back("epics_data");
    tags.push_back("control_system");
    tags.push_back("device_" + epics_signal.info.device);
    tags.push_back("area_" + epics_signal.info.area);
    tags.push_back("type_" + epics_signal.info.signal_type);
    
    if (epics_signal.query_status == "success") {
        tags.push_back("valid_data");
    } else {
        tags.push_back("no_data");
    }
    
    // Create event metadata
    auto event_metadata = makeEventMetadata(
        "EPICS Archiver Data: " + epics_signal.info.pv_name + " (" + epics_signal.info.signal_type + ")",
        epics_signal.time_range.from_epoch,
        0,
        epics_signal.time_range.to_epoch,
        0
    );
    
    // Create the complete ingestion request
    return makeIngestDataRequest(
        providerId,
        requestId,
        attributes,
        tags,
        event_metadata,
        sampling_clock,
        {data_column}
    );
}

// Generate unique request ID for EPICS data
std::string generateRequestId(const EPICSSignalData& epics_signal, size_t batch_index = 0) {
    return "epics_" + epics_signal.info.device + "_" + epics_signal.info.area + "_" +
           epics_signal.info.location + "_" + epics_signal.info.attribute + "_" +
           std::to_string(batch_index) + "_" + std::to_string(getCurrentEpochSeconds());
}

// Individual signal ingestion
bool ingestEPICSSignalsIndividually(IngestClient& client,
                                   const std::vector<EPICSSignalData>& signals,
                                   const std::string& providerId) {
    std::cout << "\n=== Individual EPICS Signal Ingestion ===" << std::endl;
    std::cout << "Processing " << signals.size() << " signals individually" << std::endl;
    
    size_t success_count = 0;
    size_t total_data_points = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < signals.size(); ++i) {
        const auto& signal = signals[i];
        
        // Skip signals with no data
        if (signal.query_status != "success" || signal.data_points.empty()) {
            std::cout << "  Skipping " << signal.info.pv_name << " (" << signal.query_status << ")" << std::endl;
            continue;
        }
        
        try {
            // Create MLDP request
            std::string requestId = generateRequestId(signal, i);
            auto request = createMLDPRequest(signal, providerId, requestId);
            
            // Send to MLDP
            std::string result = client.ingestData(request);
            
            if (result.find("Success") != std::string::npos) {
                success_count++;
                total_data_points += signal.sample_count;
            } else {
                std::cerr << "  Failed to ingest: " << signal.info.pv_name
                          << " - " << result << std::endl;
            }
            
            // Progress update every 25 signals
            if ((i + 1) % 25 == 0) {
                std::cout << "  Progress: " << (i + 1) << "/" << signals.size()
                          << " signals processed..." << std::endl;
                printMemoryUsage("Progress Update");
            }
            
        } catch (const std::exception& e) {
            std::cerr << "  Error ingesting " << signal.info.pv_name
                      << ": " << e.what() << std::endl;
            continue;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\n=== Individual Ingestion Results ===" << std::endl;
    std::cout << "Successfully ingested: " << success_count << "/" << signals.size()
              << " signals" << std::endl;
    std::cout << "Total data points: " << total_data_points << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    
    return success_count > 0;
}

// Streaming signal ingestion
bool ingestEPICSSignalsStreaming(IngestClient& client,
                                const std::vector<EPICSSignalData>& signals,
                                const std::string& providerId,
                                size_t batch_size = 10) {
    std::cout << "\n=== Streaming EPICS Signal Ingestion ===" << std::endl;
    
    // Filter out signals with no data
    std::vector<EPICSSignalData> valid_signals;
    for (const auto& signal : signals) {
        if (signal.query_status == "success" && !signal.data_points.empty()) {
            valid_signals.push_back(signal);
        }
    }
    
    std::cout << "Processing " << valid_signals.size() << " valid signals (out of " 
              << signals.size() << " total) in batches of " << batch_size << std::endl;
    
    if (valid_signals.empty()) {
        std::cout << "No valid signals to ingest" << std::endl;
        return false;
    }
    
    // Batch processing
    size_t total_success = 0;
    size_t total_data_points = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < valid_signals.size(); i += batch_size) {
        size_t batch_end = std::min(i + batch_size, valid_signals.size());
        size_t current_batch_size = batch_end - i;
        
        std::cout << "Processing batch " << (i / batch_size + 1) << ": signals " 
                  << i << "-" << (batch_end - 1) << " (" << current_batch_size << " signals)" << std::endl;
        
        try {
            // Convert batch to MLDP requests
            std::vector<IngestDataRequest> stream_requests;
            stream_requests.reserve(current_batch_size);
            
            for (size_t j = i; j < batch_end; ++j) {
                const auto& signal = valid_signals[j];
                std::string requestId = generateRequestId(signal, j);
                auto request = createMLDPRequest(signal, providerId, requestId);
                stream_requests.push_back(std::move(request));
                total_data_points += signal.sample_count;
            }
            
            printMemoryUsage("Before Stream Send");
            
            // Send batch via streaming
            std::string result = client.ingestDataStream(stream_requests);
            
            if (result.find("Success") != std::string::npos) {
                total_success += current_batch_size;
                std::cout << "  Batch success: " << current_batch_size << " signals ingested" << std::endl;
            } else {
                std::cerr << "  Batch failed: " << result << std::endl;
            }
            
            printMemoryUsage("After Stream Send");
            
            // Small delay between batches
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
        } catch (const std::exception& e) {
            std::cerr << "  Error processing batch: " << e.what() << std::endl;
            continue;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\n=== Streaming Ingestion Results ===" << std::endl;
    std::cout << "Successfully ingested: " << total_success << "/" << valid_signals.size()
              << " signals" << std::endl;
    std::cout << "Total data points: " << total_data_points << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    
    return total_success > 0;
}

// Time range parsing helper
EPICSTimeRange parseTimeRange(const std::string& time_spec) {
    if (time_spec.find("last") == 0) {
        // Parse "last Nh" format
        std::regex pattern(R"(last\s+(\d+)h?)");
        std::smatch matches;
        if (std::regex_match(time_spec, matches, pattern)) {
            int hours = std::stoi(matches[1].str());
            return EPICSTimeUtils::createRecentTimeRange(hours);
        }
    }
    
    // Default: last 24 hours
    return EPICSTimeUtils::createRecentTimeRange(24);
}

// Progress monitoring class
class EPICSIngestionMonitor {
private:
    size_t total_pvs_;
    size_t processed_pvs_;
    size_t successful_pvs_;
    std::chrono::high_resolution_clock::time_point start_time_;
    
public:
    EPICSIngestionMonitor(size_t total_pvs)
        : total_pvs_(total_pvs), processed_pvs_(0), successful_pvs_(0),
          start_time_(std::chrono::high_resolution_clock::now()) {}
    
    void pvProcessed(bool success) {
        processed_pvs_++;
        if (success) successful_pvs_++;
        
        if (processed_pvs_ % 50 == 0 || processed_pvs_ == total_pvs_) {
            printProgress();
        }
    }
    
    void printProgress() {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
        
        double percentage = (double)processed_pvs_ / total_pvs_ * 100.0;
        double success_rate = (double)successful_pvs_ / std::max(processed_pvs_, size_t(1)) * 100.0;
        
        std::cout << "[PROGRESS] " << processed_pvs_ << "/" << total_pvs_
                  << " (" << std::fixed << std::setprecision(1) << percentage << "%) - "
                  << successful_pvs_ << " successful (" << success_rate << "%) - "
                  << elapsed.count() << "s elapsed" << std::endl;
    }
    
    void printFinalSummary() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time_);
        
        std::cout << "\n=== EPICS Ingestion Summary ===" << std::endl;
        std::cout << "Total PVs: " << total_pvs_ << std::endl;
        std::cout << "Successfully processed: " << successful_pvs_ << std::endl;
        std::cout << "Failed: " << (processed_pvs_ - successful_pvs_) << std::endl;
        std::cout << "Success rate: " << std::fixed << std::setprecision(1)
                  << ((double)successful_pvs_ / total_pvs_ * 100.0) << "%" << std::endl;
        std::cout << "Total time: " << total_time.count() << " seconds" << std::endl;
    }
};

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command> [options]" << std::endl;
        std::cerr << "Commands:" << std::endl;
        std::cerr << "  discover [pattern]                  - Discover available EPICS PVs" << std::endl;
        std::cerr << "  test                               - Test EPICS archiver connection" << std::endl;
        std::cerr << "  ingest <pattern> [time_range]      - Ingest PVs matching pattern" << std::endl;
        std::cerr << "  ingest-list <file> [time_range]    - Ingest PVs from file list" << std::endl;
        std::cerr << "  ingest-device <device> [time_range] - Ingest all PVs for device" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  --time-range <spec>                - Time range (e.g., 'last 6h', 'last 2d')" << std::endl;
        std::cerr << "  --limit <N>                        - Limit number of PVs (default: 100)" << std::endl;
        std::cerr << "  --streaming                        - Use streaming ingestion" << std::endl;
        std::cerr << "  --batch-size <N>                   - Batch size for streaming (default: 10)" << std::endl;
        std::cerr << "  --mldp-server <address>            - MLDP server address (default: localhost:50051)" << std::endl;
        std::cerr << "  --epics-mgmt <url>                 - EPICS management URL" << std::endl;
        std::cerr << "  --epics-data <url>                 - EPICS data retrieval URL" << std::endl;
        std::cerr << "  --verbose                          - Verbose output" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Examples:" << std::endl;
        std::cerr << "  " << argv[0] << " discover \"IOC.*\"" << std::endl;
        std::cerr << "  " << argv[0] << " ingest \"BPMS:.*\" --time-range \"last 12h\"" << std::endl;
        std::cerr << "  " << argv[0] << " ingest-device KLYS --streaming --limit 50" << std::endl;
        return 1;
    }
    
    std::string command = argv[1];
    
    // Parse command line arguments
    std::string pattern = ".*";
    std::string time_range_spec = "last 24h";
    size_t limit = 100;
    bool use_streaming = false;
    size_t batch_size = 10;
    bool verbose = false;
    std::string mldp_server = "localhost:50051";
    std::string epics_mgmt = "http://lcls-archapp01:17665/mgmt/bpl/";
    std::string epics_data = "http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json";
    std::string pv_list_file;
    
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--time-range" && i + 1 < argc) {
            time_range_spec = argv[++i];
        } else if (arg == "--limit" && i + 1 < argc) {
            limit = std::stoul(argv[++i]);
        } else if (arg == "--streaming") {
            use_streaming = true;
        } else if (arg == "--batch-size" && i + 1 < argc) {
            batch_size = std::stoul(argv[++i]);
        } else if (arg == "--mldp-server" && i + 1 < argc) {
            mldp_server = argv[++i];
        } else if (arg == "--epics-mgmt" && i + 1 < argc) {
            epics_mgmt = argv[++i];
        } else if (arg == "--epics-data" && i + 1 < argc) {
            epics_data = argv[++i];
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (pattern == ".*" && command != "test") {
            // First non-option argument is the pattern/file
            if (command == "ingest-list") {
                pv_list_file = arg;
            } else {
                pattern = arg;
            }
        }
    }
    
    try {
        printMemoryUsage("Application Start");
        
        std::cout << "=== EPICS to MongoDB Ingestion ===" << std::endl;
        std::cout << "EPICS Management URL: " << epics_mgmt << std::endl;
        std::cout << "EPICS Data URL: " << epics_data << std::endl;
        std::cout << "MLDP Server: " << mldp_server << std::endl;
        std::cout << "Mode: " << (use_streaming ? "Streaming ingestion" : "Individual ingestion") << std::endl;
        
        // Initialize EPICS parser
        EPICSParser parser(epics_mgmt, epics_data);
        parser.setVerbose(verbose);
        
        // === COMMAND: TEST ===
        if (command == "test") {
            std::cout << "\n--- Testing EPICS Connection ---" << std::endl;
            bool success = parser.testConnection();
            return success ? 0 : 1;
        }
        
        // === COMMAND: DISCOVER ===
        if (command == "discover") {
            std::cout << "\n--- Step 1: EPICS PV Discovery ---" << std::endl;
            auto discovered_pvs = parser.discoverPVs(pattern, limit);
            
            if (discovered_pvs.empty()) {
                std::cout << "No PVs found matching pattern: " << pattern << std::endl;
                return 1;
            }
            
            parser.printDiscoveryStats(discovered_pvs);
            
            std::cout << "\nFirst 20 PVs:" << std::endl;
            for (size_t i = 0; i < std::min(size_t(20), discovered_pvs.size()); i++) {
                std::cout << "  " << (i + 1) << ". " << discovered_pvs[i] << std::endl;
            }
            
            return 0;
        }
        
        // === INGESTION COMMANDS ===
        if (command == "ingest" || command == "ingest-device" || command == "ingest-list") {
            std::cout << "\n--- Step 1: EPICS PV Discovery ---" << std::endl;
            
            std::vector<std::string> target_pvs;
            
            if (command == "ingest-list") {
                // Read PV names from file
                std::ifstream file(pv_list_file);
                if (!file.is_open()) {
                    std::cerr << "Failed to open PV list file: " << pv_list_file << std::endl;
                    return 1;
                }
                
                std::string pv_name;
                while (std::getline(file, pv_name)) {
                    if (!pv_name.empty() && pv_name[0] != '#') { // Skip comments
                        target_pvs.push_back(pv_name);
                    }
                }
                file.close();
                
                std::cout << "Loaded " << target_pvs.size() << " PVs from file: " << pv_list_file << std::endl;
                
            } else if (command == "ingest-device") {
                // Discover PVs for specific device
                target_pvs = parser.discoverByDevice(pattern, limit);
                std::cout << "Found " << target_pvs.size() << " PVs for device: " << pattern << std::endl;
                
            } else {
                // Regular pattern-based discovery
                target_pvs = parser.discoverPVs(pattern, limit);
                std::cout << "Found " << target_pvs.size() << " PVs matching pattern: " << pattern << std::endl;
            }
            
            if (target_pvs.empty()) {
                std::cout << "No PVs to process" << std::endl;
                return 1;
            }
            
            // Apply limit if specified
            if (target_pvs.size() > limit) {
                std::cout << "Limiting to first " << limit << " PVs" << std::endl;
                target_pvs.resize(limit);
            }
            
            parser.printDiscoveryStats(target_pvs);
            
            std::cout << "\n--- Step 2: EPICS Data Query ---" << std::endl;
            
            // Parse time range
            EPICSTimeRange time_range = parseTimeRange(time_range_spec);
            std::cout << "Time range: " << time_range.from_iso << " to " << time_range.to_iso << std::endl;
            
            printMemoryUsage("Before EPICS Query");
            
            // Query EPICS data
            auto epics_signals = parser.queryMultiplePVs(target_pvs, time_range, true);
            
            printMemoryUsage("After EPICS Query");
            
            parser.printDataStats(epics_signals);
            
            if (epics_signals.empty()) {
                std::cout << "No data retrieved from EPICS" << std::endl;
                return 1;
            }
            
            std::cout << "\n--- Step 3: MLDP Registration ---" << std::endl;
            
            // Connect to MLDP and register provider
            IngestClient client(mldp_server);
            
            uint64_t nowSec = getCurrentEpochSeconds();
            uint64_t nowNano = getCurrentEpochNanos();
            
            std::vector<Attribute> provider_attributes;
            provider_attributes.push_back(makeAttribute("data_source", "epics_archiver"));
            provider_attributes.push_back(makeAttribute("archiver_management", epics_mgmt));
            provider_attributes.push_back(makeAttribute("archiver_retrieval", epics_data));
            provider_attributes.push_back(makeAttribute("time_range", time_range_spec));
            provider_attributes.push_back(makeAttribute("query_pattern", pattern));
            provider_attributes.push_back(makeAttribute("signal_count", std::to_string(epics_signals.size())));
            provider_attributes.push_back(makeAttribute("parser_version", "epics_parser_v1.0"));
            
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
            
            printMemoryUsage("After MLDP Registration");
            
            std::cout << "\n--- Step 4: Data Ingestion ---" << std::endl;
            
            // Ingest signals
            bool success;
            if (use_streaming) {
                success = ingestEPICSSignalsStreaming(client, epics_signals, providerId, batch_size);
            } else {
                success = ingestEPICSSignalsIndividually(client, epics_signals, providerId);
            }
            
            printMemoryUsage("After Data Ingestion");
            
            // Final summary
            std::cout << "\n=== Final Summary ===" << std::endl;
            if (success) {
                std::cout << "✓ EPICS ingestion completed successfully" << std::endl;
                std::cout << "✓ Provider ID: " << providerId << std::endl;
                std::cout << "✓ Mode: " << (use_streaming ? "Streaming" : "Individual") << std::endl;
                std::cout << "✓ Time range: " << time_range_spec << std::endl;
                std::cout << "✓ PVs processed: " << target_pvs.size() << std::endl;
                
                // Calculate total data points from successful signals
                size_t total_points = 0;
                size_t successful_signals = 0;
                for (const auto& signal : epics_signals) {
                    if (signal.query_status == "success") {
                        total_points += signal.sample_count;
                        successful_signals++;
                    }
                }
                std::cout << "✓ Successful signals: " << successful_signals << std::endl;
                std::cout << "✓ Total data points: " << total_points << std::endl;
                
                return 0;
            } else {
                std::cerr << "✗ EPICS ingestion failed" << std::endl;
                return 1;
            }
        }
        
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        printMemoryUsage("Error State");
        return 1;
    }
}
