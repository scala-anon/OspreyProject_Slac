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
    std::cout << "Dataset ID: " << dataset.id() << "\n"
              << "Name: " << dataset.name() << "\n"
              << "Owner: " << dataset.ownerid() << "\n";
    
    if (!dataset.description().empty()) {
        std::cout << "Description: " << dataset.description() << "\n";
    }
    
    std::cout << "Data Blocks: " << dataset.datablocks_size() << "\n";
    
    if (verbose) {
        for (int i = 0; i < dataset.datablocks_size(); i++) {
            const auto& block = dataset.datablocks(i);
            std::cout << "  Block " << (i + 1) << ":\n"
                      << "    Start: " << block.begintime().epochseconds() << "\n"
                      << "    End: " << block.endtime().epochseconds() << "\n"
                      << "    PV Count: " << block.pvnames_size() << "\n";
            
            if (block.pvnames_size() <= 10) {
                std::cout << "    PVs: ";
                for (int j = 0; j < block.pvnames_size(); j++) {
                    if (j > 0) std::cout << ", ";
                    std::cout << block.pvnames(j);
                }
                std::cout << "\n";
            } else {
                std::cout << "    PVs: " << block.pvnames(0) << ", " << block.pvnames(1) 
                          << ", ... (+" << (block.pvnames_size() - 2) << " more)\n";
            }
        }
    }
    std::cout << "\n";
}

struct ProgramArgs {
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
};

ProgramArgs parseArguments(int argc, char* argv[]) {
    ProgramArgs args;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            exit(0);
        }
        else if (arg.find("--server=") == 0) {
            args.server_address = arg.substr(9);
        }
        else if (arg == "--create") {
            args.create_mode = true;
        }
        else if (arg == "--query") {
            args.query_mode = true;
        }
        else if (arg.find("--name=") == 0) {
            args.dataset_name = arg.substr(7);
        }
        else if (arg.find("--owner=") == 0) {
            if (args.create_mode) {
                args.owner_id = arg.substr(8);
            } else {
                args.query_owner = arg.substr(8);
            }
        }
        else if (arg.find("--desc=") == 0) {
            args.description = arg.substr(7);
        }
        else if (arg.find("--pvs=") == 0) {
            args.pv_names = splitString(arg.substr(6), ',');
        }
        else if (arg.find("--start=") == 0) {
            args.start_time = std::stoull(arg.substr(8));
        }
        else if (arg.find("--end=") == 0) {
            args.end_time = std::stoull(arg.substr(6));
        }
        else if (arg.find("--text=") == 0) {
            args.query_text = arg.substr(7);
        }
        else if (arg.find("--pv=") == 0) {
            args.query_pv = arg.substr(5);
        }
        else if (arg.find("--id=") == 0) {
            args.query_id = arg.substr(5);
        }
        else if (arg == "--verbose" || arg == "-v") {
            args.verbose = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            exit(1);
        }
    }
    
    return args;
}

bool validateCreateArgs(const ProgramArgs& args) {
    if (args.dataset_name.empty() || args.owner_id.empty() || args.pv_names.empty() || 
        args.start_time == 0 || args.end_time == 0) {
        std::cerr << "Error: Missing required parameters for dataset creation\n"
                  << "Required: --name, --owner, --pvs, --start, --end\n";
        return false;
    }
    
    if (args.start_time >= args.end_time) {
        std::cerr << "Error: Start time must be before end time\n";
        return false;
    }
    
    return true;
}

void performCreateDataset(AnnotationClient& client, const ProgramArgs& args) {
    std::cout << "Creating dataset '" << args.dataset_name << "'...\n"
              << "Owner: " << args.owner_id << "\n"
              << "PVs: " << args.pv_names.size() << "\n"
              << "Time range: " << args.start_time << " to " << args.end_time 
              << " (" << (args.end_time - args.start_time) << " seconds)\n";
    
    // Create the dataset
    TimeRange time_range(args.start_time, 0, args.end_time, 0);
    DataBlock block = makeDataBlock(time_range, args.pv_names);
    DataSet dataset = makeDataSet(args.dataset_name, args.owner_id, args.description, {block});
    
    auto response = client.createDataSet(makeCreateDataSetRequest(dataset));
    
    if (response.has_createdatasetresult()) {
        std::cout << "\nDataset created successfully!\n"
                  << "Dataset ID: " << response.createdatasetresult().datasetid() << std::endl;
    } else {
        std::cout << "Dataset creation failed\n";
        if (response.has_exceptionalresult()) {
            std::cout << "Error details available in response\n";
        }
        exit(1);
    }
}

void performQueryDatasets(AnnotationClient& client, const ProgramArgs& args) {
    QueryDataSetsRequest request;
    
    // Build query based on parameters
    if (!args.query_id.empty()) {
        request = makeQueryDataSetsById(args.query_id);
    } else if (!args.query_owner.empty()) {
        request = makeQueryDataSetsByOwner(args.query_owner);
    } else if (!args.query_text.empty()) {
        request = makeQueryDataSetsByText(args.query_text);
    } else if (!args.query_pv.empty()) {
        request = makeQueryDataSetsByPvName(args.query_pv);
    } else {
        // Query all datasets (empty text search)
        request = makeQueryDataSetsByText("");
    }
    
    auto response = client.queryDataSets(request);
    
    if (response.has_datasetsresult()) {
        const auto& result = response.datasetsresult();
        std::cout << "Found " << result.datasets_size() << " dataset(s):\n\n";
        
        for (const auto& dataset : result.datasets()) {
            printDataSet(dataset, args.verbose);
        }
    } else {
        std::cout << "No datasets found\n";
        if (response.has_exceptionalresult()) {
            std::cout << "Query returned exceptional result\n";
        }
    }
}

int main(int argc, char* argv[]) {
    auto args = parseArguments(argc, argv);
    
    if (!args.create_mode && !args.query_mode) {
        std::cerr << "Error: Must specify either --create or --query\n";
        printUsage(argv[0]);
        return 1;
    }
    
    if (args.create_mode && args.query_mode) {
        std::cerr << "Error: Cannot specify both --create and --query\n";
        return 1;
    }
    
    try {
        AnnotationClient client(args.server_address);
        
        if (args.create_mode) {
            if (!validateCreateArgs(args)) {
                return 1;
            }
            performCreateDataset(client, args);
        } else {
            performQueryDatasets(client, args);
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}