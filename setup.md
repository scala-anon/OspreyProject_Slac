# Setup for Data Platform and Data Provider

## Prerequisites

**Core Requirements:**
- **Java**: OpenJDK or Oracle JDK 21
- **MongoDB**: Version 7.0.5 (local installation or Docker container)
- **Maven**: Version 3.6+ (for building and dependency management)
- **Git**: For cloning repositories

**Development Environment:**
- **Java IDE**: IntelliJ IDEA, Eclipse, or VS Code with Java extensions
- **Operating System**: Cross-platform (Windows, macOS, Linux distributions)

**System Dependencies:**
- **build-essential**: GCC compiler and build tools
- **cmake**: Build system generator (version 3.13+)
- **git**: Version control system
- **libprotobuf-dev**: Protocol Buffers development libraries
- **libgrpc++-dev**: gRPC C++ development libraries
- **protobuf-compiler-grpc**: Protocol Buffers gRPC compiler
- **protobuf-compiler**: Protocol Buffers compiler
- **libhdf5-dev**: HDF5 development libraries
- **libhdf5-cpp-103**: HDF5 C++ runtime libraries
- **nlohmann-json3-dev**: JSON for Modern C++ development headers

**Optional Components:**
- **Docker**: Version 20.10+ (if using containerized MongoDB)

---

## Part 1: Data Platform Installation

The Data Platform provides ingestion, query, and annotation services for the ecosystem.

### Java Installation

The Data Platform Java applications require Java 21. Download and install from the [official Java 21 download page](https://www.oracle.com/java/technologies/downloads/#java21).

### MongoDB Installation

MongoDB version 7.0.5 is the current reference version. You can install it locally or use Docker.

#### Option A: Local MongoDB Installation

1. Follow the [MongoDB installation guide](https://www.mongodb.com/docs/manual/administration/install-community/) for your platform
2. Create a user for the data platform applications:

```bash
# Start MongoDB shell
mongosh

# Switch to admin database
use admin

# Create admin user (change password from "admin"!)
db.createUser({user: "admin", pwd: "admin", roles: [ { role: "root", db: "admin" } ]})

# Exit shell
exit
```

#### Option B: Docker MongoDB Installation

Use the provided scripts in the dp-support repo to run MongoDB in a Docker container.

### Data Platform Setup

#### Development Installation

1. Clone the required repositories:
```bash
git clone https://github.com/osprey-dcs/dp-grpc
git clone https://github.com/osprey-dcs/dp-service
git clone https://github.com/osprey-dcs/dp-support  # Optional but recommended
```

2. Install dp-grpc and dp-common using Maven:
```bash
cd dp-grpc
mvn install
cd ../dp-service
mvn compile
```

3. Create environment config file `~/.dp.env`:
```bash
export DP_HOME=~/data-platform
```

4. Customize configuration in `data-platform/config/dp.yml` with your MongoDB credentials.

---

## Part 2: Data Provider Installation

The Data Provider handles data collection and communication with the Data Platform.

### Clone and Build

```bash
# Clone the repository
git clone <repository-url>
cd OspreyProject_Slac

# Create build and bin directories
mkdir build bin
cd build

# Configure and compile
cmake ..
make -j$(nproc)  # or make -j16 for 16 cores
```

---

## Part 3: Starting the Ecosystem

Once both Data Platform and Data Provider are installed, start the services in this order:

### 1. Start MongoDB

**For local MongoDB:**
```bash
data-platform/bin/mongodb-systemctl-start
data-platform/bin/mongodb-systemctl-status  # Check status
```

**For Docker MongoDB:**
```bash
data-platform/bin/mongodb-docker-create     # First time only
data-platform/bin/mongodb-docker-start
```

### 2. Start Data Platform Services

```bash
# Start ingestion service
data-platform/bin/server-ingest-start
data-platform/bin/server-ingest-status      # Check status

# Start query service  
data-platform/bin/server-query-start
data-platform/bin/server-query-status       # Check status

# Start annotation service
data-platform/bin/server-annotation-start
data-platform/bin/server-annotation-status  # Check status
```

### 3. Run Data Provider

With the Data Platform services running, you can now start your Data Provider application from the `OspreyProject_Slac` directory.

---

## Additional Installation Options

### Installer Package
The easiest way to get started is using the installer package. See the [Quick Start guide](https://github.com/osprey-dcs/data-platform#data-platform-quick-start) for details.

### JAR Installation
For production deployments, download pre-built JAR files from the releases pages:
- [dp-grpc releases](https://github.com/osprey-dcs/dp-grpc/releases)
- [dp-service releases](https://github.com/osprey-dcs/dp-service/releases)

---



