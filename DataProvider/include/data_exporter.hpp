//
// data_exporter.hpp - Comprehensive Data Export Utility for MLDP
//
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <unordered_map>
#include <chrono>

// gRPC and protobuf includes
#include <grpcpp/grpcpp.h>
#include "annotation.grpc.pb.h"
#include "query.grpc.pb.h"
#include "common.pb.h"

// HDF5 includes for native format support
#include <H5Cpp.h>

#ifdef HAVE_XLSXWRITER
#include <xlsxwriter.h>
#endif

#ifdef HAVE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace osprey {
namespace data_platform {

enum class ExportFormat {
    CSV,
#ifdef HAVE_XLSXWRITER
    XLSX,
#endif
    HDF5,
#ifdef HAVE_NLOHMANN_JSON
    JSON,
#endif
    PARQUET
};

struct ExportConfig {
    ExportFormat format = ExportFormat::CSV;
    std::string output_path;
    bool include_metadata = true;
    bool include_timestamps = true;
    bool compress_output = false;
    std::string time_format = "ISO8601"; // ISO8601, UNIX, HUMAN
    char csv_delimiter = ',';
    bool csv_header = true;
    size_t max_rows_per_file = 1000000; // Split large exports
};

struct DataSetInfo {
    std::string id;
    std::string name;
    std::string owner;
    std::string description;
    std::vector<std::string> tags;
    std::vector<Attribute> attributes;
    std::vector<dp::service::annotation::DataBlock> data_blocks;
};

struct CalculationsInfo {
    std::string id;
    std::vector<dp::service::annotation::Calculations::CalculationsDataFrame> data_frames;
};

class DataExporter {
private:
    std::unique_ptr<dp::service::annotation::DpAnnotationService::Stub> annotation_stub_;
    std::unique_ptr<dp::service::query::DpQueryService::Stub> query_stub_;
    ExportConfig config_;
    
public:
    DataExporter(std::shared_ptr<grpc::Channel> channel, const ExportConfig& config);
    
    // Main export methods
    bool exportDataSet(const std::string& dataset_id);
    bool exportCalculations(const std::string& annotation_id);
    bool exportQueryResult(const dp::service::query::QueryDataResponse& query_result, 
                          const std::string& output_name);
    
    // Batch export methods
    bool exportMultipleDataSets(const std::vector<std::string>& dataset_ids);
    bool exportAnnotationData(const std::string& annotation_id);
    
    // Format-specific export methods
    bool exportToCSV(const DataSetInfo& dataset, const std::string& output_path);
#ifdef HAVE_XLSXWRITER
    bool exportToXLSX(const DataSetInfo& dataset, const std::string& output_path);
#endif
    bool exportToHDF5(const DataSetInfo& dataset, const std::string& output_path);
#ifdef HAVE_NLOHMANN_JSON
    bool exportToJSON(const DataSetInfo& dataset, const std::string& output_path);
#endif
    
    // Utility methods
    void setConfig(const ExportConfig& config) { config_ = config; }
    std::vector<std::string> getAvailableFormats() const;
    bool validateExportPath(const std::string& path) const;
    
private:
    // Helper methods
    DataSetInfo fetchDataSetInfo(const std::string& dataset_id);
    CalculationsInfo fetchCalculationsInfo(const std::string& annotation_id);
    
    std::vector<dp::service::query::QueryDataResponse> queryDataSetData(const DataSetInfo& dataset);
    std::string formatTimestamp(const Timestamp& timestamp) const;
    std::string sanitizeFilename(const std::string& name) const;
    
    // Format-specific helpers
    bool writeCSVHeader(std::ofstream& file, const std::vector<std::string>& pv_names) const;
    bool writeCSVData(std::ofstream& file, const dp::service::query::QueryDataResponse& data) const;
    
#ifdef HAVE_XLSXWRITER
    bool createXLSXWorkbook(const std::string& path, const DataSetInfo& dataset);
    bool addXLSXWorksheet(lxw_workbook* workbook, const std::string& sheet_name, 
                         const dp::service::query::QueryDataResponse& data);
#endif
    
    bool createHDF5File(const std::string& path, const DataSetInfo& dataset);
    bool addHDF5Dataset(H5::H5File& file, const std::string& dataset_name,
                       const dp::service::query::QueryDataResponse& data);
    
#ifdef HAVE_NLOHMANN_JSON
    nlohmann::json convertToJSON(const DataSetInfo& dataset);
    nlohmann::json convertDataToJSON(const dp::service::query::QueryDataResponse& data);
#endif
};

//
// data_exporter.cpp - Implementation
//

DataExporter::DataExporter(std::shared_ptr<grpc::Channel> channel, const ExportConfig& config)
    : config_(config) {
    annotation_stub_ = dp::service::annotation::DpAnnotationService::NewStub(channel);
    query_stub_ = dp::service::query::DpQueryService::NewStub(channel);
}

bool DataExporter::exportDataSet(const std::string& dataset_id) {
    try {
        // Fetch dataset information
        DataSetInfo dataset = fetchDataSetInfo(dataset_id);
        if (dataset.id.empty()) {
            std::cerr << "Failed to fetch dataset info for: " << dataset_id << std::endl;
            return false;
        }
        
        // Generate output filename
        std::string filename = sanitizeFilename(dataset.name + "_" + dataset_id);
        std::string output_path = config_.output_path + "/" + filename;
        
        // Export based on format
        switch (config_.format) {
            case ExportFormat::CSV:
                return exportToCSV(dataset, output_path + ".csv");
#ifdef HAVE_XLSXWRITER
            case ExportFormat::XLSX:
                return exportToXLSX(dataset, output_path + ".xlsx");
#endif
            case ExportFormat::HDF5:
                return exportToHDF5(dataset, output_path + ".h5");
#ifdef HAVE_NLOHMANN_JSON
            case ExportFormat::JSON:
                return exportToJSON(dataset, output_path + ".json");
#endif
            default:
                std::cerr << "Unsupported export format" << std::endl;
                return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Export failed: " << e.what() << std::endl;
        return false;
    }
}

bool DataExporter::exportToCSV(const DataSetInfo& dataset, const std::string& output_path) {
    std::ofstream file(output_path);
    if (!file.is_open()) {
        std::cerr << "Cannot open output file: " << output_path << std::endl;
        return false;
    }
    
    // Query data for each data block
    auto query_results = queryDataSetData(dataset);
    if (query_results.empty()) {
        std::cerr << "No data found for dataset: " << dataset.id << std::endl;
        return false;
    }
    
    // Write metadata header if requested
    if (config_.include_metadata) {
        file << "# Dataset: " << dataset.name << "\n";
        file << "# Owner: " << dataset.owner << "\n";
        file << "# Description: " << dataset.description << "\n";
        file << "# Export Time: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n";
        file << "#\n";
    }
    
    // Collect all PV names
    std::vector<std::string> all_pv_names;
    for (const auto& block : dataset.data_blocks) {
        for (const auto& pv : block.pvnames()) {
            if (std::find(all_pv_names.begin(), all_pv_names.end(), pv) == all_pv_names.end()) {
                all_pv_names.push_back(pv);
            }
        }
    }
    
    // Write CSV header
    if (config_.csv_header) {
        writeCSVHeader(file, all_pv_names);
    }
    
    // Write data
    for (const auto& result : query_results) {
        writeCSVData(file, result);
    }
    
    file.close();
    std::cout << "CSV export completed: " << output_path << std::endl;
    return true;
}

#ifdef HAVE_XLSXWRITER
bool DataExporter::exportToXLSX(const DataSetInfo& dataset, const std::string& output_path) {
    // Create new workbook
    lxw_workbook* workbook = workbook_new(output_path.c_str());
    if (!workbook) {
        std::cerr << "Cannot create XLSX file: " << output_path << std::endl;
        return false;
    }
    
    try {
        // Add metadata worksheet
        if (config_.include_metadata) {
            lxw_worksheet* metadata_sheet = workbook_add_worksheet(workbook, "Metadata");
            worksheet_write_string(metadata_sheet, 0, 0, "Dataset Name", nullptr);
            worksheet_write_string(metadata_sheet, 0, 1, dataset.name.c_str(), nullptr);
            worksheet_write_string(metadata_sheet, 1, 0, "Owner", nullptr);
            worksheet_write_string(metadata_sheet, 1, 1, dataset.owner.c_str(), nullptr);
            worksheet_write_string(metadata_sheet, 2, 0, "Description", nullptr);
            worksheet_write_string(metadata_sheet, 2, 1, dataset.description.c_str(), nullptr);
        }
        
        // Query data and create worksheets
        auto query_results = queryDataSetData(dataset);
        
        int sheet_num = 1;
        for (const auto& result : query_results) {
            std::string sheet_name = "Data_" + std::to_string(sheet_num++);
            addXLSXWorksheet(workbook, sheet_name, result);
        }
        
        workbook_close(workbook);
        std::cout << "XLSX export completed: " << output_path << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        workbook_close(workbook);
        std::cerr << "XLSX export failed: " << e.what() << std::endl;
        return false;
    }
}
#endif

bool DataExporter::exportToHDF5(const DataSetInfo& dataset, const std::string& output_path) {
    try {
        H5::H5File file(output_path, H5F_ACC_TRUNC);
        
        // Add metadata attributes
        if (config_.include_metadata) {
            H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);
            H5::StrType strdatatype(H5::PredType::C_S1, dataset.name.size());
            
            H5::Attribute attr_name = file.createAttribute("dataset_name", strdatatype, attr_dataspace);
            attr_name.write(strdatatype, dataset.name);
            
            strdatatype = H5::StrType(H5::PredType::C_S1, dataset.owner.size());
            H5::Attribute attr_owner = file.createAttribute("owner", strdatatype, attr_dataspace);
            attr_owner.write(strdatatype, dataset.owner);
        }
        
        // Query and store data
        auto query_results = queryDataSetData(dataset);
        
        int dataset_num = 1;
        for (const auto& result : query_results) {
            std::string dataset_name = "dataset_" + std::to_string(dataset_num++);
            addHDF5Dataset(file, dataset_name, result);
        }
        
        file.close();
        std::cout << "HDF5 export completed: " << output_path << std::endl;
        return true;
        
    } catch (const H5::Exception& e) {
        std::cerr << "HDF5 export failed: " << e.getDetailMsg() << std::endl;
        return false;
    }
}

#ifdef HAVE_NLOHMANN_JSON
bool DataExporter::exportToJSON(const DataSetInfo& dataset, const std::string& output_path) {
    try {
        nlohmann::json export_data = convertToJSON(dataset);
        
        std::ofstream file(output_path);
        if (!file.is_open()) {
            std::cerr << "Cannot open JSON file: " << output_path << std::endl;
            return false;
        }
        
        file << export_data.dump(2); // Pretty print with 2-space indentation
        file.close();
        
        std::cout << "JSON export completed: " << output_path << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "JSON export failed: " << e.what() << std::endl;
        return false;
    }
}
#endif

// Helper method implementations
DataSetInfo DataExporter::fetchDataSetInfo(const std::string& dataset_id) {
    dp::service::annotation::QueryDataSetsRequest request;
    auto* criterion = request.add_criteria();
    criterion->mutable_idcriterion()->set_id(dataset_id);
    
    grpc::ClientContext context;
    dp::service::annotation::QueryDataSetsResponse response;
    
    grpc::Status status = annotation_stub_->queryDataSets(&context, request, &response);
    
    DataSetInfo info;
    if (status.ok() && response.has_datasetsresult() && 
        response.datasetsresult().datasets_size() > 0) {
        
        const auto& dataset = response.datasetsresult().datasets(0);
        info.id = dataset.id();
        info.name = dataset.name();
        info.owner = dataset.ownerid();
        info.description = dataset.description();
        
        for (const auto& block : dataset.datablocks()) {
            info.data_blocks.push_back(block);
        }
    }
    
    return info;
}

std::vector<dp::service::query::QueryDataResponse> DataExporter::queryDataSetData(
    const DataSetInfo& dataset) {
    
    std::vector<dp::service::query::QueryDataResponse> results;
    
    for (const auto& block : dataset.data_blocks) {
        dp::service::query::QueryDataRequest request;
        auto* query_spec = request.mutable_queryspec();
        
        query_spec->mutable_begintime()->CopyFrom(block.begintime());
        query_spec->mutable_endtime()->CopyFrom(block.endtime());
        
        for (const auto& pv_name : block.pvnames()) {
            query_spec->add_pvnames(pv_name);
        }
        
        query_spec->set_useserializeddatacolumns(false);
        
        grpc::ClientContext context;
        dp::service::query::QueryDataResponse response;
        
        grpc::Status status = query_stub_->queryData(&context, request, &response);
        
        if (status.ok() && response.has_querydata()) {
            results.push_back(response);
        }
    }
    
    return results;
}

bool DataExporter::writeCSVHeader(std::ofstream& file, const std::vector<std::string>& pv_names) const {
    if (config_.include_timestamps) {
        file << "timestamp" << config_.csv_delimiter;
    }
    
    for (size_t i = 0; i < pv_names.size(); ++i) {
        file << pv_names[i];
        if (i < pv_names.size() - 1) {
            file << config_.csv_delimiter;
        }
    }
    file << "\n";
    return true;
}

bool DataExporter::writeCSVData(std::ofstream& file, const dp::service::query::QueryDataResponse& data) const {
    if (!data.has_querydata()) return false;
    
    const auto& query_data = data.querydata();
    
    for (const auto& bucket : query_data.databuckets()) {
        // Handle timestamps
        std::vector<std::string> timestamps;
        if (bucket.has_datatimestamps()) {
            if (bucket.datatimestamps().has_timestamplist()) {
                for (const auto& ts : bucket.datatimestamps().timestamplist().timestamps()) {
                    timestamps.push_back(formatTimestamp(ts));
                }
            } else if (bucket.datatimestamps().has_samplingclock()) {
                // Generate timestamps from sampling clock
                const auto& clock = bucket.datatimestamps().samplingclock();
                uint64_t start_time = clock.starttime().nanoseconds();
                uint64_t period = clock.periodnanos();
                uint32_t count = clock.count();
                
                for (uint32_t i = 0; i < count; ++i) {
                    Timestamp ts;
                    uint64_t time_ns = start_time + i * period;
                    ts.set_nanoseconds(time_ns);
                    timestamps.push_back(formatTimestamp(ts));
                }
            }
        }
        
        // Write data values
        if (bucket.has_datacolumn()) {
            const auto& column = bucket.datacolumn();
            for (int i = 0; i < column.datavalues_size(); ++i) {
                if (config_.include_timestamps && i < timestamps.size()) {
                    file << timestamps[i] << config_.csv_delimiter;
                }
                
                const auto& value = column.datavalues(i);
                if (value.value_case() == DataValue::kDoubleValue) {
                    file << std::fixed << std::setprecision(6) << value.doublevalue();
                } else if (value.value_case() == DataValue::kLongValue) {
                    file << value.longvalue();
                } else if (value.value_case() == DataValue::kStringValue) {
                    file << "\"" << value.stringvalue() << "\"";
                } else {
                    file << "null";
                }
                
                file << "\n";
            }
        }
    }
    
    return true;
}

std::string DataExporter::formatTimestamp(const Timestamp& timestamp) const {
    if (config_.time_format == "UNIX") {
        return std::to_string(timestamp.nanoseconds() / 1000000000) + "." + 
               std::to_string((timestamp.nanoseconds() % 1000000000) / 1000000);
    } else if (config_.time_format == "ISO8601") {
        std::time_t time_t_val = timestamp.nanoseconds() / 1000000000;
        std::tm* tm_val = std::gmtime(&time_t_val);
        
        std::ostringstream oss;
        oss << std::put_time(tm_val, "%Y-%m-%dT%H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(9) << (timestamp.nanoseconds() % 1000000000) << "Z";
        return oss.str();
    } else {
        // HUMAN readable format
        std::time_t time_t_val = timestamp.nanoseconds() / 1000000000;
        std::tm* tm_val = std::localtime(&time_t_val);
        
        std::ostringstream oss;
        oss << std::put_time(tm_val, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
}

std::string DataExporter::sanitizeFilename(const std::string& name) const {
    std::string result = name;
    std::replace_if(result.begin(), result.end(), 
                   [](char c) { return !std::isalnum(c) && c != '_' && c != '-'; }, '_');
    return result;
}

#ifdef HAVE_NLOHMANN_JSON
nlohmann::json DataExporter::convertToJSON(const DataSetInfo& dataset) {
    nlohmann::json result;
    
    // Metadata
    result["metadata"]["id"] = dataset.id;
    result["metadata"]["name"] = dataset.name;
    result["metadata"]["owner"] = dataset.owner;
    result["metadata"]["description"] = dataset.description;
    result["metadata"]["export_time"] = std::chrono::system_clock::now().time_since_epoch().count();
    
    // Data blocks
    result["data_blocks"] = nlohmann::json::array();
    
    auto query_results = queryDataSetData(dataset);
    for (size_t i = 0; i < query_results.size() && i < dataset.data_blocks.size(); ++i) {
        nlohmann::json block_data = convertDataToJSON(query_results[i]);
        
        block_data["time_range"]["start"] = formatTimestamp(dataset.data_blocks[i].begintime());
        block_data["time_range"]["end"] = formatTimestamp(dataset.data_blocks[i].endtime());
        
        block_data["pv_names"] = nlohmann::json::array();
        for (const auto& pv : dataset.data_blocks[i].pvnames()) {
            block_data["pv_names"].push_back(pv);
        }
        
        result["data_blocks"].push_back(block_data);
    }
    
    return result;
}

nlohmann::json DataExporter::convertDataToJSON(const dp::service::query::QueryDataResponse& data) {
    nlohmann::json result;
    
    if (!data.has_querydata()) {
        return result;
    }
    
    result["buckets"] = nlohmann::json::array();
    
    const auto& query_data = data.querydata();
    for (const auto& bucket : query_data.databuckets()) {
        nlohmann::json bucket_json;
        
        // Timestamps
        if (bucket.has_datatimestamps()) {
            bucket_json["timestamps"] = nlohmann::json::array();
            
            if (bucket.datatimestamps().has_timestamplist()) {
                for (const auto& ts : bucket.datatimestamps().timestamplist().timestamps()) {
                    bucket_json["timestamps"].push_back(formatTimestamp(ts));
                }
            }
        }
        
        // Data values
        if (bucket.has_datacolumn()) {
            bucket_json["values"] = nlohmann::json::array();
            
            for (const auto& value : bucket.datacolumn().datavalues()) {
                nlohmann::json value_json;
                
                if (value.value_case() == DataValue::kDoubleValue) {
                    value_json = value.doublevalue();
                } else if (value.value_case() == DataValue::kLongValue) {
                    value_json = value.longvalue();
                } else if (value.value_case() == DataValue::kStringValue) {
                    value_json = value.stringvalue();
                } else {
                    value_json = nullptr;
                }
                
                bucket_json["values"].push_back(value_json);
            }
        }
        
        result["buckets"].push_back(bucket_json);
    }
    
    return result;
}
#endif


// Missing method implementations
#ifdef HAVE_XLSXWRITER
bool DataExporter::addXLSXWorksheet(lxw_workbook* workbook, const std::string& sheet_name,
                                   const dp::service::query::QueryDataResponse& data) {
    std::cerr << "addXLSXWorksheet not yet implemented" << std::endl;
    return false;
}
#endif

bool DataExporter::addHDF5Dataset(H5::H5File& file, const std::string& dataset_name,
                                 const dp::service::query::QueryDataResponse& data) {
    std::cerr << "addHDF5Dataset not yet implemented" << std::endl;
    return false;
}

bool DataExporter::exportCalculations(const std::string& annotation_id) {
    std::cerr << "exportCalculations not yet implemented" << std::endl;
    return false;
}
} // namespace data_platform
} // namespace osprey
