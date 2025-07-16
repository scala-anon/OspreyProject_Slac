cmake_minimum_required(VERSION 3.13)
project(DataProvider LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Suppress verbose output
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_RULE_MESSAGES OFF)

# Create bin directory for executables only
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Dependencies - quiet find
find_package(Protobuf REQUIRED QUIET)
find_package(HDF5 REQUIRED COMPONENTS CXX QUIET)

# Optional dependencies
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{HOME}/local")
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
    PATHS $ENV{HOME}/local/include /usr/local/include /usr/include
    NO_DEFAULT_PATH QUIET)
find_library(XLSXWRITER_LIBRARY xlsxwriter
    PATHS $ENV{HOME}/local/lib /usr/local/lib /usr/lib64 /usr/lib
    NO_DEFAULT_PATH QUIET)

if(NLOHMANN_JSON_INCLUDE_DIR)
    add_definitions(-DHAVE_NLOHMANN_JSON)
endif()

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
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
    /usr/include
)

# Common sources
set(COMMON_SOURCES
    src/clients/ingest_client.cpp
    src/clients/annotate_client.cpp
    src/clients/query_client.cpp
    src/parsers/PacketParser.cpp
    src/parsers/h5_parser.cpp
    src/parsers/mongo_to_npy_parser.cpp
    src/parsers/data_decoder.cpp
)

# Common libraries
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

if(XLSXWRITER_LIBRARY)
    list(APPEND COMMON_LIBRARIES ${XLSXWRITER_LIBRARY})
endif()

# Executables
add_executable(DataProvider_CLI apps/DataProvider_CLI.cpp ${COMMON_SOURCES})
target_link_libraries(DataProvider_CLI PRIVATE ${COMMON_LIBRARIES})

add_executable(h5_to_dp apps/h5_to_dp.cpp ${COMMON_SOURCES})
target_link_libraries(h5_to_dp PRIVATE ${COMMON_LIBRARIES})

add_executable(data_decoder apps/data_decoder.cpp ${COMMON_SOURCES})
target_link_libraries(data_decoder PRIVATE ${COMMON_LIBRARIES})

add_executable(mongo_to_npy apps/mongo_to_npy.cpp ${COMMON_SOURCES})
target_link_libraries(mongo_to_npy PRIVATE ${COMMON_LIBRARIES})

# Install targets
install(TARGETS DataProvider_CLI h5_to_dp data_decoder mongo_to_npy
    RUNTIME DESTINATION bin
)
