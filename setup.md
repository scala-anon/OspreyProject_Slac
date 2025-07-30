# Setup for Data platform and Data Provider

## Prerequisites 
**Core Requirements:**
- **Java**: OpenJDK or Oracle JDK 21
- **MongoDB**: Version 7.0.5 (local installation or Docker container)
- **Maven**: Version 3.6+ (for building and dependency management)
- **Git**: For cloning repositories during development installation

**Development Environment:**
- **Java IDE**: IntelliJ IDEA, Eclipse, or VS Code with Java extensions
- **Operating System**: Cross-platform (Windows, macOS, Linux distributions)

**Optional Components:**
- **Docker**: Version 20.10+ (if using containerized MongoDB)

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

# Data platform

## Data platform installation prerequisites

The primary prerequisites for installing the Data Platform are Java and MongoDB.  Mongodb-compass is a GUI for navigating MongoDB databases, and can be extremely useful during development and testing.  Its installation is optional.

### java installation

The Data Platform Java applications are compiled using Java 21.  Here is a link for [downloading and installing Java 21](https://www.oracle.com/java/technologies/downloads/#java21).

### mongodb installation

MongoDB version 7.0.5 is the current reference version for the Data Platform service implementations.  It may be installed as a local package, or via a Docker container.  Each approach is described in more detail below.

#### mongodb installation as local package

Installation will vary by platform and instructions for doing so should be fairly easy to find.  MongoDB provides [documentation for installing on a variety of platforms](https://www.mongodb.com/docs/manual/administration/install-community/), which is a good place to start.

After installing MongoDB, create a user for the data platform applications.  The following example creates an "admin" user (password="admin") with root privileges in the "admin" database.  You can also create a user with privileges scoped only to the "dp" database in MongoDB.

1. start the MongoDB shell
```
mongosh
```
2. switch to "admin" database
```
use admin 
```
3. create admin user with root role (change password, this example uses "admin"!)
```
db.createUser({user: "admin", pwd: "admin", roles: [ { role: "root", db: "admin" } ]})
```
4. exit shell
```
exit
```

#### mongodb installation as docker container

It is also possible (and relatively simple) to run MongoDB from a docker container.  While probably not appropriate for a production installation or system under heavy load, this approach might be useful for development, evaluation, and other applications.  The Quick Start section above demonstrates the steps for creating and running MongoDB via a Docker container.  See the [dp-support repo](https://github.com/osprey-dcs/dp-support) for more details about the scripts.

## Data platform installation options

There are three main options for installing the Data Platform.

1. This data-platform repo contains an installer with everything needed to run the Data Platform and is the easiest way to get started.  See the [Quick Start](https://github.com/osprey-dcs/data-platform#data-platform-quick-start) for details on downloading and using the installer.

2. To learn more about installing the Data Platform in your development environment, see the instructions for [development installation](./installation.md#development-installation).

3. The [jar installation section](./installation.md#jar-installation) describes the process for downloading the latest Data Platform jar files.

### development installation

Developer installation consists of cloning the [dp-grpc](https://github.com/osprey-dcs/dp-grpc) and [dp-service](https://github.com/osprey-dcs/dp-service) repos, and adding a project for each to the IDE. The dp-support repo is optional, and the dp-benchmark is probably not useful unless you are interested in performance benchmarks outside the Data Platform).  After cloning the repos, use maven to "install" the dp-grpc and dp-common projects (either from the command line or using your Java IDE).  Then use maven to compile the dp-service project.

To run the ingestion service, execute IngestionGrpcServer.main().  To run the performance benchmark (with the server running), execute BenchmarkStreamingIngestion.main().

There are jUnit tests for the elements of the Data Platform services in dp-service/src/test/java.

### jar installation

In situations where the Data Platform code will be used without the ecosystem support provided by the dp-support repo or for Java development, source code and/or jar files can be installed directly by using the desired github release.  Here are links to the releases page for each repo: [dp-grpc](https://github.com/osprey-dcs/dp-grpc/releases) and [dp-service](https://github.com/osprey-dcs/dp-service/releases).

## Create data platform environment config file

The dp-support scripts require an environment configuration file in the user's home directory that specifies the location of the data platform installation.  The file must be called ".dp.env" (note the leading "dot" character).  The file contents should look like this (for an installation in the user's home directory):

```
export DP_HOME=~/data-platform
```

If the data-platform installer was extracted in a different location, use the appropriate installation path for DP_HOME.

## Customize config files

The "data-platform/config" directory includes template config files for the installation.  Minimally, you'll need to edit "dp.yml" to include the proper "dbUser" and "dbPassword" for your MongoDB installation.  The included log4j config file sets up logging output to the console and can be customized as desired.

## Start ecosystem processes

The "data-platform/bin" directory includes a set of scripts for managing the data platform ecosystem.  These can be used to quickly get the system up and running.  Below are some of the basics for getting the ecosystem up and running, including starting the MongoDB database, running the Data Platform services, and running the performance benchmarks.  See the [dp-support repo](https://github.com/osprey-dcs/dp-support) for additional details about the available scripts.

### manage local mongodb

Assuming you've installed MongoDB as a local package, use the following scripts to start the database and check status via systemctl.

#### start local mongodb
```
data-platform/bin/mongodb-systemctl-start
```

#### check status of local mongodb
```
data-platform/bin/mongodb-systemctl-status
```

### manage docker mongodb

If you choose to run MongoDB via a Docker container, use the following commands to create the container, start it, and run a database shell (mongosh) against it.

#### create mongodb docker container
```
data-platform/bin/mongodb-docker-create
```

#### start mongodb docker container
```
data-platform/bin/mongodb-docker-start
```

#### create mongodb docker container
```
data-platform/bin/mongodb-docker-create
```

#### run mongosh against mongodb docker container
```
data-platform/bin/mongodb-docker-shell
```

### run data platform services

Use the following commands to start the standard Ingestion and Query Services.

#### start ingestion service
```
data-platform/bin/server-ingest-start
```

#### check ingestion service status
```
data-platform/bin/server-ingest-status
```

#### start query service
```
data-platform/bin/server-query-start
```

#### check query service status
```
data-platform/bin/server-query-status
```

#### start annotation service
```
data-platform/bin/server-annotation-start
```

#### check annotation service status
```
data-platform/bin/server-annotation-status
```
