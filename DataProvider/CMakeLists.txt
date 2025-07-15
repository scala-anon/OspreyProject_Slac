cmake_minimum_required(VERSION 3.13)
project(DataProvider LANGUAGES CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

#
# Output directory setup
#
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

#
# Core dependencies (existing)
#
find_package(Protobuf REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX)

#
# Optional dependencies
#
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{HOME}/local")

# JSON library (for EPICS parser)
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
    PATHS $ENV{HOME}/local/include /usr/local/include /usr/include)

# XlsxWriter (existing optional)
find_library(XLSXWRITER_LIBRARY xlsxwriter
    PATHS $ENV{HOME}/local/lib /usr/local/lib /usr/lib64 /usr/lib)

#
# Feature flags and includes
#
if(NLOHMANN_JSON_INCLUDE_DIR)
    add_definitions(-DHAVE_NLOHMANN_JSON)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
endif()

if(XLSXWRITER_LIBRARY)
    add_definitions(-DHAVE_XLSXWRITER)
endif()

#
# Include directories
#
add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)
include_directories(/usr/include)
include_directories(include)
include_directories(include/clients)
include_directories(include/parsers)
include_directories(${CMAKE_BINARY_DIR}/proto)


#
# Add proto subdirectory
#
add_subdirectory(proto)

#
# Source files
#
set(COMMON_SOURCES
    src/clients/ingest_client.cpp
    src/clients/annotate_client.cpp
    src/clients/query_client.cpp
    src/parsers/PacketParser.cpp
    src/parsers/h5_parser.cpp
    src/parsers/mongo_to_npy_parser.cpp
    src/parsers/data_decoder.cpp
)

#
# Libraries
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

# Add optional libraries
if(XLSXWRITER_LIBRARY)
    list(APPEND COMMON_LIBRARIES ${XLSXWRITER_LIBRARY})
endif()

#
# Include directories for all targets
#
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clients
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parsers
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
)

#
# Build executables
#
set(EXECUTABLES
    h5_to_dp
    mongo_to_npy
    data_decoder
    DataProvider_CLI
)

# Create all executables
foreach(EXECUTABLE ${EXECUTABLES})
    add_executable(${EXECUTABLE} apps/${EXECUTABLE}.cpp ${COMMON_SOURCES})
    target_link_libraries(${EXECUTABLE} PRIVATE ${COMMON_LIBRARIES})
    target_include_directories(${EXECUTABLE} PRIVATE ${COMMON_INCLUDE_DIRS})
endforeach()

#
# Copy scripts to bin directory
#
file(GLOB SCRIPT_FILES scripts/*)
if(SCRIPT_FILES)
    foreach(script_file ${SCRIPT_FILES})
        get_filename_component(script_name ${script_file} NAME)
        configure_file(${script_file} ${CMAKE_CURRENT_SOURCE_DIR}/bin/${script_name} COPYONLY)
    endforeach()
endif()

#
# Install targets
#
install(TARGETS ${EXECUTABLES}
    RUNTIME DESTINATION bin
    COMPONENT executables
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/
        DESTINATION bin
        COMPONENT scripts
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
    )
endif()

#
# Build summary
#
message(STATUS "")
message(STATUS "=== DataProvider Build Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Directory: ${CMAKE_CURRENT_SOURCE_DIR}/bin/")
message(STATUS "")
message(STATUS "Core Executables:")
message(STATUS "  ✓ DataProvider_CLI")
message(STATUS "  ✓ h5_to_dp")
message(STATUS "  ✓ mongo_to_npy")
message(STATUS "  ✓ data_decoder")

message(STATUS "")
message(STATUS "Optional Features:")
if(NLOHMANN_JSON_INCLUDE_DIR)
    message(STATUS "  ✓ JSON support")
else()
    message(STATUS "  ✗ JSON support")
endif()
if(XLSXWRITER_LIBRARY)
    message(STATUS "  ✓ Excel export")
else()
    message(STATUS "  ✗ Excel export")
endif()
if(CURL_FOUND)
    message(STATUS "  ✓ HTTP client (CURL)")
else()
    message(STATUS "  ✗ HTTP client (CURL)")
endif()

message(STATUS "=========================================")
message(STATUS "")
