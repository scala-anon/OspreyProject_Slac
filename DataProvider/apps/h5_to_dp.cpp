#include "h5_parser.hpp"
#include "ingest_client.hpp" // Your existing MLDP client
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <fstream>
#include <set>

// Memory monitoring helper
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

// MLDP ingestion helper functions
uint64_t getCurrentEpochSeconds() {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
}

uint64_t getCurrentEpochNanos() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

// Convert SignalData to MLDP IngestDataRequest
IngestDataRequest createMLDPRequest(const SignalData& signal,
                                   const std::string& providerId,
                                   const std::string& requestId) {

    // Create sampling clock based on regularity
    SamplingClock sampling_clock;

    if (signal.timestamps->is_regular_sampling && signal.timestamps->count > 1) {
        // Use regular sampling clock
        sampling_clock = makeSamplingClock(
            signal.timestamps->start_time_sec,
            signal.timestamps->start_time_nano,
            signal.timestamps->period_nanos,
            static_cast<uint32_t>(signal.timestamps->count)
        );
    } else {
        // For irregular sampling, we'll need to create a different approach
        // Since your API only supports SamplingClock, we'll use average period
        uint64_t avg_period = signal.timestamps->period_nanos;
        if (avg_period == 0) avg_period = 1000000000; // Default 1 second

        sampling_clock = makeSamplingClock(
            signal.timestamps->start_time_sec,
            signal.timestamps->start_time_nano,
            avg_period,
            static_cast<uint32_t>(signal.timestamps->count)
        );
    }

    // Create data values
    std::vector<DataValue> data_values;
    data_values.reserve(signal.values.size());

    for (double value : signal.values) {
        data_values.push_back(makeDataValueWithDouble(value));
    }

    // Create data column
    auto data_column = makeDataColumn(signal.info.full_name, data_values);

    // Create enhanced attributes - using correct member names
    std::vector<Attribute> attributes;

    // Device metadata
    attributes.push_back(makeAttribute("device", signal.info.device));
    attributes.push_back(makeAttribute("device_area", signal.info.device_area));
    attributes.push_back(makeAttribute("device_location", signal.info.device_location));
    attributes.push_back(makeAttribute("device_attribute", signal.info.device_attribute));
    attributes.push_back(makeAttribute("signal_type", signal.info.signal_type));
    attributes.push_back(makeAttribute("units", signal.info.units));

    // H5 attributes
    if (!signal.info.label.empty()) {
        attributes.push_back(makeAttribute("h5_label", signal.info.label));
    }
    if (!signal.info.matlab_class.empty()) {
        attributes.push_back(makeAttribute("matlab_class", signal.info.matlab_class));
    }

    // File metadata
    attributes.push_back(makeAttribute("origin", signal.file_metadata.origin));
    attributes.push_back(makeAttribute("pathway", signal.file_metadata.pathway));
    attributes.push_back(makeAttribute("project", signal.file_metadata.project));
    attributes.push_back(makeAttribute("acquisition_date", signal.file_metadata.date));
    attributes.push_back(makeAttribute("acquisition_time", signal.file_metadata.time));

    // Data characteristics
    attributes.push_back(makeAttribute("sample_count", std::to_string(signal.values.size())));
    attributes.push_back(makeAttribute("sampling_type",
                                     signal.timestamps->is_regular_sampling ? "regular" : "irregular"));
    if (signal.timestamps->is_regular_sampling && signal.timestamps->period_nanos > 0) {
        double freq_hz = 1000000000.0 / signal.timestamps->period_nanos;
        attributes.push_back(makeAttribute("sampling_rate_hz", std::to_string(freq_hz)));
    }

    // Create tags
    std::vector<std::string> tags;
    tags.push_back("h5_data");
    tags.push_back("device_" + signal.info.device);
    tags.push_back("device_area_" + signal.info.device_area);
    tags.push_back("project_" + signal.file_metadata.project);
    tags.push_back("signal_type_" + signal.info.signal_type);

    if (signal.timestamps->is_regular_sampling) {
        tags.push_back("regular_sampling");
    } else {
        tags.push_back("irregular_sampling");
    }

    // Create event metadata
    auto event_metadata = makeEventMetadata(
        "H5 Signal Data: " + signal.info.full_name + " from " + signal.file_metadata.project,
        signal.timestamps->start_time_sec,
        signal.timestamps->start_time_nano,
        signal.timestamps->end_time_sec,
        signal.timestamps->end_time_nano
    );

    // Create the complete ingestion request using your existing API
    return makeIngestDataRequest(
        providerId,
        requestId,
        attributes,
        tags,
        event_metadata,
        sampling_clock,  // Use SamplingClock directly
        {data_column}
    );
}

// Batch signals for streaming
std::vector<std::vector<SignalData>> chunkSignals(const std::vector<SignalData>& signals,
                                                  size_t chunk_size) {
    std::vector<std::vector<SignalData>> chunks;

    for (size_t i = 0; i < signals.size(); i += chunk_size) {
        size_t end = std::min(i + chunk_size, signals.size());
        chunks.emplace_back(signals.begin() + i, signals.begin() + end);
    }

    return chunks;
}

// Generate unique request ID - Fixed to use correct member names
std::string generateRequestId(const SignalData& signal, size_t batch_index = 0) {
    return "h5_" + signal.file_metadata.project + "_" +
           signal.info.device + "_" + signal.info.device_area + "_" +
           signal.info.device_location + "_" + signal.info.device_attribute + "_" +
           std::to_string(batch_index) + "_" + std::to_string(getCurrentEpochSeconds());
}

// Individual signal ingestion
bool ingestSignalsIndividually(IngestClient& client,
                              const std::vector<SignalData>& signals,
                              const std::string& providerId) {
    std::cout << "\n=== Individual Signal Ingestion ===" << std::endl;
    std::cout << "Processing " << signals.size() << " signals individually" << std::endl;

    size_t success_count = 0;
    size_t total_data_points = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < signals.size(); ++i) {
        const auto& signal = signals[i];

        try {
            // Create MLDP request
            std::string requestId = generateRequestId(signal, i);
            auto request = createMLDPRequest(signal, providerId, requestId);

            // Send to MLDP
            std::string result = client.ingestData(request);

            if (result.find("Success") != std::string::npos) {
                success_count++;
                total_data_points += signal.values.size();
            } else {
                std::cerr << "  Failed to ingest: " << signal.info.full_name
                          << " - " << result << std::endl;
            }

            // Progress update every 50 signals
            if ((i + 1) % 50 == 0) {
                std::cout << "  Progress: " << (i + 1) << "/" << signals.size()
                          << " signals processed..." << std::endl;
                printMemoryUsage("Progress Update");
            }

        } catch (const std::exception& e) {
            std::cerr << "  Error ingesting " << signal.info.full_name
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
bool ingestSignalsStreaming(IngestClient& client,
                           const std::vector<SignalData>& signals,
                           const std::string& providerId,
                           size_t batch_size = 10) {
    std::cout << "\n=== Streaming Signal Ingestion ===" << std::endl;
    std::cout << "Processing " << signals.size() << " signals in batches of "
              << batch_size << std::endl;

    auto signal_chunks = chunkSignals(signals, batch_size);
    size_t total_success = 0;
    size_t total_data_points = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t chunk_idx = 0; chunk_idx < signal_chunks.size(); ++chunk_idx) {
        const auto& chunk = signal_chunks[chunk_idx];

        std::cout << "Processing batch " << (chunk_idx + 1) << "/"
                  << signal_chunks.size() << " (" << chunk.size() << " signals)" << std::endl;

        try {
            // Convert chunk to MLDP requests
            std::vector<IngestDataRequest> stream_requests;
            stream_requests.reserve(chunk.size());

            for (size_t i = 0; i < chunk.size(); ++i) {
                const auto& signal = chunk[i];
                std::string requestId = generateRequestId(signal, chunk_idx * batch_size + i);
                auto request = createMLDPRequest(signal, providerId, requestId);
                stream_requests.push_back(std::move(request));
                total_data_points += signal.values.size();
            }

            printMemoryUsage("Before Stream Send");

            // Send batch via streaming
            std::string result = client.ingestDataStream(stream_requests);

            if (result.find("Success") != std::string::npos) {
                total_success += chunk.size();
                std::cout << "  Batch success: " << chunk.size() << " signals ingested" << std::endl;
            } else {
                std::cerr << "  Batch failed: " << result << std::endl;
            }

            printMemoryUsage("After Stream Send");

            // Small delay between batches to prevent overwhelming the server
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        } catch (const std::exception& e) {
            std::cerr << "  Error processing batch " << (chunk_idx + 1)
                      << ": " << e.what() << std::endl;
            continue;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\n=== Streaming Ingestion Results ===" << std::endl;
    std::cout << "Successfully ingested: " << total_success << "/" << signals.size()
              << " signals" << std::endl;
    std::cout << "Total data points: " << total_data_points << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    std::cout << "Average batch time: " << (duration.count() / signal_chunks.size())
              << " ms/batch" << std::endl;

    return total_success > 0;
}

// Filter signals by criteria
std::vector<SignalData> filterSignals(const std::vector<SignalData>& signals,
                                      const std::string& device_filter = "",
                                      const std::string& device_area_filter = "",
                                      const std::string& project_filter = "",
                                      size_t max_signals = 0) {
    std::vector<SignalData> filtered;

    for (const auto& signal : signals) {
        // Apply filters
        if (!device_filter.empty() && signal.info.device != device_filter) {
            continue;
        }
        if (!device_area_filter.empty() && signal.info.device_area != device_area_filter) {
            continue;
        }
        if (!project_filter.empty() && signal.file_metadata.project != project_filter) {
            continue;
        }

        filtered.push_back(signal);

        // Apply max limit
        if (max_signals > 0 && filtered.size() >= max_signals) {
            break;
        }
    }

    return filtered;
}

// Print filter options
void printFilterOptions(const std::vector<SignalData>& signals) {
    std::cout << "\n=== Available Filter Options ===" << std::endl;

    // Device types
    std::set<std::string> devices;
    std::set<std::string> device_areas;
    std::set<std::string> projects;

    for (const auto& signal : signals) {
        devices.insert(signal.info.device);
        device_areas.insert(signal.info.device_area);
        projects.insert(signal.file_metadata.project);
    }

    std::cout << "Devices: ";
    for (const auto& device : devices) {
        std::cout << device << " ";
    }
    std::cout << std::endl;

    std::cout << "Device Areas: ";
    for (const auto& area : device_areas) {
        std::cout << area << " ";
    }
    std::cout << std::endl;

    std::cout << "Projects: ";
    for (const auto& project : projects) {
        std::cout << project << " ";
    }
    std::cout << std::endl;
}

// Progress monitoring class for large ingestions
class IngestionMonitor {
private:
    size_t total_signals_;
    size_t processed_signals_;
    size_t successful_signals_;
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::high_resolution_clock::time_point last_update_;

public:
    IngestionMonitor(size_t total_signals)
        : total_signals_(total_signals), processed_signals_(0), successful_signals_(0),
          start_time_(std::chrono::high_resolution_clock::now()),
          last_update_(start_time_) {}

    void signalProcessed(bool success) {
        processed_signals_++;
        if (success) successful_signals_++;

        auto now = std::chrono::high_resolution_clock::now();
        auto since_last = std::chrono::duration_cast<std::chrono::seconds>(now - last_update_);

        // Update every 10 seconds or every 100 signals
        if (since_last.count() >= 10 || processed_signals_ % 100 == 0 || processed_signals_ == total_signals_) {
            printProgress();
            last_update_ = now;
        }
    }

    void printProgress() {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);

        double percentage = (double)processed_signals_ / total_signals_ * 100.0;
        double success_rate = (double)successful_signals_ / std::max(processed_signals_, size_t(1)) * 100.0;

        std::cout << std::fixed << std::setprecision(1);
        std::cout << "[PROGRESS] " << processed_signals_ << "/" << total_signals_
                  << " (" << percentage << "%) - " << successful_signals_
                  << " successful (" << success_rate << "%) - "
                  << elapsed.count() << "s elapsed";

        if (processed_signals_ > 0) {
            auto estimated_total = elapsed.count() * total_signals_ / processed_signals_;
            auto eta = estimated_total - elapsed.count();
            std::cout << " - ETA: " << eta << "s";
        }

        std::cout << std::endl;
    }

    void printFinalSummary() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time_);

        std::cout << "\n=== Ingestion Summary ===" << std::endl;
        std::cout << "Total signals: " << total_signals_ << std::endl;
        std::cout << "Successfully processed: " << successful_signals_ << std::endl;
        std::cout << "Failed: " << (processed_signals_ - successful_signals_) << std::endl;
        std::cout << "Success rate: " << std::fixed << std::setprecision(1)
                  << ((double)successful_signals_ / total_signals_ * 100.0) << "%" << std::endl;
        std::cout << "Total time: " << total_time.count() << " seconds" << std::endl;

        if (successful_signals_ > 0) {
            std::cout << "Average time per signal: " << std::fixed << std::setprecision(2)
                      << ((double)total_time.count() / successful_signals_) << " seconds" << std::endl;
        }
    }
};

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <h5_directory> [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  --local-only                  Parse only, don't send to MLDP" << std::endl;
        std::cerr << "  --device=DEVICE               Filter by device (e.g., KLYS, BPMS)" << std::endl;
        std::cerr << "  --device-area=AREA            Filter by device area (e.g., LI23, DMPH)" << std::endl;
        std::cerr << "  --project=PROJECT             Filter by project (e.g., CoAD)" << std::endl;
        std::cerr << "  --max-signals=N               Limit to N signals" << std::endl;
        std::cerr << "  --streaming                   Use streaming ingestion" << std::endl;
        std::cerr << "  --batch-size=N                Batch size for streaming (default: 10)" << std::endl;
        std::cerr << "  --show-filters                Show available filter options" << std::endl;
        std::cerr << "  --mldp-server=ADDRESS         MLDP server address (default: localhost:50051)" << std::endl;
        return 1;
    }

    std::string h5_directory = argv[1];
    bool local_only = false;
    bool use_streaming = false;
    bool show_filters = false;
    std::string device_filter = "";
    std::string device_area_filter = "";
    std::string project_filter = "";
    size_t max_signals = 0;
    size_t batch_size = 10;
    std::string mldp_server = "localhost:50051";

    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--local-only") {
            local_only = true;
        } else if (arg == "--streaming") {
            use_streaming = true;
        } else if (arg == "--show-filters") {
            show_filters = true;
        } else if (arg.find("--device=") == 0) {
            device_filter = arg.substr(9);
        } else if (arg.find("--device-area=") == 0) {
            device_area_filter = arg.substr(14);
        } else if (arg.find("--project=") == 0) {
            project_filter = arg.substr(10);
        } else if (arg.find("--max-signals=") == 0) {
            max_signals = std::stoul(arg.substr(14));
        } else if (arg.find("--batch-size=") == 0) {
            batch_size = std::stoul(arg.substr(13));
        } else if (arg.find("--mldp-server=") == 0) {
            mldp_server = arg.substr(14);
        }
    }

    try {
        printMemoryUsage("Application Start");

        std::cout << "=== H5 to MongoDB Ingestion ===" << std::endl;
        std::cout << "H5 Directory: " << h5_directory << std::endl;
        std::cout << "MLDP Server: " << mldp_server << std::endl;
        std::cout << "Mode: " << (local_only ? "Local parsing only" :
                                  (use_streaming ? "Streaming ingestion" : "Individual ingestion")) << std::endl;

        // Parse H5 files
        std::cout << "\n--- Step 1: Parsing H5 Files ---" << std::endl;
        H5Parser parser(h5_directory);

        printMemoryUsage("Before H5 Parsing");

        if (!parser.parseDirectory()) {
            std::cerr << "Failed to parse H5 directory" << std::endl;
            return 1;
        }

        printMemoryUsage("After H5 Parsing");

        parser.printDetailedSummary();
        auto all_signals = parser.getAllSignals();

        if (all_signals.empty()) {
            std::cerr << "No signals found to process" << std::endl;
            return 1;
        }

        // Show filter options if requested
        if (show_filters) {
            printFilterOptions(all_signals);
            return 0;
        }

        // Apply filters
        auto signals_to_process = filterSignals(all_signals, device_filter,
                                               device_area_filter, project_filter, max_signals);

        if (signals_to_process.size() != all_signals.size()) {
            std::cout << "\n--- Filtering Results ---" << std::endl;
            std::cout << "Filtered from " << all_signals.size() << " to "
                      << signals_to_process.size() << " signals" << std::endl;

            if (!device_filter.empty()) {
                std::cout << "  Device filter: " << device_filter << std::endl;
            }
            if (!device_area_filter.empty()) {
                std::cout << "  Device area filter: " << device_area_filter << std::endl;
            }
            if (!project_filter.empty()) {
                std::cout << "  Project filter: " << project_filter << std::endl;
            }
            if (max_signals > 0) {
                std::cout << "  Max signals limit: " << max_signals << std::endl;
            }
        }

        if (signals_to_process.empty()) {
            std::cerr << "No signals match the specified filters" << std::endl;
            return 1;
        }

        if (local_only) {
            std::cout << "\nLocal-only mode: Parsing complete. Exiting." << std::endl;
            printMemoryUsage("Local-only Complete");
            return 0;
        }

        // Connect to MLDP and register provider
        std::cout << "\n--- Step 2: MLDP Registration ---" << std::endl;

        IngestClient client(mldp_server);

        // Register provider with enhanced metadata
        uint64_t nowSec = getCurrentEpochSeconds();
        uint64_t nowNano = getCurrentEpochNanos();

        std::vector<Attribute> provider_attributes;
        if (!signals_to_process.empty()) {
            const auto& first_signal = signals_to_process[0];
            provider_attributes.push_back(makeAttribute("origin", first_signal.file_metadata.origin));
            provider_attributes.push_back(makeAttribute("pathway", first_signal.file_metadata.pathway));
            if (!project_filter.empty()) {
                provider_attributes.push_back(makeAttribute("project_filter", project_filter));
            }
            if (!device_filter.empty()) {
                provider_attributes.push_back(makeAttribute("device_filter", device_filter));
            }
        }
        provider_attributes.push_back(makeAttribute("parser_version", "h5_parser_v1.0"));
        provider_attributes.push_back(makeAttribute("signal_count", std::to_string(signals_to_process.size())));

        auto regReq = makeRegisterProviderRequest("H5_Parser_Provider", provider_attributes, nowSec, nowNano);
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

        // Ingest signals
        std::cout << "\n--- Step 3: Data Ingestion ---" << std::endl;

        bool success;
        if (use_streaming) {
            success = ingestSignalsStreaming(client, signals_to_process, providerId, batch_size);
        } else {
            success = ingestSignalsIndividually(client, signals_to_process, providerId);
        }

        printMemoryUsage("After Data Ingestion");

        // Final summary
        std::cout << "\n=== Final Summary ===" << std::endl;
        if (success) {
            std::cout << "✓ Ingestion completed successfully" << std::endl;
            std::cout << "✓ Provider ID: " << providerId << std::endl;
            std::cout << "✓ Mode: " << (use_streaming ? "Streaming" : "Individual") << std::endl;
            std::cout << "✓ Signals processed: " << signals_to_process.size() << std::endl;

            // Calculate total data points
            size_t total_points = 0;
            for (const auto& signal : signals_to_process) {
                total_points += signal.values.size();
            }
            std::cout << "✓ Total data points: " << total_points << std::endl;

            return 0;
        } else {
            std::cerr << "✗ Ingestion failed" << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        printMemoryUsage("Error State");
        return 1;
    }
}
