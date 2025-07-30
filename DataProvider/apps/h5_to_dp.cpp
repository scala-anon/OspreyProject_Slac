#include "h5_parser.hpp"
#include "ingest_client.hpp"
#include <H5Cpp.h>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <regex>
#include <iomanip>

struct PvComponents {
    std::string device_type;
    std::string device_area;
    std::string device_location;
    std::string measurement_type;
    bool is_valid = false;
};

struct FileComponents {
    std::string beam_line;
    std::string date;
    std::string time_id;
    std::string filename;
    bool is_valid = false;
};

PvComponents parsePvName(const std::string& pv_name) {
    PvComponents components;

    std::vector<std::string> parts;
    std::stringstream ss(pv_name);
    std::string part;

    while (std::getline(ss, part, '_')) {
        parts.push_back(part);
    }

    if (parts.size() >= 4) {
        components.device_type = parts[0];
        components.device_area = parts[1];
        components.device_location = parts[2];
        components.measurement_type = parts[3];
        components.is_valid = true;
    }

    return components;
}

FileComponents parseFilename(const std::string& filepath) {
    FileComponents components;

    std::filesystem::path path(filepath);
    std::string filename = path.stem().string(); // Remove .h5 extension
    components.filename = path.filename().string();

    std::vector<std::string> parts;
    std::stringstream ss(filename);
    std::string part;

    while (std::getline(ss, part, '_')) {
        parts.push_back(part);
    }

    if (parts.size() >= 4) {
        components.beam_line = parts[0] + "_" + parts[1]; // "CU_HXR"
        components.date = parts[2];  // "20250716"
        components.time_id = parts[3]; // "224347"
        components.is_valid = true;
    }

    return components;
}

class CleanH5Parser : public H5Parser {
private:
    static constexpr size_t BATCH_SIZE = 50;

public:
    explicit CleanH5Parser(const std::string& h5_directory_path) : H5Parser(h5_directory_path) {
        enableSpatialEnrichment(false);
    }

    bool parseFile(const std::string& filepath) override {
        try {
            H5FileMetadata file_metadata = H5Parser::parseFilename(filepath);
            H5::Exception::dontPrint();

            H5::H5File file(filepath, H5F_ACC_RDONLY);

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

            processSignals(file, signal_names, timestamps, file_metadata, filepath);
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
                if (dims[0] > 0) {
                    timestamps->start_time_sec = timestamps->seconds[0];
                    timestamps->start_time_nano = timestamps->nanoseconds[0];
                    timestamps->end_time_sec = timestamps->seconds[dims[0]-1];
                    timestamps->end_time_nano = timestamps->nanoseconds[dims[0]-1];

                    // Calculate sampling period
                    if (dims[0] > 1) {
                        uint64_t first_time = timestamps->seconds[0] * 1000000000ULL + timestamps->nanoseconds[0];
                        uint64_t second_time = timestamps->seconds[1] * 1000000000ULL + timestamps->nanoseconds[1];
                        timestamps->period_nanos = second_time - first_time;

                        // Validate period (1ms to 10s)
                        if (timestamps->period_nanos < 1000000 || timestamps->period_nanos > 10000000000ULL) {
                            timestamps->period_nanos = 1000000000; // 1 second fallback
                        }
                    } else {
                        timestamps->period_nanos = 1000000000;
                    }
                }
                timestamps->is_regular_sampling = true;

                seconds_ds.close();
                space.close();
            } else {
                // No timestamps found - create dummy
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
                    // Skip timestamp datasets
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

    void processSignals(H5::H5File& file,
                       const std::vector<std::string>& signal_names,
                       std::shared_ptr<TimestampData> timestamps,
                       const H5FileMetadata& file_metadata,
                       const std::string& filepath) {

        parsed_signals_.reserve(parsed_signals_.size() + signal_names.size());

        for (const auto& signal_name : signal_names) {
            try {
                SignalData signal = createSignal(file, signal_name, timestamps, file_metadata, filepath);
                parsed_signals_.push_back(std::move(signal));
            } catch (...) {
                continue;
            }
        }
    }

    SignalData createSignal(H5::H5File& file,
                           const std::string& signal_name,
                           std::shared_ptr<TimestampData> timestamps,
                           const H5FileMetadata& file_metadata,
                           const std::string& filepath) {

        SignalData signal;
        signal.info.full_name = signal_name;
        signal.timestamps = timestamps;
        signal.file_metadata = file_metadata;
        signal.spatial_enrichment_ready = false;

        // Parse PV name for structured metadata
        auto pv_components = parsePvName(signal_name);
        if (pv_components.is_valid) {
            signal.info.device = pv_components.device_type;
            signal.info.device_area = pv_components.device_area;
            signal.info.device_location = pv_components.device_location;
            signal.info.device_attribute = pv_components.measurement_type;
        } else {
            // Fallback for unparseable names
            signal.info.device = "UNKNOWN";
            signal.info.device_area = "UNKNOWN";
            signal.info.device_location = "UNKNOWN";
            signal.info.device_attribute = "UNKNOWN";
        }

        signal.info.units = "unknown";
        signal.info.signal_type = "measurement";

        // Read data values
        H5::DataSet dataset = file.openDataSet(signal_name);
        H5::DataSpace dataspace = dataset.getSpace();

        hsize_t dims[1];
        dataspace.getSimpleExtentDims(dims);
        signal.values.resize(dims[0]);

        // Try reading as double, fallback to float, then zeros
        try {
            dataset.read(signal.values.data(), H5::PredType::NATIVE_DOUBLE);
        } catch (...) {
            try {
                std::vector<float> float_data(dims[0]);
                dataset.read(float_data.data(), H5::PredType::NATIVE_FLOAT);
                std::transform(float_data.begin(), float_data.end(), signal.values.begin(),
                             [](float f) { return static_cast<double>(f); });
            } catch (...) {
                std::fill(signal.values.begin(), signal.values.end(), 0.0);
            }
        }

        dataset.close();
        dataspace.close();
        return signal;
    }
};

std::vector<IngestDataRequest> createRequests(const std::vector<SignalData>& signals,
                                            const std::string& providerId,
                                            const std::string& source_filepath) {
    std::vector<IngestDataRequest> requests;
    requests.reserve(signals.size());

    // Parse filename once for all signals from this file
    auto file_components = parseFilename(source_filepath);

    for (size_t i = 0; i < signals.size(); ++i) {
        const auto& signal = signals[i];
        std::string requestId = IngestUtils::generateRequestId("signal_" + std::to_string(i));

        // Build comprehensive attributes
        std::vector<Attribute> attributes;
        attributes.push_back(makeAttribute("pv_name", signal.info.full_name));
        attributes.push_back(makeAttribute("device_type", signal.info.device));
        attributes.push_back(makeAttribute("device_area", signal.info.device_area));
        attributes.push_back(makeAttribute("device_location", signal.info.device_location));
        attributes.push_back(makeAttribute("measurement_type", signal.info.device_attribute));

        if (file_components.is_valid) {
            attributes.push_back(makeAttribute("beam_line", file_components.beam_line));
            attributes.push_back(makeAttribute("acquisition_date", file_components.date));
            attributes.push_back(makeAttribute("acquisition_time", file_components.time_id));
        }

        std::vector<std::string> tags = {"h5_data"};

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

        std::vector<DataColumn> dataColumns = {
            makeDataColumn(signal.info.full_name, dataValues)
        };

        requests.push_back(makeIngestDataRequest(providerId, requestId, attributes, tags,
                                               eventMetadata, samplingClock, dataColumns));
    }
    return requests;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

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
            return 1;
        }

        IngestClient* client = nullptr;
        std::string provider_id;

        if (!local_only) {
            IngestClientConfig config;
            config.enable_spatial_enrichment = false;
            client = new IngestClient(config);

            std::vector<Attribute> attrs;
            attrs.push_back(makeAttribute("source", "h5_parser"));
            attrs.push_back(makeAttribute("version", "2.0"));
            std::vector<std::string> tags = {"h5_data", "accelerator_data"};

            auto response = client->registerProvider("H5DataProvider", attrs, tags);
            provider_id = response.registrationresult().providerid();
        }

        for (const auto& file : h5_files) {
            CleanH5Parser parser(h5_directory);

            if (parser.parseFile(file)) {
                auto signals = parser.getAllSignals();

                if (!local_only && !signals.empty()) {
                    auto requests = createRequests(signals, provider_id, file);
                    
                    try {
                        auto ingest_result = client->ingestBatch(requests, provider_id);
                    } catch (const std::exception& e) {
                        // Continue with next file
                    }
                }
            }
        }

        delete client;
        return 0;

    } catch (const std::exception& e) {
        return 1;
    }
}