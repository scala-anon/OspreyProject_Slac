// Demonstration of complete PV spatial enrichment
#include "ingest_client.hpp"
#include <iostream>
#include <iomanip>

void demonstratePvEnrichment(const std::string& pv_name) {
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "PV SPATIAL ENRICHMENT DEMONSTRATION" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    try {
        // Initialize client with spatial enrichment
        IngestClientConfig config;
        config.enable_spatial_enrichment = true;
        config.dictionaries_path = "config/dictionaries/";
        
        IngestClient client(config);
        
        std::cout << "\nInput PV: " << pv_name << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        // Get spatial context
        SpatialContext context = client.enrichPvName(pv_name);
        
        if (context.isValid()) {
            std::cout << "✅ SPATIAL ENRICHMENT SUCCESSFUL" << std::endl;
            std::cout << std::endl;
            
            // Parse components
            std::cout << "PV COMPONENTS:" << std::endl;
            std::cout << "  Device Type:      " << context.device_type << std::endl;        // "BPMS"
            std::cout << "  Device Area:      " << context.area << std::endl;               // "DMPH"
            std::cout << "  Device Location:  " << context.device_location << std::endl;   // "502"
            std::cout << "  Device Attribute: " << context.device_attribute << std::endl;  // "TMITBR"
            std::cout << std::endl;
            
            // Spatial context
            std::cout << "SPATIAL CONTEXT:" << std::endl;
            std::cout << "  Beam Path:        " << context.beam_path << std::endl;         // "CU_HXR"
            std::cout << "  Area Description: " << context.area_description << std::endl;  // "HXR post-undulator line"
            std::cout << "  Sequence Index:   " << context.area_sequence_index << std::endl; // 17
            std::cout << "  Device Class:     " << context.device_class << std::endl;      // "beam_position_monitor"
            std::cout << "  Sector:           " << context.sector << std::endl;            // "92H"
            std::cout << "  Location ID:      " << context.getLocationId() << std::endl;  // "DMPH_502"
            std::cout << std::endl;
            
            // Coordinates
            std::cout << "COORDINATES:" << std::endl;
            auto coords = context.getDeviceCoordinates();
            if (!coords.empty()) {
                if (coords.size() == 1) {
                    std::cout << "  Z Position:       " << std::fixed << std::setprecision(3) 
                              << coords[0] << " m" << std::endl;
                } else if (coords.size() >= 2) {
                    std::cout << "  Z Range:          " << std::fixed << std::setprecision(3)
                              << coords[0] << " - " << coords[1] << " m" << std::endl;
                }
            }
            std::cout << std::endl;
            
            // Show what gets added to MongoDB
            std::cout << "MONGODB ATTRIBUTES ADDED:" << std::endl;
            auto attributes = SpatialEnrichmentEngine("config/dictionaries/").generateSpatialAttributes(context);
            for (const auto& attr : attributes) {
                std::cout << "  " << std::setw(20) << std::left << attr.name() 
                          << " = " << attr.value() << std::endl;
            }
            std::cout << std::endl;
            
            std::cout << "MONGODB TAGS ADDED:" << std::endl;
            auto tags = SpatialEnrichmentEngine("config/dictionaries/").generateSpatialTags(context);
            for (const auto& tag : tags) {
                std::cout << "  " << tag << std::endl;
            }
            
        } else {
            std::cout << "❌ SPATIAL ENRICHMENT FAILED" << std::endl;
            std::cout << "PV name could not be parsed or no spatial data found." << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ ERROR: " << e.what() << std::endl;
    }
    
    std::cout << std::string(80, '=') << std::endl;
}

int main() {
    // Test various PV name patterns
    std::vector<std::string> test_pvs = {
        "BPMS:DMPH:502:TMITBR",      // Your example
        "KLYS:LI23:21:PHAS",         // Klystron phase
        "QUAD:LTUH:620:BCTRL",       // Quadrupole control
        "WIRE:LTUH:755:MOTR",        // Wire scanner motor
        "TORO:DMPS:693:TMIT"         // Toroidal current monitor
    };
    
    for (const auto& pv : test_pvs) {
        demonstratePvEnrichment(pv);
        std::cout << std::endl;
    }
    
    return 0;
}

/*
Expected output for "BPMS:DMPH:502:TMITBR":

================================================================================
PV SPATIAL ENRICHMENT DEMONSTRATION
================================================================================

Input PV: BPMS:DMPH:502:TMITBR
--------------------------------------------------
✅ SPATIAL ENRICHMENT SUCCESSFUL

PV COMPONENTS:
  Device Type:      BPMS
  Device Area:      DMPH
  Device Location:  502
  Device Attribute: TMITBR

SPATIAL CONTEXT:
  Beam Path:        CU_HXR
  Area Description: HXR post-undulator line
  Sequence Index:   17
  Device Class:     beam_position_monitor
  Sector:           92H
  Location ID:      DMPH_502

COORDINATES:
  Z Position:       3718.483 m

MONGODB ATTRIBUTES ADDED:
  beam_path            = CU_HXR
  area                 = DMPH
  area_sequence_index  = 17
  device_class         = beam_position_monitor
  area_description     = HXR post-undulator line
  sector               = 92H
  device_type          = BPMS
  device_location      = 502
  location_id          = DMPH_502
  device_attribute     = TMITBR
  z_coordinates        = 3718.483
  z_min                = 3718.483

MONGODB TAGS ADDED:
  spatial_enriched
  beam_path_CU_HXR
  area_DMPH
  device_class_beam_position_monitor
  device_type_BPMS
  location_502
  device_location_DMPH_502
  attribute_TMITBR
  sector_92H
================================================================================
*/
