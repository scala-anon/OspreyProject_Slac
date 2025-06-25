# OspreyProject (MLDP Backend) — C++

**Author**: Nicholas Mamais  
**Affiliation**: Osprey Distributed Control Systems
**Project**: Machine Learning Data Platform (MLDP) Backend Integration  
**Language**: C++17  
**Build System**: CMake  
**Dependencies**: gRPC, Protocol Buffers, MongoDB

---

## 🔍 Overview

This repository provides a modular C++ client for integrating with the Osprey Machine Learning Data Platform (MLDP). The backend supports structured ingestion, annotation, and query operations via gRPC and Protocol Buffers. It is designed for lab reuse and scalability.

---

## 📁 Directory Structure

```text
OspreyProject/
├── client/                    # Client-side source logic
│   ├── CMakeLists.txt         # Client-specific build file
│   └── src/
│       ├── main.cpp               # Application entry point
│       ├── ingest_client.cpp      # Ingestion RPC client
│       ├── annotate_client.cpp    # Annotation RPC client
│       ├── query_client.cpp       # Query RPC client
│       ├── ingest_client.hpp      # Client interface header
│       ├── PacketParser.cpp       # Binary .dat file parser
│       └── PacketParser.h         # Parser header
│
├── proto/                    # Protobuf and gRPC interface definitions
│   ├── CMakeLists.txt        # Proto-specific CMake config
│   ├── common.proto          # Shared types (timestamps, data, etc.)
│   └── ingestion.proto       # Ingestion service interface
│
├── build/  (🛑 ignored)       # CMake build output — excluded via .gitignore
│   └── ...                   # Compiled binaries, objects, and generated files
│
├── CMakeLists.txt            # Top-level CMake configuration
├── README.md                 # Project documentation
└── .gitignore                # Excludes build/ and other local artifacts
```
---

## ⚙️ Dependencies

Make sure the following are installed:

```bash
sudo apt install \
  libprotobuf-dev \
  libgrpc++-dev \
  protobuf-compiler-grpc \
  cmake \
  g++  # Or clang++
```
---

## 🛠️ Build Instructions

Follow these steps to configure and build the project:

# Alter location for data 
```bash
cd /client/src
nano main.cpp
```
```text
alter PacketParser parser("data/mic1-8-CH17-20240511-121442.dat");
```
# Create and enter a build directory
```bash
mkdir build && cd build
```
# Run CMake to configure the project
```bash
cmake ..
```
# Build the project using all available cores
```bash
make -j$(nproc)
```
