#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SpatialAnalyzer {
public:
    struct SpatialMetadata {
        // Parsed PV name
        std::string deviceType, area, position, attribute;
        
        // Device classification
        std::string category, function;
        bool controllable;
        
        // Signal classification  
        std::string signalType, units, description;
        
        // Spatial info
        double zPosition = 0.0, zUncertainty = 1.0;
        std::string beamPath, areaDescription;
        
        // Generated tags
        std::vector<std::string> tags;
        
        bool isValid = false;
    };

    bool loadDictionaries(const std::string& dictionariesPath);
    SpatialMetadata analyzePV(const std::string& pvName);

private:
    json deviceClassifications;
    json beamlineBoundaries;
    
    bool parsePVName(const std::string& pvName, std::string& deviceType, 
                     std::string& area, std::string& position, std::string& attribute);
    void classifyDevice(const std::string& deviceType, SpatialMetadata& metadata);
    void classifySignal(const std::string& deviceType, const std::string& attribute, SpatialMetadata& metadata);
    void findCoordinates(const std::string& area, const std::string& position, SpatialMetadata& metadata);
    void generateTags(SpatialMetadata& metadata);
};
