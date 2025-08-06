#include "annotation_client.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <ctime>

struct Config {
    std::string server = "localhost:50053";
    std::string operation;
    std::string name;
    std::string owner;
    std::string description;
    std::string comment;
    std::string pv;
    std::string pvs;
    std::string pattern;
    std::string date;
    std::string time;
    std::string end_date;
    std::string end_time;
    std::string dataset_id;
    std::string annotation_id;
    std::string tags;
    std::string search;
    std::string format = "table";
    std::string export_format = "csv";
    bool verbose = false;
};

uint64_t parseDateTime(const std::string& date_str, const std::string& time_str = "000000") {
    if (date_str.length() != 8 || time_str.length() != 6) {
        throw std::runtime_error("Invalid date/time format. Use date=MMDDYYYY time=HHMMSS");
    }
    
    std::string mm = date_str.substr(0, 2);
    std::string dd = date_str.substr(2, 2);
    std::string yyyy = date_str.substr(4, 4);
    std::string hh = time_str.substr(0, 2);
    std::string min = time_str.substr(2, 2);
    std::string ss = time_str.substr(4, 2);
    
    std::tm tm = {};
    tm.tm_year = std::stoi(yyyy) - 1900;
    tm.tm_mon = std::stoi(mm) - 1;
    tm.tm_mday = std::stoi(dd);
    tm.tm_hour = std::stoi(hh);
    tm.tm_min = std::stoi(min);
    tm.tm_sec = std::stoi(ss);
    
    return static_cast<uint64_t>(std::mktime(&tm));
}

std::string formatTimestamp(uint64_t epoch_seconds) {
    std::time_t time = static_cast<std::time_t>(epoch_seconds);
    std::tm* tm = std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::istringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

std::vector<std::string> getPvList(const Config& config) {
    std::vector<std::string> pv_names;
    
    if (!config.pv.empty()) {
        pv_names.push_back(config.pv);
    } else if (!config.pvs.empty()) {
        pv_names = splitString(config.pvs, ',');
    }
    
    return pv_names;
}

void executeCreateDataset(AnnotationClient& client, const Config& config) {
    if (config.name.empty() || config.owner.empty()) {
        std::cerr << "Required: --name, --owner" << std::endl;
        return;
    }
    
    auto pv_names = getPvList(config);
    if (pv_names.empty() && config.pattern.empty()) {
        std::cerr << "Must specify PVs using --pv, --pvs, or --pattern" << std::endl;
        return;
    }
    
    if (!config.pattern.empty() && pv_names.empty()) {
        std::cerr << "Pattern resolution not implemented - use specific PV names" << std::endl;
        return;
    }
    
    // Create time range - if date provided use it, otherwise use broad range
    uint64_t start_time, end_time;
    
    if (!config.date.empty()) {
        std::string time_str = config.time.empty() ? "000000" : config.time;
        std::string end_time_str = config.end_time.empty() ? time_str : config.end_time;
        std::string end_date_str = config.end_date.empty() ? config.date : config.end_date;
        
        start_time = parseDateTime(config.date, time_str);
        end_time = parseDateTime(end_date_str, end_time_str);
        
        // If no end date specified and no end time specified, default to 24 hours later
        if (config.end_date.empty() && config.end_time.empty()) {
            end_time = start_time + 86400;  // Add 24 hours
        }
    } else {
        // Use broad range for "all available data"
        // Start from Unix epoch (1970-01-01) instead of 0
        start_time = 1;  // January 1, 1970 00:00:01 UTC
        end_time = 2147483647;  // January 19, 2038 (max 32-bit timestamp)
    }
    
    Timestamp begin_ts = client.GetCommonClient().CreateTimestamp(start_time, 0);
    Timestamp end_ts = client.GetCommonClient().CreateTimestamp(end_time, 0);
    
    auto data_block = client.CreateDataBlock(begin_ts, end_ts, pv_names);
    auto dataset_id = client.CreateDataSet(config.name, config.owner, {data_block}, config.description);
    
    if (dataset_id) {
        std::cout << "Dataset created successfully" << std::endl;
        std::cout << "Dataset ID: " << *dataset_id << std::endl;
        if (!config.date.empty()) {
            std::cout << "Time range: " << formatTimestamp(start_time) 
                      << " to " << formatTimestamp(end_time) << std::endl;
        } else {
            std::cout << "Time range: All available data" << std::endl;
        }
        std::cout << "PVs: " << pv_names.size() << std::endl;
    } else {
        std::cerr << "Failed to create dataset: " << client.GetLastError() << std::endl;
    }
}

void executeCreateAnnotation(AnnotationClient& client, const Config& config) {
    if (config.name.empty() || config.owner.empty() || config.dataset_id.empty()) {
        std::cerr << "Required: --name, --owner, --dataset-id" << std::endl;
        return;
    }
    
    std::vector<std::string> dataset_ids = splitString(config.dataset_id, ',');
    std::vector<std::string> tag_list = config.tags.empty() ? 
        std::vector<std::string>{} : splitString(config.tags, ',');
    
    auto annotation_id = client.CreateAnnotation(
        config.owner, dataset_ids, config.name, config.comment, tag_list);
    
    if (annotation_id) {
        std::cout << "Annotation created successfully" << std::endl;
        std::cout << "Annotation ID: " << *annotation_id << std::endl;
    } else {
        std::cerr << "Failed to create annotation: " << client.GetLastError() << std::endl;
    }
}

void executeQueryDatasets(AnnotationClient& client, const Config& config) {
    std::vector<DataSet> datasets;
    
    if (!config.dataset_id.empty()) {
        datasets = client.QueryDataSetById(config.dataset_id);
    } else if (!config.owner.empty()) {
        datasets = client.QueryDataSetsByOwner(config.owner);
    } else if (!config.search.empty()) {
        datasets = client.QueryDataSetsByText(config.search);
    } else if (!config.pv.empty()) {
        datasets = client.QueryDataSetsByPvName(config.pv);
    } else {
        datasets = client.QueryAllDataSets();
    }
    
    std::cout << "Found " << datasets.size() << " dataset(s)" << std::endl;
    
    for (const auto& dataset : datasets) {
        std::cout << "\nDataset ID: " << dataset.id() << std::endl;
        std::cout << "Name: " << dataset.name() << std::endl;
        std::cout << "Owner: " << dataset.ownerid() << std::endl;
        
        if (!dataset.description().empty()) {
            std::cout << "Description: " << dataset.description() << std::endl;
        }
        
        std::cout << "Data Blocks: " << dataset.datablocks_size() << std::endl;
        
        if (config.verbose) {
            for (int i = 0; i < dataset.datablocks_size(); ++i) {
                const auto& block = dataset.datablocks(i);
                std::cout << "  Block " << (i + 1) << ":" << std::endl;
                std::cout << "    Start: " << block.begintime().epochseconds() << std::endl;
                std::cout << "    End: " << block.endtime().epochseconds() << std::endl;
                std::cout << "    PVs: " << block.pvnames_size() << std::endl;
                
                if (block.pvnames_size() <= 5) {
                    std::cout << "    Names: ";
                    for (int j = 0; j < block.pvnames_size(); ++j) {
                        if (j > 0) std::cout << ", ";
                        std::cout << block.pvnames(j);
                    }
                    std::cout << std::endl;
                } else {
                    std::cout << "    Names: " << block.pvnames(0) << ", " 
                              << block.pvnames(1) << ", ... (+" 
                              << (block.pvnames_size() - 2) << " more)" << std::endl;
                }
            }
        }
    }
}

void executeQueryAnnotations(AnnotationClient& client, const Config& config) {
    std::vector<Annotation> annotations;
    
    if (!config.annotation_id.empty()) {
        annotations = client.QueryAnnotationById(config.annotation_id);
    } else if (!config.owner.empty()) {
        annotations = client.QueryAnnotationsByOwner(config.owner);
    } else if (!config.dataset_id.empty()) {
        annotations = client.QueryAnnotationsByDataSet(config.dataset_id);
    } else if (!config.search.empty()) {
        annotations = client.QueryAnnotationsByText(config.search);
    } else if (!config.tags.empty()) {
        // For simplicity, query by first tag only
        std::vector<std::string> tag_list = splitString(config.tags, ',');
        if (!tag_list.empty()) {
            annotations = client.QueryAnnotationsByTag(tag_list[0]);
        }
    } else {
        annotations = client.QueryAllAnnotations();
    }
    
    std::cout << "Found " << annotations.size() << " annotation(s)" << std::endl;
    
    for (const auto& annotation : annotations) {
        std::cout << "\nAnnotation ID: " << annotation.id() << std::endl;
        std::cout << "Name: " << annotation.name() << std::endl;
        std::cout << "Owner: " << annotation.ownerid() << std::endl;
        
        if (!annotation.comment().empty()) {
            std::cout << "Comment: " << annotation.comment() << std::endl;
        }
        
        std::cout << "Datasets: " << annotation.datasetids_size() << std::endl;
        if (config.verbose) {
            for (int i = 0; i < annotation.datasetids_size(); ++i) {
                std::cout << "  " << annotation.datasetids(i) << std::endl;
            }
        }
        
        if (annotation.tags_size() > 0) {
            std::cout << "Tags: ";
            for (int i = 0; i < annotation.tags_size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << annotation.tags(i);
            }
            std::cout << std::endl;
        }
        
        if (config.verbose) {
            if (annotation.has_eventmetadata()) {
                const auto& event = annotation.eventmetadata();
                std::cout << "Event: " << event.description() << std::endl;
                if (event.has_starttimestamp()) {
                    std::cout << "  Start: " << event.starttimestamp().epochseconds() << std::endl;
                }
                if (event.has_stoptimestamp()) {
                    std::cout << "  Stop: " << event.stoptimestamp().epochseconds() << std::endl;
                }
            }
            
            if (annotation.has_calculations()) {
                const auto& calc = annotation.calculations();
                std::cout << "Calculations: " << calc.calculationdataframes_size() 
                          << " data frames" << std::endl;
            }
        }
    }
}

void executeExport(AnnotationClient& client, const Config& config) {
    if (config.dataset_id.empty()) {
        std::cerr << "Required: --dataset-id" << std::endl;
        return;
    }
    
    ExportOutputFormat format = ExportOutputFormat::ExportDataRequest_ExportOutputFormat_EXPORT_FORMAT_CSV;
    if (config.export_format == "hdf5") {
        format = ExportOutputFormat::ExportDataRequest_ExportOutputFormat_EXPORT_FORMAT_HDF5;
    } else if (config.export_format == "xlsx") {
        format = ExportOutputFormat::ExportDataRequest_ExportOutputFormat_EXPORT_FORMAT_XLSX;
    }
    
    auto file_path = client.ExportDataSetToFile(config.dataset_id, format);
    
    if (file_path) {
        std::cout << "Export completed successfully" << std::endl;
        std::cout << "File path: " << *file_path << std::endl;
    } else {
        std::cerr << "Export failed: " << client.GetLastError() << std::endl;
    }
}

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " OPERATION [OPTIONS]\n\n"
              << "OPERATIONS:\n"
              << "  create-dataset     - Create a new dataset\n"
              << "  create-annotation  - Create a new annotation\n"
              << "  query-datasets     - Query existing datasets\n"
              << "  query-annotations  - Query existing annotations\n"
              << "  export            - Export dataset to file\n\n"
              << "CREATE DATASET OPTIONS:\n"
              << "  --name=NAME           Dataset name (required)\n"
              << "  --owner=OWNER         Owner ID (required)\n"
              << "  --description=DESC    Dataset description\n"
              << "  --date=MMDDYYYY       Start date (optional - defaults to all available data)\n"
              << "  --time=HHMMSS         Start time (default: 000000)\n"
              << "  --end-date=MMDDYYYY   End date (default: start date + 1 day)\n"
              << "  --end-time=HHMMSS     End time (default: start time)\n\n"
              << "CREATE ANNOTATION OPTIONS:\n"
              << "  --name=NAME           Annotation name (required)\n"
              << "  --owner=OWNER         Owner ID (required)\n"
              << "  --dataset-id=ID       Dataset ID(s), comma-separated (required)\n"
              << "  --comment=TEXT        Comment text\n"
              << "  --tags=TAG1,TAG2      Comma-separated tags\n\n"
              << "PV SPECIFICATION (for datasets):\n"
              << "  --pv=NAME             Single PV name\n"
              << "  --pvs=NAME1,NAME2     Comma-separated PV list\n"
              << "  --pattern=REGEX       PV name pattern\n\n"
              << "QUERY OPTIONS:\n"
              << "  --dataset-id=ID       Filter by dataset ID\n"
              << "  --annotation-id=ID    Filter by annotation ID\n"
              << "  --owner=OWNER         Filter by owner\n"
              << "  --search=TEXT         Text search\n"
              << "  --tags=TAG            Filter by tag\n\n"
              << "EXPORT OPTIONS:\n"
              << "  --dataset-id=ID       Dataset to export (required)\n"
              << "  --export-format=FMT   Format: csv, hdf5, xlsx (default: csv)\n\n"
              << "GENERAL OPTIONS:\n"
              << "  --server=ADDRESS      Server address (default: localhost:50053)\n"
              << "  --format=FORMAT       Output format: table, csv (default: table)\n"
              << "  --verbose             Show detailed information\n"
              << "  --help                Show this help\n\n"
              << "EXAMPLES:\n"
              << "  " << program << " create-dataset --name=\"beam_study\" --owner=user123 \\\n"
              << "                   --pvs=BPMS:LI20:2445:X,BPMS:LI20:2445:Y --date=01152024\n\n"
              << "  " << program << " create-annotation --name=\"analysis\" --owner=user123 \\\n"
              << "                   --dataset-id=dataset_abc --comment=\"Statistical analysis\"\n\n"
              << "  " << program << " query-datasets --owner=user123\n"
              << "  " << program << " export --dataset-id=dataset_abc --export-format=csv\n";
}

Config parseArgs(int argc, char* argv[]) {
    Config config;
    
    if (argc < 2) {
        printUsage(argv[0]);
        exit(1);
    }
    
    // Check for help as first argument
    if (std::string(argv[1]) == "--help") {
        printUsage(argv[0]);
        exit(0);
    }
    
    config.operation = argv[1];
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        } else if (arg.find("--server=") == 0) {
            config.server = arg.substr(9);
        } else if (arg.find("--name=") == 0) {
            config.name = arg.substr(7);
        } else if (arg.find("--owner=") == 0) {
            config.owner = arg.substr(8);
        } else if (arg.find("--description=") == 0) {
            config.description = arg.substr(14);
        } else if (arg.find("--comment=") == 0) {
            config.comment = arg.substr(10);
        } else if (arg.find("--pv=") == 0) {
            config.pv = arg.substr(5);
        } else if (arg.find("--pvs=") == 0) {
            config.pvs = arg.substr(6);
        } else if (arg.find("--pattern=") == 0) {
            config.pattern = arg.substr(10);
        } else if (arg.find("--date=") == 0) {
            config.date = arg.substr(7);
        } else if (arg.find("--time=") == 0) {
            config.time = arg.substr(7);
        } else if (arg.find("--end-date=") == 0) {
            config.end_date = arg.substr(11);
        } else if (arg.find("--end-time=") == 0) {
            config.end_time = arg.substr(11);
        } else if (arg.find("--dataset-id=") == 0) {
            config.dataset_id = arg.substr(13);
        } else if (arg.find("--annotation-id=") == 0) {
            config.annotation_id = arg.substr(16);
        } else if (arg.find("--tags=") == 0) {
            config.tags = arg.substr(7);
        } else if (arg.find("--search=") == 0) {
            config.search = arg.substr(9);
        } else if (arg.find("--format=") == 0) {
            config.format = arg.substr(9);
        } else if (arg.find("--export-format=") == 0) {
            config.export_format = arg.substr(16);
        } else if (arg == "--verbose") {
            config.verbose = true;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            exit(1);
        }
    }
    
    return config;
}

int main(int argc, char* argv[]) {
    try {
        Config config = parseArgs(argc, argv);
        AnnotationClient client(config.server);
        
        if (config.operation == "create-dataset") {
            executeCreateDataset(client, config);
        } else if (config.operation == "create-annotation") {
            executeCreateAnnotation(client, config);
        } else if (config.operation == "query-datasets") {
            executeQueryDatasets(client, config);
        } else if (config.operation == "query-annotations") {
            executeQueryAnnotations(client, config);
        } else if (config.operation == "export") {
            executeExport(client, config);
        } else {
            std::cerr << "Unknown operation: " << config.operation << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}