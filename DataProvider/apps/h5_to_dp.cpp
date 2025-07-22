#include "h5_parser.hpp"
#include "ingest_client.hpp"
#include <H5Cpp.h>
#include <iostream>
#include <filesystem>
#include <future>
#include <thread>

class OptimizedH5Parser : public H5Parser {
private:
    static constexpr size_t CACHE_SIZE = 64 * 1024 * 1024;   // 64MB cache
    static constexpr size_t BATCH_SIZE = 50;                 // Process signals in batches

public:
    explicit OptimizedH5Parser(const std::string& h5_directory_path) : H5Parser(h5_directory_path) {
        enableSpatialEnrichment(false);
    }

    bool parseFile(const std::string& filepath) override {
        try {
            H5FileMetadata file_metadata = parseFilename(filepath);
            H5::Exception::dontPrint();
            
            // Optimized file access
            H5::FileAccPropList fapl;
            fapl.setCache(0, 1021, CACHE_SIZE, 0.75);
            fapl.setFcloseDegree(H5F_CLOSE_STRONG);
            
            H5::H5File file(filepath, H5F_ACC_RDONLY, H5::FileCreatPropList::DEFAULT, fapl);
            
            // Get timestamps and signals efficiently
            auto timestamps = getTimestamps(file);
            if (!timestamps) {
                file.close();
                return false;
            }
            
            file_timestamps_[filepath] = timestamps;
            
            auto signal_names = getSignalNames(file);
            if (signal_names.empty()) {
                file.close();
                return false;
            }
            
            // Process signals in batches for better memory usage
            processSignalsBatch(file, signal_names, timestamps, file_metadata);
            
            file.close();
            return true;
            
        } catch (...) {
            return false;
        }
    }

private:
    std::shared_ptr<TimestampData> getTimestamps(H5::H5File& file) {
        auto timestamps = std::make_shared<TimestampData>();
        
        try {
            if (file.nameExists("secondsPastEpoch")) {
                H5::DataSet seconds_ds = file.openDataSet("secondsPastEpoch");
                H5::DataSpace space = seconds_ds.getSpace();
                
                hsize_t dims[1];
                space.getSimpleExtentDims(dims);
                
                timestamps->seconds.resize(dims[0]);
                timestamps->nanoseconds.resize(dims[0], 0);
                
                seconds_ds.read(timestamps->seconds.data(), H5::PredType::NATIVE_UINT64);
                
                if (file.nameExists("nanoseconds")) {
                    H5::DataSet nanos_ds = file.openDataSet("nanoseconds");
                    nanos_ds.read(timestamps->nanoseconds.data(), H5::PredType::NATIVE_UINT64);
                    nanos_ds.close();
                }
                
                timestamps->count = dims[0];
                timestamps->start_time_sec = dims[0] > 0 ? timestamps->seconds[0] : 0;
                timestamps->start_time_nano = dims[0] > 0 ? timestamps->nanoseconds[0] : 0;
                timestamps->end_time_sec = dims[0] > 0 ? timestamps->seconds[dims[0]-1] : 0;
                timestamps->end_time_nano = dims[0] > 0 ? timestamps->nanoseconds[dims[0]-1] : 0;
                
                // Calculate actual sampling period from timestamps
                if (dims[0] > 1) {
                    uint64_t first_time = timestamps->seconds[0] * 1000000000ULL + timestamps->nanoseconds[0];
                    uint64_t second_time = timestamps->seconds[1] * 1000000000ULL + timestamps->nanoseconds[1];
                    timestamps->period_nanos = second_time - first_time;
                    
                    // Validate period (between 1ms and 10s)
                    if (timestamps->period_nanos < 1000000 || timestamps->period_nanos > 10000000000ULL) {
                        timestamps->period_nanos = 1000000000; // fallback to 1s
                    }
                } else {
                    timestamps->period_nanos = 1000000000; // 1 second default
                }
                timestamps->is_regular_sampling = true;
                
                seconds_ds.close();
                space.close();
            } else {
                // Dummy timestamps
                timestamps->count = 1;
                timestamps->seconds = {0};
                timestamps->nanoseconds = {0};
                timestamps->start_time_sec = 0;
                timestamps->start_time_nano = 0;
                timestamps->end_time_sec = 0;
                timestamps->end_time_nano = 0;
                timestamps->period_nanos = 1000000000;
                timestamps->is_regular_sampling = true;
            }
        } catch (...) {
            return nullptr;
        }
        
        return timestamps;
    }
    
    std::vector<std::string> getSignalNames(H5::H5File& file) {
        std::vector<std::string> names;
        
        try {
            H5::Group root = file.openGroup("/");
            hsize_t num_objects = root.getNumObjs();
            
            for (hsize_t i = 0; i < num_objects; i++) {
                if (root.getObjTypeByIdx(i) == H5G_DATASET) {
                    std::string obj_name = root.getObjnameByIdx(i);
                    if (obj_name != "secondsPastEpoch" && obj_name != "nanoseconds") {
                        names.push_back(obj_name);
                    }
                }
            }
            root.close();
        } catch (...) {
            // Return empty on error
        }
        
        return names;
    }
    
    void processSignalsBatch(H5::H5File& file,
                            const std::vector<std::string>& signal_names,
                            std::shared_ptr<TimestampData> timestamps,
                            const H5FileMetadata& file_metadata) {
        
        parsed_signals_.reserve(parsed_signals_.size() + signal_names.size());
        
        // Sequential processing is faster for this workload
        for (size_t start = 0; start < signal_names.size(); start += BATCH_SIZE) {
            size_t end = std::min(start + BATCH_SIZE, signal_names.size());
            
            std::vector<SignalData> batch_signals;
            batch_signals.reserve(end - start);
            
            for (size_t i = start; i < end; ++i) {
                try {
                    SignalData signal = processSignalFast(file, signal_names[i], timestamps, file_metadata);
                    batch_signals.push_back(std::move(signal));
                } catch (...) {
                    continue;
                }
            }
            
            parsed_signals_.insert(parsed_signals_.end(),
                                 std::make_move_iterator(batch_signals.begin()),
                                 std::make_move_iterator(batch_signals.end()));
        }
    }
    
    SignalData processSignalFast(H5::H5File& file,
                                const std::string& signal_name,
                                std::shared_ptr<TimestampData> timestamps,
                                const H5FileMetadata& file_metadata) {
        
        SignalData signal;
        signal.info.full_name = signal_name;
        signal.info.device = "DEVICE";
        signal.info.device_area = "AREA";  
        signal.info.device_location = "LOC";
        signal.info.device_attribute = "ATTR";
        signal.info.units = "units";
        signal.info.signal_type = "measurement";
        signal.timestamps = timestamps;
        signal.file_metadata = file_metadata;
        signal.spatial_enrichment_ready = false;
        
        H5::DataSet dataset = file.openDataSet(signal_name);
        H5::DataSpace dataspace = dataset.getSpace();
        
        hsize_t dims[1];
        dataspace.getSimpleExtentDims(dims);
        signal.values.resize(dims[0]);
        
        // Optimized read with multiple fallback paths
        try {
            dataset.read(signal.values.data(), H5::PredType::NATIVE_DOUBLE);
        } catch (...) {
            try {
                std::vector<float> float_data(dims[0]);
                dataset.read(float_data.data(), H5::PredType::NATIVE_FLOAT);
                for (size_t i = 0; i < dims[0]; ++i) {
                    signal.values[i] = static_cast<double>(float_data[i]);
                }
            } catch (...) {
                // Fill with zeros if all else fails
                std::fill(signal.values.begin(), signal.values.end(), 0.0);
            }
        }
        
        dataset.close();
        dataspace.close();
        return signal;
    }
};

std::vector<IngestDataRequest> createRequests(const std::vector<SignalData>& signals, const std::string& providerId) {
    std::vector<IngestDataRequest> requests;
    requests.reserve(signals.size());

    for (size_t i = 0; i < signals.size(); ++i) {
        const auto& signal = signals[i];
        std::string requestId = IngestUtils::generateRequestId("signal_" + std::to_string(i));

        std::vector<Attribute> attributes;
        attributes.push_back(makeAttribute("pv_name", signal.info.full_name));

        std::vector<std::string> tags;
        tags.push_back("h5_data");

        EventMetadata eventMetadata = makeEventMetadata(
            "H5: " + signal.info.full_name,
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
        
        // Add timestamp columns if we have timestamp data
        if (signal.timestamps->seconds.size() == signal.values.size()) {
            std::vector<DataValue> timestampSeconds, timestampNanos;
            timestampSeconds.reserve(signal.timestamps->seconds.size());
            timestampNanos.reserve(signal.timestamps->nanoseconds.size());
            
            for (size_t j = 0; j < signal.timestamps->seconds.size(); ++j) {
                timestampSeconds.push_back(makeDataValueWithUInt64(signal.timestamps->seconds[j]));
                timestampNanos.push_back(makeDataValueWithUInt64(signal.timestamps->nanoseconds[j]));
            }
            
            dataColumns.push_back(makeDataColumn(signal.info.full_name + "_timestamp_seconds", timestampSeconds));
            dataColumns.push_back(makeDataColumn(signal.info.full_name + "_timestamp_nanos", timestampNanos));
        }

        requests.push_back(makeIngestDataRequest(providerId, requestId, attributes, tags,
                                                 eventMetadata, samplingClock, dataColumns));
    }
    return requests;
}

// Sequential ingestion (current approach)
void ingestSequential(const std::vector<SignalData>& signals, const std::string& provider_id, IngestClient* client) {
    auto requests = createRequests(signals, provider_id);
    client->ingestBatch(requests, provider_id);
}

// Parallel ingestion for network I/O
void ingestParallel(const std::vector<SignalData>& signals, const std::string& provider_id, IngestClient* client) {
    const size_t num_threads = std::min(static_cast<size_t>(4), static_cast<size_t>(std::thread::hardware_concurrency()));
    const size_t signals_per_thread = (signals.size() + num_threads - 1) / num_threads;
    
    std::vector<std::future<void>> futures;
    
    for (size_t t = 0; t < num_threads && t * signals_per_thread < signals.size(); ++t) {
        size_t start = t * signals_per_thread;
        size_t end = std::min(start + signals_per_thread, signals.size());
        
        std::vector<SignalData> thread_signals(signals.begin() + start, signals.begin() + end);
        
        auto future = std::async(std::launch::async, [thread_signals, provider_id, client]() {
            auto requests = createRequests(thread_signals, provider_id);
            client->ingestBatch(requests, provider_id);
        });
        
        futures.push_back(std::move(future));
    }
    
    // Wait for all ingestion threads to complete
    for (auto& future : futures) {
        try {
            future.get();
        } catch (...) {
            // Continue with other threads even if one fails
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <h5_directory> [--local-only]\n";
        return 1;
    }

    std::string h5_directory = argv[1];
    bool local_only = false;

    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--local-only") {
            local_only = true;
        }
    }

    try {
        std::vector<std::string> h5_files;
        for (const auto& entry : std::filesystem::directory_iterator(h5_directory)) {
            if (entry.path().extension() == ".h5") {
                h5_files.push_back(entry.path().string());
            }
        }

        if (h5_files.empty()) {
            std::cerr << "No H5 files found\n";
            return 1;
        }

        IngestClient* client = nullptr;
        std::string provider_id;

        if (!local_only) {
            IngestClientConfig config;
            config.enable_spatial_enrichment = false;
            client = new IngestClient(config);

            std::vector<Attribute> attrs;
            attrs.push_back(makeAttribute("source", "h5_simple"));
            std::vector<std::string> tags = {"h5_data"};

            auto response = client->registerProvider("H5SimpleProvider", attrs, tags);
            provider_id = response.registrationresult().providerid();
        }

        auto start_time = std::chrono::high_resolution_clock::now();
        size_t total_signals = 0;

        for (const auto& file : h5_files) {
            OptimizedH5Parser parser(h5_directory);
            
            if (parser.parseFile(file)) {
                auto signals = parser.getAllSignals();
                total_signals += signals.size();
                std::cout << std::filesystem::path(file).filename() << ": " << signals.size() << " signals\n";

                if (!local_only && !signals.empty()) {
                    // Adaptive processing: parallel for network I/O, sequential for small batches
                    if (signals.size() > 100) {
                        ingestParallel(signals, provider_id, client);
                    } else {
                        ingestSequential(signals, provider_id, client);
                    }
                }
            } else {
                std::cout << "Failed: " << std::filesystem::path(file).filename() << "\n";
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        double total_time = std::chrono::duration<double>(end_time - start_time).count();
        
        std::cout << "Total: " << total_signals << " signals in " 
                  << std::fixed << std::setprecision(2) << total_time << "s\n";

        delete client;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}