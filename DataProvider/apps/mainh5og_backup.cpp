#include "ingest_client.hpp"
#include "h5_parser.hpp"
#include <chrono>
#include <iostream>
#include <fstream>
#include <thread>

// Memory monitoring structure
struct MemoryInfo {
    size_t virtual_memory_kb = 0;
    size_t resident_memory_kb = 0;
    size_t peak_memory_kb = 0;
};

// Helper function to get current time
uint64_t getCurrentEpochSeconds() {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
}

uint64_t getCurrentEpochNanos() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

// Memory monitoring function
MemoryInfo getCurrentMemoryUsage() {
    MemoryInfo info;
    
    // Read from /proc/self/status
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
    
    return info;
}

void printMemoryUsage(const std::string& context) {
    MemoryInfo info = getCurrentMemoryUsage();
    std::cout << "[MEMORY] " << context << ": "
              << "RSS=" << (info.resident_memory_kb / 1024) << "MB, "
              << "Virtual=" << (info.virtual_memory_kb / 1024) << "MB, "
              << "Peak=" << (info.peak_memory_kb / 1024) << "MB" << std::endl;
}

// Convert H5 PV data to MLDP format
IngestDataRequest createMLDPStubFromPVData(const std::vector<CorrelatedPVData>& correlated_data,
                                           const std::string& providerId,
                                           const std::string& requestId) {
    if (correlated_data.empty()) {
        throw std::runtime_error("No correlated data to process");
    }
    
    // Get all unique PV names
    std::set<std::string> all_pv_names;
    for (const auto& snapshot : correlated_data) {
        for (const auto& [pv_name, value] : snapshot.pv_values) {
            all_pv_names.insert(pv_name);
        }
    }
    
    std::cout << "Creating MLDP stub for " << all_pv_names.size() << " PVs, " 
              << correlated_data.size() << " time points" << std::endl;
    
    // Create data columns for each PV
    std::vector<DataColumn> columns;
    std::vector<DataValue> timestamp_values;
    
    // Create timestamp column
    for (const auto& snapshot : correlated_data) {
        timestamp_values.push_back(makeDataValueWithTimestamp(
            snapshot.timestamp_seconds, snapshot.timestamp_nanoseconds));
    }
    columns.push_back(makeDataColumn("Timestamps", timestamp_values));
    
    // Create column for each PV
    for (const std::string& pv_name : all_pv_names) {
        std::vector<DataValue> pv_values;
        
        for (const auto& snapshot : correlated_data) {
            auto it = snapshot.pv_values.find(pv_name);
            if (it != snapshot.pv_values.end()) {
                // Use actual value
                pv_values.push_back(makeDataValueWithSInt32(static_cast<int32_t>(it->second)));
            } else {
                // Use 0 as default for missing values
                pv_values.push_back(makeDataValueWithSInt32(0));
            }
        }
        
        columns.push_back(makeDataColumn(pv_name, pv_values));
        std::cout << "  Added column for " << pv_name << " with " << pv_values.size() << " values" << std::endl;
    }
    
    // Create sampling clock based on data
    uint64_t start_time = correlated_data.front().timestamp_seconds;
    uint64_t start_nano = correlated_data.front().timestamp_nanoseconds;
    
    // Calculate period (assuming regular sampling)
    uint64_t period_nanos = 1000000000; // Default 1 second
    if (correlated_data.size() > 1) {
        uint64_t time_diff = correlated_data[1].timestamp_seconds - correlated_data[0].timestamp_seconds;
        period_nanos = time_diff * 1000000000; // Convert to nanoseconds
    }
    
    auto clock = makeSamplingClock(start_time, start_nano, period_nanos, correlated_data.size());
    
    // Create event metadata
    uint64_t end_time = correlated_data.back().timestamp_seconds;
    uint64_t end_nano = correlated_data.back().timestamp_nanoseconds;
   

    auto metadata = makeEventMetadata("H5 PV Time-Series Data", 
                                      start_time, start_nano,
                                      end_time, end_nano);
    
    // Create the complete ingestion request
    return makeIngestDataRequest(providerId, requestId, {}, {}, metadata, clock, columns);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <h5_path> [--local-only] [--stream-chunk-size=N]" << std::endl;
        std::cerr << "  h5_path: Directory containing H5 files or single H5 file" << std::endl;
        std::cerr << "  --local-only: Parse only, don't send to MLDP" << std::endl;
        std::cerr << "  --stream-chunk-size=N: Number of requests to send in stream (default: 10)" << std::endl;
        return 1;
    }
    
    std::string h5_path = argv[1];
    bool local_only = false;
    size_t stream_chunk_size = 10; // Number of IngestDataRequests per stream
    
    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--local-only") {
            local_only = true;
        } else if (arg.find("--stream-chunk-size=") == 0) {
            stream_chunk_size = std::stoul(arg.substr(20));
        }
    }
    
    printMemoryUsage("Application Start");
    
    std::cout << "=== H5 to MLDP Data Ingestion (Streaming) ===" << std::endl;
    std::cout << "H5 Path: " << h5_path << std::endl;
    std::cout << "Mode: " << (local_only ? "Local parsing only" : "Parse and send to MLDP (streaming)") << std::endl;
    std::cout << "Stream chunk size: " << stream_chunk_size << " requests per stream" << std::endl;
    
    // Step 1: Parse H5 files
    std::cout << "\n--- Step 1: Parsing H5 Files ---" << std::endl;
    H5Parser parser(h5_path);
    
    printMemoryUsage("Before H5 Parsing");
    
    if (!parser.parseDirectory()) {
        std::cerr << "Failed to parse H5 files from: " << h5_path << std::endl;
        return 1;
    }
    
    printMemoryUsage("After H5 Parsing");
    
    // Print summary of parsed data
    parser.printSummary();
    
    auto pv_data = parser.getPVData();
    if (pv_data.pv_series.empty()) {
        std::cerr << "No PV data found in H5 files" << std::endl;
        return 1;
    }
    
    if (local_only) {
        std::cout << "\nLocal-only mode: Parsing complete. Exiting." << std::endl;
        printMemoryUsage("Local-only Complete");
        return 0;
    }
    
    // Step 2: Connect to MLDP and register
    std::cout << "\n--- Step 2: Connecting to MLDP ---" << std::endl;
    std::string server_address = "localhost:50051";
    OspreyClient client(server_address);
    
    try {
        // Get current time for registration
        uint64_t nowSec = getCurrentEpochSeconds();
        uint64_t nowNano = getCurrentEpochNanos();
        
        // Register as data provider
        auto regReq = makeRegisterProviderRequest("H5_DataProvider_Streaming", {}, nowSec, nowNano);
        auto regResp = client.sendRegisterProvider(regReq);
        
        if (!regResp.has_registrationresult()) {
            std::cerr << "Registration with MLDP failed" << std::endl;
            return 1;
        }
        
        std::string providerId = regResp.registrationresult().providerid();
        std::cout << "Successfully registered with MLDP as provider: " << providerId << std::endl;
        
        printMemoryUsage("After MLDP Registration");
        
        // Step 3: Get correlated PV data
        std::cout << "\n--- Step 3: Preparing Data for MLDP ---" << std::endl;
        auto correlated_data = parser.getCorrelatedData(
            pv_data.earliest_time, 
            pv_data.latest_time
        );
        
        if (correlated_data.empty()) {
            std::cerr << "No correlated PV data found for ingestion" << std::endl;
            return 1;
        }
        
        std::cout << "Found " << correlated_data.size() << " correlated time snapshots" << std::endl;
        printMemoryUsage("After Data Correlation");
        
        // Step 4: Send data to MLDP using streaming
        std::cout << "\n--- Step 4: Sending Data to MLDP (Streaming) ---" << std::endl;
        
        // Data chunk size for individual requests (smaller than before)
        const size_t DATA_CHUNK_SIZE = 5000; // Smaller chunks since we're using streaming
        size_t total_requests_sent = 0;
        size_t total_data_points_sent = 0;
        
        // Process data in groups for streaming
        for (size_t stream_start = 0; stream_start < correlated_data.size(); stream_start += (DATA_CHUNK_SIZE * stream_chunk_size)) {
            
            std::cout << "\nStarting new stream batch..." << std::endl;
            printMemoryUsage("Before Stream Batch");
            
            // Prepare requests for this stream
            std::vector<IngestDataRequest> stream_requests;
            
            for (size_t req_idx = 0; req_idx < stream_chunk_size; req_idx++) {
                size_t data_start = stream_start + (req_idx * DATA_CHUNK_SIZE);
                size_t data_end = std::min(data_start + DATA_CHUNK_SIZE, correlated_data.size());
                
                if (data_start >= correlated_data.size()) {
                    break; // No more data
                }
                
                std::vector<CorrelatedPVData> chunk(
                    correlated_data.begin() + data_start, 
                    correlated_data.begin() + data_end
                );
                
                std::string requestId = "h5_stream_" + std::to_string(total_requests_sent + req_idx);
                
                try {
                    auto ingestRequest = createMLDPStubFromPVData(chunk, providerId, requestId);
                    stream_requests.push_back(std::move(ingestRequest));
                    total_data_points_sent += chunk.size();
                    
                } catch (const std::exception& e) {
                    std::cerr << "Error creating request " << requestId << ": " << e.what() << std::endl;
                    continue;
                }
            }
            
            if (stream_requests.empty()) {
                std::cout << "No more data to send." << std::endl;
                break;
            }
            
            printMemoryUsage("Before Stream Send");
            
            // Send the stream
            try {
                std::string stream_result = client.ingestDataStream(stream_requests);
                total_requests_sent += stream_requests.size();
                
                std::cout << "Stream batch complete: " << stream_result 
                          << " (sent " << stream_requests.size() << " requests, "
                          << total_requests_sent << " total requests, "
                          << total_data_points_sent << "/" << correlated_data.size() << " data points)" << std::endl;
                
                printMemoryUsage("After Stream Send");
                
                // Small delay to allow memory cleanup and prevent overwhelming the server
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
            } catch (const std::exception& e) {
                std::cerr << "Error sending stream batch: " << e.what() << std::endl;
                continue; // Try next batch
            }
        }
        
        printMemoryUsage("After All Streams");
        
        std::cout << "\n=== Streaming Ingestion Complete ===" << std::endl;
        std::cout << "Successfully sent " << total_requests_sent << " requests in streaming mode" << std::endl;
        std::cout << "Total data points sent: " << total_data_points_sent << std::endl;
        std::cout << "Data from " << pv_data.source_files.size() << " H5 files ingested" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "MLDP communication error: " << e.what() << std::endl;
        printMemoryUsage("Error State");
        return 1;
    }
    
    printMemoryUsage("Application Complete");
    return 0;
}
