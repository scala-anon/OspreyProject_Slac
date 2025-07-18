#include "h5_parser.hpp"
#include "ingest_client.hpp"
#include <iostream>
#include <chrono>

void printUsage(const std::string& program_name) {
    std::cout << "H5 to DataProvider Ingestion\n";
    std::cout << "USAGE: " << program_name << " <h5_directory> [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  --config=PATH                 Use config file (default: config/ingestion_config.json)\n";
    std::cout << "  --local-only                  Parse only, don't send to MLDP\n";
    std::cout << "  --device=DEVICE               Filter by device (e.g., KLYS, BPMS)\n";
    std::cout << "  --project=PROJECT             Filter by project (e.g., CoAD)\n";
    std::cout << "  --max-signals=N               Limit to N signals\n";
    std::cout << "  --batch-size=N                Batch size (default from config)\n";
    std::cout << "  --no-spatial                  Disable spatial enrichment\n";
    std::cout << "  --server=ADDRESS              MLDP server address\n";
    std::cout << "  --verbose                     Enable detailed output\n\n";
}

// Convert SignalData to MLDP IngestDataRequest using spatial enrichment
IngestDataRequest createIngestRequest(const SignalData& signal,
                                     const std::string& providerId,
                                     const std::string& requestId) {

    // Create sampling clock
    SamplingClock sampling_clock;
    if (signal.timestamps->is_regular_sampling && signal.timestamps->count > 1) {
        sampling_clock = makeSamplingClock(
            signal.timestamps->start_time_sec,
            signal.timestamps->start_time_nano,
            signal.timestamps->period_nanos,
            static_cast<uint32_t>(signal.timestamps->count)
        );
    } else {
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

    // Create base attributes (spatial enrichment will add more automatically)
    std::vector<Attribute> attributes;
    attributes.push_back(makeAttribute("device", signal.info.device));
    attributes.push_back(makeAttribute("device_area", signal.info.device_area));
    attributes.push_back(makeAttribute("device_location", signal.info.device_location));
    attributes.push_back(makeAttribute("device_attribute", signal.info.device_attribute));
    attributes.push_back(makeAttribute("signal_type", signal.info.signal_type));
    attributes.push_back(makeAttribute("units", signal.info.units));
    attributes.push_back(makeAttribute("origin", signal.file_metadata.origin));
    attributes.push_back(makeAttribute("project", signal.file_metadata.project));

    // Create base tags (spatial enrichment will add more automatically)
    std::vector<std::string> tags;
    tags.push_back("h5_data");
    tags.push_back("device_" + signal.info.device);
    tags.push_back("project_" + signal.file_metadata.project);

    // Create event metadata
    auto event_metadata = makeEventMetadata(
        "H5 Signal Data: " + signal.info.full_name + " from " + signal.file_metadata.project,
        signal.timestamps->start_time_sec,
        signal.timestamps->start_time_nano,
        signal.timestamps->end_time_sec,
        signal.timestamps->end_time_nano
    );

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

// Filter signals by criteria
std::vector<SignalData> filterSignals(const std::vector<SignalData>& signals,
                                      const std::string& device_filter = "",
                                      const std::string& project_filter = "",
                                      size_t max_signals = 0) {
    std::vector<SignalData> filtered;

    for (const auto& signal : signals) {
        if (!device_filter.empty() && signal.info.device != device_filter) {
            continue;
        }
        if (!project_filter.empty() && signal.file_metadata.project != project_filter) {
            continue;
        }

        filtered.push_back(signal);

        if (max_signals > 0 && filtered.size() >= max_signals) {
            break;
        }
    }

    return filtered;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string h5_directory = argv[1];
    std::string config_path = "config/ingestion_config.json";
    bool local_only = false;
    bool disable_spatial = false;
    bool verbose = false;
    std::string device_filter = "";
    std::string project_filter = "";
    size_t max_signals = 0;
    size_t batch_size = 0; // 0 means use config default
    std::string server_override = "";

    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--local-only") {
            local_only = true;
        } else if (arg == "--no-spatial") {
            disable_spatial = true;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg.find("--config=") == 0) {
            config_path = arg.substr(9);
        } else if (arg.find("--device=") == 0) {
            device_filter = arg.substr(9);
        } else if (arg.find("--project=") == 0) {
            project_filter = arg.substr(10);
        } else if (arg.find("--max-signals=") == 0) {
            max_signals = std::stoul(arg.substr(14));
        } else if (arg.find("--batch-size=") == 0) {
            batch_size = std::stoul(arg.substr(13));
        } else if (arg.find("--server=") == 0) {
            server_override = arg.substr(9);
        }
    }

    try {
        if (verbose) {
            std::cout << "H5 TO MONGODB INGESTION\n";
            std::cout << "Directory: " << h5_directory << std::endl;
            std::cout << "Config: " << config_path << std::endl;
        }

        // STEP 1: Parse H5 files
        if (verbose) std::cout << "\nParsing H5 files...\n";
        H5Parser parser(h5_directory);
        parser.enableSpatialEnrichment(!disable_spatial);

        if (!parser.parseDirectory()) {
            std::cerr << "Failed to parse H5 directory" << std::endl;
            return 1;
        }

        auto all_signals = parser.getAllSignals();
        if (all_signals.empty()) {
            std::cerr << "No signals found" << std::endl;
            return 1;
        }

        // Apply filters
        auto signals_to_process = filterSignals(all_signals, device_filter, project_filter, max_signals);
        if (signals_to_process.empty()) {
            std::cerr << "No signals match filters" << std::endl;
            return 1;
        }

        std::cout << "Found " << signals_to_process.size() << " signals to process" << std::endl;

        if (local_only) {
            std::cout << "Local parsing complete" << std::endl;
            return 0;
        }

        // STEP 2: Initialize IngestClient
        if (verbose) std::cout << "Initializing ingest client...\n";
        
        IngestClientConfig client_config;
        try {
            client_config = IngestClientConfig::fromConfigFile(config_path);
        } catch (const std::exception& e) {
            if (verbose) std::cout << "Using default config: " << e.what() << std::endl;
        }

        // Apply overrides
        if (!server_override.empty()) client_config.server_address = server_override;
        if (disable_spatial) client_config.enable_spatial_enrichment = false;
        if (batch_size > 0) client_config.default_batch_size = batch_size;

        if (verbose) {
            std::cout << "Server: " << client_config.server_address << std::endl;
            std::cout << "Spatial enrichment: " << (client_config.enable_spatial_enrichment ? "enabled" : "disabled") << std::endl;
        }

        IngestClient client(client_config);

        // Progress callback - only show every 10%
        size_t last_percentage = 0;
        client.setProgressCallback([&](size_t processed, size_t total, size_t successful) {
            size_t percentage = (processed * 100) / total;
            if (percentage >= last_percentage + 10 || processed == total) {
                std::cout << "Progress: " << processed << "/" << total 
                          << " (" << percentage << "%) - " << successful << " successful" << std::endl;
                last_percentage = percentage;
            }
        });

        client.setErrorCallback([verbose](const std::string& error, const std::string& context) {
            if (verbose) {
                std::cerr << "Error in " << context << ": " << error << std::endl;
            }
        });

        // STEP 3: Register Provider
        if (verbose) std::cout << "Registering provider...\n";
        
        std::vector<Attribute> provider_attributes;
        provider_attributes.push_back(makeAttribute("facility", "SLAC"));
        provider_attributes.push_back(makeAttribute("accelerator", "LCLS-II"));
        provider_attributes.push_back(makeAttribute("signal_count", std::to_string(signals_to_process.size())));

        std::vector<std::string> provider_tags = {"lcls-ii", "h5_parser", "spatial_enriched"};

        auto registration_response = client.registerProvider("H5_Parser_Provider", provider_attributes, provider_tags);
        if (!registration_response.has_registrationresult()) {
            std::cerr << "Failed to register provider" << std::endl;
            return 1;
        }

        std::string providerId = registration_response.registrationresult().providerid();
        std::cout << "Registered as provider: " << providerId << std::endl;

        // STEP 4: Create requests
        if (verbose) std::cout << "Creating ingestion requests...\n";
        
        std::vector<IngestDataRequest> requests;
        requests.reserve(signals_to_process.size());

        for (size_t i = 0; i < signals_to_process.size(); ++i) {
            const auto& signal = signals_to_process[i];
            std::string requestId = IngestUtils::generateRequestId("h5_signal_" + std::to_string(i));
            auto request = createIngestRequest(signal, providerId, requestId);
            requests.push_back(std::move(request));
        }

        // STEP 5: Ingest with spatial enrichment
        std::cout << "Starting ingestion..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();

        IngestionResult result = client.ingestWithSpatialEnrichment(requests, providerId);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration<double>(end_time - start_time);

        // STEP 6: Summary
        std::cout << "\nIngestion complete:" << std::endl;
        std::cout << "  Processed: " << result.successful_requests << "/" << result.total_requests << std::endl;
        std::cout << "  Success rate: " << std::fixed << std::setprecision(1)
                  << (result.getSuccessRate() * 100.0) << "%" << std::endl;
        std::cout << "  Time: " << std::fixed << std::setprecision(1)
                  << total_time.count() << "s" << std::endl;

        if (result.failed_requests > 0) {
            std::cout << "  Failed: " << result.failed_requests << " requests" << std::endl;
            if (verbose && !result.error_messages.empty()) {
                std::cout << "  First error: " << result.error_messages[0] << std::endl;
            }
        }

        return result.success ? 0 : 1;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
