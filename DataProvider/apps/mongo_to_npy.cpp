// mongo_to_npy.cpp - Main app for converting MongoDB/MLDP data to NPY format
#include "mongo_to_npy_parser.hpp"
#include "query_client.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

void printUsage(const char* program_name) {
    std::cout << "=== MONGODB TO NPY CONVERTER ===" << std::endl;
    std::cout << "Converts your ingested H5 data (now in MongoDB via MLDP) to NPY format for PyTorch\n" << std::endl;
    
    std::cout << "Usage: " << program_name << " <command> [options]\n" << std::endl;
    
    std::cout << "Commands:" << std::endl;
    std::cout << "  discover                          - Find available PVs in MongoDB" << std::endl;
    std::cout << "  export-all [hours_back]           - Export all PVs (default: 24 hours)" << std::endl;
    std::cout << "  export-pattern <pattern>          - Export PVs matching regex pattern" << std::endl;
    std::cout << "  export-pvs <pv1,pv2,pv3...>      - Export specific PVs (comma-separated)" << std::endl;
    std::cout << "  export-timeseries <pvs> [seq_len] - Export for LSTM/RNN (default seq: 50)" << std::endl;
    std::cout << "  export-dataset <pvs> <name>       - Export named dataset for ML" << std::endl;
    
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << program_name << " discover" << std::endl;
    std::cout << "  " << program_name << " export-all 48                    # Last 48 hours" << std::endl;
    std::cout << "  " << program_name << " export-pattern \".*BPM.*\"        # All BPM PVs" << std::endl;
    std::cout << "  " << program_name << " export-pvs S01-BPM-X,S01-BPM-Y   # Specific PVs" << std::endl;
    std::cout << "  " << program_name << " export-timeseries \".*BPM.*\" 100 # LSTM with 100-step sequences" << std::endl;
    std::cout << "  " << program_name << " export-dataset \".*GCC.*\" gas_chambers" << std::endl;
    
    std::cout << "\nOutput:" << std::endl;
    std::cout << "  Files saved to: ./npy_exports/" << std::endl;
    std::cout << "  - <name>_data.npy          (main data matrix)" << std::endl;
    std::cout << "  - <name>_timestamps.npy    (timestamps)" << std::endl;
    std::cout << "  - <name>_pv_names.txt      (PV names for reference)" << std::endl;
    std::cout << "  - <name>_metadata.json     (dataset info)" << std::endl;
    std::cout << "  - <name>_loader.py         (PyTorch loading script)" << std::endl;
    
    std::cout << "\nPyTorch Usage:" << std::endl;
    std::cout << "  cd npy_exports && python3 <name>_loader.py" << std::endl;
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    try {
        // Connect to MLDP QUERY SERVICE (not ingestion!)
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
            std::cout << "\nDISCOVERING AVAILABLE PVS IN MONGODB..." << std::endl;
            
            auto all_pvs = parser.findAvailablePVs(".*");
            if (all_pvs.empty()) {
                std::cout << "No PVs found! Make sure you've run h5_to_dp to ingest H5 data first." << std::endl;
                return 1;
            }
            
            std::cout << "\nFound " << all_pvs.size() << " total PVs" << std::endl;
            
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
                
                // Copper Linac Legacy Areas
                {".*GUN.*", "Copper Linac Gun"},
                {".*GSPEC.*", "Gun Spectrometer"},
                {".*L0.*", "L0 Linac (L0A + L0B)"},
                {".*DL1.*", "Laser Heater + Diagnostics"},
                {".*SPEC.*", "135 MeV Spectrometer"},
                {".*L1.*", "L1 Linac (L1S)"},
                {".*BC1.*|.*BC2.*", "Copper Linac Bunch Compressors"},
                {".*L2.*|.*L3.*", "L2/L3 Linac"},
                {".*CLTH.*|.*CLTS.*", "Copper Linac to Transport"},
                {".*BSYA.*", "BSY to A-line"},
                
                // Sector-based Patterns
                {".*S00.*|.*S[0-2][0-9].*|.*S30.*", "SLAC Linac Sectors (S00-S30)"},
                {".*BPN[0-2][0-9].*", "Bypass Line Sectors"},
                {".*BSY.*", "Beam Switch Yard"},
                {".*[7-8][0-9]S.*", "SXR Transport Sectors (71S-82S)"},
                {".*U0[1-4]S.*", "SXR Undulator Sectors"},
                {".*[9][0-2]S.*", "SXR End Station Sectors"},
                {".*[7-8][0-9]H.*", "HXR Transport Sectors (71H-82H)"},
                {".*U0[1-4]H.*", "HXR Undulator Sectors"},
                {".*[9][0-2]H.*", "HXR End Station Sectors"},
                {".*XTESS.*|.*XTESH.*", "X-ray Experimental Stations"},
                {".*ALINE.*", "A-line Experimental Area"},
                
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
                {".*MOTO.*|.*PICO.*", "Motors/Actuators"},
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
            
            std::cout << "\nPV PATTERNS:" << std::endl;
            for (const auto& [pattern, description] : patterns) {
                auto matching = parser.findAvailablePVs(pattern);
                std::cout << "  " << pattern << " (" << description << "): " << matching.size() << " PVs" << std::endl;
                
                // Show first few examples
                for (size_t i = 0; i < std::min(size_t(3), matching.size()); i++) {
                    std::cout << "    - " << matching[i] << std::endl;
                }
                if (matching.size() > 3) {
                    std::cout << "    ... and " << (matching.size() - 3) << " more" << std::endl;
                }
            }
            
            std::cout << "\nFIRST 10 PVS:" << std::endl;
            for (size_t i = 0; i < std::min(size_t(10), all_pvs.size()); i++) {
                std::cout << "  " << (i+1) << ". " << all_pvs[i] << std::endl;
            }
            
        }
        // === COMMAND: EXPORT-ALL ===
        else if (command == "export-all") {
            int hours_back = 24;
            if (argc > 2) {
                hours_back = std::stoi(argv[2]);
            }
            
            std::cout << "\nEXPORTING ALL PVS (last " << hours_back << " hours)..." << std::endl;
            
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
                std::cout << "All PVs exported successfully!" << std::endl;
                std::cout << "   Load in PyTorch: cd npy_exports && python3 all_pvs_raw_loader.py" << std::endl;
            }
            
        }
        // === COMMAND: EXPORT-PATTERN ===
        else if (command == "export-pattern") {
            if (argc < 3) {
                std::cout << "Usage: " << argv[0] << " export-pattern <pattern>" << std::endl;
                return 1;
            }
            
            std::string pattern = argv[2];
            std::cout << "\nEXPORTING PVS MATCHING PATTERN: " << pattern << std::endl;
            
            auto matching_pvs = parser.findAvailablePVs(pattern);
            if (matching_pvs.empty()) {
                std::cout << "No PVs found matching pattern: " << pattern << std::endl;
                return 1;
            }
            
            auto [start_time, end_time] = getTimeRange(24);
            
            // Create safe filename from pattern
            std::string safe_name = pattern;
            std::replace_if(safe_name.begin(), safe_name.end(), 
                           [](char c) { return !std::isalnum(c); }, '_');
            
            bool success = parser.exportRawDataMatrix(matching_pvs, start_time, end_time, "pattern_" + safe_name);
            if (success) {
                std::cout << "Pattern export successful!" << std::endl;
                std::cout << "   " << matching_pvs.size() << " PVs exported" << std::endl;
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
            
            std::cout << "\nEXPORTING SPECIFIC PVS:" << std::endl;
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
                std::cout << "Specific PVs exported successfully!" << std::endl;
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
            
            std::cout << "\nEXPORTING FOR TIME SERIES ML (LSTM/RNN)..." << std::endl;
            std::cout << "   Sequence length: " << sequence_length << std::endl;
            
            // Try as pattern first, then as comma-separated PVs
            auto timeseries_pvs = parser.findAvailablePVs(pv_spec);
            if (timeseries_pvs.empty()) {
                timeseries_pvs = parseCommaSeparatedPVs(pv_spec);
            }
            
            if (timeseries_pvs.empty()) {
                std::cout << "No PVs found for: " << pv_spec << std::endl;
                return 1;
            }
            
            std::cout << "   Using " << timeseries_pvs.size() << " PVs" << std::endl;
            
            auto [start_time, end_time] = getTimeRange(24);
            
            bool success = parser.exportForTimeSeriesML(timeseries_pvs, start_time, end_time, "lstm_model");
            if (success) {
                std::cout << "Time series ML data exported!" << std::endl;
                std::cout << "   Train with: cd npy_exports && python3 lstm_model_train.py" << std::endl;
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
            
            std::cout << "\nCREATING NAMED DATASET: " << dataset_name << std::endl;
            
            // Try as pattern first, then as comma-separated PVs
            auto dataset_pvs = parser.findAvailablePVs(pv_spec);
            if (dataset_pvs.empty()) {
                dataset_pvs = parseCommaSeparatedPVs(pv_spec);
            }
            
            if (dataset_pvs.empty()) {
                std::cout << "No PVs found for: " << pv_spec << std::endl;
                return 1;
            }
            
            auto [start_time, end_time] = getTimeRange(24);
            
            bool success = parser.exportRawDataMatrix(dataset_pvs, start_time, end_time, dataset_name);
            if (success) {
                std::cout << "Dataset '" << dataset_name << "' created successfully!" << std::endl;
                std::cout << "   " << dataset_pvs.size() << " PVs included" << std::endl;
                std::cout << "   Load with: cd npy_exports && python3 " << dataset_name << "_loader.py" << std::endl;
            }
            
        }
        else {
            std::cout << "Unknown command: " << command << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Make sure MLDP is running: cd data-platform && docker-compose up" << std::endl;
        return 1;
    }
    
    return 0;
}
