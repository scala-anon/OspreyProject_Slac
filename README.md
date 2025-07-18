# OspreyProject_Slac

## Description
This project provides a high-performance C++ data processing pipeline specifically designed for SLAC accelerator data cleaning and ingestion into the Osprey Machine Learning Data Platform (MLDP). The system transforms raw H5 files from accelerator operations into structured, spatially-enriched MongoDB documents optimized for analysis and machine learning workflows.

## Motivation
SLAC generates massive volumes of accelerator data stored in H5 files that require systematic processing before analysis. This raw data often contains significant amounts of invalid values (NaN), lacks spatial context about device locations, and exists in formats that are difficult to query efficiently. The existing workflow required manual data cleaning and lacked automated spatial enrichment capabilities.

## Why did we build this project?
We built this system to solve critical data processing challenges specific to SLAC's accelerator operations:

1. **Data Quality Issues:** Raw accelerator data contains high percentages of NaN values (often 95%+) that need statistical analysis and quality assessment
2. **Spatial Context Loss:** Device names like `KLYS_LI20_61_AMPL` contain spatial information that wasn't being systematically extracted and enriched
3. **Query Inefficiency:** H5 files are not queryable; researchers needed a way to efficiently search and analyze data by device location, type, and time ranges
4. **Statistical Processing:** Large datasets required automated statistical analysis to identify operational periods and data quality metrics
5. **Workflow Automation:** Manual data cleaning processes needed to be automated for consistent, repeatable results

## What problem does it solve?
The **OspreyProject MLDP Backend** addresses these challenges by providing:

- **Automated Data Cleaning:** Processes raw H5 files and calculates comprehensive statistics including NaN percentages, valid data counts, and quality metrics
- **Spatial Enrichment:** Automatically extracts and enriches device location information, beamline associations, and device classifications from PV names
- **Structured Storage:** Converts H5 files into MongoDB documents with consistent schema and spatial metadata
- **Efficient Querying:** Enables fast queries by device type, location, time range, and data quality criteria
- **Statistical Analysis:** Provides immediate statistical summaries of large datasets without requiring full data retrieval
- **Accelerator-Aware Processing:** Understands accelerator-specific data patterns, device types, and operational characteristics

The system transforms isolated H5 files into an accessible, queryable database that enables accelerator physicists and data scientists to quickly identify relevant data, assess data quality, and focus analysis efforts on meaningful operational periods.

## MongoDB Bucket Format

Data is stored in MongoDB using a structured bucket format optimized for accelerator data:

### Bucket Document Structure
```javascript
{
  _id: "KLYS_LI20_61_AMPL-1752695016-338222271",           // Unique identifier
  
  // Core Data
  pvName: "KLYS_LI20_61_AMPL",                             // Process Variable name
  dataColumn: {                                            // Time-series data (binary)
    name: "KLYS_LI20_61_AMPL",
    bytes: Binary(...),                                    // Compressed data values
    valueType: "DOUBLEVALUE"
  },
  dataTimestamps: {                                        // Temporal information
    firstTime: ISODate("2025-07-16T19:43:36.338Z"),
    lastTime: ISODate("2025-07-16T20:46:00.760Z"),
    sampleCount: 453456,
    samplePeriod: 8257539                                  // nanoseconds
  },
  
  // Device Information
  attributes: {
    // Basic device parsing
    device: "KLYS",                                        // Device type
    device_area: "LI20",                                   // Beamline area  
    device_location: "61",                                 // Position number
    device_attribute: "AMPL",                              // Signal type
    
    // Spatial enrichment (automatically added)
    spatial_area: "LI20",                                  // Normalized area
    spatial_category: "rf_system",                         // Device category
    spatial_controllable: "true",                          // Control capability
    spatial_device_type: "KLYS",                           // Normalized type
    spatial_function: "Provides RF power for acceleration", // Description
    spatial_position: "61",                                // Position in beamline
    spatial_signal_type: "rf_amplitude",                   // Signal classification
    spatial_z_position: "0.000000",                        // Longitudinal position
    spatial_z_uncertainty: "1.000000",                     // Position uncertainty
    
    // Data provenance
    origin: "CU",                                          // Beam path origin
    project: "default",                                    // Data collection project
    units: "MV/m"                                          // Engineering units
  },
  
  // Searchable tags
  tags: [
    "h5_data",                                             // Data source type
    "device_KLYS",                                         // Device-based tag
    "project_default",                                     // Project tag
    "spatial_enriched",                                    // Processing status
    "spatial_rf_system",                                   // Category tag
    "spatial_rf_amplitude",                                // Signal type tag
    "spatial_li20",                                        // Area tag
    "spatial_controllable"                                 // Capability tag
  ],
  
  // Metadata
  providerId: "687a7a734b72a5133bc395ac",                  // Data provider ID
  createdAt: ISODate("2025-07-18T16:53:33.969Z")          // Ingestion timestamp
}
```

### Key Features of the Bucket Format:
- **Binary Data Storage:** Raw time-series data stored efficiently as compressed binary
- **Spatial Metadata:** Comprehensive location and device classification information
- **Temporal Indexing:** Optimized for time-range queries
- **Tag-Based Search:** Multiple tag dimensions for flexible querying
- **Data Provenance:** Complete traceability from source H5 files
- **Statistical Ready:** Structure supports statistical analysis without data retrieval

## Tools

### h5_to_dp
**Data Ingestion and Processing Tool**
- Processes H5 files from SLAC accelerator data collection
- Performs spatial enrichment by parsing device names and adding location metadata
- Calculates data quality statistics including NaN counts and valid data percentages
- Ingests processed data into MongoDB with structured bucket format
- Supports batch processing of multiple H5 files with configurable options
- Provides progress monitoring and error handling for large datasets

### query_pv
**Data Query and Analysis Tool**
- Queries MongoDB for specific process variables by name, pattern matching, or multiple PV lists
- Supports human-readable date ranges (MM-DD-YYYY format) instead of Unix timestamps
- Calculates comprehensive statistics (mean, median, std dev, min, max, NaN counts) for large datasets
- Handles streaming queries for datasets with millions of data points
- Provides flexible output options including statistics-only mode and raw data value display
- Enables regex pattern matching for device discovery (e.g., all BPMS devices, all LI20 klystrons)

## Installation

### Prerequisites

- **Operating System**: SLAC computing environment (RHEL/CentOS-based) or Ubuntu 20.04+
- **Compiler**: GCC 9+ with C++17 support
- **CMake**: Version 3.13 or higher
- **Memory**: Minimum 8GB RAM (recommended for accelerator datasets)

### System Dependencies

#### Install Required Libraries
```bash
# Core dependencies
sudo apt install -y \
    build-essential \
    cmake \
    git \
    libprotobuf-dev \
    libgrpc++-dev \
    protobuf-compiler-grpc \
    protobuf-compiler \
    libhdf5-dev \
    libhdf5-cpp-103 \
    nlohmann-json3-dev
```

### Building the Project

```bash
# Clone and build
git clone <repository-url>
cd OspreyProject_Slac
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Verification
```bash
# Test H5 parsing (local only)
./bin/h5_to_dp /path/to/h5/files --local-only

# Test data querying
./bin/query_pv --pv=KLYS_LI20_61_AMPL --date=07-16-2025 --stats-only
```

## Usage

### Data Ingestion Workflow
```bash
# Process H5 files with spatial enrichment
./bin/h5_to_dp /sdf/data/ad/store/bsas/nc_CoAD/2025/07/16/

# Process with filtering options
./bin/h5_to_dp /path/to/h5/files --device=KLYS --max-signals=100
```

### Data Query Workflow
```bash
# Query specific device
./bin/query_pv --pv=KLYS_LI20_61_AMPL --date=07-16-2025

# Find all beam position monitors
./bin/query_pv --pattern="BPMS.*" --date=07-16-2025 --stats-only

# Analyze multiple devices
./bin/query_pv --pvs=KLYS_LI20_61_AMPL,BPMS_DMPH_502_TMITBR --show-values=10
```

## Credits

### Project Team
**Lead Developer & Architect**  
Nicholas Mamais - *Osprey Distributed Control Systems*  
- System architecture and C++ implementation
- Spatial enrichment algorithms
- Statistical processing pipeline
- MongoDB integration and optimization

### Institutional Affiliations
**Osprey Distributed Control Systems**  
- MLDP platform development
- Data infrastructure architecture

**SLAC National Accelerator Laboratory**  
- Accelerator data requirements
- Scientific use case validation
- Production testing environment

### Core Technologies
- [**gRPC**](https://grpc.io/) - High-performance RPC framework
- [**Protocol Buffers**](https://developers.google.com/protocol-buffers) - Data serialization
- [**HDF5**](https://www.hdfgroup.org/solutions/hdf5/) - Scientific data format support
- [**MongoDB**](https://www.mongodb.com/) - Document database backend
- [**CMake**](https://cmake.org/) - Build system
