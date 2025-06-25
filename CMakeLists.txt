cmake_minimum_required(VERSION 3.13)
project(OspreyProject LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

#
# Dependencies
#
# apt install libprotobuf-dev libgrpc++-dev protobuf-compiler-grpc
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

add_subdirectory(proto)
add_subdirectory(client)
