#include "archiver_client.hpp"
#include "ingest_client.hpp"
#include "h5_parser.hpp"
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>

std::vector<SignalData> convertArchiverToSignalData(const ArchiverResponse &archiver_response) {
    std::vector<SignalData> signals;

    if (!archiver_response.success || archiver_response.data_points.empty()) {
        return signals;
    }

    SignalData signal;

    std::string pv_name = archiver_response.metadata.name;
    signal.info.full_name = pv_name;

    std::vector<std::string> parts;
    std::stringstream ss(pv_name);
    std::string part;
    while (std::getline(ss, part, ':')) {
        parts.push_back(part);
    }

    if (parts.size() >= 3) {
        signal.info.device = parts[0];
        signal.info.device_area = parts[1];
        signal.info.device_location = parts[2];
        if (parts.size() > 3) {
            signal.info.device_attribute = parts[3];
        }
    } else {
        signal.info.device = "UNKNOWN";
        signal.info.device_area = "UNKNOWN";
        signal.info.device_location = "UNKNOWN";
        signal.info.device_attribute = "UNKNOWN";
    }

    if (!archiver_response.metadata.enums.empty()) {
        signal.info.signal_type = "enum";
    } else if (signal.info.device == "IOC") {
        signal.info.signal_type = "control";
    } else if (signal.info.device == "STPR") {
        signal.info.signal_type = "status";
    } else {
        signal.info.signal_type = "scalar";
    }

    signal.info.units = archiver_response.metadata.properties.count("EGU") ? 
                       archiver_response.metadata.properties.at("EGU") : "";

    signal.timestamps = std::make_shared<TimestampData>();
    signal.timestamps->count = archiver_response.data_points.size();
    signal.timestamps->is_regular_sampling = false;

    if (!archiver_response.data_points.empty()) {
        const auto &first_point = archiver_response.data_points.front();
        const auto &last_point = archiver_response.data_points.back();

        signal.timestamps->start_time_sec = first_point.secs;
        signal.timestamps->start_time_nano = first_point.nanos;
        signal.timestamps->end_time_sec = last_point.secs;
        signal.timestamps->end_time_nano = last_point.nanos;

        if (archiver_response.data_points.size() > 1) {
            uint64_t total_time_ns = (last_point.secs - first_point.secs) * 1000000000ULL +
                                     (last_point.nanos - first_point.nanos);
            signal.timestamps->period_nanos = total_time_ns / (archiver_response.data_points.size() - 1);
        } else {
            signal.timestamps->period_nanos = 1000000000;
        }
    }

    signal.values.reserve(archiver_response.data_points.size());
    for (const auto &point : archiver_response.data_points) {
        if (ArchiverUtils::isValidEpicsValue(point.value, point.severity, point.status)) {
            signal.values.push_back(point.value);
        } else {
            signal.values.push_back(std::numeric_limits<double>::quiet_NaN());
        }
    }

    signal.file_metadata.origin = "ARCHIVER";
    signal.file_metadata.project = "epics_archive";
    signal.file_metadata.pathway = "lcls-archapp.slac.stanford.edu";

    signals.push_back(std::move(signal));
    return signals;
}

IngestDataRequest createArchiverIngestRequest(const SignalData &signal,
                                              const ArchiverResponse &archiver_response,
                                              const std::string &providerId,
                                              const std::string &requestId) {

    SamplingClock sampling_clock;
    if (signal.timestamps->count > 1) {
        sampling_clock = makeSamplingClock(
            signal.timestamps->start_time_sec,
            signal.timestamps->start_time_nano,
            signal.timestamps->period_nanos,
            static_cast<uint32_t>(signal.timestamps->count));
    } else {
        sampling_clock = makeSamplingClock(
            signal.timestamps->start_time_sec,
            signal.timestamps->start_time_nano,
            1000000000,
            1);
    }

    std::vector<DataValue> data_values;
    data_values.reserve(signal.values.size());
    for (double value : signal.values) {
        data_values.push_back(makeDataValueWithDouble(value));
    }

    auto data_column = makeDataColumn(signal.info.full_name, data_values);

    std::vector<Attribute> attributes;
    attributes.push_back(makeAttribute("device", signal.info.device));
    attributes.push_back(makeAttribute("device_area", signal.info.device_area));
    attributes.push_back(makeAttribute("device_location", signal.info.device_location));
    attributes.push_back(makeAttribute("device_attribute", signal.info.device_attribute));
    attributes.push_back(makeAttribute("signal_type", signal.info.signal_type));
    attributes.push_back(makeAttribute("units", signal.info.units));
    attributes.push_back(makeAttribute("origin", signal.file_metadata.origin));
    attributes.push_back(makeAttribute("project", signal.file_metadata.project));
    attributes.push_back(makeAttribute("data_source", "archiver"));

    for (const auto &[key, value] : archiver_response.metadata.properties) {
        std::string attr_key = "epics_" + key;
        std::transform(attr_key.begin(), attr_key.end(), attr_key.begin(), ::tolower);
        attributes.push_back(makeAttribute(attr_key, value));
    }

    for (const auto &[key, value] : archiver_response.metadata.enums) {
        std::string attr_key = "epics_enum_" + key;
        attributes.push_back(makeAttribute(attr_key, value));
    }

    if (!archiver_response.metadata.description.empty()) {
        attributes.push_back(makeAttribute("epics_description", archiver_response.metadata.description));
    }

    size_t valid_count = 0;
    size_t alarm_count = 0;
    for (const auto &point : archiver_response.data_points) {
        if (ArchiverUtils::isValidEpicsValue(point.value, point.severity, point.status)) {
            valid_count++;
        }
        if (point.severity > 0) {
            alarm_count++;
        }
    }

    double valid_percentage = signal.values.empty() ? 0.0 : (double)valid_count / signal.values.size() * 100.0;

    attributes.push_back(makeAttribute("data_valid_count", std::to_string(valid_count)));
    attributes.push_back(makeAttribute("data_total_count", std::to_string(signal.values.size())));
    attributes.push_back(makeAttribute("data_valid_percentage", std::to_string(valid_percentage)));
    attributes.push_back(makeAttribute("data_alarm_count", std::to_string(alarm_count)));
    attributes.push_back(makeAttribute("has_enums", archiver_response.metadata.enums.empty() ? "false" : "true"));

    std::vector<std::string> tags;
    tags.push_back("archiver_data");
    tags.push_back("epics_data");
    tags.push_back("device_" + signal.info.device);
    tags.push_back("project_" + signal.file_metadata.project);

    if (!archiver_response.metadata.enums.empty()) {
        tags.push_back("has_enums");
    }
    if (alarm_count > 0) {
        tags.push_back("has_alarms");
    }
    if (valid_percentage > 90.0) {
        tags.push_back("high_quality_data");
    } else if (valid_percentage < 10.0) {
        tags.push_back("low_quality_data");
    }

    auto event_metadata = makeEventMetadata(
        "EPICS Archiver Data: " + signal.info.full_name,
        signal.timestamps->start_time_sec,
        signal.timestamps->start_time_nano,
        signal.timestamps->end_time_sec,
        signal.timestamps->end_time_nano);

    return makeIngestDataRequest(
        providerId,
        requestId,
        attributes,
        tags,
        event_metadata,
        sampling_clock,
        {data_column});
}

void printUsage(const std::string &program_name) {
    std::cout << "EPICS Archiver to DataProvider Ingestion\n";
    std::cout << "USAGE: " << program_name << " [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  --pv=NAME                     Query single PV\n";
    std::cout << "  --pvs=NAME1,NAME2,...         Query multiple PVs (comma-separated)\n";
    std::cout << "  --date=MM-DD-YYYY             Date to query (default: today)\n";
    std::cout << "  --hours=N                     Hours from date (default: 24)\n";
    std::cout << "  --local-only                  Parse only, don't send to server\n";
    std::cout << "  --server=ADDRESS              Server address\n";
    std::cout << "  --config=PATH                 Config file path\n\n";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string single_pv, date_str, config_path = "config/ingestion_config.json";
    std::vector<std::string> multiple_pvs;
    int hours = 24;
    bool local_only = false;
    std::string server_address;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg.find("--pv=") == 0) {
            single_pv = arg.substr(5);
        } else if (arg.find("--pvs=") == 0) {
            std::stringstream ss(arg.substr(6));
            std::string pv;
            while (std::getline(ss, pv, ',')) {
                multiple_pvs.push_back(pv);
            }
        } else if (arg.find("--date=") == 0) {
            date_str = arg.substr(7);
        } else if (arg.find("--hours=") == 0) {
            hours = std::stoi(arg.substr(8));
        } else if (arg == "--local-only") {
            local_only = true;
        } else if (arg.find("--server=") == 0) {
            server_address = arg.substr(9);
        } else if (arg.find("--config=") == 0) {
            config_path = arg.substr(9);
        }
    }

    // Default to today if no date specified
    if (date_str.empty()) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%m-%d-%Y");
        date_str = oss.str();
    }

    // Determine PVs to query
    std::vector<std::string> pv_names;
    if (!single_pv.empty()) {
        pv_names.push_back(single_pv);
    } else if (!multiple_pvs.empty()) {
        pv_names = multiple_pvs;
    } else {
        std::cerr << "Must specify --pv or --pvs" << std::endl;
        return 1;
    }

    try {
        ArchiverClient archiver_client;
        std::vector<ArchiverResponse> archiver_responses;
        std::vector<SignalData> all_signals;

        // Query archiver for each PV
        for (const auto &pv_name : pv_names) {
            auto response = archiver_client.queryPvByDate(pv_name, date_str, hours);

            if (!response.success) {
                std::cerr << "Failed to query " << pv_name << ": " << response.error_message << std::endl;
                continue;
            }

            archiver_responses.push_back(response);

            auto signals = convertArchiverToSignalData(response);
            all_signals.insert(all_signals.end(), signals.begin(), signals.end());
        }

        if (all_signals.empty()) {
            std::cerr << "No valid signals to process" << std::endl;
            return 1;
        }

        std::cout << "Processed " << all_signals.size() << " signals from archiver" << std::endl;

        if (local_only) {
            std::cout << "Local processing complete" << std::endl;
            return 0;
        }

        IngestClientConfig client_config;
        try {
            client_config = IngestClientConfig::fromConfigFile(config_path);
        } catch (const std::exception&) {
            // Use default config
        }

        if (!server_address.empty()) {
            client_config.server_address = server_address;
        }

        IngestClient client(client_config);

        // Register provider
        std::vector<Attribute> provider_attributes;
        provider_attributes.push_back(makeAttribute("facility", "SLAC"));
        provider_attributes.push_back(makeAttribute("data_source", "epics_archiver"));
        provider_attributes.push_back(makeAttribute("signal_count", std::to_string(all_signals.size())));

        std::vector<std::string> provider_tags = {"epics", "archiver", "slac"};

        auto registration_response = client.registerProvider("EPICS_Archiver_Provider",
                                                             provider_attributes, provider_tags);

        std::string providerId;
        if (registration_response.has_registrationresult()) {
            providerId = registration_response.registrationresult().providerid();   
        }

        if (providerId.empty()) {
            std::cerr << "Failed to get provider ID from registration response" << std::endl;
            return 1;
        }

        std::cout << "Registered as provider: " << providerId << std::endl;

        // Create ingestion requests
        std::vector<IngestDataRequest> requests;
        requests.reserve(all_signals.size());

        for (size_t i = 0; i < all_signals.size(); ++i) {
            std::string requestId = IngestUtils::generateRequestId("archiver_signal_" + std::to_string(i));
            auto request = createArchiverIngestRequest(all_signals[i], archiver_responses[i],
                                                       providerId, requestId);
            requests.push_back(std::move(request));
        }

        // Fixed: Use ingestBatch instead of ingestWithSpatialEnrichment
        std::cout << "Starting ingestion..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();

        IngestionResult result = client.ingestBatch(requests, providerId);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration<double>(end_time - start_time);

        std::cout << "Ingestion complete:" << std::endl;
        std::cout << "  Processed: " << result.successful_requests << "/" << result.total_requests << std::endl;
        std::cout << "  Success rate: " << std::fixed << std::setprecision(1)
                  << (result.getSuccessRate() * 100.0) << "%" << std::endl;
        std::cout << "  Time: " << std::setprecision(1) << total_time.count() << "s" << std::endl;

        if (result.failed_requests > 0) {
            std::cout << "  Failed: " << result.failed_requests << " requests" << std::endl;
        }

        return result.success ? 0 : 1;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}