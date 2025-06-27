cmake_minimum_required(VERSION 3.13)
project(DataProvider LANGUAGES CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

#
# Dependencies
#
find_package(Protobuf REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX)

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

#
# Target Include Directories
#
target_include_directories(mainh5 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
)

#
# Install Scripts
#
install(PROGRAMS 
    scripts/stub_h5 
    DESTINATION bin
)
