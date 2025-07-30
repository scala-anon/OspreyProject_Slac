#include "annotation_client.hpp"
#include <iostream>
#include <sstream>

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " [OPTIONS]\n\n"
              << "Create and query DataSets in Data Platform\n\n"
              << "OPTIONS:\n"
              << "  --help, -h              Show this help\n"
              << "  --server=<addr>         Server address (default: localhost:50051)\n\n"
              << "CREATE DATASET:\n"
              << "  --create                Create a new dataset (required)\n"
              << "  --name=<name>           Dataset name (required)\n"
              << "  --owner=<owner>         Owner ID (required)\n"
              << "  --desc=<description>    Dataset description (optional)\n"
              << "  --pvs=<pv1,pv2,...>     Comma-separated PV names (required)\n"
              << "  --start=<epoch_sec>     Start time in epoch seconds (required)\n"
              << "  --end=<epoch_sec>       End time in epoch seconds (required)\n\n"
              << "QUERY DATASETS:\n"
              << "  --query                 Query existing datasets\n"
              << "  --owner=<owner>         Filter by owner\n"
              << "  --text=<search>         Text search in name/description\n"
              << "  --pv=<pv_name>          Filter by PV name\n"
              << "  --id=<dataset_id>       Get specific dataset by ID\n\n"
              << "OUTPUT OPTIONS:\n"
              << "  --verbose, -v           Show detailed information\n\n"
              << "EXAMPLES:\n"
              << "  Create dataset:\n"
              << "    " << program << " --create --name=beam_study_1 --owner=user123 \\\n"
              << "                  --pvs=BPMS:LTUH:250:X,BPMS:LTUH:250:Y \\\n"
              << "                  --start=1609459200 --end=1609462800 \\\n"
              << "                  --desc=\"LCLS beam position analysis\"\n\n"
              << "  Query datasets by owner:\n"
              << "    " << program << " --query --owner=user123\n\n"
              << "  Search datasets:\n"
              << "    " << program << " --query --text=\"beam position\" --verbose\n\n"
              << "  Find datasets with specific PV:\n"
              << "    " << program << " --query --pv=BPMS:LTUH:250:X\n";
}

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

void printDataSet(const DataSet& dataset, bool verbose) {
    std::cout << "Dataset ID: " << dataset.id() << std::endl;
    std::cout << "Name: " << dataset.name() << std::endl;
    std::cout << "Owner: " << dataset.ownerid() << std::endl;
    
    if (!dataset.description().empty()) {
        std::cout << "Description: " << dataset.description() << std::endl;
    }
    
    std::cout << "Data Blocks: " << dataset.datablocks_size() << std::endl;
    
    if (verbose) {
        for (int i = 0; i < dataset.datablocks_size(); i++) {
            const auto& block = dataset.datablocks(i);
            std::cout << "  Block " << (i + 1) << ":\n";
            std::cout << "    Start: " << block.begintime().epochseconds() << std::endl;
            std::cout << "    End: " << block.endtime().epochseconds() << std::endl;
            std::cout << "    PV Count: " << block.pvnames_size() << std::endl;
            
            if (block.pvnames_size() <= 10) {
                std::cout << "    PVs: ";
                for (int j = 0; j < block.pvnames_size(); j++) {
                    if (j > 0) std::cout << ", ";
                    std::cout << block.pvnames(j);
                }
                std::cout << std::endl;
            } else {
                std::cout << "    PVs: " << block.pvnames(0) << ", " << block.pvnames(1) 
                          << ", ... (+" << (block.pvnames_size() - 2) << " more)" << std::endl;
            }
        }
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    std::string server_address = "localhost:50051";
    bool create_mode = false;
    bool query_mode = false;
    bool verbose = false;
    
    // Create parameters
    std::string dataset_name, owner_id, description;
    std::vector<std::string> pv_names;
    uint64_t start_time = 0, end_time = 0;
    
    // Query parameters
    std::string query_owner, query_text, query_pv, query_id;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg.find("--server=") == 0) {
            server_address = arg.substr(9);
        }
        else if (arg == "--create") {
            create_mode = true;
        }
        else if (arg == "--query") {
            query_mode = true;
        }
        else if (arg.find("--name=") == 0) {
            dataset_name = arg.substr(7);
        }
        else if (arg.find("--owner=") == 0) {
            if (create_mode) {
                owner_id = arg.substr(8);
            } else {
                query_owner = arg.substr(8);
            }
        }
        else if (arg.find("--desc=") == 0) {
            description = arg.substr(7);
        }
        else if (arg.find("--pvs=") == 0) {
            pv_names = splitString(arg.substr(6), ',');
        }
        else if (arg.find("--start=") == 0) {
            start_time = std::stoull(arg.substr(8));
        }
        else if (arg.find("--end=") == 0) {
            end_time = std::stoull(arg.substr(6));
        }
        else if (arg.find("--text=") == 0) {
            query_text = arg.substr(7);
        }
        else if (arg.find("--pv=") == 0) {
            query_pv = arg.substr(5);
        }
        else if (arg.find("--id=") == 0) {
            query_id = arg.substr(5);
        }
        else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    if (!create_mode && !query_mode) {
        std::cerr << "Error: Must specify either --create or --query\n";
        printUsage(argv[0]);
        return 1;
    }
    
    if (create_mode && query_mode) {
        std::cerr << "Error: Cannot specify both --create and --query\n";
        return 1;
    }
    
    try {
        AnnotationClient client(server_address);
        
        if (create_mode) {
            // Validate required parameters
            if (dataset_name.empty() || owner_id.empty() || pv_names.empty() || 
                start_time == 0 || end_time == 0) {
                std::cerr << "Error: Missing required parameters for dataset creation\n";
                std::cerr << "Required: --name, --owner, --pvs, --start, --end\n";
                return 1;
            }
            
            if (start_time >= end_time) {
                std::cerr << "Error: Start time must be before end time\n";
                return 1;
            }
            
            std::cout << "Creating dataset '" << dataset_name << "'...\n";
            std::cout << "Owner: " << owner_id << std::endl;
            std::cout << "PVs: " << pv_names.size() << std::endl;
            std::cout << "Time range: " << start_time << " to " << end_time 
                      << " (" << (end_time - start_time) << " seconds)" << std::endl;
            
            // Create the dataset
            TimeRange time_range(start_time, 0, end_time, 0);
            DataBlock block = makeDataBlock(time_range, pv_names);
            DataSet dataset = makeDataSet(dataset_name, owner_id, description, {block});
            
            auto response = client.createDataSet(makeCreateDataSetRequest(dataset));
            
            if (response.has_createdatasetresult()) {
                std::cout << "\nDataset created successfully!\n";
                std::cout << "Dataset ID: " << response.createdatasetresult().datasetid() << std::endl;
            } else {
                std::cout << "Dataset creation failed\n";
                if (response.has_exceptionalresult()) {
                    std::cout << "Error details available in response\n";
                }
                return 1;
            }
        }
        else if (query_mode) {
            QueryDataSetsRequest request;
            
            // Build query based on parameters
            if (!query_id.empty()) {
                request = makeQueryDataSetsById(query_id);
            } else if (!query_owner.empty()) {
                request = makeQueryDataSetsByOwner(query_owner);
            } else if (!query_text.empty()) {
                request = makeQueryDataSetsByText(query_text);
            } else if (!query_pv.empty()) {
                request = makeQueryDataSetsByPvName(query_pv);
            } else {
                // Query all datasets (empty text search)
                request = makeQueryDataSetsByText("");
            }
            
            auto response = client.queryDataSets(request);
            
            if (response.has_datasetsresult()) {
                const auto& result = response.datasetsresult();
                std::cout << "Found " << result.datasets_size() << " dataset(s):\n\n";
                
                for (const auto& dataset : result.datasets()) {
                    printDataSet(dataset, verbose);
                }
            } else {
                std::cout << "No datasets found\n";
                if (response.has_exceptionalresult()) {
                    std::cout << "Query returned exceptional result\n";
                }
            }
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
