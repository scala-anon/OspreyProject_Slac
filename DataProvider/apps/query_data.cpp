#include "query_client.hpp"
#include <iostream>
#include <sstream>

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " [OPTIONS]\n\n"
              << "Query time-series data from Data Platform\n\n"
              << "OPTIONS:\n"
              << "  --help, -h              Show this help\n"
              << "  --server=<addr>         Server address (default: localhost:50051)\n"
              << "  --pvs=<pv1,pv2,...>     Comma-separated PV names (required)\n"
              << "  --start=<epoch_sec>     Start time in epoch seconds (required)\n"
              << "  --end=<epoch_sec>       End time in epoch seconds (required)\n"
              << "  --stream                Use streaming query (faster for large data)\n"
              << "  --table                 Return tabular format\n"
              << "  --max-buckets=<n>       Limit number of response buckets\n\n"
              << "EXAMPLES:\n"
              << "  Query single PV:\n"
              << "    " << program << " --pvs=BPMS:LTUH:250:X --start=1609459200 --end=1609462800\n\n"
              << "  Query multiple PVs with streaming:\n"
              << "    " << program << " --pvs=BPMS:LTUH:250:X,BPMS:LTUH:250:Y \\\n"
              << "                  --start=1609459200 --end=1609462800 --stream\n\n"
              << "  Tabular output:\n"
              << "    " << program << " --pvs=KLYS:LI20:61:AMPL --start=1609459200 \\\n"
              << "                  --end=1609462800 --table\n";
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

void printQueryResults(const std::vector<QueryDataResponse>& responses) {
    size_t total_buckets = 0;
    size_t total_points = 0;
    
    for (const auto& response : responses) {
        if (response.has_querydata()) {
            total_buckets += response.querydata().databuckets_size();
            
            for (const auto& bucket : response.querydata().databuckets()) {
                if (bucket.has_datacolumn()) {
                    std::cout << "PV: " << bucket.datacolumn().name() 
                              << " (" << bucket.datacolumn().datavalues_size() << " values)\n";
                    total_points += bucket.datacolumn().datavalues_size();
                }
            }
        }
    }
    
    std::cout << "\nSummary: " << total_buckets << " buckets, " 
              << total_points << " total data points\n";
}

void printTableResults(const QueryTableResponse& response) {
    if (!response.has_tableresult()) {
        std::cout << "No table data returned\n";
        return;
    }
    
    const auto& result = response.tableresult();
    
    if (result.has_columntable()) {
        const auto& table = result.columntable();
        std::cout << "Column table with " << table.datacolumns_size() << " columns\n";
        
        for (const auto& column : table.datacolumns()) {
            std::cout << "Column: " << column.name() 
                      << " (" << column.datavalues_size() << " values)\n";
        }
    }
    else if (result.has_rowmaptable()) {
        const auto& table = result.rowmaptable();
        std::cout << "Row table with " << table.rows_size() << " rows\n";
        std::cout << "Columns: ";
        for (int i = 0; i < table.columnnames_size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << table.columnnames(i);
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string server_address = "localhost:50051";
    std::vector<std::string> pv_names;
    uint64_t start_time = 0, end_time = 0;
    bool use_stream = false;
    bool use_table = false;
    int max_buckets = 0;
    
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
        else if (arg.find("--pvs=") == 0) {
            pv_names = splitString(arg.substr(6), ',');
        }
        else if (arg.find("--start=") == 0) {
            start_time = std::stoull(arg.substr(8));
        }
        else if (arg.find("--end=") == 0) {
            end_time = std::stoull(arg.substr(6));
        }
        else if (arg == "--stream") {
            use_stream = true;
        }
        else if (arg == "--table") {
            use_table = true;
        }
        else if (arg.find("--max-buckets=") == 0) {
            max_buckets = std::stoi(arg.substr(14));
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    // Validate required parameters
    if (pv_names.empty() || start_time == 0 || end_time == 0) {
        std::cerr << "Error: Missing required parameters\n";
        std::cerr << "Required: --pvs, --start, --end\n";
        printUsage(argv[0]);
        return 1;
    }
    
    if (start_time >= end_time) {
        std::cerr << "Error: Start time must be before end time\n";
        return 1;
    }
    
    try {
        QueryClient client(server_address);
        
        std::cout << "Querying " << pv_names.size() << " PVs from " 
                  << start_time << " to " << end_time << std::endl;
        
        if (use_table) {
            // Table query
            auto request = makeQueryTableRequest(
                pv_names,
                makeTimestamp(start_time, 0),
                makeTimestamp(end_time, 0),
                QueryTableRequest::TABLE_FORMAT_ROW_MAP
            );
                        
            auto response = client.queryTable(request);
            printTableResults(response);
        }
        else {
            // Regular data query
            auto request = makeQueryDataRequest(
                pv_names,
                makeTimestamp(start_time, 0),
                makeTimestamp(end_time, 0)
            );
            
            if (use_stream) {
                auto responses = client.queryDataStream(request);
                
                if (max_buckets > 0 && responses.size() > max_buckets) {
                    responses.resize(max_buckets);
                    std::cout << "Limited to " << max_buckets << " buckets\n";
                }
                
                printQueryResults(responses);
            }
            else {
                auto response = client.queryData(request);
                printQueryResults({response});
            }
        }
        
        std::cout << "Query completed successfully\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
