#
# Sources
#
set(SOURCES src/main.cpp src/ingest_client.cpp src/PacketParser.cpp
            src/HDF5/HDF5_Parser.cpp)

#
# Target
#
add_executable(client ${SOURCES})

include_directories(/usr/include/hdf5/serial)
add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)

target_include_directories(
  client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/HDF5 ${CMAKE_BINARY_DIR}/proto)

target_link_libraries(client PRIVATE myproto hdf5_serial_cpp hdf5_serial)
