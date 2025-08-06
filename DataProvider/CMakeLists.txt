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
find_package(OpenMP REQUIRED)

# Find gRPC
find_package(gRPC CONFIG)
if(NOT gRPC_FOUND)
    # Fallback to pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GRPC REQUIRED grpc++)
endif()

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
    ${PROTOBUF_INCLUDE_DIRS}
)

if(gRPC_FOUND)
    include_directories(${gRPC_INCLUDE_DIRS})
elseif(GRPC_FOUND)
    include_directories(${GRPC_INCLUDE_DIRS})
endif()

if(NLOHMANN_JSON_INCLUDE_DIR)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
endif()

# HDF5 definition
add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)

# ========== IMPORTANT: Build Common Client First ==========
# Common client library (used by all other clients)
add_library(common_client STATIC
    src/clients/common_client.cpp
)

target_link_libraries(common_client PUBLIC
    myproto
    ${PROTOBUF_LIBRARIES}
    grpc++
    grpc
    Threads::Threads
)

# ========== Service-specific client libraries ==========
# Ingestion client library
add_library(ingest_client STATIC
    src/clients/ingest_client.cpp
)

target_link_libraries(ingest_client PUBLIC
    common_client
    myproto
    ${PROTOBUF_LIBRARIES}
    grpc++
    grpc
    Threads::Threads
)

# Query client library
add_library(query_client_lib STATIC  # Named query_client_lib to avoid confusion
    src/clients/query_client.cpp
)

target_link_libraries(query_client_lib PUBLIC
    common_client
    myproto
    ${PROTOBUF_LIBRARIES}
    grpc++
    grpc
    Threads::Threads
)

# Annotation client library
add_library(annotation_client STATIC
    src/clients/annotation_client.cpp
)

target_link_libraries(annotation_client PUBLIC
    common_client
    myproto
    ${PROTOBUF_LIBRARIES}
    grpc++
    grpc
    Threads::Threads
)

# ========== Combined client library with all services ==========
# This includes parsers and any other utilities
add_library(dp_clients STATIC
    src/clients/archiver_client.cpp
    src/clients/spatial_analyzer.cpp
    src/parsers/h5_parser.cpp
)

target_link_libraries(dp_clients PUBLIC
    common_client
    ingest_client
    query_client_lib
    annotation_client
    myproto
    ${HDF5_CXX_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${CURL_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
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

# ========== Executables ==========
# Basic executables
add_executable(h5_to_dp_bare apps/h5_to_dp_bare.cpp)
target_link_libraries(h5_to_dp_bare PRIVATE dp_clients)

add_executable(h5_to_dp_bare_optimal apps/h5_to_dp_bare_optimal.cpp)
target_link_libraries(h5_to_dp_bare_optimal PRIVATE dp_clients)
target_compile_options(h5_to_dp_bare_optimal PRIVATE
    -O2
    -march=native
    $<$<CONFIG:Release>:-DNDEBUG>
)

#add_executable(archiver_to_dp apps/archiver_to_dp.cpp)
#target_link_libraries(archiver_to_dp PRIVATE dp_clients)

#add_executable(query_mongo apps/query_mongo.cpp)
#target_link_libraries(query_mongo PRIVATE dp_clients)

#add_executable(create_dataset apps/create_dataset.cpp)
#target_link_libraries(create_dataset PRIVATE dp_clients)

#add_executable(manage_annotations apps/manage_annotations.cpp)
#target_link_libraries(manage_annotations PRIVATE dp_clients)

#add_executable(cli apps/cli.cpp)
#target_link_libraries(cli PRIVATE dp_clients)

# ========== Install targets ==========
install(TARGETS
    h5_to_dp_bare
    h5_to_dp_bare_optimal
    #archiver_to_dp
    #query_mongo
    #create_dataset
    #manage_annotations
    #cli
    RUNTIME DESTINATION bin
)

install(TARGETS 
    common_client
    ingest_client
    query_client_lib
    annotation_client
    dp_clients
    ARCHIVE DESTINATION lib)

install(DIRECTORY include/ DESTINATION include/dp
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

