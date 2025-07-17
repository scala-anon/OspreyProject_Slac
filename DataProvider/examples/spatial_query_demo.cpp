// Example: Spatial-aware queries using the new QueryClient library
#include "query_client.hpp"
#include <iostream>

int main() {
    try {
        // 1. Initialize query client with spatial capabilities
        QueryClient client("config/ingestion_config.json", true);
        
        std::cout << "=== SPATIAL QUERY EXAMPLES ===\n\n";
        
        // 2. Create time range for last hour
        auto time_range = QueryUtils::createRecentTimeRange(3600); // Last hour
        
        // 3. Query by area - find all devices in DMPH area
        std::cout << "1. Query all BPMS devices in DMPH area:\n";
        auto result1 = client.queryByArea("DMPH", time_range, {"BPMS:.*"});
        
        if (result1.success) {
            std::cout << "   Found " << result1.pv_names_found.size() << " PVs\n";
            std::cout << "   Total data points: " << result1.getTotalDataPoints() << "\n";
            for (const auto& pv : result1.pv_names_found) {
                std::cout << "   - " << pv << "\n";
            }
        }
        std::cout << "\n";
        
        // 4. Query by beam path - find all devices along CU_HXR
        std::cout << "2. Query all devices along CU_HXR beam path:\n";
        auto result2 = client.queryByBeamPath("CU_HXR", time_range);
        
        if (result2.success) {
            std::cout << "   Found " << result2.pv_names_found.size() << " PVs\n";
            std::cout << "   Processing time: " << result2.processing_time_seconds << "s\n";
        }
        std::cout << "\n";
        
        // 5. Query by device class - find all beam position monitors
        std::cout << "3. Query all beam position monitors:\n";
        auto result3 = client.queryByDeviceClass("beam_position_monitor", time_range);
        
        if (result3.success) {
            std::cout << "   Found " << result3.pv_names_found.size() << " BPMs\n";
        }
        std::cout << "\n";
        
        // 6. Query by Z coordinate range
        std::cout << "4. Query devices between Z=3700m and Z=3750m:\n";
        auto result4 = client.queryByZRange(3700.0, 3750.0, time_range);
        
        if (result4.success) {
            std::cout << "   Found " << result4.pv_names_found.size() << " devices in range\n";
        }
        std::cout << "\n";
        
        // 7. Advanced spatial query combining multiple criteria
        std::cout << "5. Advanced query - BPMS devices in DMPH area with TMIT attributes:\n";
        
        SpatialQueryParams spatial_params;
        spatial_params.area = "DMPH";
        spatial_params.device_class = "beam_position_monitor";
        
        auto result5 = client.spatialQuery(spatial_params, time_range, {".*TMIT.*"});
        
        if (result5.success) {
            std::cout << "   Found " << result5.pv_names_found.size() << " matching PVs:\n";
            for (const auto& pv : result5.pv_names_found) {
                std::cout << "   - " << pv << "\n";
            }
        }
        std::cout << "\n";
        
        // 8. PV Discovery - find what's available
        std::cout << "6. PV Discovery:\n";
        
        auto areas = client.getAvailableAreas();
        std::cout << "   Available areas: ";
        for (const auto& area : areas) {
            std::cout << area << " ";
        }
        std::cout << "\n";
        
        auto beam_paths = client.getAvailableBeamPaths();
        std::cout << "   Available beam paths: ";
        for (const auto& path : beam_paths) {
            std::cout << path << " ";
        }
        std::cout << "\n";
        
        auto device_classes = client.getAvailableDeviceClasses();
        std::cout << "   Available device classes: ";
        for (const auto& cls : device_classes) {
            std::cout << cls << " ";
        }
        std::cout << "\n\n";
        
        // 9. Find specific PVs in an area
        std::cout << "7. Find all TMITBR PVs in DMPH:\n";
        auto dmph_pvs = client.findPvsByArea("DMPH", ".*TMITBR.*");
        for (const auto& pv : dmph_pvs) {
            std::cout << "   - " << pv << "\n";
        }
        
        std::cout << "\n=== SPATIAL QUERIES COMPLETE ===\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

/*
Expected output:

=== SPATIAL QUERY EXAMPLES ===

1. Query all BPMS devices in DMPH area:
   Found 3 PVs
   Total data points: 15000
   - BPMS:DMPH:502:TMITBR
   - BPMS:DMPH:502:X
   - BPMS:DMPH:502:Y

2. Query all devices along CU_HXR beam path:
   Found 156 PVs
   Processing time: 2.3s

3. Query all beam position monitors:
   Found 23 BPMs

4. Query devices between Z=3700m and Z=3750m:
   Found 8 devices in range

5. Advanced query - BPMS devices in DMPH area with TMIT attributes:
   Found 1 matching PVs:
   - BPMS:DMPH:502:TMITBR

6. PV Discovery:
   Available areas: DMPH LTUH DMPS LI23 LI24
   Available beam paths: CU_HXR SC_SXR CU_SXR
   Available device classes: beam_position_monitor klystron quadrupole wire_scanner

7. Find all TMITBR PVs in DMPH:
   - BPMS:DMPH:502:TMITBR

=== SPATIAL QUERIES COMPLETE ===
*/
