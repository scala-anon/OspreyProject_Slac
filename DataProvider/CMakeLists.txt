cmake_minimum_required(VERSION 3.13)
project(DataProvider LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Suppress all output
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_RULE_MESSAGES OFF)
set(CMAKE_INSTALL_MESSAGE NEVER)

# Create directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Find dependencies quietly
find_package(Protobuf REQUIRED QUIET)
find_package(HDF5 REQUIRED COMPONENTS CXX QUIET)

# Check for nlohmann/json
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
    PATHS $ENV{HOME}/local/include /usr/local/include /usr/include
    NO_DEFAULT_PATH QUIET)

if(NLOHMANN_JSON_INCLUDE_DIR)
    add_definitions(-DHAVE_NLOHMANN_JSON)
endif()

# Optional xlsxwriter
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{HOME}/local")
find_library(XLSXWRITER_LIBRARY xlsxwriter
    PATHS $ENV{HOME}/local/lib /usr/local/lib /usr/lib64 /usr/lib
    NO_DEFAULT_PATH QUIET)

if(XLSXWRITER_LIBRARY)
    add_definitions(-DHAVE_XLSXWRITER)
endif()

# Proto subdirectory
add_subdirectory(proto)

# Include directories
add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)
include_directories(
    include
    include/clients
    include/parsers
    include/tools
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
    /usr/include
)

if(NLOHMANN_JSON_INCLUDE_DIR)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
endif()

# Client Libraries
add_library(ingest_client_lib STATIC
    src/clients/ingest_client.cpp
    src/clients/spatial_analyzer.cpp
)
target_link_libraries(ingest_client_lib PUBLIC
    myproto
    grpc++
    grpc
    protobuf
    absl_synchronization
    absl_strings
    absl_time
    absl_status
    absl_base
)

add_library(query_client_lib STATIC
    src/clients/query_client.cpp
)
target_link_libraries(query_client_lib PUBLIC
    myproto
    grpc++
    grpc
    protobuf
    absl_synchronization
    absl_strings
    absl_time
    absl_status
    absl_base
)

# Metadata Recovery Library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/metadata_recovery.cpp")
    add_library(metadata_recovery_lib STATIC
        src/clients/metadata_recovery.cpp
    )
    target_link_libraries(metadata_recovery_lib PUBLIC
        query_client_lib
        myproto
        grpc++
        grpc
        protobuf
        absl_synchronization
        absl_strings
        absl_time
        absl_status
        absl_base
    )
endif()

# Parser Library
add_library(parser_lib STATIC
    src/parsers/PacketParser.cpp
    src/parsers/h5_parser.cpp
)
target_link_libraries(parser_lib PUBLIC
    ${HDF5_CXX_LIBRARIES}
    ${HDF5_LIBRARIES}
)

# Combined interface library
add_library(dp_clients INTERFACE)
target_link_libraries(dp_clients INTERFACE
    ingest_client_lib
    query_client_lib
)

# Add metadata recovery to combined library if it exists
if(TARGET metadata_recovery_lib)
    target_link_libraries(dp_clients INTERFACE metadata_recovery_lib)
endif()

# Common libraries
set(COMMON_LIBRARIES
    dp_clients
    parser_lib
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

if(XLSXWRITER_LIBRARY)
    list(APPEND COMMON_LIBRARIES ${XLSXWRITER_LIBRARY})
endif()

# Executables
add_executable(h5_to_dp apps/h5_to_dp.cpp)
target_link_libraries(h5_to_dp PRIVATE ${COMMON_LIBRARIES})

add_executable(query_pv apps/query_pv.cpp)
target_link_libraries(query_pv PRIVATE ${COMMON_LIBRARIES})

# Examples
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/metadata_recovery_example.cpp")
    add_executable(metadata_recovery_demo examples/metadata_recovery_example.cpp)
    target_link_libraries(metadata_recovery_demo PRIVATE ${COMMON_LIBRARIES})
endif()

# Config directory setup (silent)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/config")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/config/dictionaries")
endif()

# Install targets
install(TARGETS h5_to_dp query_pv
    RUNTIME DESTINATION bin
)

# Install libraries including metadata recovery if available
set(INSTALL_LIBS dp_clients ingest_client_lib query_client_lib parser_lib)
if(TARGET metadata_recovery_lib)
    list(APPEND INSTALL_LIBS metadata_recovery_lib)
endif()

install(TARGETS ${INSTALL_LIBS}
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include/dp
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)
