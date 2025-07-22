cmake_minimum_required(VERSION 3.13)
project(DataProvider LANGUAGES C CXX)

# ============================================================================
# BASIC OPTIMIZATION FLAGS (Compatible with existing code)
# ============================================================================

# Set C++ standard and enable optimizations
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler-specific optimization flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # GCC/Clang optimization flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops")
    
    # Threading optimizations
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC optimization flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /Oy")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # Multi-processor compilation
endif()

# ============================================================================
# THREADING LIBRARIES
# ============================================================================

# Find threading package
find_package(Threads REQUIRED)

# Find TBB (Intel Threading Building Blocks) for advanced parallel algorithms
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(TBB tbb)
    if(TBB_FOUND)
        add_definitions(-DHAVE_TBB)
        message(STATUS "Intel TBB found - enabling advanced parallelization")
    endif()
endif()

# ============================================================================
# MEMORY OPTIMIZATION LIBRARIES
# ============================================================================

# Find jemalloc for better memory allocation performance
find_library(JEMALLOC_LIB jemalloc)
if(JEMALLOC_LIB)
    add_definitions(-DHAVE_JEMALLOC)
    message(STATUS "jemalloc found - enabling optimized memory allocation")
endif()

# Find tcmalloc as alternative
find_library(TCMALLOC_LIB tcmalloc)
if(TCMALLOC_LIB AND NOT JEMALLOC_LIB)
    add_definitions(-DHAVE_TCMALLOC)
    message(STATUS "tcmalloc found - enabling optimized memory allocation")
endif()

# ============================================================================
# OPTIMIZATION FEATURE DETECTION
# ============================================================================

# Check for CPU features
include(CheckCXXSourceCompiles)

# Check for AVX2 support (for vectorized operations)
check_cxx_source_compiles("
    #include <immintrin.h>
    int main() { 
        __m256i a = _mm256_setzero_si256(); 
        return 0; 
    }" HAVE_AVX2)

if(HAVE_AVX2)
    add_definitions(-DHAVE_AVX2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
    message(STATUS "AVX2 support detected - enabling vectorized operations")
endif()

# Check for atomic operations
check_cxx_source_compiles("
    #include <atomic>
    int main() { 
        std::atomic<uint64_t> counter{0};
        counter.fetch_add(1, std::memory_order_relaxed);
        return 0; 
    }" HAVE_ATOMIC_UINT64)

if(HAVE_ATOMIC_UINT64)
    add_definitions(-DHAVE_ATOMIC_UINT64)
    message(STATUS "64-bit atomic operations supported")
endif()

# ============================================================================
# BASIC OPTIMIZATION FLAGS
# ============================================================================

# Add preprocessor definitions for basic optimization
add_definitions(-DENABLE_PERFORMANCE_MONITORING)
add_definitions(-DENABLE_DETAILED_TIMING)
add_definitions(-DDEFAULT_BATCH_SIZE=500)
add_definitions(-DMAX_BATCH_SIZE=1000)

# Determine optimal thread count
cmake_host_system_information(RESULT CPU_CORES QUERY NUMBER_OF_LOGICAL_CORES)
math(EXPR DEFAULT_THREAD_COUNT "${CPU_CORES}")
add_definitions(-DDEFAULT_THREAD_COUNT=${DEFAULT_THREAD_COUNT})

message(STATUS "Detected ${CPU_CORES} CPU cores - setting default thread count to ${DEFAULT_THREAD_COUNT}")

# Set default build type to Release for maximum performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Release for performance)" FORCE)
endif()

# ============================================================================
# ORIGINAL CMAKE CONFIGURATION (PRESERVED)
# ============================================================================

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
find_package(CURL REQUIRED QUIET)

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

# Include directories (only include existing directories)
add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)
include_directories(
    include
    include/clients
    include/parsers
    ${CMAKE_BINARY_DIR}/proto
    ${HDF5_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    /usr/include
)

if(NLOHMANN_JSON_INCLUDE_DIR)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
endif()

# ============================================================================
# CLIENT LIBRARIES (with basic optimizations)
# ============================================================================

# Client Libraries
add_library(ingest_client_lib STATIC
    src/clients/ingest_client.cpp
    src/clients/spatial_analyzer.cpp
)

# Add optimization properties
set_target_properties(ingest_client_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(ingest_client_lib PRIVATE
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

# Add TBB if available
if(TBB_FOUND)
    target_link_libraries(ingest_client_lib PRIVATE ${TBB_LIBRARIES})
    target_include_directories(ingest_client_lib PRIVATE ${TBB_INCLUDE_DIRS})
endif()

# Add optimized memory allocator if available
if(JEMALLOC_LIB)
    target_link_libraries(ingest_client_lib PRIVATE ${JEMALLOC_LIB})
elseif(TCMALLOC_LIB)
    target_link_libraries(ingest_client_lib PRIVATE ${TCMALLOC_LIB})
endif()

# Add compile definitions for optimizations
target_compile_definitions(ingest_client_lib PRIVATE
    ENABLE_ALL_OPTIMIZATIONS
    TARGET_THROUGHPUT=50000
    OPTIMIZATION_LEVEL=50
)

add_library(query_client_lib STATIC
    src/clients/query_client.cpp
)
set_target_properties(query_client_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(query_client_lib PRIVATE
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

add_library(archiver_client_lib STATIC
    src/clients/archiver_client.cpp
)
set_target_properties(archiver_client_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(archiver_client_lib PRIVATE
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

# Optional libraries (if files exist)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/metadata_recovery.cpp")
    add_library(metadata_recovery_lib STATIC
        src/clients/metadata_recovery.cpp
    )
    set_target_properties(metadata_recovery_lib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(metadata_recovery_lib PRIVATE
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
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/annotate_client.cpp")
    add_library(annotate_client_lib STATIC
        src/clients/annotate_client.cpp
    )
    set_target_properties(annotate_client_lib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(annotate_client_lib PRIVATE
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
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/spatial.cpp")
    add_library(spatial_lib STATIC
        src/clients/spatial.cpp
    )
    set_target_properties(spatial_lib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(spatial_lib PRIVATE
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
endif()

add_library(parser_lib STATIC
    src/parsers/PacketParser.cpp
    src/parsers/h5_parser.cpp
)
set_target_properties(parser_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(parser_lib PRIVATE
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

# Combined dp_clients library with all existing source files
add_library(dp_clients STATIC
    src/clients/ingest_client.cpp
    src/clients/query_client.cpp
    src/clients/archiver_client.cpp
    src/clients/spatial_analyzer.cpp
    src/clients/annotate_client.cpp
    src/clients/metadata_recovery.cpp
    src/clients/spatial.cpp
    src/parsers/h5_parser.cpp
    src/parsers/PacketParser.cpp
)
set_target_properties(dp_clients PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
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

# ============================================================================
# COMMON LIBRARIES LIST
# ============================================================================

# Common libraries list (enhanced with optimization libraries)
set(COMMON_LIBRARIES 
    dp_clients
    ingest_client_lib
    query_client_lib
    archiver_client_lib
    parser_lib
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

# Add optional libraries if they exist
if(TARGET metadata_recovery_lib)
    list(APPEND COMMON_LIBRARIES metadata_recovery_lib)
endif()

if(TARGET annotate_client_lib)
    list(APPEND COMMON_LIBRARIES annotate_client_lib)
endif()

if(TARGET spatial_lib)
    list(APPEND COMMON_LIBRARIES spatial_lib)
endif()

# Add optimization libraries if available
if(TBB_FOUND)
    list(APPEND COMMON_LIBRARIES ${TBB_LIBRARIES})
endif()

if(JEMALLOC_LIB)
    list(APPEND COMMON_LIBRARIES ${JEMALLOC_LIB})
elseif(TCMALLOC_LIB)
    list(APPEND COMMON_LIBRARIES ${TCMALLOC_LIB})
endif()

if(XLSXWRITER_LIBRARY)
    list(APPEND COMMON_LIBRARIES ${XLSXWRITER_LIBRARY})
endif()

# ============================================================================
# EXECUTABLES (with optimizations)
# ============================================================================

# Executables using existing apps files
add_executable(h5_to_dp apps/h5_to_dp.cpp)
set_target_properties(h5_to_dp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_compile_definitions(h5_to_dp PRIVATE
    ENABLE_ALL_OPTIMIZATIONS
    TARGET_THROUGHPUT=50000
    OPTIMIZATION_LEVEL=50
)
target_link_libraries(h5_to_dp PRIVATE ${COMMON_LIBRARIES})

add_executable(query_pv apps/query_pv.cpp)
set_target_properties(query_pv PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(query_pv PRIVATE ${COMMON_LIBRARIES})

add_executable(archiver_to_dp apps/archiver_to_dp.cpp)
set_target_properties(archiver_to_dp PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(archiver_to_dp PRIVATE ${COMMON_LIBRARIES})

add_executable(query_state apps/query_state.cpp)
set_target_properties(query_state PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(query_state PRIVATE ${COMMON_LIBRARIES})

# Optional executables
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/apps/DataProvider_CLI.cpp")
    add_executable(DataProvider_CLI apps/DataProvider_CLI.cpp)
    set_target_properties(DataProvider_CLI PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
    target_link_libraries(DataProvider_CLI PRIVATE ${COMMON_LIBRARIES})
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/apps/epics_to_mongo.cpp")
    add_executable(epics_to_mongo apps/epics_to_mongo.cpp)
    set_target_properties(epics_to_mongo PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
    target_link_libraries(epics_to_mongo PRIVATE ${COMMON_LIBRARIES})
endif()

# ============================================================================
# CONFIGURATION AND INSTALLATION
# ============================================================================

# Config directory setup (silent)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/config")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/config/dictionaries")
endif()

# Install targets (include all existing executables)
install(TARGETS h5_to_dp query_pv archiver_to_dp query_state
    RUNTIME DESTINATION bin
)

# Install optional executables if they were built
if(TARGET DataProvider_CLI)
    install(TARGETS DataProvider_CLI RUNTIME DESTINATION bin)
endif()

if(TARGET epics_to_mongo)
    install(TARGETS epics_to_mongo RUNTIME DESTINATION bin)
endif()

# Install libraries including all client libs
set(INSTALL_LIBS dp_clients ingest_client_lib query_client_lib archiver_client_lib parser_lib)

if(TARGET metadata_recovery_lib)
    list(APPEND INSTALL_LIBS metadata_recovery_lib)
endif()

if(TARGET annotate_client_lib)
    list(APPEND INSTALL_LIBS annotate_client_lib)
endif()

if(TARGET spatial_lib)
    list(APPEND INSTALL_LIBS spatial_lib)
endif()

install(TARGETS ${INSTALL_LIBS}
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include/dp
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# ============================================================================
# OPTIMIZATION SUMMARY
# ============================================================================

# Print optimization summary
message(STATUS "==================================================")
message(STATUS "LCLS-II BASIC OPTIMIZATION CONFIGURATION")
message(STATUS "==================================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CPU Cores: ${CPU_CORES}")
message(STATUS "Default Thread Count: ${DEFAULT_THREAD_COUNT}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

if(HAVE_AVX2)
    message(STATUS "Vectorization: AVX2 enabled")
else()
    message(STATUS "Vectorization: Not available")
endif()

if(TBB_FOUND)
    message(STATUS "Threading: Intel TBB enabled")
else()
    message(STATUS "Threading: Standard C++ threads only")
endif()

if(JEMALLOC_LIB)
    message(STATUS "Memory Allocator: jemalloc")
elseif(TCMALLOC_LIB)
    message(STATUS "Memory Allocator: tcmalloc")
else()
    message(STATUS "Memory Allocator: System default")
endif()

