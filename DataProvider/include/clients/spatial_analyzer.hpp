#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <string_view>
#include <memory>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SpatialAnalyzer {
public:
    struct SpatialMetadata {
        // Parsed PV name
        std::string deviceType, area, position, attribute;
        
        // Device classification
        std::string category, function;
        bool controllable = false;
        
        // Signal classification  
        std::string signalType, units, description;
        
        // Spatial info
        double zPosition = 0.0, zUncertainty = 1.0;
        std::string beamPath, areaDescription;
        
        // Generated tags
        std::vector<std::string> tags;
        
        bool isValid = false;
    };

    // Constructor
    explicit SpatialAnalyzer(size_t num_threads = std::thread::hardware_concurrency());
    ~SpatialAnalyzer();

    // Core interface
    bool loadDictionaries(const std::string& dictionariesPath);
    SpatialMetadata analyzePV(const std::string& pvName);
    
    // Batch processing methods
    std::vector<SpatialMetadata> analyzePVsBatch(const std::vector<std::string>& pvNames);
    std::future<std::vector<SpatialMetadata>> analyzePVsAsync(const std::vector<std::string>& pvNames);

    // Cache management
    void clearCache();
    size_t getCacheSize() const;
    void setCacheMaxSize(size_t maxSize);

private:
    // Caching structures
    struct CachedDeviceInfo {
        std::string category;
        std::string function;
        bool controllable = false;
        std::vector<std::string> tags;
    };

    struct CachedAreaInfo {
        double z_min = 0.0, z_max = 0.0;
        std::string beamPath;
        std::string description;
    };

    // Simple LRU cache implementation
    template<typename Key, typename Value>
    class SimpleLRUCache {
    private:
        struct CacheEntry {
            Key key;
            Value value;
            std::chrono::steady_clock::time_point last_access;
            
            CacheEntry(const Key& k, const Value& v) 
                : key(k), value(v), last_access(std::chrono::steady_clock::now()) {}
        };
        
        mutable std::shared_mutex mutex_;
        std::unordered_map<Key, std::unique_ptr<CacheEntry>> cache_;
        size_t max_size_;
        
        void evictOldest() {
            if (cache_.size() <= max_size_) return;
            
            auto oldest = cache_.begin();
            for (auto it = cache_.begin(); it != cache_.end(); ++it) {
                if (it->second->last_access < oldest->second->last_access) {
                    oldest = it;
                }
            }
            cache_.erase(oldest);
        }
        
    public:
        explicit SimpleLRUCache(size_t max_size) : max_size_(max_size) {}
        
        bool get(const Key& key, Value& value) const {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = cache_.find(key);
            if (it != cache_.end()) {
                it->second->last_access = std::chrono::steady_clock::now();
                value = it->second->value;
                return true;
            }
            return false;
        }
        
        void put(const Key& key, const Value& value) {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            cache_[key] = std::make_unique<CacheEntry>(key, value);
            if (cache_.size() > max_size_) {
                evictOldest();
            }
        }
        
        void clear() {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            cache_.clear();
        }
        
        size_t size() const {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return cache_.size();
        }
        
        void resize(size_t new_max_size) {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            max_size_ = new_max_size;
            while (cache_.size() > max_size_) {
                evictOldest();
            }
        }
    };

    // Data members
    json deviceClassifications;
    json beamlineBoundaries;
    
    // Pre-computed lookup tables
    std::unordered_map<std::string, CachedDeviceInfo> deviceTypeCache_;
    std::unordered_map<std::string, CachedAreaInfo> areaCache_;
    std::unordered_map<std::string, std::vector<std::string>> patternTagsCache_;
    
    // LRU cache for analyzed PVs
    mutable std::unique_ptr<SimpleLRUCache<std::string, SpatialMetadata>> pvAnalysisCache_;
    
    // Thread pool
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> shutdown_;
    size_t num_threads_;
    
    // Thread safety
    mutable std::shared_mutex cache_mutex_;
    
    // Cache building methods
    void buildDeviceTypeCache();
    void buildAreaCache();
    void buildPatternCache();
    
    // Core analysis methods
    bool parsePVNameOptimized(std::string_view pvName, 
                             std::string& deviceType, std::string& area, 
                             std::string& position, std::string& attribute);
    
    void classifyDeviceCached(const std::string& deviceType, SpatialMetadata& metadata);
    void classifySignalCached(const std::string& deviceType, const std::string& attribute, SpatialMetadata& metadata);
    void findCoordinatesCached(const std::string& area, const std::string& position, SpatialMetadata& metadata);
    void generateTagsCached(SpatialMetadata& metadata);
    
    // Internal methods
    void workerThreadFunction();
    SpatialMetadata analyzePVInternal(const std::string& pvName);
    void preallocateVectors(SpatialMetadata& metadata);
};