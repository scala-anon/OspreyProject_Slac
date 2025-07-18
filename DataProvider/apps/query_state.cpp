#include "query_client.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <ctime>
#include <algorithm>

struct StateInfo
{
    std::string state_name;
    uint64_t timestamp_sec;
    uint64_t timestamp_nano;
    int severity;
    int status;
};

struct StateSummary
{
    std::map<std::string, size_t> state_counts;
    std::map<std::string, double> state_percentages;
    size_t total_points = 0;
    size_t alarm_count = 0;
    std::string description;
};

uint64_t parseDate(const std::string &date_str)
{
    std::tm tm = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%m-%d-%Y");
    if (ss.fail())
    {
        throw std::runtime_error("Invalid date format. Use MM-DD-YYYY");
    }
    return std::mktime(&tm);
}

TimeRange createTimeRange(const std::string &date_str, int hours = 24)
{
    uint64_t start_epoch = parseDate(date_str);
    return TimeRange(start_epoch, 0, start_epoch + (hours * 3600), 0);
}

std::string formatTimestamp(uint64_t epoch_sec, uint64_t nano_sec = 0)
{
    auto time_t = static_cast<std::time_t>(epoch_sec);
    auto tm = *std::localtime(&time_t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (nano_sec > 0)
    {
        oss << "." << std::setfill('0') << std::setw(3) << (nano_sec / 1000000);
    }

    return oss.str();
}

std::map<std::string, std::string> extractEnumMapping(const std::map<std::string, std::string> &attributes)
{
    std::map<std::string, std::string> enum_map;

    for (const auto &[key, value] : attributes)
    {
        if (key.find("epics_enum_") == 0)
        {
            std::string enum_key = key.substr(11); // Remove "epics_enum_" prefix
            enum_map[enum_key] = value;
        }
    }

    return enum_map;
}

std::string translateEnumValue(double raw_value, const std::map<std::string, std::string> &enum_map)
{
    std::string key = std::to_string((int)raw_value);
    auto it = enum_map.find(key);
    if (it != enum_map.end())
    {
        return it->second;
    }
    return "Unknown (" + std::to_string((int)raw_value) + ")";
}

std::vector<StateInfo> extractStateData(const QueryDataResponse &response,
                                        const std::map<std::string, std::string> &enum_map)
{
    std::vector<StateInfo> states;

    if (!response.has_querydata())
        return states;

    for (const auto &bucket : response.querydata().databuckets())
    {
        if (bucket.has_datacolumn() && bucket.has_datatimestamps())
        {
            const auto &data_column = bucket.datacolumn();
            const auto &timestamps = bucket.datatimestamps();

            // Extract timestamps
            std::vector<uint64_t> time_secs, time_nanos;
            if (timestamps.has_samplingclock())
            {
                const auto &clock = timestamps.samplingclock();
                uint64_t start_sec = clock.starttime().epochseconds();
                uint64_t start_nano = clock.starttime().nanoseconds();
                uint64_t period_ns = clock.periodnanos();

                for (uint32_t i = 0; i < clock.count(); ++i)
                {
                    uint64_t total_ns = start_nano + (i * period_ns);
                    time_secs.push_back(start_sec + total_ns / 1000000000);
                    time_nanos.push_back(total_ns % 1000000000);
                }
            }

            // Extract values
            for (size_t i = 0; i < data_column.datavalues_size() && i < time_secs.size(); ++i)
            {
                const auto &data_value = data_column.datavalues(i);

                StateInfo state;
                state.timestamp_sec = time_secs[i];
                state.timestamp_nano = time_nanos[i];
                state.severity = 0; // Default values
                state.status = 0;

                if (data_value.value_case() == DataValue::kDoubleValue)
                {
                    double raw_value = data_value.doublevalue();
                    state.state_name = translateEnumValue(raw_value, enum_map);
                }
                else
                {
                    state.state_name = "Invalid";
                }

                states.push_back(state);
            }
        }
    }

    return states;
}

StateSummary calculateStateSummary(const std::vector<StateInfo> &states,
                                   const std::map<std::string, std::string> &attributes)
{
    StateSummary summary;
    summary.total_points = states.size();

    // Extract description
    auto desc_it = attributes.find("epics_description");
    if (desc_it != attributes.end())
    {
        summary.description = desc_it->second;
    }

    // Count states
    for (const auto &state : states)
    {
        summary.state_counts[state.state_name]++;
        if (state.severity > 0)
        {
            summary.alarm_count++;
        }
    }

    // Calculate percentages
    for (const auto &[state_name, count] : summary.state_counts)
    {
        summary.state_percentages[state_name] =
            summary.total_points > 0 ? (double)count / summary.total_points * 100.0 : 0.0;
    }

    return summary;
}

void printStateSummary(const std::string &pv_name, const StateSummary &summary,
                       const std::vector<StateInfo> &states, const TimeRange &time_range,
                       size_t show_recent = 10)
{

    std::cout << "PV: " << pv_name;
    if (!summary.description.empty())
    {
        std::cout << " (" << summary.description << ")";
    }
    std::cout << std::endl;

    std::cout << "  Time Range: " << formatTimestamp(time_range.start_epoch_sec)
              << " to " << formatTimestamp(time_range.end_epoch_sec) << std::endl;
    std::cout << "  Total State Changes: " << summary.total_points << std::endl;

    if (!states.empty())
    {
        std::cout << "\nRecent States:" << std::endl;
        size_t start_idx = states.size() > show_recent ? states.size() - show_recent : 0;
        for (size_t i = start_idx; i < states.size(); ++i)
        {
            const auto &state = states[i];
            std::cout << "  [" << formatTimestamp(state.timestamp_sec, state.timestamp_nano)
                      << "] " << state.state_name << std::endl;
        }
    }

    if (!summary.state_counts.empty())
    {
        std::cout << "\nState Summary:" << std::endl;

        // Sort by count (descending)
        std::vector<std::pair<std::string, size_t>> sorted_states(
            summary.state_counts.begin(), summary.state_counts.end());
        std::sort(sorted_states.begin(), sorted_states.end(),
                  [](const auto &a, const auto &b)
                  { return a.second > b.second; });

        for (const auto &[state_name, count] : sorted_states)
        {
            double percentage = summary.state_percentages.at(state_name);
            std::cout << "  " << state_name << ": " << count << " occurrences ("
                      << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
        }
    }

    if (summary.alarm_count > 0)
    {
        std::cout << "\nAlarm Status: " << summary.alarm_count
                  << " alarm conditions detected" << std::endl;
    }

    std::cout << std::endl;
}

void printUsage(const std::string &program_name)
{
    std::cout << "Control System State Query Tool\n";
    std::cout << "USAGE: " << program_name << " [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  --pv=NAME                     Query single state PV\n";
    std::cout << "  --pvs=NAME1,NAME2,...         Query multiple state PVs (comma-separated)\n";
    std::cout << "  --pattern=REGEX               Query PVs matching regex pattern\n";
    std::cout << "  --date=MM-DD-YYYY             Date to query (default: today)\n";
    std::cout << "  --hours=N                     Hours from date (default: 24)\n";
    std::cout << "  --recent=N                    Show N recent state changes (default: 10)\n";
    std::cout << "  --summary-only                Show only state summaries\n";
    std::cout << "  --server=ADDRESS              Server address (default: localhost:50052)\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  " << program_name << " --pv=IOC:BSY0:MP01:PC_RATE --date=07-14-2025\n";
    std::cout << "  " << program_name << " --pattern=\"IOC.*RATE\" --date=07-14-2025 --summary-only\n";
    std::cout << "  " << program_name << " --pvs=IOC:BSY0:MP01:PC_RATE,STPR:BSYH:2:STD2_IN_A --recent=5\n";
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    std::string single_pv, pattern, date_str, server_address = "localhost:50052";
    std::vector<std::string> multiple_pvs;
    int hours = 24;
    size_t show_recent = 10;
    bool summary_only = false;

    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg.find("--pv=") == 0)
        {
            single_pv = arg.substr(5);
        }
        else if (arg.find("--pvs=") == 0)
        {
            std::stringstream ss(arg.substr(6));
            std::string pv;
            while (std::getline(ss, pv, ','))
            {
                multiple_pvs.push_back(pv);
            }
        }
        else if (arg.find("--pattern=") == 0)
        {
            pattern = arg.substr(10);
        }
        else if (arg.find("--date=") == 0)
        {
            date_str = arg.substr(7);
        }
        else if (arg.find("--hours=") == 0)
        {
            hours = std::stoi(arg.substr(8));
        }
        else if (arg.find("--recent=") == 0)
        {
            show_recent = std::stoul(arg.substr(9));
        }
        else if (arg == "--summary-only")
        {
            summary_only = true;
        }
        else if (arg.find("--server=") == 0)
        {
            server_address = arg.substr(9);
        }
    }

    // Default to today if no date specified
    if (date_str.empty())
    {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%m-%d-%Y");
        date_str = oss.str();
    }

    // Determine PVs to query
    std::vector<std::string> pv_names;
    if (!single_pv.empty())
    {
        pv_names.push_back(single_pv);
    }
    else if (!multiple_pvs.empty())
    {
        pv_names = multiple_pvs;
    }
    else if (!pattern.empty())
    {
        // Use pattern matching (simplified - would need metadata query)
        std::cerr << "Pattern matching not yet implemented. Use --pv or --pvs for now." << std::endl;
        return 1;
    }
    else
    {
        std::cerr << "Must specify --pv, --pvs, or --pattern" << std::endl;
        return 1;
    }

    try
    {
        QueryClient client(server_address);
        TimeRange time_range = createTimeRange(date_str, hours);

        // Query each PV
        for (const auto &pv_name : pv_names)
        {
            try
            {
                // Query the PV data
                auto request = makeQueryDataRequest({pv_name},
                                                    time_range.getStartTimestamp(),
                                                    time_range.getEndTimestamp());
                auto responses = client.queryDataStream(request);

                if (responses.empty())
                {
                    std::cout << "PV: " << pv_name << " - No data found" << std::endl;
                    continue;
                }

                // Query PV metadata to get enum mappings
                auto metadata_request = makeQueryPvMetadataRequest({pv_name});
                auto metadata_response = client.queryPvMetadata(metadata_request);

                // For now, we'll need to query the bucket directly to get attributes
                // This is a simplified approach - in practice you'd need to extract
                // attributes from the bucket metadata
                std::map<std::string, std::string> attributes;
                std::map<std::string, std::string> enum_map;

                // Extract enum mapping (simplified - you'd get this from bucket attributes)
                // For demonstration, using hardcoded mapping for PC_RATE
                if (pv_name.find("PC_RATE") != std::string::npos)
                {
                    enum_map["0"] = "Invalid";
                    enum_map["1"] = "0 Hz";
                    enum_map["4"] = "1 Hz";
                    enum_map["5"] = "10 Hz";
                    enum_map["6"] = "30 Hz";
                    enum_map["7"] = "60 Hz";
                    enum_map["8"] = "120 Hz";
                    enum_map["9"] = "Unknown";
                    attributes["epics_description"] = "Pockels Cell Rate";
                }
                else if (pv_name.find("RG02_ACTRATE") != std::string::npos)
                {
                    enum_map["1"] = "HXR 00 / SXR 00";
                    enum_map["10"] = "HXR 120 / SXR 00";
                    enum_map["11"] = "HXR 1 / SXR 00";
                    enum_map["12"] = "HXR 10 / SXR 00";
                    attributes["epics_description"] = "Event Generator Rate";
                }
                else if (pv_name.find("STD2_IN_A") != std::string::npos)
                {
                    enum_map["0"] = "NOT_IN";
                    enum_map["1"] = "IN";
                    attributes["epics_description"] = "Beam Stopper Status";
                }

                // Extract state information
                std::vector<StateInfo> states;
                for (const auto &response : responses)
                {
                    auto response_states = extractStateData(response, enum_map);
                    states.insert(states.end(), response_states.begin(), response_states.end());
                }

                if (states.empty())
                {
                    std::cout << "PV: " << pv_name << " - No state data found" << std::endl;
                    continue;
                }

                // Calculate summary
                StateSummary summary = calculateStateSummary(states, attributes);

                // Print results
                if (summary_only)
                {
                    // Sort by percentage (descending)
                    std::vector<std::pair<std::string, double>> sorted_states;
                    for (const auto &[state_name, percentage] : summary.state_percentages)
                    {
                        sorted_states.push_back({state_name, percentage});
                    }
                    std::sort(sorted_states.begin(), sorted_states.end(),
                              [](const auto &a, const auto &b)
                              { return a.second > b.second; });

                    std::cout << pv_name << ": ";
                    for (size_t i = 0; i < sorted_states.size(); ++i)
                    {
                        const auto &[state_name, percentage] = sorted_states[i];
                        std::cout << state_name << " (" << std::fixed << std::setprecision(1)
                                  << percentage << "%)";
                        if (i < sorted_states.size() - 1)
                        {
                            std::cout << ", ";
                        }
                    }
                    std::cout << std::endl;
                }
                else
                {
                    printStateSummary(pv_name, summary, states, time_range, show_recent);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error querying " << pv_name << ": " << e.what() << std::endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
