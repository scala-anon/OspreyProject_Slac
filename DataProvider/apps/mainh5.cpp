#include "ingest_client.hpp"
#include "h5_parser.hpp"
#include <chrono>
#include <iostream>

// Helper function to get current time
uint64_t getCurrentEpochSeconds() {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
}

uint64_t getCurrentEpochNanos() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
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
        std::cerr << "Usage: " << argv[0] << " <h5_path> [--local-only]" << std::endl;
        std::cerr << "  h5_path: Directory containing H5 files or single H5 file" << std::endl;
        std::cerr << "  --local-only: Parse only, don't send to MLDP" << std::endl;
        return 1;
    }
    
    std::string h5_path = argv[1];
    bool local_only = (argc > 2 && std::string(argv[2]) == "--local-only");
    
    std::cout << "=== H5 to MLDP Data Ingestion ===" << std::endl;
    std::cout << "H5 Path: " << h5_path << std::endl;
    std::cout << "Mode: " << (local_only ? "Local parsing only" : "Parse and send to MLDP") << std::endl;
    
    // Step 1: Parse H5 files
    std::cout << "\n--- Step 1: Parsing H5 Files ---" << std::endl;
    H5Parser parser(h5_path);
    
    if (!parser.parseDirectory()) {
        std::cerr << "Failed to parse H5 files from: " << h5_path << std::endl;
        return 1;
    }
    
    // Print summary of parsed data
    parser.printSummary();
    
    auto pv_data = parser.getPVData();
    if (pv_data.pv_series.empty()) {
        std::cerr << "No PV data found in H5 files" << std::endl;
        return 1;
    }
    
    if (local_only) {
        std::cout << "\nLocal-only mode: Parsing complete. Exiting." << std::endl;
        return 0;
    }
    
    // Step 2: Connect to MLDP and register
    std::cout << "\n--- Step 2: Connecting to MLDP ---" << std::endl;
    std::string server_address = "localhost:50051"; //TODO change later once mldp runs on ssh 
    OspreyClient client(server_address);
    
    try {
        // Get current time for registration
        uint64_t nowSec = getCurrentEpochSeconds();
        uint64_t nowNano = getCurrentEpochNanos();
        
        // Register as data provider
        auto regReq = makeRegisterProviderRequest("H5_DataProvider", {}, nowSec, nowNano);
        auto regResp = client.sendRegisterProvider(regReq);
        
        if (!regResp.has_registrationresult()) {
            std::cerr << "Registration with MLDP failed" << std::endl;
            return 1;
        }
        
        std::string providerId = regResp.registrationresult().providerid();
        std::cout << "Successfully registered with MLDP as provider: " << providerId << std::endl;
        
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
        
        // Step 4: Send data to MLDP
        std::cout << "\n--- Step 4: Sending Data to MLDP ---" << std::endl;
        
        // For large datasets, you might want to send in chunks
        const size_t CHUNK_SIZE = 1000; // Adjust based on your needs
        size_t total_sent = 0;
        
        for (size_t i = 0; i < correlated_data.size(); i += CHUNK_SIZE) {
            size_t end_idx = std::min(i + CHUNK_SIZE, correlated_data.size());
            
            std::vector<CorrelatedPVData> chunk(
                correlated_data.begin() + i, 
                correlated_data.begin() + end_idx
            );
            
            std::string requestId = "h5_chunk_" + std::to_string(i / CHUNK_SIZE);
            
            try {
                auto ingestRequest = createMLDPStubFromPVData(chunk, providerId, requestId);
                std::string result = client.ingestData(ingestRequest);
                
                total_sent += chunk.size();
                std::cout << "Chunk " << (i / CHUNK_SIZE + 1) << ": " << result 
                          << " (" << total_sent << "/" << correlated_data.size() << " snapshots)" << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "Error sending chunk " << (i / CHUNK_SIZE + 1) << ": " << e.what() << std::endl;
                continue; // Try next chunk
            }
        }
        
        std::cout << "\n=== Ingestion Complete ===" << std::endl;
        std::cout << "Successfully sent " << total_sent << " time snapshots to MLDP" << std::endl;
        std::cout << "Data from " << pv_data.source_files.size() << " H5 files ingested" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "MLDP communication error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
