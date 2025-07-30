cmake_minimum_required(VERSION 3.13)
project(DataProvider LANGUAGES C CXX)

# Basic configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set release build by default for performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Find required dependencies
find_package(Protobuf REQUIRED QUIET)
find_package(HDF5 REQUIRED COMPONENTS CXX QUIET)
find_package(CURL REQUIRED QUIET)
find_package(Threads REQUIRED)

# Find nlohmann/json
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
    PATHS $ENV{HOME}/local/include /usr/local/include /usr/include)

# Add proto subdirectory
add_subdirectory(proto)

# Include directories
include_directories(
    include
    include/clients
    include/parsers
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

if(NLOHMANN_JSON_INCLUDE_DIR)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
endif()

# HDF5 definition
add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)

# Core client library - ADD ANNOTATION CLIENT
add_library(dp_clients STATIC
    src/clients/ingest_client.cpp
    src/clients/query_client.cpp
    src/clients/archiver_client.cpp
    src/clients/spatial_analyzer.cpp
    src/clients/annotation_client.cpp
    src/parsers/h5_parser.cpp
)

target_link_libraries(dp_clients PRIVATE
    myproto
    ${HDF5_CXX_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${CURL_LIBRARIES}
    grpc++
    grpc
    protobuf
    absl_synchronization
    absl_strings
    absl_time
    absl_status
    absl_base
    Threads::Threads
)

# Existing executables
add_executable(h5_to_dp apps/h5_to_dp.cpp)
target_link_libraries(h5_to_dp PRIVATE dp_clients)

add_executable(archiver_to_dp apps/archiver_to_dp.cpp)
target_link_libraries(archiver_to_dp PRIVATE dp_clients)

add_executable(query_data apps/query_data.cpp)
target_link_libraries(query_data PRIVATE dp_clients)

add_executable(query_metadata apps/query_metadata.cpp)
target_link_libraries(query_metadata PRIVATE dp_clients)

add_executable(create_dataset apps/create_dataset.cpp)
target_link_libraries(create_dataset PRIVATE dp_clients)

add_executable(manage_annotations apps/manage_annotations.cpp)
target_link_libraries(manage_annotations PRIVATE dp_clients)

install(TARGETS h5_to_dp archiver_to_dp query_data 
    RUNTIME DESTINATION bin)

 install(TARGETS query_metadata create_dataset manage_annotations
     RUNTIME DESTINATION bin)

install(TARGETS dp_clients
    ARCHIVE DESTINATION lib)

install(DIRECTORY include/ DESTINATION include/dp
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
