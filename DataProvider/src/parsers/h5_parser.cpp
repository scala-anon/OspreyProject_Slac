#include "h5_parser.hpp"
#include <filesystem>
#include <regex>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <stdexcept>
#include <set>
#include <cmath>

H5Parser::H5Parser(const std::string& h5_directory_path) 
    : h5_directory_(h5_directory_path) {
    // Constructor - validate directory exists
    if (!std::filesystem::exists(h5_directory_)) {
        throw std::runtime_error("H5 directory does not exist: " + h5_directory_);
    }
}

H5Parser::~H5Parser() {
    // Destructor - cleanup handled by smart pointers
}

bool H5Parser::parseDirectory() {
    try {
        std::cout << "=== H5 Directory Parsing ===" << std::endl;
        std::cout << "Scanning directory: " << h5_directory_ << std::endl;
        
        // Discover all H5 files
        auto h5_files = discoverH5Files();
        
        if (h5_files.empty()) {
            std::cerr << "No H5 files found in directory" << std::endl;
            return false;
        }
        
        std::cout << "Found " << h5_files.size() << " H5 files" << std::endl;
        
        // Filter and parse valid files
        size_t valid_files = 0;
        size_t total_signals = 0;
        
        for (const auto& filepath : h5_files) {
            if (matchesNamingConvention(filepath)) {
                std::cout << "\nParsing: " << std::filesystem::path(filepath).filename() << std::endl;
                
                size_t signals_before = parsed_signals_.size();
                
                if (parseFile(filepath)) {
                    valid_files++;
                    size_t signals_added = parsed_signals_.size() - signals_before;
                    total_signals += signals_added;
                    std::cout << "  Success: " << signals_added << " signals parsed" << std::endl;
                } else {
                    std::cerr << "  Failed to parse file" << std::endl;
                }
            } else {
                std::cout << "Skipping file with invalid naming: " 
                          << std::filesystem::path(filepath).filename() << std::endl;
            }
        }
        
        std::cout << "\n=== Parsing Summary ===" << std::endl;
        std::cout << "Valid files processed: " << valid_files << "/" << h5_files.size() << std::endl;
        std::cout << "Total signals parsed: " << total_signals << std::endl;
        
        return valid_files > 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in parseDirectory: " << e.what() << std::endl;
        return false;
    }
}

bool H5Parser::parseFile(const std::string& filepath) {
    try {
        // Parse filename metadata
        H5FileMetadata file_metadata = parseFilename(filepath);
        
        // Turn off HDF5 error printing to console
        H5::Exception::dontPrint();
        
        // Open H5 file
        H5::H5File file(filepath, H5F_ACC_RDONLY);
        
        // Extract or reuse timestamps
        std::shared_ptr<TimestampData> timestamps;
        auto timestamp_it = file_timestamps_.find(filepath);
        if (timestamp_it != file_timestamps_.end()) {
            timestamps = timestamp_it->second;
        } else {
            timestamps = extractTimestamps(file);
            file_timestamps_[filepath] = timestamps;
        }
        
        if (!timestamps || timestamps->count == 0) {
            std::cerr << "  No valid timestamps found" << std::endl;
            return false;
        }
        
        // Get all signal datasets
        auto signal_names = getSignalDatasets(file);
        
        std::cout << "  Found " << signal_names.size() << " signals with " 
                  << timestamps->count << " timestamps";
        if (timestamps->is_regular_sampling) {
            std::cout << " (regular " << (1000000000.0 / timestamps->period_nanos) << " Hz)";
        } else {
            std::cout << " (irregular sampling)";
        }
        std::cout << std::endl;
        
        // Process each signal
        size_t successful_signals = 0;
        for (const auto& signal_name : signal_names) {
            try {
                SignalData signal_data = processSignal(file, signal_name, timestamps, file_metadata);
                
                // Validate data consistency
                if (validateDataConsistency(signal_data.values, *timestamps, signal_name)) {
                    parsed_signals_.push_back(std::move(signal_data));
                    successful_signals++;
                } else {
                    std::cerr << "    Data validation failed for: " << signal_name << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "    Failed to process signal " << signal_name 
                          << ": " << e.what() << std::endl;
                continue;
            }
        }
        
        file.close();
        
        if (successful_signals == 0) {
            std::cerr << "  No signals successfully processed" << std::endl;
            return false;
        }
        
        return true;
        
    } catch (const H5::Exception& e) {
        std::cerr << "  HDF5 error: " << e.getDetailMsg() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "  Error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> H5Parser::discoverH5Files() const {
    std::vector<std::string> h5_files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(h5_directory_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".h5") {
                h5_files.push_back(entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    
    // Sort for consistent processing order
    std::sort(h5_files.begin(), h5_files.end());
    return h5_files;
}

// In h5_parser.cpp, replace the matchesNamingConvention function:

bool H5Parser::matchesNamingConvention(const std::string& filepath) const {
    std::filesystem::path path(filepath);
    std::string filename = path.stem().string();

    // Updated pattern: origin_pathway_date_time[_project] (project is optional)
    std::regex pattern(R"(([A-Z]+)_([A-Z]+)_(\d{8})_(\d{6})(?:_([A-Za-z0-9]+))?)");
    return std::regex_match(filename, pattern);
}

// Also update the parseFilename function to handle optional project:

H5FileMetadata H5Parser::parseFilename(const std::string& filepath) const {
    H5FileMetadata metadata;
    metadata.full_path = filepath;

    std::filesystem::path path(filepath);
    std::string filename = path.stem().string();

    // Updated pattern with optional project
    std::regex pattern(R"(([A-Z]+)_([A-Z]+)_(\d{8})_(\d{6})(?:_([A-Za-z0-9]+))?)");
    std::smatch matches;

    if (std::regex_match(filename, matches, pattern)) {
        metadata.origin = matches[1].str();
        metadata.pathway = matches[2].str();
        metadata.date = matches[3].str();
        metadata.time = matches[4].str();
        
        // Project is optional (group 5)
        if (matches.size() > 5 && matches[5].matched) {
            metadata.project = matches[5].str();
        } else {
            metadata.project = "default";  // Or extract from directory name
        }

        // Convert to timestamp
        metadata.file_timestamp_seconds = parseTimestamp(metadata.date, metadata.time);
        metadata.valid_timestamp = (metadata.file_timestamp_seconds > 0);

        std::cout << "  Parsed metadata: " << metadata.origin << "_" << metadata.pathway
                  << " " << metadata.date << "_" << metadata.time
                  << " project=" << metadata.project << std::endl;
    } else {
        std::cerr << "  Warning: Filename doesn't match expected pattern: " << filename << std::endl;
        metadata.origin = "unknown";
        metadata.pathway = "unknown";
        metadata.project = "unknown";
        metadata.date = "unknown";
        metadata.time = "unknown";
        metadata.file_timestamp_seconds = 0;
        metadata.valid_timestamp = false;
    }

    return metadata;
}

uint64_t H5Parser::parseTimestamp(const std::string& date, const std::string& time) const {
    try {
        if (date.length() != 8 || time.length() != 6) {
            return 0;
        }
        
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(4, 2));
        int day = std::stoi(date.substr(6, 2));
        int hour = std::stoi(time.substr(0, 2));
        int minute = std::stoi(time.substr(2, 2));
        int second = std::stoi(time.substr(4, 2));
        
        // Create tm structure
        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1; // Let system determine DST
        
        std::time_t time_t_val = std::mktime(&tm);
        return static_cast<uint64_t>(time_t_val);
        
    } catch (const std::exception& e) {
        std::cerr << "    Warning: Failed to parse timestamp " << date << "_" << time 
                  << ": " << e.what() << std::endl;
        return 0;
    }
}

std::shared_ptr<TimestampData> H5Parser::extractTimestamps(H5::H5File& file) const {
    auto timestamps = std::make_shared<TimestampData>();
    
    try {
        // Read secondsPastEpoch
        if (!file.nameExists("secondsPastEpoch")) {
            throw std::runtime_error("No secondsPastEpoch dataset found");
        }
        
        H5::DataSet seconds_ds = file.openDataSet("secondsPastEpoch");
        H5::DataSpace seconds_space = seconds_ds.getSpace();
        hsize_t dims[1];
        seconds_space.getSimpleExtentDims(dims);
        
        timestamps->seconds.resize(dims[0]);
        seconds_ds.read(timestamps->seconds.data(), H5::PredType::NATIVE_UINT64);
        
        // Read nanoseconds
        if (file.nameExists("nanoseconds")) {
            H5::DataSet nanos_ds = file.openDataSet("nanoseconds");
            H5::DataSpace nanos_space = nanos_ds.getSpace();
            hsize_t nano_dims[1];
            nanos_space.getSimpleExtentDims(nano_dims);
            
            if (nano_dims[0] != dims[0]) {
                throw std::runtime_error("Timestamp array size mismatch");
            }
            
            timestamps->nanoseconds.resize(dims[0]);
            nanos_ds.read(timestamps->nanoseconds.data(), H5::PredType::NATIVE_UINT64);
            
            nanos_ds.close();
            nanos_space.close();
        } else {
            // Fill with zeros if nanoseconds not available
            timestamps->nanoseconds.resize(dims[0], 0);
        }
        
        // Set basic properties
        timestamps->count = dims[0];
        if (timestamps->count > 0) {
            timestamps->start_time_sec = timestamps->seconds[0];
            timestamps->start_time_nano = timestamps->nanoseconds[0];
            timestamps->end_time_sec = timestamps->seconds[timestamps->count - 1];
            timestamps->end_time_nano = timestamps->nanoseconds[timestamps->count - 1];
        }
        
        // Calculate period and check regularity
        timestamps->period_nanos = calculatePeriodNanos(timestamps->seconds, timestamps->nanoseconds);
        timestamps->is_regular_sampling = checkRegularSampling(timestamps->seconds, 
                                                              timestamps->nanoseconds, 
                                                              timestamps->period_nanos);
        
        seconds_ds.close();
        seconds_space.close();
        
        std::cout << "  Extracted " << timestamps->count << " timestamps, period=" 
                  << timestamps->period_nanos << "ns, regular=" 
                  << (timestamps->is_regular_sampling ? "yes" : "no") << std::endl;
        
    } catch (const H5::Exception& e) {
        std::cerr << "  HDF5 error reading timestamps: " << e.getDetailMsg() << std::endl;
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "  Error reading timestamps: " << e.what() << std::endl;
        return nullptr;
    }
    
    return timestamps;
}

std::vector<std::string> H5Parser::getSignalDatasets(H5::H5File& file) const {
    std::vector<std::string> signal_names;
    
    try {
        H5::Group root = file.openGroup("/");
        hsize_t num_objects = root.getNumObjs();
        
        for (hsize_t i = 0; i < num_objects; i++) {
            std::string obj_name = root.getObjnameByIdx(i);
            H5G_obj_t obj_type = root.getObjTypeByIdx(i);
            
            // Skip timestamp datasets
            if (obj_name == "secondsPastEpoch" || obj_name == "nanoseconds") {
                continue;
            }
            
            if (obj_type == H5G_DATASET) {
                signal_names.push_back(obj_name);
            }
        }
        
        root.close();
        
    } catch (const H5::Exception& e) {
        std::cerr << "  Error reading dataset names: " << e.getDetailMsg() << std::endl;
    }
    
    return signal_names;
}

SignalInfo H5Parser::parseSignalName(const std::string& signal_name) const {
    SignalInfo info;
    info.full_name = signal_name;
    
    // Updated pattern: DEVICE_AREA_LOCATION_ATTRIBUTE
    // Examples: KLYS_LI23_31_AMPL, BPMS_DMPH_502_TMITBR
    std::regex pattern(R"(([A-Z]+)_([A-Z0-9]+)_(\d+)_([A-Z0-9_]+))");
    std::smatch matches;
    
    if (std::regex_match(signal_name, matches, pattern)) {
        info.device = matches[1].str();              // "KLYS", "BPMS"
        info.device_area = matches[2].str();         // "LI23", "DMPH"
        info.device_location = matches[3].str();     // "31", "502"
        info.device_attribute = matches[4].str();    // "AMPL", "TMITBR", "PHAS_FASTBR"
        
        // Infer units and signal type
        info.units = inferUnits(info.device_attribute);
        info.signal_type = inferSignalType(info.device_attribute);
        
    } else {
        std::cerr << "    Warning: Signal name doesn't match pattern: " << signal_name << std::endl;
        info.device = "unknown";
        info.device_area = "unknown";
        info.device_location = "unknown";
        info.device_attribute = signal_name;
        info.units = "unknown";
        info.signal_type = "unknown";
    }
    
    return info;
}

SignalData H5Parser::processSignal(H5::H5File& file, 
                                   const std::string& signal_name,
                                   std::shared_ptr<TimestampData> timestamps,
                                   const H5FileMetadata& file_metadata) const {
    SignalData signal_data;
    
    // Parse signal name
    signal_data.info = parseSignalName(signal_name);
    signal_data.timestamps = timestamps;
    signal_data.file_metadata = file_metadata;
    
    try {
        // Open signal dataset
        H5::DataSet dataset = file.openDataSet(signal_name);
        H5::DataSpace dataspace = dataset.getSpace();
        
        // Get dimensions and handle any reasonable dataset shape
        const int MAX_DIMS = 3;  // Support up to 3D datasets
        hsize_t dims[MAX_DIMS];
        int ndims = dataspace.getSimpleExtentDims(dims);
        
        if (ndims == 0 || ndims > MAX_DIMS) {
            throw std::runtime_error("Signal dataset must be 1D-3D, got " + std::to_string(ndims) + "D");
        }
        
        // Print dataset shape for debugging
        std::cout << "    Dataset shape: [";
        for (int i = 0; i < ndims; i++) {
            std::cout << dims[i];
            if (i < ndims - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        
        // Find which dimension matches our timestamp count
        int time_dim_index = -1;
        size_t time_dimension = 0;
        size_t total_elements = 1;
        
        // Calculate total elements
        for (int i = 0; i < ndims; i++) {
            total_elements *= dims[i];
        }
        
        // Find the dimension that matches timestamp count
        for (int i = 0; i < ndims; i++) {
            if (dims[i] == timestamps->count) {
                time_dim_index = i;
                time_dimension = dims[i];
                break;
            }
        }
        
        // If no exact match, check if any dimension is close (within 1%)
        if (time_dim_index == -1) {
            for (int i = 0; i < ndims; i++) {
                double ratio = (double)dims[i] / timestamps->count;
                if (ratio > 0.99 && ratio < 1.01) {
                    time_dim_index = i;
                    time_dimension = dims[i];
                    std::cout << "    Using dimension " << i << " (size " << dims[i] 
                              << ") as time axis (close to " << timestamps->count << ")" << std::endl;
                    break;
                }
            }
        }
        
        // Special handling for common cases
        if (time_dim_index == -1) {
            if (ndims == 1) {
                // 1D but wrong size - might be different sampling
                time_dim_index = 0;
                time_dimension = dims[0];
                std::cout << "    Warning: 1D dataset size (" << dims[0] 
                          << ") doesn't match timestamps (" << timestamps->count 
                          << ") - using anyway" << std::endl;
            } else if (ndims == 2 && (dims[0] == 1 || dims[1] == 1)) {
                // Shape like [693766, 1] or [1, 693766] - effectively 1D
                time_dim_index = (dims[0] > dims[1]) ? 0 : 1;
                time_dimension = dims[time_dim_index];
                std::cout << "     2D dataset with singleton dimension - using dimension " 
                          << time_dim_index << " (size " << time_dimension << ")" << std::endl;
            } else {
                // Use the largest dimension as time
                time_dim_index = 0;
                for (int i = 1; i < ndims; i++) {
                    if (dims[i] > dims[time_dim_index]) {
                        time_dim_index = i;
                    }
                }
                time_dimension = dims[time_dim_index];
                std::cout << "    Using largest dimension " << time_dim_index 
                          << " (size " << time_dimension << ") as time axis" << std::endl;
            }
        }
        
        std::cout << "    Extracting " << time_dimension << " samples from dimension " 
                  << time_dim_index << std::endl;
        
        // Dynamic data extraction based on dataset dimensionality
        signal_data.values.resize(time_dimension);
        
        if (ndims == 1) {
            // Simple 1D read
            try {
                dataset.read(signal_data.values.data(), H5::PredType::NATIVE_DOUBLE);
            } catch (const H5::Exception& e) {
                std::vector<float> float_values(dims[0]);
                dataset.read(float_values.data(), H5::PredType::NATIVE_FLOAT);
                for (size_t i = 0; i < float_values.size(); ++i) {
                    signal_data.values[i] = static_cast<double>(float_values[i]);
                }
            }
        } else if (ndims == 2) {
            // 2D dataset - extract based on which dimension is time
            if (time_dim_index == 0) {
                // Time is first dimension [time, channels] - extract first column
                hsize_t offset[2] = {0, 0};
                hsize_t count[2] = {dims[0], 1};
                hsize_t mem_dims[1] = {dims[0]};
                
                H5::DataSpace memspace(1, mem_dims);
                dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
                
                try {
                    dataset.read(signal_data.values.data(), H5::PredType::NATIVE_DOUBLE, memspace, dataspace);
                } catch (const H5::Exception& e) {
                    std::vector<float> float_values(dims[0]);
                    dataset.read(float_values.data(), H5::PredType::NATIVE_FLOAT, memspace, dataspace);
                    for (size_t i = 0; i < float_values.size(); ++i) {
                        signal_data.values[i] = static_cast<double>(float_values[i]);
                    }
                }
            } else {
                // Time is second dimension [channels, time] - extract first row
                hsize_t offset[2] = {0, 0};
                hsize_t count[2] = {1, dims[1]};
                hsize_t mem_dims[1] = {dims[1]};
                
                H5::DataSpace memspace(1, mem_dims);
                dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
                
                try {
                    dataset.read(signal_data.values.data(), H5::PredType::NATIVE_DOUBLE, memspace, dataspace);
                } catch (const H5::Exception& e) {
                    std::vector<float> float_values(dims[1]);
                    dataset.read(float_values.data(), H5::PredType::NATIVE_FLOAT, memspace, dataspace);
                    for (size_t i = 0; i < float_values.size(); ++i) {
                        signal_data.values[i] = static_cast<double>(float_values[i]);
                    }
                }
            }
        } else if (ndims == 3) {
            // 3D dataset - extract slice from time dimension (assume other dims = 1 or take first slice)
            hsize_t offset[3] = {0, 0, 0};
            hsize_t count[3] = {1, 1, 1};
            count[time_dim_index] = dims[time_dim_index];
            
            hsize_t mem_dims[1] = {dims[time_dim_index]};
            H5::DataSpace memspace(1, mem_dims);
            dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
            
            try {
                dataset.read(signal_data.values.data(), H5::PredType::NATIVE_DOUBLE, memspace, dataspace);
            } catch (const H5::Exception& e) {
                std::vector<float> float_values(dims[time_dim_index]);
                dataset.read(float_values.data(), H5::PredType::NATIVE_FLOAT, memspace, dataspace);
                for (size_t i = 0; i < float_values.size(); ++i) {
                    signal_data.values[i] = static_cast<double>(float_values[i]);
                }
            }
        }
        
        // Read H5 attributes
        signal_data.info.label = readStringAttribute(dataset, "label");
        signal_data.info.matlab_class = readStringAttribute(dataset, "MATLAB_class");
        
        dataset.close();
        dataspace.close();
        
    } catch (const H5::Exception& e) {
        throw std::runtime_error("HDF5 error processing signal: " + std::string(e.getDetailMsg()));
    }
    
    return signal_data;
}

std::string H5Parser::readStringAttribute(H5::DataSet& dataset, const std::string& attr_name) const {
    try {
        if (!dataset.attrExists(attr_name)) {
            return "";
        }
        
        H5::Attribute attribute = dataset.openAttribute(attr_name);
        H5::DataType datatype = attribute.getDataType();
        
        if (datatype.getClass() == H5T_STRING) {
            H5::StrType str_type = attribute.getStrType();
            std::string value;
            attribute.read(str_type, value);
            attribute.close();
            return value;
        }
        
        attribute.close();
        
    } catch (const H5::Exception& e) {
        // Attribute reading is optional
    }
    
    return "";
}

uint64_t H5Parser::calculatePeriodNanos(const std::vector<uint64_t>& seconds, 
                                        const std::vector<uint64_t>& nanoseconds) const {
    if (seconds.size() < 2) {
        return 1000000000; // Default 1 second
    }
    
    uint64_t first_total_ns = seconds[0] * 1000000000ULL + nanoseconds[0];
    uint64_t second_total_ns = seconds[1] * 1000000000ULL + nanoseconds[1];
    
    return second_total_ns - first_total_ns;
}

bool H5Parser::checkRegularSampling(const std::vector<uint64_t>& seconds, 
                                    const std::vector<uint64_t>& nanoseconds,
                                    uint64_t expected_period) const {
    if (seconds.size() < 3) {
        return true; // Assume regular if too few samples
    }
    
    const uint64_t tolerance = 1000; // 1 microsecond tolerance
    size_t check_count = std::min(size_t(10), seconds.size() - 1);
    
    for (size_t i = 1; i <= check_count; ++i) {
        uint64_t current_ns = seconds[i] * 1000000000ULL + nanoseconds[i];
        uint64_t prev_ns = seconds[i-1] * 1000000000ULL + nanoseconds[i-1];
        uint64_t actual_period = current_ns - prev_ns;
        
        if (std::abs(static_cast<int64_t>(actual_period - expected_period)) > static_cast<int64_t>(tolerance)) {
            return false;
        }
    }
    
    return true;
}

std::string H5Parser::inferUnits(const std::string& device_attribute) const {
    // Position measurements
    if (device_attribute == "X" || device_attribute == "Y" || device_attribute == "Z") return "mm";
    
    // Charge measurements
    if (device_attribute == "TMIT" || device_attribute == "TMITBR") return "pC";
    
    // Magnetic field controls
    if (device_attribute == "BCTRL" || device_attribute == "BDES" || device_attribute == "BACT") return "kG";
    
    // RF measurements
    if (device_attribute.find("PHAS") != std::string::npos) return "deg";
    if (device_attribute.find("AMPL") != std::string::npos) return "MV/m";
    if (device_attribute.find("POW") != std::string::npos) return "MW";
    
    // Generic measurements
    if (device_attribute.find("TEMP") != std::string::npos) return "°C";
    if (device_attribute.find("PRESS") != std::string::npos) return "Torr";
    if (device_attribute.find("CURR") != std::string::npos) return "A";
    if (device_attribute.find("VOLT") != std::string::npos) return "V";
    
    return "unknown";
}

std::string H5Parser::inferSignalType(const std::string& device_attribute) const {
    // Position measurements
    if (device_attribute == "X" || device_attribute == "Y" || device_attribute == "Z") return "position";
    
    // Charge measurements
    if (device_attribute == "TMIT" || device_attribute == "TMITBR") return "charge";
    
    // Control signals
    if (device_attribute == "BCTRL" || device_attribute == "BDES") return "control";
    if (device_attribute == "BACT") return "actual";
    
    // RF measurements
    if (device_attribute.find("PHAS") != std::string::npos) return "phase";
    if (device_attribute.find("AMPL") != std::string::npos) return "amplitude";
    if (device_attribute.find("POW") != std::string::npos) return "power";
    
    // Generic measurements
    if (device_attribute.find("TEMP") != std::string::npos) return "temperature";
    if (device_attribute.find("PRESS") != std::string::npos) return "pressure";
    if (device_attribute.find("CURR") != std::string::npos) return "current";
    if (device_attribute.find("VOLT") != std::string::npos) return "voltage";
    
    return "measurement";
}

bool H5Parser::validateDataConsistency(const std::vector<double>& values, 
                                       const TimestampData& timestamps,
                                       const std::string& signal_name) const {
    if (values.size() != timestamps.count) {
        std::cerr << "    Size mismatch for " << signal_name << ": values=" 
                  << values.size() << ", timestamps=" << timestamps.count << std::endl;
        return false;
    }
    
    if (values.empty()) {
        std::cerr << "    Empty values array for " << signal_name << std::endl;
        return false;
    }
    
    // Check for all NaN or infinite values
    size_t invalid_count = 0;
    for (double value : values) {
        if (std::isnan(value) || std::isinf(value)) {
            invalid_count++;
        }
    }
    
    if (invalid_count == values.size()) {
        std::cerr << "    All values are NaN/infinite for " << signal_name << std::endl;
        return false;
    }
    
    if (invalid_count > values.size() / 2) {
        std::cerr << "    Warning: " << invalid_count << "/" << values.size() 
                  << " invalid values for " << signal_name << std::endl;
    }
    
    return true;
}

// Getter methods
std::vector<SignalData> H5Parser::getAllSignals() const {
    return parsed_signals_;
}

std::vector<SignalData> H5Parser::getSignalsByDevice(const std::string& device) const {
    std::vector<SignalData> filtered;
    
    for (const auto& signal : parsed_signals_) {
        if (signal.info.device == device) {
            filtered.push_back(signal);
        }
    }
    
    return filtered;
}

std::vector<SignalData> H5Parser::getSignalsByDeviceArea(const std::string& device_area) const {
    std::vector<SignalData> filtered;
    
    for (const auto& signal : parsed_signals_) {
        if (signal.info.device_area == device_area) {
            filtered.push_back(signal);
        }
    }
    
    return filtered;
}

std::vector<SignalData> H5Parser::getSignalsByDeviceAttribute(const std::string& device_attribute) const {
    std::vector<SignalData> filtered;
    
    for (const auto& signal : parsed_signals_) {
        if (signal.info.device_attribute == device_attribute) {
            filtered.push_back(signal);
        }
    }
    
    return filtered;
}

std::vector<SignalData> H5Parser::getSignalsByProject(const std::string& project) const {
    std::vector<SignalData> filtered;
    
    for (const auto& signal : parsed_signals_) {
        if (signal.file_metadata.project == project) {
            filtered.push_back(signal);
        }
    }
    
    return filtered;
}

SignalData* H5Parser::findSignal(const std::string& signal_name) {
    for (auto& signal : parsed_signals_) {
        if (signal.info.full_name == signal_name) {
            return &signal;
        }
    }
    return nullptr;
}

const SignalData* H5Parser::findSignal(const std::string& signal_name) const {
    for (const auto& signal : parsed_signals_) {
        if (signal.info.full_name == signal_name) {
            return &signal;
        }
    }
    return nullptr;
}

std::vector<H5FileMetadata> H5Parser::getFileMetadata() const {
    std::vector<H5FileMetadata> metadata_list;
    
    // Extract unique file metadata from parsed signals
    std::set<std::string> seen_files;
    for (const auto& signal : parsed_signals_) {
        if (seen_files.find(signal.file_metadata.full_path) == seen_files.end()) {
            metadata_list.push_back(signal.file_metadata);
            seen_files.insert(signal.file_metadata.full_path);
        }
    }
    
    return metadata_list;
}

void H5Parser::printSummary() const {
    std::cout << "\n=== H5 Parser Summary ===" << std::endl;
    std::cout << "Total signals parsed: " << parsed_signals_.size() << std::endl;
    std::cout << "Total files processed: " << file_timestamps_.size() << std::endl;
    
    if (!parsed_signals_.empty()) {
        // Calculate total data points
        size_t total_points = 0;
        for (const auto& signal : parsed_signals_) {
            total_points += signal.values.size();
        }
        std::cout << "Total data points: " << total_points << std::endl;
        
        printDeviceSummary();
        printDeviceAreaSummary();
        printProjectSummary();
    }
}

void H5Parser::printDetailedSummary() const {
    printDeviceSummary();
    printDeviceAreaSummary();
    printProjectSummary();
    printTimingSummary();
    
    std::cout << "\nSignal Types:" << std::endl;
    std::map<std::string, size_t> signal_type_counts;
    for (const auto& signal : parsed_signals_) {
        signal_type_counts[signal.info.signal_type]++;
    }
    for (const auto& [type, count] : signal_type_counts) {
        std::cout << "  " << type << ": " << count << " signals" << std::endl;
    }
}

void H5Parser::printDeviceSummary() const {
    std::cout << "\nDevices:" << std::endl;
    std::map<std::string, size_t> device_counts;
    for (const auto& signal : parsed_signals_) {
        device_counts[signal.info.device]++;
    }
    for (const auto& [device, count] : device_counts) {
        std::cout << "  " << device << ": " << count << " signals" << std::endl;
    }
}

void H5Parser::printDeviceAreaSummary() const {
    std::cout << "\nDevice Areas:" << std::endl;
    std::map<std::string, size_t> area_counts;
    for (const auto& signal : parsed_signals_) {
        area_counts[signal.info.device_area]++;
    }
    for (const auto& [area, count] : area_counts) {
        std::cout << "  " << area << ": " << count << " signals" << std::endl;
    }
}

void H5Parser::printProjectSummary() const {
    std::cout << "\nProjects:" << std::endl;
    std::map<std::string, size_t> project_counts;
    for (const auto& signal : parsed_signals_) {
        project_counts[signal.file_metadata.project]++;
    }
    for (const auto& [project, count] : project_counts) {
        std::cout << "  " << project << ": " << count << " signals" << std::endl;
    }
}

void H5Parser::printTimingSummary() const {
    if (file_timestamps_.empty()) return;
    
    std::cout << "\nTiming Summary:" << std::endl;
    for (const auto& [filepath, timestamps] : file_timestamps_) {
        std::filesystem::path path(filepath);
        std::cout << "  " << path.filename().string() << ":" << std::endl;
        std::cout << "    Samples: " << timestamps->count << std::endl;
        std::cout << "    Period: " << timestamps->period_nanos << " ns";
        if (timestamps->period_nanos > 0) {
            std::cout << " (" << std::fixed << std::setprecision(1) 
                      << (1000000000.0 / timestamps->period_nanos) << " Hz)";
        }
        std::cout << std::endl;
        std::cout << "    Regular: " << (timestamps->is_regular_sampling ? "Yes" : "No") << std::endl;
    }
}
