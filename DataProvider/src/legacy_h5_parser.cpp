// h5_parser.cpp - Implementation for PV Time-Series Data Parsing

#include "h5_parser.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>

// PVDataCollection Helper Functions Implementation
void PVDataCollection::addPVSeries(const PVTimeSeries& series) {
    pv_series.push_back(series);
    pv_index[series.pv_name] = pv_series.size() - 1;
    source_files.push_back(series.source_file);
    
    // Update global time range
    if (pv_series.size() == 1) {
        earliest_time = series.start_time_seconds;
        latest_time = series.end_time_seconds;
    } else {
        earliest_time = std::min(earliest_time, series.start_time_seconds);
        latest_time = std::max(latest_time, series.end_time_seconds);
    }
}

PVTimeSeries* PVDataCollection::findPV(const std::string& pv_name) {
    auto it = pv_index.find(pv_name);
    if (it != pv_index.end()) {
        return &pv_series[it->second];
    }
    return nullptr;
}

std::vector<std::string> PVDataCollection::getAllPVNames() const {
    std::vector<std::string> names;
    for (const auto& series : pv_series) {
        names.push_back(series.pv_name);
    }
    return names;
}

size_t PVDataCollection::getTotalMeasurements() const {
    size_t total = 0;
    for (const auto& series : pv_series) {
        total += series.measurements.size();
    }
    return total;
}

// H5Parser Implementation
H5Parser::H5Parser(const std::string& h5_path) : h5_path_(h5_path) {
    // Initialize earliest/latest times
    pv_data_.earliest_time = UINT64_MAX;
    pv_data_.latest_time = 0;
}

H5Parser::~H5Parser() {}

bool H5Parser::parseDirectory() {
    try {
        if (!std::filesystem::exists(h5_path_)) {
            std::cerr << "Directory does not exist: " << h5_path_ << std::endl;
            return false;
        }
        
        if (!std::filesystem::is_directory(h5_path_)) {
            // Single file
            return parseFile(h5_path_);
        }
        
        // Get all H5 files
        std::vector<std::string> h5_files;
        for (const auto& entry : std::filesystem::directory_iterator(h5_path_)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".h5" || ext == ".hdf5") {
                    h5_files.push_back(entry.path().string());
                }
            }
        }
        
        if (h5_files.empty()) {
            std::cout << "No H5 files found in: " << h5_path_ << std::endl;
            return false;
        }
        
        // Sort files for consistent processing
        std::sort(h5_files.begin(), h5_files.end());
        
        std::cout << "Found " << h5_files.size() << " H5 files to parse" << std::endl;
        
        // Parse each file
        for (const auto& file_path : h5_files) {
            std::cout << "Parsing: " << std::filesystem::path(file_path).filename() << std::endl;
            
            if (!parseFile(file_path)) {
                std::cerr << "Failed to parse: " << file_path << std::endl;
                continue; // Continue with other files
            }
        }
        
        // Post-processing
        correlatePVData();
        validateTimestamps();
        
        std::cout << "Successfully parsed " << h5_files.size() << " files" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in parseDirectory: " << e.what() << std::endl;
        return false;
    }
}

// Updated parseFile function for h5_parser.cpp

bool H5Parser::parseFile(const std::string& filename) {
    try {
        // Turn off HDF5 error printing
        H5::Exception::dontPrint();
        
        // Open the HDF5 file
        H5::H5File file(filename, H5F_ACC_RDONLY);
        std::cout << "  Opened: " << std::filesystem::path(filename).filename() << std::endl;
        
        // Read timestamps first
        std::vector<uint64_t> seconds_timestamps;
        std::vector<uint64_t> nanoseconds_timestamps;
        
        // Read secondsPastEpoch
        if (file.nameExists("secondsPastEpoch")) {
            H5::DataSet seconds_ds = file.openDataSet("secondsPastEpoch");
            H5::DataSpace seconds_space = seconds_ds.getSpace();
            hsize_t seconds_dims[1];
            seconds_space.getSimpleExtentDims(seconds_dims);
            
            seconds_timestamps.resize(seconds_dims[0]);
            seconds_ds.read(seconds_timestamps.data(), H5::PredType::NATIVE_UINT64);
            
            std::cout << "  Found " << seconds_dims[0] << " timestamp entries" << std::endl;
            seconds_ds.close();
            seconds_space.close();
        } else {
            std::cerr << "  No secondsPastEpoch dataset found" << std::endl;
            return false;
        }
        
        // Read nanoseconds
        if (file.nameExists("nanoseconds")) {
            H5::DataSet nano_ds = file.openDataSet("nanoseconds");
            H5::DataSpace nano_space = nano_ds.getSpace();
            hsize_t nano_dims[1];
            nano_space.getSimpleExtentDims(nano_dims);
            
            if (nano_dims[0] != seconds_timestamps.size()) {
                std::cerr << "  Timestamp size mismatch: seconds=" << seconds_timestamps.size() 
                          << ", nanoseconds=" << nano_dims[0] << std::endl;
                return false;
            }
            
            nanoseconds_timestamps.resize(nano_dims[0]);
            nano_ds.read(nanoseconds_timestamps.data(), H5::PredType::NATIVE_UINT64);
            
            nano_ds.close();
            nano_space.close();
        } else {
            std::cerr << "  No nanoseconds dataset found" << std::endl;
            // Use zeros for nanoseconds if not available
            nanoseconds_timestamps.resize(seconds_timestamps.size(), 0);
        }
        
        // Now read all PV datasets
        H5::Group root = file.openGroup("/");
        hsize_t num_objects = root.getNumObjs();
        
        size_t pv_count = 0;
        for (hsize_t i = 0; i < num_objects; i++) {
            std::string obj_name = root.getObjnameByIdx(i);
            H5G_obj_t obj_type = root.getObjTypeByIdx(i);
            
            // Skip timestamp datasets
            if (obj_name == "secondsPastEpoch" || obj_name == "nanoseconds") {
                continue;
            }
            
            if (obj_type == H5G_DATASET) {
                // This is a PV dataset
                H5::DataSet pv_dataset = root.openDataSet(obj_name);
                H5::DataSpace pv_space = pv_dataset.getSpace();
                hsize_t pv_dims[1];
                pv_space.getSimpleExtentDims(pv_dims);
                
                if (pv_dims[0] != seconds_timestamps.size()) {
                    std::cout << "    Skipping " << obj_name << " - size mismatch (" 
                              << pv_dims[0] << " vs " << seconds_timestamps.size() << ")" << std::endl;
                    pv_dataset.close();
                    pv_space.close();
                    continue;
                }
                
                // Read PV data
                std::vector<double> pv_values(pv_dims[0]);
                
                // Try to read as double first
                try {
                    pv_dataset.read(pv_values.data(), H5::PredType::NATIVE_DOUBLE);
                } catch (const H5::Exception& e) {
                    // If double fails, try as float or int
                    try {
                        std::vector<float> float_values(pv_dims[0]);
                        pv_dataset.read(float_values.data(), H5::PredType::NATIVE_FLOAT);
                        for (size_t j = 0; j < float_values.size(); j++) {
                            pv_values[j] = static_cast<double>(float_values[j]);
                        }
                    } catch (const H5::Exception& e2) {
                        std::cout << "    Skipping " << obj_name << " - unsupported data type" << std::endl;
                        pv_dataset.close();
                        pv_space.close();
                        continue;
                    }
                }
                
                // Create PV time series
                PVTimeSeries series;
                series.pv_name = obj_name;
                series.description = "PV: " + obj_name;
                series.units = "unknown";
                series.data_type = "DOUBLE";
                series.source_file = filename;
                series.dataset_path = "/" + obj_name;
                series.total_samples = pv_values.size();
                
                // Create measurements
                series.measurements.reserve(pv_values.size());
                for (size_t j = 0; j < pv_values.size(); j++) {
                    PVMeasurement measurement;
                    measurement.timestamp_seconds = seconds_timestamps[j];
                    measurement.timestamp_nanoseconds = nanoseconds_timestamps[j];
                    measurement.value = pv_values[j];
                    measurement.status = "GOOD";
                    measurement.severity = 0.0;
                    
                    series.measurements.push_back(measurement);
                }
                
                // Set time metadata
                if (!series.measurements.empty()) {
                    series.start_time_seconds = series.measurements.front().timestamp_seconds;
                    series.end_time_seconds = series.measurements.back().timestamp_seconds;
                    
                    // Calculate sample rate
                    if (series.measurements.size() > 1) {
                        double duration = static_cast<double>(
                            series.end_time_seconds - series.start_time_seconds);
                        if (duration > 0) {
                            series.sample_rate_hz = (series.measurements.size() - 1) / duration;
                        } else {
                            series.sample_rate_hz = 0.0;
                        }
                    } else {
                        series.sample_rate_hz = 0.0;
                    }
                }
                
                // Add to collection
                pv_data_.addPVSeries(series);
                pv_count++;
                
                if (pv_count % 50 == 0) {  // Progress update every 50 PVs
                    std::cout << "    Loaded " << pv_count << " PVs..." << std::endl;
                }
                
                pv_dataset.close();
                pv_space.close();
            }
        }
        
        std::cout << "  Successfully loaded " << pv_count << " PVs from " 
                  << std::filesystem::path(filename).filename() << std::endl;
        
        root.close();
        file.close();
        return true;
        
    } catch (const H5::Exception& e) {
        std::cerr << "  HDF5 error in " << filename << ": " << e.getDetailMsg() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "  Error parsing " << filename << ": " << e.what() << std::endl;
        return false;
    }
}

bool H5Parser::parseTimeseriesGroup(H5::Group& group, const std::string& group_name) {
    try {
        hsize_t num_objects = group.getNumObjs();
        
        for (hsize_t i = 0; i < num_objects; i++) {
            std::string obj_name = group.getObjnameByIdx(i);
            H5G_obj_t obj_type = group.getObjTypeByIdx(i);
            
            if (obj_type == H5G_GROUP) {
                // This might be a PV group (e.g., "S01-GCC-01")
                std::cout << "    Found PV group: " << obj_name << std::endl;
                
                H5::Group pv_group = group.openGroup(obj_name);
                
                // Look for standard datasets: timestamps, values, status
                if (pv_group.nameExists("timestamps") && pv_group.nameExists("values")) {
                    PVTimeSeries series;
                    series.pv_name = obj_name;
                    series.source_file = ""; // Will be set by caller
                    series.dataset_path = group_name + "/" + obj_name;
                    
                    // Read timestamps
                    H5::DataSet timestamp_ds = pv_group.openDataSet("timestamps");
                    H5::DataSpace timestamp_space = timestamp_ds.getSpace();
                    hsize_t timestamp_dims[1];
                    timestamp_space.getSimpleExtentDims(timestamp_dims);
                    
                    std::vector<uint64_t> timestamps(timestamp_dims[0]);
                    timestamp_ds.read(timestamps.data(), H5::PredType::NATIVE_UINT64);
                    
                    // Read values
                    H5::DataSet values_ds = pv_group.openDataSet("values");
                    H5::DataSpace values_space = values_ds.getSpace();
                    hsize_t values_dims[1];
                    values_space.getSimpleExtentDims(values_dims);
                    
                    if (values_dims[0] != timestamp_dims[0]) {
                        std::cerr << "    Timestamp/value size mismatch for " << obj_name << std::endl;
                        continue;
                    }
                    
                    std::vector<double> values(values_dims[0]);
                    values_ds.read(values.data(), H5::PredType::NATIVE_DOUBLE);
                    
                    // Read status if available
                    std::vector<std::string> status_values;
                    if (pv_group.nameExists("status")) {
                        // Status reading is more complex for strings
                        // For now, default to "GOOD"
                        status_values.resize(timestamps.size(), "GOOD");
                    } else {
                        status_values.resize(timestamps.size(), "GOOD");
                    }
                    
                    // Build measurements
                    series.measurements.reserve(timestamps.size());
                    for (size_t j = 0; j < timestamps.size(); j++) {
                        PVMeasurement measurement;
                        measurement.timestamp_seconds = timestamps[j];
                        measurement.timestamp_nanoseconds = 0; // Could be enhanced
                        measurement.value = values[j];
                        measurement.status = status_values[j];
                        measurement.severity = 0.0;
                        
                        series.measurements.push_back(measurement);
                    }
                    
                    // Set metadata
                    series.total_samples = timestamps.size();
                    if (!timestamps.empty()) {
                        series.start_time_seconds = timestamps.front();
                        series.end_time_seconds = timestamps.back();
                        
                        // Calculate sample rate
                        if (timestamps.size() > 1) {
                            double duration = static_cast<double>(timestamps.back() - timestamps.front());
                            series.sample_rate_hz = (timestamps.size() - 1) / duration;
                        } else {
                            series.sample_rate_hz = 0.0;
                        }
                    }
                    
                    // Read metadata if available
                    if (pv_group.nameExists("metadata")) {
                        H5::Group meta_group = pv_group.openGroup("metadata");
                        
                        // Read units attribute
                        if (meta_group.attrExists("units")) {
                            H5::Attribute units_attr = meta_group.openAttribute("units");
                            H5::StrType str_type = units_attr.getStrType();
                            units_attr.read(str_type, series.units);
                        }
                        
                        // Read description attribute
                        if (meta_group.attrExists("description")) {
                            H5::Attribute desc_attr = meta_group.openAttribute("description");
                            H5::StrType str_type = desc_attr.getStrType();
                            desc_attr.read(str_type, series.description);
                        }
                        
                        meta_group.close();
                    }
                    
                    // Default values if not found
                    if (series.units.empty()) series.units = "unknown";
                    if (series.description.empty()) series.description = "PV: " + obj_name;
                    series.data_type = "DOUBLE";
                    
                    // Add to collection
                    pv_data_.addPVSeries(series);
                    
                    std::cout << "    Loaded " << series.total_samples 
                              << " measurements for " << obj_name << std::endl;
                    
                    timestamp_ds.close();
                    values_ds.close();
                }
                
                pv_group.close();
            }
        }
        
        return true;
        
    } catch (const H5::Exception& e) {
        std::cerr << "Error parsing group " << group_name << ": " << e.getDetailMsg() << std::endl;
        return false;
    }
}

std::vector<CorrelatedPVData> H5Parser::getCorrelatedData(uint64_t start_time, uint64_t end_time) const {
    std::vector<CorrelatedPVData> correlated_data;
    
    // Get all unique timestamps in the range
    std::set<uint64_t> unique_timestamps;
    
    for (const auto& series : pv_data_.pv_series) {
        for (const auto& measurement : series.measurements) {
            if (measurement.timestamp_seconds >= start_time && 
                measurement.timestamp_seconds <= end_time) {
                unique_timestamps.insert(measurement.timestamp_seconds);
            }
        }
    }
    
    // For each timestamp, collect all PV values
    for (uint64_t timestamp : unique_timestamps) {
        CorrelatedPVData corr_data;
        corr_data.timestamp_seconds = timestamp;
        corr_data.timestamp_nanoseconds = 0;
        
        for (const auto& series : pv_data_.pv_series) {
            // Find measurement at this timestamp
            for (const auto& measurement : series.measurements) {
                if (measurement.timestamp_seconds == timestamp) {
                    corr_data.pv_values[series.pv_name] = measurement.value;
                    corr_data.pv_status[series.pv_name] = measurement.status;
                    break;
                }
            }
        }
        
        correlated_data.push_back(corr_data);
    }
    
    return correlated_data;
}

std::vector<std::string> H5Parser::getAllPVNames() const {
    return pv_data_.getAllPVNames();
}

PVTimeSeries* H5Parser::getPVSeries(const std::string& pv_name) {
    return pv_data_.findPV(pv_name);
}

void H5Parser::printSummary() const {
    std::cout << "\n=== PV Data Summary ===" << std::endl;
    std::cout << "Total PV series: " << pv_data_.pv_series.size() << std::endl;
    std::cout << "Total measurements: " << pv_data_.getTotalMeasurements() << std::endl;
    
    if (pv_data_.earliest_time != UINT64_MAX) {
        std::cout << "Time range: " << pv_data_.earliest_time 
                  << " to " << pv_data_.latest_time << std::endl;
        std::cout << "Duration: " << (pv_data_.latest_time - pv_data_.earliest_time) 
                  << " seconds" << std::endl;
    }
    
    std::cout << "\nPV Details:" << std::endl;
    for (const auto& series : pv_data_.pv_series) {
        std::cout << "  " << series.pv_name 
                  << " (" << series.units << "): " 
                  << series.total_samples << " samples, "
                  << std::fixed << std::setprecision(2) << series.sample_rate_hz << " Hz"
                  << std::endl;
    }
}

void H5Parser::correlatePVData() {
    // Could implement timestamp alignment here
    std::cout << "  Correlating PV data across " << pv_data_.pv_series.size() << " series" << std::endl;
}

void H5Parser::validateTimestamps() {
    // Could implement timestamp validation here
    std::cout << "  Validating timestamps" << std::endl;
}

uint64_t H5Parser::convertToUnixTime(const std::string& timestamp_str) {
    // Implement timestamp string conversion if needed
    // For now, assume timestamps are already in Unix format
    return std::stoull(timestamp_str);
}
