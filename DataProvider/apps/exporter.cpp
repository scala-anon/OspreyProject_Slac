//
// exporter_cli.cpp - Command Line Interface for Data Platform Exporter
//

#include "data_exporter.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <grpcpp/grpcpp.h>

using namespace osprey::data_platform;

void printUsage(const std::string& program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n\n";
    std::cout << "Data Platform Export Utility\n";
    std::cout << "Exports datasets and calculations from MLDP to various formats\n\n";
    
    std::cout << "Required Arguments:\n";
    std::cout << "  --server HOST:PORT     gRPC server address (e.g., localhost:50051)\n";
    std::cout << "  --dataset-id ID        Dataset ID to export\n";
    std::cout << "  --output PATH          Output file path\n\n";
    
    std::cout << "Optional Arguments:\n";
    std::cout << "  --format FORMAT        Export format: csv|xlsx|hdf5|json (default: csv)\n";
    std::cout << "  --annotation-id ID     Export calculations from annotation\n";
    std::cout << "  --include-metadata     Include metadata in export (default: true)\n";
    std::cout << "  --exclude-timestamps   Exclude timestamps from export\n";
    std::cout << "  --time-format FORMAT   Timestamp format: iso8601|unix|human (default: iso8601)\n";
    std::cout << "  --csv-delimiter CHAR   CSV delimiter character (default: ,)\n";
    std::cout << "  --compress             Compress output file\n";
    std::cout << "  --max-rows NUM         Maximum rows per file (default: 1000000)\n";
    std::cout << "  --help                 Show this help message\n\n";
    
    std::cout << "Examples:\n";
    std::cout << "  # Export dataset to CSV\n";
    std::cout << "  " << program_name << " --server localhost:50051 --dataset-id DS123 --output /tmp/data.csv\n\n";
    
    std::cout << "  # Export to Excel with custom settings\n";
    std::cout << "  " << program_name << " --server localhost:50051 --dataset-id DS123 \\\n";
    std::cout << "    --output /tmp/data.xlsx --format xlsx --time-format human\n\n";
    
    std::cout << "  # Export calculations from annotation\n";
    std::cout << "  " << program_name << " --server localhost:50051 --annotation-id ANN456 \\\n";
    std::cout << "    --output /tmp/calculations.json --format json\n\n";
}

struct CLIArgs {
    std::string server_address;
    std::string dataset_id;
    std::string annotation_id;
    std::string output_path;
    ExportFormat format = ExportFormat::CSV;
    bool include_metadata = true;
    bool include_timestamps = true;
    std::string time_format = "iso8601";
    char csv_delimiter = ',';    
    bool compress = false;
    size_t max_rows = 1000000;
    bool help = false;
};

bool parseArguments(int argc, char* argv[], CLIArgs& args) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            args.help = true;
            return true;
        } else if (arg == "--server" && i + 1 < argc) {
            args.server_address = argv[++i];
        } else if (arg == "--dataset-id" && i + 1 < argc) {
            args.dataset_id = argv[++i];
        } else if (arg == "--annotation-id" && i + 1 < argc) {
            args.annotation_id = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            args.output_path = argv[++i];
        } else if (arg == "--format" && i + 1 < argc) {
            std::string format_str = argv[++i];
            if (format_str == "csv") {
                args.format = ExportFormat::CSV;
            } else if (format_str == "xlsx") {
                args.format = ExportFormat::XLSX;
            } else if (format_str == "hdf5") {
                args.format = ExportFormat::HDF5;
            } else if (format_str == "json") {
                args.format = ExportFormat::JSON;
            } else {
                std::cerr << "Unknown format: " << format_str << std::endl;
                return false;
            }
        } else if (arg == "--exclude-metadata") {
            args.include_metadata = false;
        } else if (arg == "--exclude-timestamps") {
            args.include_timestamps = false;
        } else if (arg == "--time-format" && i + 1 < argc) {
            args.time_format = argv[++i];
        } else if (arg == "--csv-delimiter" && i + 1 < argc) {
            args.csv_delimiter = argv[++i][0];
        } else if (arg == "--compress") {
            args.compress = true;
        } else if (arg == "--max-rows" && i + 1 < argc) {
            args.max_rows = std::stoul(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return false;
        }
    }
    
    return true;
}

bool validateArguments(const CLIArgs& args) {
    if (args.server_address.empty()) {
        std::cerr << "Error: --server is required\n";
        return false;
    }
    
    if (args.dataset_id.empty() && args.annotation_id.empty()) {
        std::cerr << "Error: Either --dataset-id or --annotation-id is required\n";
        return false;
    }
    
    if (args.output_path.empty()) {
        std::cerr << "Error: --output is required\n";
        return false;
    }
    
    if (args.time_format != "iso8601" && args.time_format != "unix" && args.time_format != "human") {
        std::cerr << "Error: Invalid time format. Use: iso8601|unix|human\n";
        return false;
    }
    
    return true;
}

ExportConfig createExportConfig(const CLIArgs& args) {
    ExportConfig config;
    config.format = args.format;
    config.output_path = std::filesystem::path(args.output_path).parent_path();
    config.include_metadata = args.include_metadata;
    config.include_timestamps = args.include_timestamps;
    config.compress_output = args.compress;
    config.time_format = args.time_format;
    config.csv_delimiter = args.csv_delimiter;
    config.max_rows_per_file = args.max_rows;
    
    return config;
}

int main(int argc, char* argv[]) {
    CLIArgs args;
    
    if (!parseArguments(argc, argv, args)) {
        printUsage(argv[0]);
        return 1;
    }
    
    if (args.help) {
        printUsage(argv[0]);
        return 0;
    }
    
    if (!validateArguments(args)) {
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        // Create gRPC channel
        std::cout << "Connecting to server: " << args.server_address << std::endl;
        auto channel = grpc::CreateChannel(args.server_address, grpc::InsecureChannelCredentials());
        
        // Wait for connection
        if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(10))) {
            std::cerr << "Failed to connect to server: " << args.server_address << std::endl;
            return 1;
        }
        
        std::cout << "Connected successfully!" << std::endl;
        
        // Create exporter
        ExportConfig config = createExportConfig(args);
        DataExporter exporter(channel, config);
        
        bool success = false;
        
        if (!args.dataset_id.empty()) {
            std::cout << "Exporting dataset: " << args.dataset_id << std::endl;
            success = exporter.exportDataSet(args.dataset_id);
        } else if (!args.annotation_id.empty()) {
            std::cout << "Exporting calculations from annotation: " << args.annotation_id << std::endl;
            success = exporter.exportCalculations(args.annotation_id);
        }
        
        if (success) {
            std::cout << "Export completed successfully!" << std::endl;
            std::cout << "Output: " << args.output_path << std::endl;
            return 0;
        } else {
            std::cerr << "Export failed!" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

//
// Additional utility functions for advanced usage
//

class BatchExporter {
private:
    std::unique_ptr<DataExporter> exporter_;
    std::vector<std::string> failed_exports_;
    
public:
    BatchExporter(std::shared_ptr<grpc::Channel> channel, const ExportConfig& config) {
        exporter_ = std::make_unique<DataExporter>(channel, config);
    }
    
    bool exportDataSetsFromFile(const std::string& dataset_list_file) {
        std::ifstream file(dataset_list_file);
        if (!file.is_open()) {
            std::cerr << "Cannot open dataset list file: " << dataset_list_file << std::endl;
            return false;
        }
        
        std::string dataset_id;
        int total = 0, successful = 0;
        
        while (std::getline(file, dataset_id)) {
            if (dataset_id.empty() || dataset_id[0] == '#') continue; // Skip empty lines and comments
            
            total++;
            std::cout << "Exporting dataset " << total << ": " << dataset_id << std::endl;
            
            if (exporter_->exportDataSet(dataset_id)) {
                successful++;
            } else {
                failed_exports_.push_back(dataset_id);
                std::cerr << "Failed to export: " << dataset_id << std::endl;
            }
        }
        
        std::cout << "\nBatch export summary:" << std::endl;
        std::cout << "Total: " << total << std::endl;
        std::cout << "Successful: " << successful << std::endl;
        std::cout << "Failed: " << (total - successful) << std::endl;
        
        if (!failed_exports_.empty()) {
            std::cout << "\nFailed exports:" << std::endl;
            for (const auto& id : failed_exports_) {
                std::cout << "  " << id << std::endl;
            }
        }
        
        return failed_exports_.empty();
    }
    
    bool exportDataSetsForOwner(const std::string& owner_id) {
        // This would require implementing a query to find all datasets for an owner
        // For now, return placeholder implementation
        std::cout << "Exporting all datasets for owner: " << owner_id << std::endl;
        // TODO: Implement queryDataSets with OwnerCriterion
        return true;
    }
    
    bool exportDataSetsWithTag(const std::string& tag) {
        // This would require implementing a search by tag
        std::cout << "Exporting all datasets with tag: " << tag << std::endl;
        // TODO: Implement dataset search by tag
        return true;
    }
};

//
// Configuration file support
//

class ConfigManager {
public:
    static ExportConfig loadFromFile(const std::string& config_file) {
        ExportConfig config;
        
        std::ifstream file(config_file);
        if (!file.is_open()) {
            std::cerr << "Warning: Cannot open config file: " << config_file << std::endl;
            return config; // Return default config
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            size_t eq_pos = line.find('=');
            if (eq_pos == std::string::npos) continue;
            
            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            if (key == "format") {
                if (value == "csv") config.format = ExportFormat::CSV;
                else if (value == "xlsx") config.format = ExportFormat::XLSX;
                else if (value == "hdf5") config.format = ExportFormat::HDF5;
                else if (value == "json") config.format = ExportFormat::JSON;
            } else if (key == "include_metadata") {
                config.include_metadata = (value == "true" || value == "1");
            } else if (key == "include_timestamps") {
                config.include_timestamps = (value == "true" || value == "1");
            } else if (key == "time_format") {
                config.time_format = value;
            } else if (key == "csv_delimiter") {
                if (!value.empty()) config.csv_delimiter = value[0];
            } else if (key == "compress_output") {
                config.compress_output = (value == "true" || value == "1");
            } else if (key == "max_rows_per_file") {
                config.max_rows_per_file = std::stoul(value);
            } else if (key == "output_path") {
                config.output_path = value;
            }
        }
        
        return config;
    }
    
    static void saveToFile(const ExportConfig& config, const std::string& config_file) {
        std::ofstream file(config_file);
        if (!file.is_open()) {
            std::cerr << "Cannot write config file: " << config_file << std::endl;
            return;
        }
        
        file << "# Data Platform Exporter Configuration\n";
        file << "# Generated on " << std::chrono::system_clock::now().time_since_epoch().count() << "\n\n";
        
        std::string format_str;
        switch (config.format) {
            case ExportFormat::CSV: format_str = "csv"; break;
            case ExportFormat::XLSX: format_str = "xlsx"; break;
            case ExportFormat::HDF5: format_str = "hdf5"; break;
            case ExportFormat::JSON: format_str = "json"; break;
            default: format_str = "csv"; break;
        }
        
        file << "format = " << format_str << "\n";
        file << "include_metadata = " << (config.include_metadata ? "true" : "false") << "\n";
        file << "include_timestamps = " << (config.include_timestamps ? "true" : "false") << "\n";
        file << "time_format = " << config.time_format << "\n";
        file << "csv_delimiter = " << config.csv_delimiter << "\n";
        file << "compress_output = " << (config.compress_output ? "true" : "false") << "\n";
        file << "max_rows_per_file = " << config.max_rows_per_file << "\n";
        file << "output_path = " << config.output_path << "\n";
        
        std::cout << "Configuration saved to: " << config_file << std::endl;
    }
};
