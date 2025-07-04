# OspreyProject_Slac

## Description
This project provides a high-performance C++ backend integration system for the Osprey Machine Learning Data Platform(MLDP), specifically designed to bridge the gap between scientific data acquisition systems and modern data analytics workflows. 

## Motivation
Scientific facilities, particularly accelerator laboratories like SLAC, generate massive volumes of time-series data from process variables(PVs) that are traditionally stored in HDF5 files or custom binary formats. However, this often remains siloed and difficult to access for machine learning applications, real-time monitoring, and collaborative analysis. The existing data pipelines lacked efficient mechanisms to ingest, correlate, and query this data at scale. 

## Why did we build this project?
We built this system to solve critical data accessibility challenges in scientific computing environments:
  1. **Data Ingestion Bottleneck:** Legacy systems struggle to efficiently process and ingest larges volumes of multi-channel scientific data into modern data Platform
  2. **Format Compatibility:** Scientific data comes in various formats (HDF5, custom binary packets, streaming data) that need unified handling
  3. **Real-time Requirements:** Experimental facilities need both historical data analysis and real-time data streaming capabilities.
  4. **Scalability:** Traditional tools can't handle the volume and velocity of modern accelerator data systems.
  5. **Lab Reusability:** Each scientific facility was reinventing similar data ingestion solutions. 

## What problem does it solve?
The **OspreyProject MLDP Backend** addresses these challenges by providing:

  - **Unified Data Ingestion:** Supports multiple input formats (HDF5 files, binary packet streams, real-time data feeds) with a single, consistent interface
  - **High-Performance Processing:** Optimized C++ implementation with memory-efficient algorithms that can handle millions of data points with minimal latency
  - **Streaming Architecture:** Built on gRPC for efficient client-server communication with support for unary, client-streaming, server-streaming, and bidirectional streaming patterns
  - **Data Correlation Engine:** Advanced algorithms to correlate time-series data across multiple process variables, enabling cross-system analysis
  - **Platform Integration:** Seamless integration with the Osprey MLDP for annotation, querying, and machine learning workflows
  - **Lab-Ready Deployment:** Modular design allows easy deployment across different scientific facilities with minimal configuration

The system transforms isolated scientific data into an accessible, queryable, and ML-ready format, enabling researchers to focus on analysis rather than data wrangling. It bridges the gap between legacy scientific data acquisition systems and modern cloud-native data platforms, making years of experimental data immediately available for advanced analytics and machine learning applications.
 
## Installation

### Prerequisites

Before building the OspreyProject MLDP Backend, ensure your system meets the following requirements:

- **Operating System**: Ubuntu 20.04+ (or compatible Linux distribution)
- **Compiler**: GCC 9+ or Clang 10+ with C++17 support
- **CMake**: Version 3.13 or higher
- **Memory**: Minimum 4GB RAM (8GB+ recommended for large datasets)

### System Dependencies

#### 1. Install Core Build Tools

```bash
# Update package manager
sudo apt update

# Install essential build tools
sudo apt install -y \
    build-essential \
    cmake \
    git \
    pkg-config
```

#### 2. Install Required Libraries

```bash
# Install core dependencies
sudo apt install -y \
    libprotobuf-dev \
    libgrpc++-dev \
    protobuf-compiler-grpc \
    protobuf-compiler \
    libhdf5-dev \
    libhdf5-cpp-103
```

#### 3. Install Optional Dependencies (Recommended)

```bash
# Install optional libraries for enhanced functionality
sudo apt install -y \
    nlohmann-json3-dev \
    libxlsxwriter-dev
```

**Note**: Optional dependencies enable additional export formats and improved JSON handling.

### Data Platform Setup

The project integrates with the Osprey Data Platform. Follow these steps to set up the platform:

#### 1. Clone and Setup Data Platform

```bash
# Clone the data platform repository in your project root
git clone https://github.com/osprey-dcs/data-platform.git

# Follow the quick install method from the data-platform documentation
cd data-platform
# Follow installation instructions from their README
```

#### 2. Verify Data Platform Installation

```bash
# Ensure the data platform is running on localhost:50051
# Check the data-platform documentation for startup instructions
```

### Building the Project

#### 1. Clone the Repository

```bash
git clone <your-repository-url>
cd OspreyProject_Slac
```

#### 2. Configure Data Path (Optional)

If you have specific data files to process, update the path in the source:

```bash
# Edit the data path in your application
nano DataProvider/apps/h5_to_dp.cpp
# Modify any hardcoded paths to match your data location
```

#### 3. Build the Project

```bash
# Create and enter build directory
mkdir build && cd build

# Configure the project with CMake
cmake ..

# Build using all available CPU cores
make -j$(nproc)
```

#### 4. Verify Build Success

```bash
# Check that executables were created
ls -la h5_to_dp

# Test basic functionality (local parsing only)
./h5_to_dp /path/to/your/h5/files --local-only
```

### Alternative Installation Methods

#### Using Docker (Coming Soon)

```bash
# Docker support is planned for future releases
# This will provide a containerized environment with all dependencies
```

#### Manual Dependency Installation

If package manager installation fails, you can build dependencies from source:

```bash
# Example for gRPC (if not available via apt)
git clone --recurse-submodules -b v1.50.0 https://github.com/grpc/grpc
cd grpc
mkdir -p cmake/build
cd cmake/build
cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF ../..
make -j$(nproc)
sudo make install
```

### Troubleshooting

#### Common Build Issues

**Issue**: CMake cannot find protobuf
```bash
# Solution: Install protobuf development packages
sudo apt install -y libprotobuf-dev protobuf-compiler
```

**Issue**: HDF5 library not found
```bash
# Solution: Install HDF5 development packages
sudo apt install -y libhdf5-dev libhdf5-cpp-103
```

**Issue**: gRPC compilation errors
```bash
# Solution: Ensure you have the correct gRPC version
sudo apt install -y libgrpc++-dev protobuf-compiler-grpc
```

#### Memory Issues During Build

If you encounter memory issues during compilation:

```bash
# Build with fewer parallel jobs
make -j2

# Or build without parallelization
make
```

#### Runtime Issues

**Issue**: Cannot connect to MLDP server
- Ensure the data platform is running on `localhost:50051`
- Check firewall settings
- Verify the data platform installation

**Issue**: HDF5 file parsing errors
- Ensure HDF5 files are not corrupted
- Check file permissions
- Verify HDF5 file format compatibility

### Verification

After successful installation, verify everything works:

```bash
# Test local parsing (no MLDP connection required)
./h5_to_dp /path/to/test/data --local-only

# Test full integration (requires running data platform)
./h5_to_dp /path/to/test/data
```

### Next Steps

Once installation is complete:

1. **Configure your data sources** in the application
2. **Start the data platform** services
3. **Run your first data ingestion** with `./h5_to_dp`
4. **Check the Usage section** for detailed command-line options

## Usage

### Basic Commands

```bash
# Parse H5 files locally (no MLDP connection)
./h5_to_dp /path/to/h5/files --local-only

# Full integration with MLDP (default)
./h5_to_dp /path/to/h5/files

# Streaming with custom chunk size
./h5_to_dp /path/to/h5/files --stream-chunk-size=20

## Credits

### Project Team

**Lead Developer & Architect**  
Nicholas Mamais - *Osprey Distributed Control Systems*  
- Project conception and architecture design
- Core C++ backend implementation
- HDF5 parsing engine and optimization
- gRPC integration and streaming protocols

### Institutional Affiliations

**Osprey Distributed Control Systems**  
- Primary development organization
- Machine Learning Data Platform (MLDP) architecture
- Scientific data infrastructure expertise

**SLAC National Accelerator Laboratory**  
- Scientific use case requirements
- Accelerator data acquisition domain expertise
- Real-world testing and validation environment

### Open Source Dependencies

This project builds upon excellent open source technologies:

**Core Technologies**
- [**gRPC**](https://grpc.io/) - High-performance RPC framework
- [**Protocol Buffers**](https://developers.google.com/protocol-buffers) - Language-neutral serialization
- [**HDF5**](https://www.hdfgroup.org/solutions/hdf5/) - High-performance data management library
- [**CMake**](https://cmake.org/) - Cross-platform build system

**Data Platform Foundation**
- [**Osprey Data Platform**](https://github.com/osprey-dcs/data-platform) - Core MLDP infrastructure
- **MongoDB** - Database backend for data storage

**Development Tools**
- [**nlohmann/json**](https://github.com/nlohmann/json) - Modern C++ JSON library
- [**xlsxwriter**](https://libxlsxwriter.github.io/) - Excel file generation
- **Google Test** - Unit testing framework
- **GitHub Actions** - Continuous integration


