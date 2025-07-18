#include "spatial_analyzer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

bool SpatialAnalyzer::loadDictionaries(const std::string& dictionariesPath) {
    try {
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
        
        return !deviceClassifications.empty() && !beamlineBoundaries.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error loading dictionaries: " << e.what() << std::endl;
        return false;
    }
}

SpatialAnalyzer::SpatialMetadata SpatialAnalyzer::analyzePV(const std::string& pvName) {
    SpatialMetadata metadata;
    
    if (!parsePVName(pvName, metadata.deviceType, metadata.area, metadata.position, metadata.attribute)) {
        return metadata;
    }
    
    classifyDevice(metadata.deviceType, metadata);
    classifySignal(metadata.deviceType, metadata.attribute, metadata);
    findCoordinates(metadata.area, metadata.position, metadata);
    generateTags(metadata);
    
    metadata.isValid = true;
    return metadata;
}

bool SpatialAnalyzer::parsePVName(const std::string& pvName, std::string& deviceType, 
                                  std::string& area, std::string& position, std::string& attribute) {
    std::istringstream ss(pvName);
    std::string token;
    std::vector<std::string> parts;
    
    while (std::getline(ss, token, ':')) {
        parts.push_back(token);
    }
    
    if (parts.size() >= 4) {
        deviceType = parts[0];
        area = parts[1];
        position = parts[2];
        attribute = parts[3];
        return true;
    }
    return false;
}

void SpatialAnalyzer::classifyDevice(const std::string& deviceType, SpatialMetadata& metadata) {
    if (deviceClassifications.contains("device_types") && 
        deviceClassifications["device_types"].contains(deviceType)) {
        
        auto device = deviceClassifications["device_types"][deviceType];
        metadata.category = device.value("category", "unknown");
        metadata.function = device.value("function", "");
        metadata.controllable = device.value("controllable", false);
    }
}

void SpatialAnalyzer::classifySignal(const std::string& deviceType, const std::string& attribute, SpatialMetadata& metadata) {
    if (deviceClassifications.contains("device_types") && 
        deviceClassifications["device_types"].contains(deviceType)) {
        
        auto device = deviceClassifications["device_types"][deviceType];
        if (device.contains("typical_signals") && device["typical_signals"].contains(attribute)) {
            auto signal = device["typical_signals"][attribute];
            metadata.description = signal.value("description", "");
            metadata.units = signal.value("units", "");
        }
    }
    
    // Determine signal type from attribute
    if (attribute.find("TMIT") != std::string::npos || attribute.find("CHRG") != std::string::npos) {
        metadata.signalType = "current_beam";
    } else if (attribute.find("AMPL") != std::string::npos || attribute.find("ADES") != std::string::npos) {
        metadata.signalType = "rf_amplitude";
    } else if (attribute.find("PHAS") != std::string::npos || attribute.find("PDES") != std::string::npos) {
        metadata.signalType = "rf_phase";
    } else if (attribute.find("BDES") != std::string::npos || attribute.find("BACT") != std::string::npos) {
        metadata.signalType = "magnetic_field";
    } else if (attribute == "X" || attribute == "Y") {
        metadata.signalType = "position";
    }
}

void SpatialAnalyzer::findCoordinates(const std::string& area, const std::string& position, SpatialMetadata& metadata) {
    if (beamlineBoundaries.contains("areas") && beamlineBoundaries["areas"].contains(area)) {
        auto areaInfo = beamlineBoundaries["areas"][area];
        
        metadata.areaDescription = areaInfo.value("description", "");
        metadata.beamPath = areaInfo.value("beam_path", "");
        
        // Try exact position first
        if (areaInfo.contains("devices") && areaInfo["devices"].contains(position)) {
            metadata.zPosition = areaInfo["devices"][position].value("z_position", 0.0);
            metadata.zUncertainty = areaInfo["devices"][position].value("z_uncertainty", 1.0);
        } 
        // Estimate from area range
        else if (areaInfo.contains("z_range") && areaInfo["z_range"].is_array() && areaInfo["z_range"].size() >= 2) {
            double zMin = areaInfo["z_range"][0];
            double zMax = areaInfo["z_range"][1];
            metadata.zPosition = (zMin + zMax) / 2.0;
            metadata.zUncertainty = (zMax - zMin) / 4.0;
        }
    }
}

void SpatialAnalyzer::generateTags(SpatialMetadata& metadata) {
    metadata.tags.clear();
    
    if (!metadata.category.empty()) metadata.tags.push_back(metadata.category);
    if (!metadata.signalType.empty()) metadata.tags.push_back(metadata.signalType);
    if (!metadata.beamPath.empty()) {
        std::string beamPathTag = metadata.beamPath;
        std::transform(beamPathTag.begin(), beamPathTag.end(), beamPathTag.begin(), ::tolower);
        metadata.tags.push_back(beamPathTag);
    }
    if (!metadata.area.empty()) {
        std::string areaTag = metadata.area;
        std::transform(areaTag.begin(), areaTag.end(), areaTag.begin(), ::tolower);
        metadata.tags.push_back(areaTag);
    }
    if (metadata.controllable) {
        metadata.tags.push_back("controllable");
    } else {
        metadata.tags.push_back("read_only");
    }
}
