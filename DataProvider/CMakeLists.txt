cmake_minimum_required(VERSION 3.13)
project(DataProvider LANGUAGES CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

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

#
# Common Sources
#
set(COMMON_SOURCES
    src/ingest_client.cpp
    src/PacketParser.cpp
    src/h5_parser.cpp
    src/annotate_client.cpp
    src/query_client.cpp
)

#
# Executables
#
add_executable(mainh5 apps/mainh5.cpp ${COMMON_SOURCES})

add_executable(exporter apps/exporter.cpp ${COMMON_SOURCES})
#
# Link Libraries
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

target_link_libraries(mainh5 PRIVATE ${COMMON_LIBRARIES})
target_link_libraries(exporter PRIVATE ${COMMON_LIBRARIES})

# Add optional libraries if found
if(XLSXWRITER_LIBRARY)
    target_link_libraries(exporter PRIVATE ${XLSXWRITER_LIBRARY})
endif()

#
# Target Include Directories
#
target_include_directories(mainh5 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
)

target_include_directories(exporter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
    $ENV{HOME}/local/include
)

if(NLOHMANN_JSON_INCLUDE_DIR)
    target_include_directories(exporter PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR})
endif()

#
# Install Scripts
#
install(PROGRAMS 
    scripts/stub_h5 
    DESTINATION bin
)
