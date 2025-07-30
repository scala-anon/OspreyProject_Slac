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
    const std::string &pv_name = archiver_response.metadata.name;
    signal.info.full_name = pv_name;

    // Parse PV name into components
    std::vector<std::string> parts;
    std::istringstream ss(pv_name);
    std::string part;
    while (std::getline(ss, part, ':')) {
        parts.push_back(part);
    }

    // Set device info with defaults
    const std::string unknown = "UNKNOWN";
    signal.info.device = parts.size() > 0 ? parts[0] : unknown;
    signal.info.device_area = parts.size() > 1 ? parts[1] : unknown;
    signal.info.device_location = parts.size() > 2 ? parts[2] : unknown;
    signal.info.device_attribute = parts.size() > 3 ? parts[3] : unknown;

    // Determine signal type
    if (!archiver_response.metadata.enums.empty()) {
        signal.info.signal_type = "enum";
    } else if (signal.info.device == "IOC") {
        signal.info.signal_type = "control";
    } else if (signal.info.device == "STPR") {
        signal.info.signal_type = "status";
    } else {
        signal.info.signal_type = "scalar";
    }

    // Set units
    auto egu_it = archiver_response.metadata.properties.find("EGU");
    signal.info.units = (egu_it != archiver_response.metadata.properties.end()) ? egu_it->second : "";

    // Initialize timestamp data
    const auto &data_points = archiver_response.data_points;
    signal.timestamps = std::make_shared<TimestampData>();
    signal.timestamps->count = data_points.size();
    signal.timestamps->is_regular_sampling = false;

    if (!data_points.empty()) {
        const auto &first = data_points.front();
        const auto &last = data_points.back();

        signal.timestamps->start_time_sec = first.secs;
        signal.timestamps->start_time_nano = first.nanos;
        signal.timestamps->end_time_sec = last.secs;
        signal.timestamps->end_time_nano = last.nanos;

        // Calculate period
        if (data_points.size() > 1) {
            uint64_t total_time_ns = (last.secs - first.secs) * 1000000000ULL + (last.nanos - first.nanos);
            signal.timestamps->period_nanos = total_time_ns / (data_points.size() - 1);
        } else {
            signal.timestamps->period_nanos = 1000000000;
        }
    }

    // Convert values
    signal.values.reserve(data_points.size());
    for (const auto &point : data_points) {
        signal.values.push_back(
            ArchiverUtils::isValidEpicsValue(point.value, point.severity, point.status) 
                ? point.value 
                : std::numeric_limits<double>::quiet_NaN()
        );
    }

    // Set file metadata
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

    // Create sampling clock
    SamplingClock sampling_clock = (signal.timestamps->count > 1) ?
        makeSamplingClock(signal.timestamps->start_time_sec, signal.timestamps->start_time_nano,
                         signal.timestamps->period_nanos, static_cast<uint32_t>(signal.timestamps->count)) :
        makeSamplingClock(signal.timestamps->start_time_sec, signal.timestamps->start_time_nano, 1000000000, 1);

    // Create data values
    std::vector<DataValue> data_values;
    data_values.reserve(signal.values.size());
    for (double value : signal.values) {
        data_values.push_back(makeDataValueWithDouble(value));
    }

    auto data_column = makeDataColumn(signal.info.full_name, data_values);

    // Create attributes - consolidated approach
    std::vector<Attribute> attributes;
    attributes.reserve(20); 

    // Core signal attributes
    const std::vector<std::pair<std::string, std::string>> core_attrs = {
        {"device", signal.info.device},
        {"device_area", signal.info.device_area},
        {"device_location", signal.info.device_location},
        {"device_attribute", signal.info.device_attribute},
        {"signal_type", signal.info.signal_type},
        {"units", signal.info.units},
        {"origin", signal.file_metadata.origin},
        {"project", signal.file_metadata.project},
        {"data_source", "archiver"}
    };

    for (const auto &[key, value] : core_attrs) {
        attributes.push_back(makeAttribute(key, value));
    }

    // Add EPICS metadata
    for (const auto &[key, value] : archiver_response.metadata.properties) {
        std::string attr_key = "epics_" + key;
        std::transform(attr_key.begin(), attr_key.end(), attr_key.begin(), ::tolower);
        attributes.push_back(makeAttribute(attr_key, value));
    }

    for (const auto &[key, value] : archiver_response.metadata.enums) {
        attributes.push_back(makeAttribute("epics_enum_" + key, value));
    }

    if (!archiver_response.metadata.description.empty()) {
        attributes.push_back(makeAttribute("epics_description", archiver_response.metadata.description));
    }

    // Calculate statistics
    size_t valid_count = 0, alarm_count = 0;
    for (const auto &point : archiver_response.data_points) {
        if (ArchiverUtils::isValidEpicsValue(point.value, point.severity, point.status)) {
            valid_count++;
        }
        if (point.severity > 0) {
            alarm_count++;
        }
    }

    double valid_percentage = signal.values.empty() ? 0.0 : 
        static_cast<double>(valid_count) / signal.values.size() * 100.0;

    // Add statistics attributes
    const std::vector<std::pair<std::string, std::string>> stats_attrs = {
        {"data_valid_count", std::to_string(valid_count)},
        {"data_total_count", std::to_string(signal.values.size())},
        {"data_valid_percentage", std::to_string(valid_percentage)},
        {"data_alarm_count", std::to_string(alarm_count)},
        {"has_enums", archiver_response.metadata.enums.empty() ? "false" : "true"}
    };

    for (const auto &[key, value] : stats_attrs) {
        attributes.push_back(makeAttribute(key, value));
    }

    // Create tags
    std::vector<std::string> tags = {
        "archiver_data", "epics_data", 
        "device_" + signal.info.device, 
        "project_" + signal.file_metadata.project
    };

    if (!archiver_response.metadata.enums.empty()) tags.push_back("has_enums");
    if (alarm_count > 0) tags.push_back("has_alarms");
    if (valid_percentage > 90.0) tags.push_back("high_quality_data");
    else if (valid_percentage < 10.0) tags.push_back("low_quality_data");

    auto event_metadata = makeEventMetadata(
        "EPICS Archiver Data: " + signal.info.full_name,
        signal.timestamps->start_time_sec, signal.timestamps->start_time_nano,
        signal.timestamps->end_time_sec, signal.timestamps->end_time_nano);

    return makeIngestDataRequest(providerId, requestId, attributes, tags, 
                               event_metadata, sampling_clock, {data_column});
}

// Date conversion function for DDMMYYYY format
std::string convertDateFormat(const std::string& ddmmyyyy) {
    if (ddmmyyyy.length() != 8) return "";
    
    std::string dd = ddmmyyyy.substr(0, 2);
    std::string mm = ddmmyyyy.substr(2, 2);
    std::string yyyy = ddmmyyyy.substr(4, 4);
    
    return mm + "-" + dd + "-" + yyyy;
}

struct ProgramArgs {
    std::string single_pv;
    std::vector<std::string> multiple_pvs;
    std::string date_str;
    std::string config_path = "config/ingestion_config.json";
    std::string server_address;
    int hours = 24;
    bool local_only = false;
};

ProgramArgs parseArguments(int argc, char *argv[]) {
    ProgramArgs args;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg.find("--pv=") == 0) {
            args.single_pv = arg.substr(5);
        } else if (arg.find("--pvs=") == 0) {
            std::istringstream ss(arg.substr(6));
            std::string pv;
            while (std::getline(ss, pv, ',')) {
                args.multiple_pvs.push_back(pv);
            }
        } else if (arg.find("--date=") == 0) {
            std::string input_date = arg.substr(7);
            // Handle both DDMMYYYY and MM-DD-YYYY formats
            if (input_date.length() == 8 && input_date.find('-') == std::string::npos) {
                args.date_str = convertDateFormat(input_date);
            } else {
                args.date_str = input_date;
            }
        } else if (arg.find("--hours=") == 0) {
            args.hours = std::stoi(arg.substr(8));
        } else if (arg == "--local-only") {
            args.local_only = true;
        } else if (arg.find("--server=") == 0) {
            args.server_address = arg.substr(9);
        } else if (arg.find("--config=") == 0) {
            args.config_path = arg.substr(9);
        }
    }

    // Set default date if empty
    if (args.date_str.empty()) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%m-%d-%Y");
        args.date_str = oss.str();
    }

    return args;
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    auto args = parseArguments(argc, argv);

    // Determine PVs to query
    std::vector<std::string> pv_names;
    if (!args.single_pv.empty()) {
        pv_names.push_back(args.single_pv);
    } else if (!args.multiple_pvs.empty()) {
        pv_names = std::move(args.multiple_pvs);
    } else {
        return 1;
    }

    try {
        ArchiverClient archiver_client;
        std::vector<ArchiverResponse> archiver_responses;
        std::vector<SignalData> all_signals;

        // Query archiver for each PV
        for (const auto &pv_name : pv_names) {
            auto response = archiver_client.queryPvByDate(pv_name, args.date_str, args.hours);

            if (!response.success) {
                continue;
            }

            archiver_responses.push_back(response);
            auto signals = convertArchiverToSignalData(response);
            all_signals.insert(all_signals.end(), signals.begin(), signals.end());
        }

        if (all_signals.empty()) {
            return 1;
        }

        if (args.local_only) {
            return 0;
        }

        // Initialize client configuration
        IngestClientConfig client_config;
        try {
            client_config = IngestClientConfig::fromConfigFile(args.config_path);
        } catch (const std::exception&) {
            // Use default config
        }

        if (!args.server_address.empty()) {
            client_config.server_address = args.server_address;
        }

        IngestClient client(client_config);

        // Register provider
        std::vector<Attribute> provider_attributes = {
            makeAttribute("facility", "SLAC"),
            makeAttribute("data_source", "epics_archiver"),
            makeAttribute("signal_count", std::to_string(all_signals.size()))
        };

        std::vector<std::string> provider_tags = {"epics", "archiver", "slac"};

        auto registration_response = client.registerProvider("EPICS_Archiver_Provider",
                                                             provider_attributes, provider_tags);

        std::string providerId;
        if (registration_response.has_registrationresult()) {
            providerId = registration_response.registrationresult().providerid();   
        }

        if (providerId.empty()) {
            return 1;
        }

        // Create ingestion requests
        std::vector<IngestDataRequest> requests;
        requests.reserve(all_signals.size());

        for (size_t i = 0; i < all_signals.size(); ++i) {
            std::string requestId = IngestUtils::generateRequestId("archiver_signal_" + std::to_string(i));
            requests.push_back(createArchiverIngestRequest(all_signals[i], archiver_responses[i],
                                                         providerId, requestId));
        }

        // Perform ingestion
        IngestionResult result = client.ingestBatch(requests, providerId);

        return result.success ? 0 : 1;
    } catch (const std::exception &e) {
        return 1;
    }
}