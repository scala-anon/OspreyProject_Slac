#!/bin/bash

# build_dp.sh - DataProvider build script with organized structure
# Usage: ./build_dp.sh [clean|rebuild|install|test|help]

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
BIN_DIR="$PROJECT_ROOT/bin"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo_header() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

# Function to check prerequisites
check_prerequisites() {
    echo_info "Checking prerequisites..."
    
    # Check for required tools
    local missing_tools=()
    
    if ! command -v cmake &> /dev/null; then
        missing_tools+=("cmake")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_tools+=("make")
    fi
    
    if ! command -v protoc &> /dev/null; then
        missing_tools+=("protobuf-compiler")
    fi
    
    if [ ${#missing_tools[@]} -gt 0 ]; then
        echo_error "Missing required tools: ${missing_tools[*]}"
        echo_info "Install with: sudo apt install cmake build-essential protobuf-compiler libprotobuf-dev libgrpc++-dev"
        exit 1
    fi
    
    # Check for HDF5
    if ! pkg-config --exists hdf5 2>/dev/null && [ ! -f "/usr/include/hdf5.h" ]; then
        echo_warn "HDF5 development files may not be installed"
        echo_info "Install with: sudo apt install libhdf5-dev"
    fi
    
    echo_info "Prerequisites check passed"
}

# Function to clean build artifacts
clean_build() {
    echo_header "Cleaning Build Artifacts"
    
    if [ -d "$BUILD_DIR" ]; then
        echo_info "Removing build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi
    
    if [ -d "$BIN_DIR" ]; then
        echo_info "Removing bin directory: $BIN_DIR"
        rm -rf "$BIN_DIR"
    fi
    
    # Clean any stray build files
    find "$PROJECT_ROOT" -name "*.pb.cc" -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "*.pb.h" -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "*.grpc.pb.cc" -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "*.grpc.pb.h" -delete 2>/dev/null || true
    
    echo_info "Clean complete"
}

# Function to setup directories
setup_directories() {
    echo_info "Setting up directories..."
    
    mkdir -p "$BUILD_DIR"
    mkdir -p "$BIN_DIR"
    mkdir -p "$PROJECT_ROOT/config"
    mkdir -p "$PROJECT_ROOT/scripts"
    
    # Create .gitkeep files to maintain directory structure
    touch "$BIN_DIR/.gitkeep"
    touch "$PROJECT_ROOT/config/.gitkeep"
    
    echo_info "Directory structure ready"
}

# Function to configure CMake
configure_cmake() {
    echo_header "Configuring CMake"
    
    cd "$BUILD_DIR"
    
    local cmake_args=(
        -S "$PROJECT_ROOT"
        -B .
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        -DCMAKE_VERBOSE_MAKEFILE=OFF
    )
    
    echo_info "Running: cmake ${cmake_args[*]}"
    cmake "${cmake_args[@]}"
    
    if [ $? -eq 0 ]; then
        echo_info "CMake configuration successful"
    else
        echo_error "CMake configuration failed"
        exit 1
    fi
}

# Function to build the project
build_project() {
    echo_header "Building Project"
    
    cd "$BUILD_DIR"
    
    local num_cores=$(nproc)
    echo_info "Building with $num_cores parallel jobs..."
    
    make -j"$num_cores"
    
    if [ $? -eq 0 ]; then
        echo_info "Build successful"
    else
        echo_error "Build failed"
        exit 1
    fi
}

# Function to run post-build fixes
post_build_fix() {
    echo_header "Post-Build Setup"
    
    # Ensure bin directory exists
    mkdir -p "$BIN_DIR"
    
    # Make all executables properly executable
    if [ -d "$BIN_DIR" ]; then
        echo_info "Setting executable permissions..."
        chmod +x "$BIN_DIR"/* 2>/dev/null || true
        
        # Make scripts executable too
        if [ -d "$PROJECT_ROOT/scripts" ]; then
            for script in "$PROJECT_ROOT"/scripts/*; do
                if [ -f "$script" ]; then
                    script_name=$(basename "$script")
                    if [ -f "$BIN_DIR/$script_name" ]; then
                        chmod +x "$BIN_DIR/$script_name"
                    fi
                fi
            done
        fi
    fi
}

# Function to run tests if available
run_tests() {
    echo_header "Running Tests"
    
    if [ -f "$BUILD_DIR/Makefile" ]; then
        cd "$BUILD_DIR"
        if make help | grep -q "test"; then
            echo_info "Running tests..."
            make test
        else
            echo_warn "No tests configured"
        fi
    else
        echo_warn "No build system found for testing"
    fi
}

# Function to show build results
show_results() {
    echo_header "Build Results"
    
    if [ -d "$BIN_DIR" ]; then
        echo_info "Executables in bin directory:"
        if ls "$BIN_DIR"/* &>/dev/null; then
            ls -la "$BIN_DIR"/
            echo ""
            
            # Show file sizes and types
            echo_info "Executable details:"
            for exe in "$BIN_DIR"/*; do
                if [ -f "$exe" ] && [ -x "$exe" ]; then
                    local size=$(du -h "$exe" | cut -f1)
                    local name=$(basename "$exe")
                    echo "  $name: $size"
                fi
            done
        else
            echo_warn "No files found in bin directory"
        fi
    else
        echo_warn "Bin directory not found"
    fi
    
    echo ""
    echo_info "Usage examples:"
    echo "  Run main CLI:           ./bin/DataProvider_CLI --help"
    echo "  H5 to DP converter:     ./bin/h5_to_dp /path/to/h5/files"
    echo "  MongoDB to NumPy:       ./bin/mongo_to_npy --help"
    echo "  Data decoder:           ./bin/data_decoder --help"
}

# Function to install system-wide
install_system() {
    echo_header "Installing System-Wide"
    
    cd "$BUILD_DIR"
    
    echo_info "Installing to system directories (requires sudo)..."
    sudo make install
    
    echo_info "Installation complete"
    echo_info "Executables should now be available in /usr/local/bin/"
}

# Function to validate build
validate_build() {
    echo_info "Validating build..."
    
    local expected_executables=("DataProvider_CLI" "h5_to_dp" "mongo_to_npy" "data_decoder")
    local missing_executables=()
    
    for exe in "${expected_executables[@]}"; do
        if [ ! -f "$BIN_DIR/$exe" ]; then
            missing_executables+=("$exe")
        elif [ ! -x "$BIN_DIR/$exe" ]; then
            echo_warn "$exe exists but is not executable"
            chmod +x "$BIN_DIR/$exe"
        fi
    done
    
    if [ ${#missing_executables[@]} -gt 0 ]; then
        echo_warn "Missing executables: ${missing_executables[*]}"
        return 1
    else
        echo_info "All expected executables are present"
        return 0
    fi
}

# Main build function
main_build() {
    check_prerequisites
    setup_directories
    configure_cmake
    build_project
    post_build_fix
    if validate_build; then
        show_results
    else
        echo_error "Build validation failed"
        exit 1
    fi
}

# Handle command line arguments
case "${1:-build}" in
    "clean")
        clean_build
        ;;
    "rebuild")
        clean_build
        main_build
        ;;
    "build"|"")
        main_build
        ;;
    "test")
        main_build
        run_tests
        ;;
    "install")
        main_build
        install_system
        ;;
    "validate")
        validate_build
        ;;
    "help"|"-h"|"--help")
        echo "DataProvider Build Script"
        echo "Usage: $0 [command]"
        echo ""
        echo "Commands:"
        echo "  build     - Build the project (default)"
        echo "  clean     - Clean build and bin directories"
        echo "  rebuild   - Clean and build"
        echo "  test      - Build and run tests"
        echo "  install   - Build and install system-wide"
        echo "  validate  - Validate existing build"
        echo "  help      - Show this help message"
        echo ""
        echo "Output:"
        echo "  All executables are built to: ./bin/"
        echo "  Main CLI: ./bin/DataProvider_CLI"
        ;;
    *)
        echo_error "Unknown command: $1"
        echo "Use '$0 help' for usage information"
        exit 1
        ;;
esac

echo_info "Done!"
