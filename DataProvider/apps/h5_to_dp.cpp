#include "h5_parser.hpp"
#include "ingest_client.hpp"
#include <H5Cpp.h>
#include <iostream>
#include <algorithm>
#include <filesystem>

void printUsage(const std::string& program_name) {
    std::cout << "H5 to DataProvider Ingestion\n";
    std::cout << "USAGE: " << program_name << " <h5_directory> [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  --config=PATH                 Use config file\n";
    std::cout << "  --local-only                  Parse only, don't send to server\n";
    std::cout << "  --device=DEVICE               Filter by device\n";
    std::cout << "  --project=PROJECT             Filter by project\n";
    std::cout << "  --max-signals=N               Limit to N signals\n";
    std::cout << "  --no-spatial                  Disable spatial enrichment\n";
    std::cout << "  --server=ADDRESS              Server address\n\n";
}

class FastH5Parser : public H5Parser {
public:
    explicit FastH5Parser(const std::string& h5_directory_path) 
        : H5Parser(h5_directory_path) {}

    bool parseFile(const std::string& filepath) override {
        try {
            H5FileMetadata file_metadata = parseFilename(filepath);
            
            H5::Exception::dontPrint();
            H5::H5File file(filepath, H5F_ACC_RDONLY);
            
            auto timestamps = extractTimestamps(file);
            if (!timestamps || timestamps->count == 0) {
                file.close();
                return false;
            }
            
            file_timestamps_[filepath] = timestamps;
            
            auto signal_datasets = getSignalDatasets(file);
            if (signal_datasets.empty()) {
                file.close();
                return false;
            }
            
            processSignals(file, signal_datasets, timestamps, file_metadata);
            
            file.close();
            return true;
            
        } catch (const std::exception&) {
            return false;
        }
    }

private:
    void processSignals(H5::H5File& file,
                       const std::vector<std::string>& signal_names,
                       std::shared_ptr<TimestampData> timestamps,
                       const H5FileMetadata& file_metadata) {
        
        for (const auto& signal_name : signal_names) {
            try {
                SignalData signal;
                signal.info = parseSignalName(signal_name);
                signal.timestamps = timestamps;
                signal.file_metadata = file_metadata;
                signal.spatial_enrichment_ready = spatial_enrichment_enabled_;
                
                H5::DataSet dataset = file.openDataSet(signal_name);
                H5::DataSpace dataspace = dataset.getSpace();
                
                hsize_t dims[1];
                dataspace.getSimpleExtentDims(dims);
                signal.values.resize(dims[0]);
                
                try {
                    dataset.read(signal.values.data(), H5::PredType::NATIVE_DOUBLE);
                } catch (const H5::Exception&) {
                    std::vector<float> float_values(dims[0]);
                    dataset.read(float_values.data(), H5::PredType::NATIVE_FLOAT);
                    for (size_t i = 0; i < float_values.size(); ++i) {
                        signal.values[i] = static_cast<double>(float_values[i]);
                    }
                }
                
                dataset.close();
                dataspace.close();
                
                parsed_signals_.push_back(std::move(signal));
                
            } catch (const std::exception&) {
                continue;
            }
        }
    }
};

std::vector<IngestDataRequest> createIngestRequests(const std::vector<SignalData>& signals,
                                                   const std::string& providerId) {
    std::vector<IngestDataRequest> requests;
    requests.reserve(signals.size());

    for (size_t i = 0; i < signals.size(); ++i) {
        const auto& signal = signals[i];
        std::string requestId = IngestUtils::generateRequestId("signal_" + std::to_string(i));
        
        std::vector<Attribute> attributes;
        attributes.push_back(makeAttribute("device", signal.info.device));
        attributes.push_back(makeAttribute("device_area", signal.info.device_area));
        attributes.push_back(makeAttribute("device_location", signal.info.device_location));
        attributes.push_back(makeAttribute("device_attribute", signal.info.device_attribute));
        attributes.push_back(makeAttribute("units", signal.info.units));
        attributes.push_back(makeAttribute("signal_type", signal.info.signal_type));

        std::vector<std::string> tags;
        tags.push_back("h5_parsed");
        tags.push_back(signal.info.device);
        tags.push_back(signal.info.signal_type);

        EventMetadata eventMetadata = makeEventMetadata(
            "H5 signal: " + signal.info.full_name,
            signal.timestamps->start_time_sec, signal.timestamps->start_time_nano,
            signal.timestamps->end_time_sec, signal.timestamps->end_time_nano);

        SamplingClock samplingClock = makeSamplingClock(
            signal.timestamps->start_time_sec,
            signal.timestamps->start_time_nano,
            signal.timestamps->period_nanos,
            static_cast<uint32_t>(signal.timestamps->count));

        std::vector<DataValue> dataValues;
        dataValues.reserve(signal.values.size());
        for (double value : signal.values) {
            dataValues.push_back(makeDataValueWithDouble(value));
        }

        std::vector<DataColumn> dataColumns;
        dataColumns.push_back(makeDataColumn(signal.info.full_name, dataValues));

        requests.push_back(makeIngestDataRequest(providerId, requestId, attributes, tags, 
                                                eventMetadata, samplingClock, dataColumns));
    }

    return requests;
}

int main(int argc, char* argv[]) {
    H5::Exception::dontPrint();

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string h5_directory = argv[1];
    std::string config_file = "config/ingestion_config.json";
    std::string device_filter, project_filter, server_address;
    size_t max_signals = 0;
    bool local_only = false;
    bool disable_spatial = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg.find("--config=") == 0) {
            config_file = arg.substr(9);
        } else if (arg.find("--device=") == 0) {
            device_filter = arg.substr(9);
        } else if (arg.find("--project=") == 0) {
            project_filter = arg.substr(10);
        } else if (arg.find("--max-signals=") == 0) {
            max_signals = std::stoull(arg.substr(14));
        } else if (arg.find("--server=") == 0) {
            server_address = arg.substr(9);
        } else if (arg == "--local-only") {
            local_only = true;
        } else if (arg == "--no-spatial") {
            disable_spatial = true;
        }
    }

    try {
        std::cout << "H5 to Data Ingestion\n";
        std::cout << "Directory: " << h5_directory << std::endl;

        std::vector<std::string> h5_files;
        for (const auto& entry : std::filesystem::directory_iterator(h5_directory)) {
            if (entry.path().extension() == ".h5" || entry.path().extension() == ".hdf5") {
                h5_files.push_back(entry.path().string());
            }
        }

        if (h5_files.empty()) {
            std::cerr << "No H5 files found" << std::endl;
            return 1;
        }
        
        std::cout << "Found " << h5_files.size() << " H5 files" << std::endl;

        IngestClient* client = nullptr;
        std::string provider_id;
        
        if (!local_only) {
            IngestClientConfig ingest_config;
            if (!server_address.empty()) {
                ingest_config.server_address = server_address;
            } else {
                ingest_config = IngestClientConfig::fromConfigFile(config_file);
            }
            
            if (!disable_spatial) {
                ingest_config.enable_spatial_enrichment = true;
            }
            
            client = new IngestClient(ingest_config);

            std::vector<Attribute> provider_attrs;
            provider_attrs.push_back(makeAttribute("source", "h5_parser"));
            provider_attrs.push_back(makeAttribute("version", "1.0"));

            std::vector<std::string> provider_tags;
            provider_tags.push_back("h5_data");
            provider_tags.push_back("automated");

            auto provider_response = client->registerProvider("H5DataProvider", provider_attrs, provider_tags);
            provider_id = provider_response.registrationresult().providerid();
            std::cout << "Registered provider: " << provider_id << std::endl;
        }

        size_t successful_files = 0;
        size_t total_signals_ingested = 0;

        for (const auto& file : h5_files) {
            FastH5Parser parser(h5_directory);
            parser.enableSpatialEnrichment(!disable_spatial);

            if (parser.parseFile(file)) {
                auto signals = parser.getAllSignals();
                successful_files++;
                
                std::cout << "File " << std::filesystem::path(file).filename() 
                          << ": " << signals.size() << " signals";

                if (local_only) {
                    std::cout << " (local only)" << std::endl;
                    continue;
                }

                auto requests = createIngestRequests(signals, provider_id);
                auto result = client->ingestBatch(requests, provider_id);

                std::cout << " -> " << result.successful_requests << "/" 
                          << result.total_requests << " ingested" << std::endl;

                total_signals_ingested += result.successful_requests;
            }
        }

        std::cout << "Processing complete:" << std::endl;
        std::cout << "  Files processed: " << successful_files << "/" << h5_files.size() << std::endl;

        if (local_only) {
            std::cout << "Local processing complete" << std::endl;
            delete client;
            return 0;
        } else {
            std::cout << "  Total signals ingested: " << total_signals_ingested << std::endl;
            delete client;
            return (total_signals_ingested > 0) ? 0 : 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}