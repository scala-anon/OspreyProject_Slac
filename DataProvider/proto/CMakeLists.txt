#
# Protobuf/Grpc source files
#


#
# Add Library target with protobuf sources
#
add_library(myproto
    common.proto
    ingestion.proto
    annotation.proto
    query.proto)

# use dynamic detection of the grpc_cpp_plugin path
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)
message(STATUS "grpc_cpp_plugin found at: ${GRPC_CPP_PLUGIN}")

target_link_libraries(myproto
    PUBLIC
        protobuf
       grpc
        grpc++
)
target_include_directories(myproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# Compile protobuf and grpc files in myproto target to cpp
#
protobuf_generate(TARGET myproto LANGUAGE cpp APPEND_PATH)

protobuf_generate(
    TARGET myproto
    LANGUAGE grpc
    APPEND_PATH
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=${GRPC_CPP_PLUGIN}"
)
