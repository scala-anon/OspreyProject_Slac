cmake_minimum_required(VERSION 3.13)
project(DataProvider LANGUAGES CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

#
# Create bin directory in source tree for executables
#
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

#
# Set output directory for all executables to bin/ in source tree
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

#
# Dependencies
#
find_package(Protobuf REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX)

# Add user local paths
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{HOME}/local")

# Try to find optional dependencies in user directories
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
    PATHS $ENV{HOME}/local/include /usr/local/include /usr/include)
find_library(XLSXWRITER_LIBRARY xlsxwriter
    PATHS $ENV{HOME}/local/lib /usr/local/lib /usr/lib64 /usr/lib)

if(NLOHMANN_JSON_INCLUDE_DIR)
    message(STATUS "Found nlohmann/json: ${NLOHMANN_JSON_INCLUDE_DIR}")
    add_definitions(-DHAVE_NLOHMANN_JSON)
endif()

if(XLSXWRITER_LIBRARY)
    message(STATUS "Found xlsxwriter: ${XLSXWRITER_LIBRARY}")
    add_definitions(-DHAVE_XLSXWRITER)
endif()

# Add subdirectory for proto files
add_subdirectory(proto)

#
# Definitions and Include Directories
#
add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)
include_directories(/usr/include)
include_directories(include)
include_directories(${CMAKE_BINARY_DIR}/proto)

# Add the new subdirectories to include path
include_directories(include/clients)
include_directories(include/parsers)

#
# Common Sources - UPDATED PATHS
#
set(COMMON_SOURCES
    src/clients/ingest_client.cpp
    src/clients/annotate_client.cpp
    src/clients/query_client.cpp
    src/parsers/PacketParser.cpp
    src/parsers/h5_parser.cpp
    src/parsers/mongo_to_npy_parser.cpp
    src/parsers/data_decoder.cpp
    src/parsers/epics_archiver.cpp
)

#
# Common Libraries
#
set(COMMON_LIBRARIES
    myproto
    ${HDF5_CXX_LIBRARIES}
    ${HDF5_LIBRARIES}
    grpc++
    grpc
    protobuf
    absl_synchronization
    absl_strings
    absl_time
    absl_status
    absl_base
)

# Add XlsxWriter if found
if(XLSXWRITER_LIBRARY)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES} ${XLSXWRITER_LIBRARY})
endif()

#
# Common Include Directories for all targets
#
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clients
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parsers
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
)

#
# Core Executables - All built to bin/ directory
#
add_executable(h5_to_dp apps/h5_to_dp.cpp ${COMMON_SOURCES})
target_link_libraries(h5_to_dp PRIVATE ${COMMON_LIBRARIES})
target_include_directories(h5_to_dp PRIVATE ${COMMON_INCLUDE_DIRS})

add_executable(mongo_to_npy apps/mongo_to_npy.cpp ${COMMON_SOURCES})
target_link_libraries(mongo_to_npy PRIVATE ${COMMON_LIBRARIES})
target_include_directories(mongo_to_npy PRIVATE ${COMMON_INCLUDE_DIRS})

add_executable(data_decoder apps/data_decoder.cpp ${COMMON_SOURCES})
target_link_libraries(data_decoder PRIVATE ${COMMON_LIBRARIES})
target_include_directories(data_decoder PRIVATE ${COMMON_INCLUDE_DIRS})

add_executable(DataProvider_CLI apps/DataProvider_CLI.cpp ${COMMON_SOURCES})
target_link_libraries(DataProvider_CLI PRIVATE ${COMMON_LIBRARIES})
target_include_directories(DataProvider_CLI PRIVATE ${COMMON_INCLUDE_DIRS})

# NEW: EPICS Archiver Integration
add_executable(epics_to_mongo apps/epics_to_mongo.cpp ${COMMON_SOURCES})
target_link_libraries(epics_to_mongo PRIVATE ${COMMON_LIBRARIES})
target_include_directories(epics_to_mongo PRIVATE ${COMMON_INCLUDE_DIRS})

#
# Copy scripts to bin directory (simple approach)
#
file(GLOB SCRIPT_FILES scripts/*)
if(SCRIPT_FILES)
    foreach(script_file ${SCRIPT_FILES})
        get_filename_component(script_name ${script_file} NAME)
        configure_file(${script_file} ${CMAKE_CURRENT_SOURCE_DIR}/bin/${script_name} COPYONLY)
    endforeach()
    message(STATUS "Scripts will be copied to bin directory")
endif()

#
# Install targets for system-wide installation (optional)
#
install(TARGETS h5_to_dp mongo_to_npy data_decoder DataProvider_CLI epics_to_mongo
    RUNTIME DESTINATION bin
    COMPONENT executables
)

# Install scripts
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/
        DESTINATION bin
        COMPONENT scripts
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
    )
endif()

#
# Print summary of what will be built
#
message(STATUS "")
message(STATUS "=== DataProvider Build Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Directory: ${CMAKE_CURRENT_SOURCE_DIR}/bin/")
message(STATUS "Build Directory: ${CMAKE_CURRENT_BINARY_DIR}/")
message(STATUS "")
message(STATUS "Executables to build:")
message(STATUS "  - DataProvider_CLI")
message(STATUS "  - h5_to_dp")
message(STATUS "  - mongo_to_npy") 
message(STATUS "  - data_decoder")
message(STATUS "  - epics_to_mongo           [NEW]")

if(SCRIPT_FILES)
    message(STATUS "")
    message(STATUS "Scripts to copy:")
    foreach(script_file ${SCRIPT_FILES})
        get_filename_component(script_name ${script_file} NAME)
        message(STATUS "  - ${script_name}")
    endforeach()
endif()

message(STATUS "")
message(STATUS "Feature Support:")
if(NLOHMANN_JSON_INCLUDE_DIR)
    message(STATUS "  ✓ JSON parsing (optional)")
else()
    message(STATUS "  - JSON parsing (optional - nlohmann/json not found)")
endif()

if(XLSXWRITER_LIBRARY)
    message(STATUS "  ✓ Excel export (optional)")
else()
    message(STATUS "  - Excel export (optional - xlsxwriter not found)")
endif()

message(STATUS "  ✓ EPICS archiver integration (using system curl)")
message(STATUS "  ✓ HDF5 parsing")
message(STATUS "  ✓ MongoDB integration via MLDP")

message(STATUS "")
message(STATUS "Note: EPICS archiver uses system curl command (no libcurl dependency)")
message(STATUS "      JSON parsing is done with lightweight custom parser")
message(STATUS "=========================================")
message(STATUS "")
