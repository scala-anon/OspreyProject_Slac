#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

void showMainMenu() {
    std::cout << "Available commands:\n";
    std::cout << "  ingest\n";
    std::cout << "  query\n";
    std::cout << "  annotate\n";
}

void showIngestMenu() {
    std::cout << "Ingest operations:\n";
    std::cout << "  h5 <directory>\n";
    std::cout << "  archiver --pv=NAME [--date=DDMMYYYY] [--hours=N]\n";
    std::cout << "  archiver --pvs=NAME1,NAME2,... [--date=DDMMYYYY] [--hours=N]\n";
}

void showQueryMenu() {
    std::cout << "Query operations:\n";
    std::cout << "  list-pvs\n";
    std::cout << "  list-pvs-timerange --start=DDMMYYYY --end=DDMMYYYY\n";
    std::cout << "  pattern-search --pattern=REGEX\n";
    std::cout << "  list-nans [--start=DDMMYYYY --end=DDMMYYYY]\n";
    std::cout << "  count-timestamps (--pv=NAME | --pvs=N1,N2 | --pattern=REGEX) [--start=DDMMYYYY --end=DDMMYYYY]\n";
    std::cout << "  statistics (--pv=NAME | --pvs=N1,N2 | --pattern=REGEX) [--start=DDMMYYYY --end=DDMMYYYY]\n";
    std::cout << "  pv-info (--pv=NAME | --pvs=N1,N2 | --pattern=REGEX)\n";
    std::cout << "  time-coverage (--pv=NAME | --pvs=N1,N2 | --pattern=REGEX)\n";
    std::cout << "  count-nans (--pv=NAME | --pvs=N1,N2 | --pattern=REGEX) [--start=DDMMYYYY --end=DDMMYYYY]\n";
}

void showAnnotateMenu() {
    std::cout << "Annotation operations:\n";
    std::cout << "  create\n";
    std::cout << "  manage\n";
}

std::string buildCommand(const std::string& executable, const std::vector<std::string>& args, int start_idx) {
    std::string command = "./" + executable;
    
    // Add all remaining arguments
    for (int i = start_idx; i < args.size(); i++) {
        command += " " + args[i];
    }
    
    return command;
}

int executeCommand(const std::string& command) {
    std::cout << "Executing: " << command << std::endl;
    return system(command.c_str());
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    
    // No arguments - show main menu
    if (args.empty()) {
        showMainMenu();
        return 0;
    }
    
    std::string command = args[0];
    
    // Handle main commands
    if (command == "ingest") {
        if (args.size() == 1) {
            showIngestMenu();
            return 0;
        }
        
        std::string operation = args[1];
        if (operation == "h5") {
            return executeCommand(buildCommand("h5_to_dp", args, 2));
        }
        else if (operation == "archiver") {
            return executeCommand(buildCommand("archiver_to_dp", args, 2));
        }
        else {
            std::cerr << "Unknown ingest operation: " << operation << std::endl;
            showIngestMenu();
            return 1;
        }
    }
    else if (command == "query") {
        if (args.size() == 1) {
            showQueryMenu();
            return 0;
        }
        
        return executeCommand(buildCommand("query_mongo", args, 1));
    }
    else if (command == "annotate") {
        if (args.size() == 1) {
            showAnnotateMenu();
            return 0;
        }
        
        std::string operation = args[1];
        if (operation == "create") {
            return executeCommand(buildCommand("annotation_create", args, 2));
        }
        else if (operation == "manage") {
            return executeCommand(buildCommand("annotation_manage", args, 2));
        }
        else {
            std::cerr << "Unknown annotate operation: " << operation << std::endl;
            showAnnotateMenu();
            return 1;
        }
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        showMainMenu();
        return 1;
    }
    
    return 0;
}