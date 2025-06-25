#
# Protobuf/Grpc source files
#


#
# Add Library target with protobuf sources
#
add_library(myproto 
    common.proto
    ingestion.proto)

target_link_libraries(myproto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(myproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# Compile protobuf and grpc files in myproto target to cpp
#
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(TARGET myproto LANGUAGE cpp APPEND_PATH)
protobuf_generate(TARGET myproto LANGUAGE grpc APPEND_PATH GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

