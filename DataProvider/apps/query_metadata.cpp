#include "query_client.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " [OPTIONS]\n\n"
              << "Query metadata and providers from Data Platform\n\n"
              << "OPTIONS:\n"
              << "  --help, -h              Show this help\n"
              << "  --server=<addr>         Server address (default: localhost:50051)\n\n"
              << "PV METADATA:\n"
              << "  --pv-list=<pv1,pv2>     Query specific PV metadata\n"
              << "  --pv-pattern=<regex>    Query PVs matching regex pattern\n\n"
              << "PROVIDER METADATA:\n"
              << "  --providers             List all providers\n"
              << "  --provider-text=<text>  Search providers by text\n"
              << "  --provider-id=<id>      Get metadata for specific provider\n\n"
              << "OUTPUT OPTIONS:\n"
              << "  --verbose, -v           Show detailed information\n"
              << "  --summary               Show summary statistics only\n\n"
              << "EXAMPLES:\n"
              << "  Query specific PVs:\n"
              << "    " << program << " --pv-list=BPMS:LTUH:250:X,KLYS:LI20:61:AMPL\n\n"
              << "  Find all BPMS devices:\n"
              << "    " << program << " --pv-pattern=\"BPMS:.*\" --summary\n\n"
              << "  List all providers:\n"
              << "    " << program << " --providers\n\n"
              << "  Provider details:\n"
              << "    " << program << " --provider-id=provider_123 --verbose\n";
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

void printPvMetadata(const QueryPvMetadataResponse& response, bool verbose, bool summary) {
    if (!response.has_metadataresult()) {
        std::cout << "No PV metadata found\n";
        return;
    }
    
    const auto& result = response.metadataresult();
    
    if (summary) {
        std::cout << "Found " << result.pvinfos_size() << " PVs\n";
        
        // Count by data type
        std::map<std::string, int> type_counts;
        uint64_t total_buckets = 0;
        uint64_t total_samples = 0;
        
        for (const auto& pv : result.pvinfos()) {
            type_counts[pv.lastbucketdatatype()]++;
            total_buckets += pv.numbuckets();
            total_samples += pv.lastbucketsamplecount();
        }
        
        std::cout << "Total buckets: " << total_buckets << std::endl;
        std::cout << "Total samples: " << total_samples << std::endl;
        std::cout << "Data types:\n";
        for (const auto& [type, count] : type_counts) {
            std::cout << "  " << type << ": " << count << std::endl;
        }
        return;
    }
    
    for (const auto& pv : result.pvinfos()) {
        std::cout << "PV: " << pv.pvname() << std::endl;
        
        if (verbose) {
            std::cout << "  Data Type: " << pv.lastbucketdatatype() << std::endl;
            std::cout << "  Timestamps Type: " << pv.lastbucketdatatimestampstype() << std::endl;
            std::cout << "  Sample Count: " << pv.lastbucketsamplecount() << std::endl;
            
            if (pv.lastbucketsampleperiod() > 0) {
                double period_ms = pv.lastbucketsampleperiod() / 1000000.0;
                std::cout << "  Sample Period: " << std::fixed << std::setprecision(3) 
                          << period_ms << " ms" << std::endl;
            }
            
            std::cout << "  Number of Buckets: " << pv.numbuckets() << std::endl;
            std::cout << "  First Data: " << pv.firstdatatimestamp().epochseconds() << std::endl;
            std::cout << "  Last Data: " << pv.lastdatatimestamp().epochseconds() << std::endl;
            std::cout << "  Bucket ID: " << pv.lastbucketid() << std::endl;
        } else {
            std::cout << "  Type: " << pv.lastbucketdatatype() 
                      << ", Buckets: " << pv.numbuckets()
                      << ", Samples: " << pv.lastbucketsamplecount() << std::endl;
        }
        std::cout << std::endl;
    }
}

void printProviders(const QueryProvidersResponse& response, bool verbose) {
    if (!response.has_providersresult()) {
        std::cout << "No providers found\n";
        return;
    }
    
    const auto& result = response.providersresult();
    
    std::cout << "Found " << result.providerinfos_size() << " providers:\n\n";
    
    for (const auto& provider : result.providerinfos()) {
        std::cout << "Provider: " << provider.name() << std::endl;
        std::cout << "  ID: " << provider.id() << std::endl;
        
        if (!provider.description().empty()) {
            std::cout << "  Description: " << provider.description() << std::endl;
        }
        
        if (verbose) {
            if (provider.tags_size() > 0) {
                std::cout << "  Tags: ";
                for (int i = 0; i < provider.tags_size(); i++) {
                    if (i > 0) std::cout << ", ";
                    std::cout << provider.tags(i);
                }
                std::cout << std::endl;
            }
            
            if (provider.attributes_size() > 0) {
                std::cout << "  Attributes:\n";
                for (const auto& attr : provider.attributes()) {
                    std::cout << "    " << attr.name() << ": " << attr.value() << std::endl;
                }
            }
        }
        std::cout << std::endl;
    }
}

void printProviderMetadata(const QueryProviderMetadataResponse& response, bool verbose) {
    if (!response.has_metadataresult()) {
        std::cout << "No provider metadata found\n";
        return;
    }
    
    const auto& result = response.metadataresult();
    
    for (const auto& metadata : result.providermetadatas()) {
        std::cout << "Provider ID: " << metadata.id() << std::endl;
        std::cout << "  PV Count: " << metadata.pvnames_size() << std::endl;
        std::cout << "  Bucket Count: " << metadata.numbuckets() << std::endl;
        std::cout << "  First Bucket: " << metadata.firstbuckettime().epochseconds() << std::endl;
        std::cout << "  Last Bucket: " << metadata.lastbuckettime().epochseconds() << std::endl;
        
        if (verbose && metadata.pvnames_size() > 0) {
            std::cout << "  PV Names:\n";
            for (const auto& pv : metadata.pvnames()) {
                std::cout << "    " << pv << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string server_address = "localhost:50051";
    std::string operation;
    std::vector<std::string> pv_list;
    std::string pv_pattern, provider_text, provider_id;
    bool verbose = false;
    bool summary = false;
    
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
        else if (arg.find("--pv-list=") == 0) {
            operation = "pv-metadata";
            pv_list = splitString(arg.substr(10), ',');
        }
        else if (arg.find("--pv-pattern=") == 0) {
            operation = "pv-pattern";
            pv_pattern = arg.substr(13);
        }
        else if (arg == "--providers") {
            operation = "providers";
        }
        else if (arg.find("--provider-text=") == 0) {
            operation = "provider-search";
            provider_text = arg.substr(16);
        }
        else if (arg.find("--provider-id=") == 0) {
            operation = "provider-metadata";
            provider_id = arg.substr(14);
        }
        else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        }
        else if (arg == "--summary") {
            summary = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    if (operation.empty()) {
        std::cerr << "Error: No operation specified\n";
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        QueryClient client(server_address);
        
        if (operation == "pv-metadata") {
            auto request = makeQueryPvMetadataRequest(pv_list);
            auto response = client.queryPvMetadata(request);
            printPvMetadata(response, verbose, summary);
        }
        else if (operation == "pv-pattern") {
            auto request = makeQueryPvMetadataRequestWithPattern(pv_pattern);
            auto response = client.queryPvMetadata(request);
            printPvMetadata(response, verbose, summary);
        }
        else if (operation == "providers") {
            auto request = makeQueryProvidersRequest("");
            auto response = client.queryProviders(request);
            printProviders(response, verbose);
        }
        else if (operation == "provider-search") {
            auto request = makeQueryProvidersRequest(provider_text);
            auto response = client.queryProviders(request);
            printProviders(response, verbose);
        }
        else if (operation == "provider-metadata") {
            auto request = makeQueryProviderMetadataRequest(provider_id);
            auto response = client.queryProviderMetadata(request);
            printProviderMetadata(response, verbose);
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}