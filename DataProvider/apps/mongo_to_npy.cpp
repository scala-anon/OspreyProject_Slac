// mongo_to_npy.cpp - Main app for converting MongoDB/MLDP data to NPY format
#include "mongo_to_npy_parser.hpp"
#include "query_client.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

void printUsage(const char* program_name) {
    std::cout << R"(
 ╔════════════════════════════════════════════════════════════════╗
 ║                                                                ║
 ║  ███╗   ███╗ ██████╗ ███╗   ██╗ ██████╗  ██████╗               ║
 ║  ████╗ ████║██╔═══██╗████╗  ██║██╔════╝ ██╔═══██╗              ║
 ║  ██╔████╔██║██║   ██║██╔██╗ ██║██║  ███╗██║   ██║              ║
 ║  ██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║██║   ██║              ║
 ║  ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║╚██████╔╝╚██████╔╝              ║
 ║  ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝  ╚═════╝               ║
 ║                                                                ║
 ║        ████████╗ ██████╗     ███╗   ██╗██████╗ ██╗   ██╗       ║
 ║        ╚══██╔══╝██╔═══██╗    ████╗  ██║██╔══██╗╚██╗ ██╔╝       ║
 ║           ██║   ██║   ██║    ██╔██╗ ██║██████╔╝ ╚████╔╝        ║
 ║           ██║   ██║   ██║    ██║╚██╗██║██╔═══╝   ╚██╔╝         ║
 ║           ██║   ╚██████╔╝    ██║ ╚████║██║        ██║          ║
 ║           ╚═╝    ╚═════╝     ╚═╝  ╚═══╝╚═╝        ╚═╝          ║
 ║                                                                ║
 ║              MongoDB to NumPy/PyTorch Converter                ║
 ║               Convert ingested H5 data for ML                  ║
 ║                            Version 2.0                         ║
 ║                                                                ║
 ╚════════════════════════════════════════════════════════════════╝

)" << std::endl;

    std::cout << "USAGE: " << program_name << " <command> [options]\n\n";

    std::cout << "COMMANDS:\n";
    std::cout << "  discover                          Find available PVs in MongoDB\n";
    std::cout << "  export-all [hours_back]           Export all PVs (default: 24 hours)\n";
    std::cout << "  export-pattern <pattern>          Export PVs matching regex pattern\n";
    std::cout << "  export-pvs <pv1,pv2,pv3...>      Export specific PVs (comma-separated)\n";
    std::cout << "  export-timeseries <pvs> [seq_len] Export for LSTM/RNN (default seq: 50)\n";
    std::cout << "  export-dataset <pvs> <name>       Export named dataset for ML\n\n";
}

std::vector<std::string> parseCommaSeparatedPVs(const std::string& pv_string) {
    std::vector<std::string> pvs;
    std::stringstream ss(pv_string);
    std::string pv;

    while (std::getline(ss, pv, ',')) {
        // Trim whitespace
        pv.erase(0, pv.find_first_not_of(" \t"));
        pv.erase(pv.find_last_not_of(" \t") + 1);
        if (!pv.empty()) {
            pvs.push_back(pv);
        }
    }

    return pvs;
}

std::pair<uint64_t, uint64_t> getTimeRange(int hours_back = 24) {
    // ALWAYS use your actual historical data range instead of current time
    uint64_t actual_data_start = 1750690485;  // June 23, 2025 14:54:45 UTC
    uint64_t actual_data_end = 1750706894;    // June 23, 2025 19:28:14 UTC

    std::cout << "Using historical data range (June 23, 2025): "
              << actual_data_start << " to " << actual_data_end << std::endl;

    return {actual_data_start, actual_data_end};
}

void printPVPatterns(MongoToNPYParser& parser) {
    std::cout << "PV PATTERNS:\n";

    // Enhanced PV patterns based on LCLS-II Physics Requirements Document
    std::vector<std::pair<std::string, std::string>> patterns = {
        // Existing patterns
        {".*BPM.*", "Beam Position Monitors"},
        {".*GCC.*", "Gas Cell Chambers"},
        {".*ACCL.*", "Accelerator components"},
        {".*BLD.*", "Beam Line Data"},
        {".*-X.*", "X-axis measurements"},
        {".*-Y.*", "Y-axis measurements"},

        // LCLS-II Beamline Area Patterns
        {".*GUNB.*", "Gun/Buncher Area (Cathode to L0B)"},
        {".*L0B.*", "L0B Linac (Cryomodule CM01)"},
        {".*HTR.*", "Heater/Emittance Measurement"},
        {".*DIAG0.*", "Post-Laser-Heater Diagnostics"},
        {".*COL0.*|.*COL1.*", "Collimation Sections"},
        {".*L1B.*", "L1B Linac (Cryomodules CM02-03)"},
        {".*L2B.*", "L2B Linac (Cryomodules CM04-15)"},
        {".*L3B.*", "L3B Linac (Cryomodules CM16-35)"},
        {".*BC1B.*|.*BC2B.*", "Bunch Compressors"},
        {".*EMIT2.*", "Post-BC2 Emittance Diagnostics"},
        {".*EXT.*", "Linac Extension"},
        {".*DOG.*", "Dogleg Extraction"},
        {".*BYP.*", "PEP-II e- Bypass Line"},
        {".*SPH.*|.*SPS.*", "Spreader Systems (HXR/SXR)"},
        {".*SLTH.*|.*SLTS.*", "Spreader BSY Lines"},
        {".*BSYS.*|.*BSYH.*", "BSY Merger Areas"},
        {".*LTUS.*|.*LTUH.*", "Transport Lines (SXR/HXR)"},
        {".*UNDS.*|.*UNDH.*", "Undulator Areas (SXR/HXR)"},
        {".*DMPS.*|.*DMPH.*", "Post-Undulator/Dump Lines"},
        {".*SFTS.*|.*SFTH.*", "Safety Dump Lines"},
        {".*SXTES.*|.*HXTES.*", "X-ray Transport/Experimental"},
        {".*SPD.*", "BSY Dump Lines"},
        {".*SLTD.*", "BSY Dump Transport"},
        {".*DASEL.*", "End Station A Line"},

        // Device Type Patterns
        {".*QUAD.*|.*Q[A-Z].*", "Quadrupole Magnets"},
        {".*BEND.*|.*B[XY].*", "Bending Magnets"},
        {".*CORR.*", "Corrector Magnets"},
        {".*TORO.*|.*TMIT.*", "Toroids/Current Monitors"},
        {".*WIRE.*|.*WS.*", "Wire Scanners"},
        {".*YAG.*", "YAG Screens"},
        {".*CAMR.*|.*CAM.*", "Cameras"},
        {".*KLYS.*|.*SBST.*", "Klystrons/RF Systems"},
        {".*TCAV.*", "Transverse Cavities"},
        {".*VALVE.*|.*VGC.*", "Vacuum Valves/Gauges"},
        {".*TEMP.*|.*RTD.*", "Temperature Monitors"},
        {".*PRES.*", "Pressure Monitors"},
        {".*FLOW.*", "Flow Monitors"},
        {".*MIRROR.*|.*MR[0-9].*", "X-ray Mirrors"},
        {".*MONO.*", "Monochromators"},
        {".*SLIT.*", "Beam Slits"},
        {".*DUMP.*", "Beam Dumps"},
        {".*STOP.*|.*STOPPER.*", "Beam Stoppers"},
        {".*SHUTTER.*", "Shutters"},

        // Measurement Type Patterns
        {".*ENERGY.*|.*EDES.*", "Energy Measurements"},
        {".*PHASE.*|.*PDES.*", "Phase Measurements"},
        {".*AMPL.*|.*AACT.*", "Amplitude Measurements"},
        {".*BACT.*", "Actual B-field Readings"},
        {".*BDES.*", "Desired B-field Settings"},
        {".*TMIT.*", "Transmitted Charge"},
        {".*EDES.*", "Energy Setpoints"},
        {".*EACT.*", "Energy Readbacks"},
        {".*POS.*", "Position Measurements"},
        {".*STATUS.*|.*STAT.*", "Status Indicators"},
        {".*CTRL.*", "Control Signals"},
        {".*HEARTBEAT.*", "Heartbeat/Alive Signals"}
    };

    std::cout << "Pattern                  Count  Description\n";
    for (const auto& [pattern, description] : patterns) {
        auto matching = parser.findAvailablePVs(pattern);
        std::cout << std::setw(20) << std::left << pattern.substr(0, 19) << " "
                  << std::setw(5) << std::right << matching.size() << "  "
                  << std::setw(36) << std::left << description.substr(0, 50) << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    try {
        // Connect to MLDP QUERY SERVICE (not ingestion!)
        std::cout << "CONNECTING TO MLDP\n";
        std::cout << "Connecting to MLDP query server..." << std::endl;
        QueryClient queryClient;

        // Configure NPY export
        NPYExportConfig config;
        config.output_directory = "./npy_exports";
        config.export_as_float32 = true;           // PyTorch prefers float32
        config.normalize_timestamps = true;        // Relative timestamps
        config.export_separate_timestamps = true;  // Separate timestamp file
        config.export_pv_metadata = true;         // PV info for analysis
        config.fill_missing_values = true;        // Handle missing data

        MongoToNPYParser parser(queryClient, config);

        // === COMMAND: DISCOVER ===
        if (command == "discover") {
            std::cout << "\nDISCOVERING AVAILABLE PVS\n";

            auto all_pvs = parser.findAvailablePVs(".*");
            if (all_pvs.empty()) {
                std::cout << "No PVs found! Make sure you've run h5_to_dp to ingest H5 data first." << std::endl;
                return 1;
            }

            std::cout << "Found " << all_pvs.size() << " total PVs in MongoDB\n" << std::endl;

            printPVPatterns(parser);

            std::cout << "FIRST 10 PVS SAMPLE\n";
            std::cout << "No.  PV Name\n";
            for (size_t i = 0; i < std::min(size_t(10), all_pvs.size()); i++) {
                std::cout << std::setw(3) << (i+1) << "  " << all_pvs[i] << std::endl;
            }

        }
        // === COMMAND: EXPORT-ALL ===
        else if (command == "export-all") {
            int hours_back = 24;
            if (argc > 2) {
                hours_back = std::stoi(argv[2]);
            }

            std::cout << "\nEXPORTING ALL PVS\n";
            std::cout << "Time range: Last " << hours_back << " hours\n" << std::endl;

            auto all_pvs = parser.findAvailablePVs(".*");
            if (all_pvs.empty()) {
                std::cout << "No PVs found!" << std::endl;
                return 1;
            }

            // Limit for performance (remove this limit in production)
            if (all_pvs.size() > 50) {
                std::cout << "WARNING: Limiting to first 50 PVs for performance (found " << all_pvs.size() << " total)" << std::endl;
                all_pvs.resize(50);
            }

            auto [start_time, end_time] = getTimeRange(hours_back);

            bool success = parser.exportRawDataMatrix(all_pvs, start_time, end_time, "all_pvs");
            if (success) {
                std::cout << "\nEXPORT COMPLETE\n";
                std::cout << "All PVs exported successfully!" << std::endl;
                std::cout << "Load in PyTorch: cd npy_exports && python3 all_pvs_raw_loader.py" << std::endl;
            }

        }
        // === COMMAND: EXPORT-PATTERN ===
        else if (command == "export-pattern") {
            if (argc < 3) {
                std::cout << "Usage: " << argv[0] << " export-pattern <pattern>" << std::endl;
                return 1;
            }

            std::string pattern = argv[2];
            std::cout << "\nEXPORTING BY PATTERN\n";
            std::cout << "Pattern: " << pattern << std::endl;

            auto matching_pvs = parser.findAvailablePVs(pattern);
            if (matching_pvs.empty()) {
                std::cout << "No PVs found matching pattern: " << pattern << std::endl;
                return 1;
            }

            std::cout << "Found " << matching_pvs.size() << " matching PVs" << std::endl;

            auto [start_time, end_time] = getTimeRange(24);

            // Create safe filename from pattern
            std::string safe_name = pattern;
            std::replace_if(safe_name.begin(), safe_name.end(),
                           [](char c) { return !std::isalnum(c); }, '_');

            bool success = parser.exportRawDataMatrix(matching_pvs, start_time, end_time, "pattern_" + safe_name);
            if (success) {
                std::cout << "\nEXPORT COMPLETE\n";
                std::cout << "Pattern export successful!" << std::endl;
                std::cout << "PVs exported: " << matching_pvs.size() << std::endl;
                std::cout << "Load in PyTorch: cd npy_exports && python3 pattern_" << safe_name << "_loader.py" << std::endl;
            }

        }
        // === COMMAND: EXPORT-PVS ===
        else if (command == "export-pvs") {
            if (argc < 3) {
                std::cout << "Usage: " << argv[0] << " export-pvs <pv1,pv2,pv3...>" << std::endl;
                return 1;
            }

            std::string pv_string = argv[2];
            auto specific_pvs = parseCommaSeparatedPVs(pv_string);

            std::cout << "\nEXPORTING SPECIFIC PVS\n";
            std::cout << "PVs to export:\n";
            for (const auto& pv : specific_pvs) {
                std::cout << "  - " << pv << std::endl;
            }

            // Validate PVs exist
            if (!parser.validatePVsExist(specific_pvs)) {
                std::cout << "Some PVs don't exist. Use 'discover' command to see available PVs." << std::endl;
                return 1;
            }

            auto [start_time, end_time] = getTimeRange(24);

            bool success = parser.exportRawDataMatrix(specific_pvs, start_time, end_time, "specific_pvs");
            if (success) {
                std::cout << "\nEXPORT COMPLETE\n";
                std::cout << "Specific PVs exported successfully!" << std::endl;
                std::cout << "Load in PyTorch: cd npy_exports && python3 specific_pvs_loader.py" << std::endl;
            }

        }
        // === COMMAND: EXPORT-TIMESERIES ===
        else if (command == "export-timeseries") {
            if (argc < 3) {
                std::cout << "Usage: " << argv[0] << " export-timeseries <pattern_or_pvs> [sequence_length]" << std::endl;
                return 1;
            }

            std::string pv_spec = argv[2];
            int sequence_length = 50;
            if (argc > 3) {
                sequence_length = std::stoi(argv[3]);
            }

            std::cout << "\nEXPORTING FOR TIME SERIES ML\n";
            std::cout << "Target: LSTM/RNN training" << std::endl;
            std::cout << "Sequence length: " << sequence_length << std::endl;

            // Try as pattern first, then as comma-separated PVs
            auto timeseries_pvs = parser.findAvailablePVs(pv_spec);
            if (timeseries_pvs.empty()) {
                timeseries_pvs = parseCommaSeparatedPVs(pv_spec);
            }

            if (timeseries_pvs.empty()) {
                std::cout << "No PVs found for: " << pv_spec << std::endl;
                return 1;
            }

            std::cout << "Using " << timeseries_pvs.size() << " PVs" << std::endl;

            auto [start_time, end_time] = getTimeRange(24);

            bool success = parser.exportForTimeSeriesML(timeseries_pvs, start_time, end_time, "lstm_model");
            if (success) {
                std::cout << "\nEXPORT COMPLETE\n";
                std::cout << "Time series ML data exported!" << std::endl;
                std::cout << "Train with: cd npy_exports && python3 lstm_model_train.py" << std::endl;
            }

        }
        // === COMMAND: EXPORT-DATASET ===
        else if (command == "export-dataset") {
            if (argc < 4) {
                std::cout << "Usage: " << argv[0] << " export-dataset <pattern_or_pvs> <dataset_name>" << std::endl;
                return 1;
            }

            std::string pv_spec = argv[2];
            std::string dataset_name = argv[3];

            std::cout << "\nCREATING NAMED DATASET\n";
            std::cout << "Dataset name: " << dataset_name << std::endl;

            // Try as pattern first, then as comma-separated PVs
            auto dataset_pvs = parser.findAvailablePVs(pv_spec);
            if (dataset_pvs.empty()) {
                dataset_pvs = parseCommaSeparatedPVs(pv_spec);
            }

            if (dataset_pvs.empty()) {
                std::cout << "No PVs found for: " << pv_spec << std::endl;
                return 1;
            }

            std::cout << "PVs included: " << dataset_pvs.size() << std::endl;

            auto [start_time, end_time] = getTimeRange(24);

            bool success = parser.exportRawDataMatrix(dataset_pvs, start_time, end_time, dataset_name);
            if (success) {
                std::cout << "\nDATASET COMPLETE\n";
                std::cout << "Dataset '" << dataset_name << "' created successfully!" << std::endl;
                std::cout << "PVs included: " << dataset_pvs.size() << std::endl;
                std::cout << "Load with: cd npy_exports && python3 " << dataset_name << "_loader.py" << std::endl;
            }

        }
        else {
            std::cout << "Unknown command: " << command << std::endl;
            printUsage(argv[0]);
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        std::cerr << "Make sure MLDP is running: cd data-platform && docker-compose up" << std::endl;
        return 1;
    }

    return 0;
}
