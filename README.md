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
OspreyProject_Slac/
├── .gitignore                    # Git ignore patterns
├── README.md                     # Main project documentation
│
├── .github/                      # GitHub Actions workflows
│   └── workflows/
│       └── build.yml             # CI/CD build configuration
│
├── DataProvider/                 # 🎯 Backend stub creator for MLDP
│   ├── CMakeLists.txt            # DataProvider build configuration
│   │
│   ├── include/                  # Header files
│   │   ├── annotation_client.hpp # Annotation client interface
│   │   ├── h5_parser.hpp         # H5 file parser interface
│   │   ├── ingest_client.hpp     # MLDP ingestion client interface
│   │   ├── PacketParser.h        # Packet parser interface
│   │   └── query_client.hpp      # Query client interface
│   │
│   ├── src/                      # Source implementations
│   │   ├── annotate_client.cpp   # Annotation client implementation
│   │   ├── h5_parser.cpp         # H5 file parser implementation
│   │   ├── ingest_client.cpp     # MLDP ingestion client implementation
│   │   ├── PacketParser.cpp      # Packet parser implementation
│   │   └── query_client.cpp      # Query client implementation
│   │
│   ├── apps/                     # Main executable programs
│   │   └── mainh5.cpp            # H5 data processor main program
│   │
│   ├── scripts/                   # MLDP integration scripts
│   │   └── [stub creator scripts] # Scripts for MLDP to call
│   │
│   ├── config/                   # Configuration files
│   │   └── [configuration files] # Settings for different data types
│   │
│   ├── proto/                    # Protocol buffer definitions
│   │   ├── CMakeLists.txt        # Proto build configuration
│   │   ├── common.proto          # Shared protobuf types
│   │   └── ingestion.proto       # Ingestion service definitions
│   │
│   ├── old_cmake_for_ref/        # Reference materials
│   │   └── CMakeLists.txt        # Old CMake for reference
│   │
│   └── build/ (🛑 ignored)       # Build output directory
├── data-platform/ (🛑 ignored)   # 📚 MLDP platform documentation & tools
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

# Setting up data-platform
```text
Follow this link and follow the quick install method in main directory of this project
```
[MLDP Data Platform](https://github.com/osprey-dcs/data-platform)
