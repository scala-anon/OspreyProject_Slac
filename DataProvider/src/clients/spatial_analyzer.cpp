#include "spatial_analyzer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>

SpatialAnalyzer::SpatialAnalyzer(size_t num_threads) 
    : shutdown_(false), num_threads_(num_threads),
      pvAnalysisCache_(std::make_unique<SimpleLRUCache<std::string, SpatialMetadata>>(10000)) {
    
    // PHASE 2: Initialize thread pool
    worker_threads_.reserve(num_threads_);
    for (size_t i = 0; i < num_threads_; ++i) {
        worker_threads_.emplace_back(&SpatialAnalyzer::workerThreadFunction, this);
    }
    
    std::cout << "SpatialAnalyzer initialized with " << num_threads_ << " worker threads and caching" << std::endl;
}

SpatialAnalyzer::~SpatialAnalyzer() {
    shutdown_ = true;
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool SpatialAnalyzer::loadDictionaries(const std::string& dictionariesPath) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Load original JSON dictionaries
        std::ifstream deviceFile(dictionariesPath + "/device_classifications.json");
        if (deviceFile.is_open()) {
            deviceFile >> deviceClassifications;
            deviceFile.close();
        }
        
        std::ifstream beamlineFile(dictionariesPath + "/lcls_beamline_boundaries.json");
        if (beamlineFile.is_open()) {
            beamlineFile >> beamlineBoundaries;
            beamlineFile.close();
        }
        
        if (deviceClassifications.empty() || beamlineBoundaries.empty()) {
            std::cerr << "Failed to load dictionary files" << std::endl;
            return false;
        }
        
        // PHASE 1: Build optimized lookup caches
        buildDeviceTypeCache();
        buildAreaCache();
        buildPatternCache();
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Dictionary loading and cache building completed in " << duration.count() << "ms" << std::endl;
        std::cout << "Device type cache entries: " << deviceTypeCache_.size() << std::endl;
        std::cout << "Area cache entries: " << areaCache_.size() << std::endl;
        std::cout << "Pattern cache entries: " << patternTagsCache_.size() << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading dictionaries: " << e.what() << std::endl;
        return false;
    }
}

// PHASE 1: Build device type lookup cache
void SpatialAnalyzer::buildDeviceTypeCache() {
    std::unique_lock<std::shared_mutex> lock(cache_mutex_);
    
    deviceTypeCache_.clear();
    deviceTypeCache_.reserve(1000); // Pre-allocate for known device types
    
    for (const auto& [deviceType, classification] : deviceClassifications.items()) {
        CachedDeviceInfo info;
        
        if (classification.contains("category")) {
            info.category = classification["category"].get<std::string>();
        }
        if (classification.contains("function")) {
            info.function = classification["function"].get<std::string>();
        }
        if (classification.contains("controllable")) {
            info.controllable = classification["controllable"].get<bool>();
        }
        if (classification.contains("tags") && classification["tags"].is_array()) {
            for (const auto& tag : classification["tags"]) {
                info.tags.push_back(tag.get<std::string>());
            }
        }
        
        deviceTypeCache_[deviceType] = std::move(info);
    }
    
    std::cout << "Built device type cache with " << deviceTypeCache_.size() << " entries" << std::endl;
}

// PHASE 1: Build area coordinate lookup cache
void SpatialAnalyzer::buildAreaCache() {
    std::unique_lock<std::shared_mutex> lock(cache_mutex_);
    
    areaCache_.clear();
    areaCache_.reserve(500); // Pre-allocate for known areas
    
    for (const auto& [area, boundary] : beamlineBoundaries.items()) {
        CachedAreaInfo info;
        
        if (boundary.contains("z_min")) {
            info.z_min = boundary["z_min"].get<double>();
        }
        if (boundary.contains("z_max")) {
            info.z_max = boundary["z_max"].get<double>();
        }
        if (boundary.contains("beam_path")) {
            info.beamPath = boundary["beam_path"].get<std::string>();
        }
        if (boundary.contains("description")) {
            info.description = boundary["description"].get<std::string>();
        }
        
        areaCache_[area] = std::move(info);
    }
    
    std::cout << "Built area cache with " << areaCache_.size() << " entries" << std::endl;
}

// PHASE 1: Build pattern-based tag cache
void SpatialAnalyzer::buildPatternCache() {
    std::unique_lock<std::shared_mutex> lock(cache_mutex_);
    
    patternTagsCache_.clear();
    
    // Common LCLS-II patterns and their associated tags
    patternTagsCache_["BPMS"] = {"beam_position", "diagnostic", "monitor"};
    patternTagsCache_["KLYS"] = {"klystron", "rf_power", "actuator"};
    patternTagsCache_["QUAD"] = {"quadrupole", "magnetic", "focusing"};
    patternTagsCache_["BEND"] = {"dipole", "magnetic", "steering"};
    patternTagsCache_["TMIT"] = {"charge", "diagnostic", "monitor"};
    patternTagsCache_["AMPL"] = {"amplitude", "rf", "measurement"};
    patternTagsCache_["PHAS"] = {"phase", "rf", "measurement"};
    patternTagsCache_["BCTRL"] = {"control", "magnetic", "actuator"};
    
    std::cout << "Built pattern cache with " << patternTagsCache_.size() << " entries" << std::endl;
}

// PHASE 4: Optimized PV name parsing using string_view
bool SpatialAnalyzer::parsePVNameOptimized(std::string_view pvName, 
                                         std::string& deviceType, std::string& area, 
                                         std::string& position, std::string& attribute) {
    constexpr char delimiter = ':';
    std::vector<std::string_view> parts;
    parts.reserve(4); // Most PV names have 4 parts
    
    size_t start = 0;
    size_t pos = 0;
    
    while ((pos = pvName.find(delimiter, start)) != std::string_view::npos) {
        parts.emplace_back(pvName.substr(start, pos - start));
        start = pos + 1;
    }
    
    if (start < pvName.length()) {
        parts.emplace_back(pvName.substr(start));
    }
    
    if (parts.size() >= 4) {
        deviceType = std::string(parts[0]);
        area = std::string(parts[1]);
        position = std::string(parts[2]);
        attribute = std::string(parts[3]);
        return true;
    }
    
    return false;
}

// PHASE 1: Cache-aware device classification
void SpatialAnalyzer::classifyDeviceCached(const std::string& deviceType, SpatialMetadata& metadata) {
    std::shared_lock<std::shared_mutex> lock(cache_mutex_);
    
    auto it = deviceTypeCache_.find(deviceType);
    if (it != deviceTypeCache_.end()) {
        const auto& cached_info = it->second;
        metadata.category = cached_info.category;
        metadata.function = cached_info.function;
        metadata.controllable = cached_info.controllable;
        
        // Reserve space and copy tags efficiently
        metadata.tags.reserve(metadata.tags.size() + cached_info.tags.size());
        metadata.tags.insert(metadata.tags.end(), cached_info.tags.begin(), cached_info.tags.end());
    }
}

// PHASE 1: Cache-aware signal classification
void SpatialAnalyzer::classifySignalCached(const std::string& deviceType, const std::string& attribute, SpatialMetadata& metadata) {
    // First check pattern cache for quick classification
    std::shared_lock<std::shared_mutex> lock(cache_mutex_);
    
    auto pattern_it = patternTagsCache_.find(deviceType);
    if (pattern_it != patternTagsCache_.end()) {
        const auto& pattern_tags = pattern_it->second;
        metadata.tags.reserve(metadata.tags.size() + pattern_tags.size());
        metadata.tags.insert(metadata.tags.end(), pattern_tags.begin(), pattern_tags.end());
    }
    
    // Attribute-specific classification
    if (attribute.find("AMPL") != std::string::npos) {
        metadata.signalType = "amplitude";
        metadata.units = "MV";
    } else if (attribute.find("PHAS") != std::string::npos) {
        metadata.signalType = "phase";
        metadata.units = "degrees";
    } else if (attribute.find("TMIT") != std::string::npos) {
        metadata.signalType = "charge";
        metadata.units = "pC";
    } else if (attribute.find("BCTRL") != std::string::npos) {
        metadata.signalType = "control";
        metadata.units = "A";
    }
}

// PHASE 1: Cache-aware coordinate lookup
void SpatialAnalyzer::findCoordinatesCached(const std::string& area, const std::string& position, SpatialMetadata& metadata) {
    std::shared_lock<std::shared_mutex> lock(cache_mutex_);
    
    auto it = areaCache_.find(area);
    if (it != areaCache_.end()) {
        const auto& cached_info = it->second;
        metadata.beamPath = cached_info.beamPath;
        metadata.areaDescription = cached_info.description;
        
        // Calculate Z position based on area boundaries and position
        try {
            double pos_offset = std::stod(position);
            double z_range = cached_info.z_max - cached_info.z_min;
            metadata.zPosition = cached_info.z_min + (pos_offset / 1000.0) * z_range; // Assuming position is in mm
            metadata.zUncertainty = 0.1; // 10cm uncertainty
        } catch (const std::exception&) {
            // Use area center if position parsing fails
            metadata.zPosition = (cached_info.z_min + cached_info.z_max) / 2.0;
            metadata.zUncertainty = (cached_info.z_max - cached_info.z_min) / 2.0;
        }
    }
}

// PHASE 1: Optimized tag generation
void SpatialAnalyzer::generateTagsCached(SpatialMetadata& metadata) {
    // Pre-allocate tag vector capacity
    metadata.tags.reserve(metadata.tags.size() + 5);
    
    // Add beam path tag
    if (!metadata.beamPath.empty()) {
        metadata.tags.push_back("beampath_" + metadata.beamPath);
    }
    
    // Add area tag
    if (!metadata.area.empty()) {
        metadata.tags.push_back("area_" + metadata.area);
    }
    
    // Add device type tag
    if (!metadata.deviceType.empty()) {
        metadata.tags.push_back("device_" + metadata.deviceType);
    }
    
    // Add signal type tag
    if (!metadata.signalType.empty()) {
        metadata.tags.push_back("signal_" + metadata.signalType);
    }
    
    // Remove duplicates efficiently
    std::sort(metadata.tags.begin(), metadata.tags.end());
    metadata.tags.erase(std::unique(metadata.tags.begin(), metadata.tags.end()), metadata.tags.end());
}

// PHASE 4: Memory optimization
void SpatialAnalyzer::preallocateVectors(SpatialMetadata& metadata) {
    metadata.tags.reserve(10); // Pre-allocate space for typical tag count
}

SpatialAnalyzer::SpatialMetadata SpatialAnalyzer::analyzePV(const std::string& pvName) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // PHASE 1: Check cache first
    SpatialMetadata cached_result;
    if (pvAnalysisCache_->get(pvName, cached_result)) {
        metrics_.cache_hits++;
        return cached_result;
    }
    
    metrics_.cache_misses++;
    auto result = analyzePVInternal(pvName);
    
    // Cache the result
    pvAnalysisCache_->put(pvName, result);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Update performance metrics
    metrics_.total_analyses++;
    double new_avg = (metrics_.avg_analysis_time_ms.load() * (metrics_.total_analyses - 1) + 
                     duration.count() / 1000.0) / metrics_.total_analyses;
    metrics_.avg_analysis_time_ms = new_avg;
    
    return result;
}

SpatialAnalyzer::SpatialMetadata SpatialAnalyzer::analyzePVInternal(const std::string& pvName) {
    SpatialMetadata metadata;
    preallocateVectors(metadata);
    
    if (!parsePVNameOptimized(pvName, metadata.deviceType, metadata.area, metadata.position, metadata.attribute)) {
        return metadata;
    }
    
    classifyDeviceCached(metadata.deviceType, metadata);
    classifySignalCached(metadata.deviceType, metadata.attribute, metadata);
    findCoordinatesCached(metadata.area, metadata.position, metadata);
    generateTagsCached(metadata);
    
    metadata.isValid = true;
    return metadata;
}

// PHASE 2: Batch processing with parallel execution
std::vector<SpatialAnalyzer::SpatialMetadata> SpatialAnalyzer::analyzePVsBatch(const std::vector<std::string>& pvNames) {
    const size_t batch_size = std::max(size_t(1), pvNames.size() / num_threads_);
    std::vector<std::future<std::vector<SpatialMetadata>>> futures;
    
    // Split work across threads
    for (size_t i = 0; i < pvNames.size(); i += batch_size) {
        size_t end = std::min(i + batch_size, pvNames.size());
        std::vector<std::string> batch(pvNames.begin() + i, pvNames.begin() + end);
        
        auto future = std::async(std::launch::async, [this, batch]() {
            std::vector<SpatialMetadata> results;
            results.reserve(batch.size());
            
            for (const auto& pvName : batch) {
                results.push_back(analyzePV(pvName));
            }
            
            return results;
        });
        
        futures.push_back(std::move(future));
    }
    
    // Collect results
    std::vector<SpatialMetadata> all_results;
    all_results.reserve(pvNames.size());
    
    for (auto& future : futures) {
        auto batch_results = future.get();
        all_results.insert(all_results.end(), 
                          std::make_move_iterator(batch_results.begin()),
                          std::make_move_iterator(batch_results.end()));
    }
    
    return all_results;
}

// PHASE 2: Async processing
std::future<std::vector<SpatialAnalyzer::SpatialMetadata>> 
SpatialAnalyzer::analyzePVsAsync(const std::vector<std::string>& pvNames) {
    return std::async(std::launch::async, [this, pvNames]() {
        return analyzePVsBatch(pvNames);
    });
}

// PHASE 2: Worker thread function (for future thread pool implementation)
void SpatialAnalyzer::workerThreadFunction() {
    // This would be used for a more sophisticated thread pool implementation
    // For now, we use std::async for simplicity
    while (!shutdown_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Cache management methods
void SpatialAnalyzer::clearCache() {
    pvAnalysisCache_->clear();
}

size_t SpatialAnalyzer::getCacheSize() const {
    return pvAnalysisCache_->size();
}

void SpatialAnalyzer::setCacheMaxSize(size_t maxSize) {
    pvAnalysisCache_->resize(maxSize);
}

// Legacy compatibility methods
bool SpatialAnalyzer::parsePVName(const std::string& pvName, std::string& deviceType, 
                                  std::string& area, std::string& position, std::string& attribute) {
    return parsePVNameOptimized(pvName, deviceType, area, position, attribute);
}

void SpatialAnalyzer::classifyDevice(const std::string& deviceType, SpatialMetadata& metadata) {
    classifyDeviceCached(deviceType, metadata);
}

void SpatialAnalyzer::classifySignal(const std::string& deviceType, const std::string& attribute, SpatialMetadata& metadata) {
    classifySignalCached(deviceType, attribute, metadata);
}

void SpatialAnalyzer::findCoordinates(const std::string& area, const std::string& position, SpatialMetadata& metadata) {
    findCoordinatesCached(area, position, metadata);
}

void SpatialAnalyzer::generateTags(SpatialMetadata& metadata) {
    generateTagsCached(metadata);
}