#include "annotation_client.hpp"
#include <iostream>
#include <sstream>

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " [OPTIONS]\n\n"
              << "Create, query, and export annotations in Data Platform\n\n"
              << "OPTIONS:\n"
              << "  --help, -h              Show this help\n"
              << "  --server=<addr>         Server address (default: localhost:50051)\n\n"
              << "CREATE ANNOTATION:\n"
              << "  --create                Create new annotation (required)\n"
              << "  --name=<n>           Annotation name (required)\n"
              << "  --owner=<owner>         Owner ID (required)\n"
              << "  --datasets=<id1,id2>    Dataset IDs (required)\n"
              << "  --comment=<text>        Comment text (optional)\n"
              << "  --tags=<tag1,tag2>      Comma-separated tags (optional)\n\n"
              << "QUERY ANNOTATIONS:\n"
              << "  --query                 Query existing annotations\n"
              << "  --owner=<owner>         Filter by owner\n"
              << "  --dataset=<id>          Filter by dataset ID\n"
              << "  --tag=<tag>             Filter by tag\n"
              << "  --text=<search>         Text search in name/comment\n"
              << "  --id=<annotation_id>    Get specific annotation\n\n"
              << "EXPORT DATA:\n"
              << "  --export=<dataset_id>   Export dataset to file\n"
              << "  --format=<fmt>          Export format: csv, hdf5, xlsx (default: csv)\n\n"
              << "OUTPUT OPTIONS:\n"
              << "  --verbose, -v           Show detailed information\n\n"
              << "EXAMPLES:\n"
              << "  Create annotation:\n"
              << "    " << program << " --create --name=beam_analysis_1 --owner=user123 \\\n"
              << "                  --datasets=dataset_abc,dataset_def \\\n"
              << "                  --comment=\"Statistical analysis of beam position\" \\\n"
              << "                  --tags=beam_physics,statistics\n\n"
              << "  Query annotations by owner:\n"
              << "    " << program << " --query --owner=user123 --verbose\n\n"
              << "  Find annotations for dataset:\n"
              << "    " << program << " --query --dataset=dataset_abc\n\n"
              << "  Search by tag:\n"
              << "    " << program << " --query --tag=beam_physics\n\n"
              << "  Export dataset:\n"
              << "    " << program << " --export=dataset_abc --format=csv\n";
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

void printAnnotation(const QueryAnnotationsResponse::AnnotationsResult::Annotation& annotation, bool verbose) {
    std::cout << "Annotation ID: " << annotation.id() << std::endl;
    std::cout << "Name: " << annotation.name() << std::endl;
    std::cout << "Owner: " << annotation.ownerid() << std::endl;
    
    if (!annotation.comment().empty()) {
        std::cout << "Comment: " << annotation.comment() << std::endl;
    }
    
    // Dataset IDs
    std::cout << "Dataset IDs (" << annotation.datasetids_size() << "): ";
    for (int i = 0; i < annotation.datasetids_size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << annotation.datasetids(i);
    }
    std::cout << std::endl;
    
    // Tags
    if (annotation.tags_size() > 0) {
        std::cout << "Tags: ";
        for (int i = 0; i < annotation.tags_size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << annotation.tags(i);
        }
        std::cout << std::endl;
    }
    
    if (verbose) {
        // Linked annotations
        if (annotation.annotationids_size() > 0) {
            std::cout << "Linked Annotations: ";
            for (int i = 0; i < annotation.annotationids_size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << annotation.annotationids(i);
            }
            std::cout << std::endl;
        }
        
        // Attributes
        if (annotation.attributes_size() > 0) {
            std::cout << "Attributes:\n";
            for (const auto& attr : annotation.attributes()) {
                std::cout << "  " << attr.name() << ": " << attr.value() << std::endl;
            }
        }
        
        // Event metadata
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
        
        // Calculations
        if (annotation.has_calculations()) {
            const auto& calc = annotation.calculations();
            std::cout << "Calculations: " << calc.calculationdataframes_size() << " data frames\n";
            for (const auto& frame : calc.calculationdataframes()) {
                std::cout << "  Frame: " << frame.name() 
                          << " (" << frame.datacolumns_size() << " columns)" << std::endl;
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
    std::string annotation_name, owner_id, comment;
    std::vector<std::string> dataset_ids, tags;
    
    // Query parameters
    std::string query_owner, query_dataset, query_tag, query_text, query_id;
    
    // Export parameters
    std::string export_dataset_id, export_format = "csv";
    
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
        else if (arg.find("--export=") == 0) {
            export_dataset_id = arg.substr(9);
        }
        else if (arg.find("--name=") == 0) {
            annotation_name = arg.substr(7);
        }
        else if (arg.find("--owner=") == 0) {
            if (create_mode) {
                owner_id = arg.substr(8);
            } else {
                query_owner = arg.substr(8);
            }
        }
        else if (arg.find("--datasets=") == 0) {
            dataset_ids = splitString(arg.substr(11), ',');
        }
        else if (arg.find("--comment=") == 0) {
            comment = arg.substr(10);
        }
        else if (arg.find("--tags=") == 0) {
            tags = splitString(arg.substr(7), ',');
        }
        else if (arg.find("--dataset=") == 0) {
            query_dataset = arg.substr(10);
        }
        else if (arg.find("--tag=") == 0) {
            query_tag = arg.substr(6);
        }
        else if (arg.find("--text=") == 0) {
            query_text = arg.substr(7);
        }
        else if (arg.find("--id=") == 0) {
            query_id = arg.substr(5);
        }
        else if (arg.find("--format=") == 0) {
            export_format = arg.substr(9);
        }
        else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    // Determine operation mode
    bool has_operation = create_mode || query_mode || !export_dataset_id.empty();
    
    if (!has_operation) {
        std::cerr << "Error: Must specify --create, --query, or --export\n";
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        AnnotationClient client(server_address);
        
        if (create_mode) {
            // Validate required parameters
            if (annotation_name.empty() || owner_id.empty() || dataset_ids.empty()) {
                std::cerr << "Error: Missing required parameters for annotation creation\n";
                std::cerr << "Required: --name, --owner, --datasets\n";
                return 1;
            }
            
            std::cout << "Creating annotation '" << annotation_name << "'...\n";
            std::cout << "Owner: " << owner_id << std::endl;
            std::cout << "Datasets: " << dataset_ids.size() << std::endl;
            if (!comment.empty()) {
                std::cout << "Comment: " << comment << std::endl;
            }
            if (!tags.empty()) {
                std::cout << "Tags: " << tags.size() << std::endl;
            }
            
            // Create the annotation
            CreateAnnotationRequest request = makeCreateAnnotationRequest(
                owner_id, annotation_name, dataset_ids);
            
            if (!comment.empty()) {
                request.set_comment(comment);
            }
            
            for (const auto& tag : tags) {
                request.add_tags(tag);
            }
            
            auto response = client.createAnnotation(request);
            
            if (response.has_createannotationresult()) {
                std::cout << "\nAnnotation created successfully!\n";
                std::cout << "Annotation ID: " << response.createannotationresult().annotationid() << std::endl;
            } else {
                std::cout << "Annotation creation failed\n";
                if (response.has_exceptionalresult()) {
                    std::cout << "Error details available in response\n";
                }
                return 1;
            }
        }
        else if (query_mode) {
            QueryAnnotationsRequest request;
            
            // Build query based on parameters
            if (!query_id.empty()) {
                request = makeQueryAnnotationsById(query_id);
            } else if (!query_owner.empty()) {
                request = makeQueryAnnotationsByOwner(query_owner);
            } else if (!query_dataset.empty()) {
                request = makeQueryAnnotationsByDataSet(query_dataset);
            } else if (!query_tag.empty()) {
                request = makeQueryAnnotationsByTag(query_tag);
            } else if (!query_text.empty()) {
                request = makeQueryAnnotationsByText(query_text);
            } else {
                // Query all annotations (empty text search)
                request = makeQueryAnnotationsByText("");
            }
            
            auto response = client.queryAnnotations(request);
            
            if (response.has_annotationsresult()) {
                const auto& result = response.annotationsresult();
                std::cout << "Found " << result.annotations_size() << " annotation(s):\n\n";
                
                for (const auto& annotation : result.annotations()) {
                    printAnnotation(annotation, verbose);
                }
            } else {
                std::cout << "No annotations found\n";
                if (response.has_exceptionalresult()) {
                    std::cout << "Query returned exceptional result\n";
                }
            }
        }
        else if (!export_dataset_id.empty()) {
            // Export dataset
            ExportDataRequest::ExportOutputFormat format = ExportDataRequest::EXPORT_FORMAT_CSV;
            
            if (export_format == "hdf5") {
                format = ExportDataRequest::EXPORT_FORMAT_HDF5;
            } else if (export_format == "xlsx") {
                format = ExportDataRequest::EXPORT_FORMAT_XLSX;
            } else if (export_format != "csv") {
                std::cerr << "Warning: Unknown format '" << export_format 
                          << "', using CSV" << std::endl;
            }
            
            std::cout << "Exporting dataset " << export_dataset_id 
                      << " in " << export_format << " format...\n";
            
            auto request = makeExportDataRequest(export_dataset_id, format);
            auto response = client.exportData(request);
            
            if (response.has_exportdataresult()) {
                const auto& result = response.exportdataresult();
                std::cout << "Export completed successfully!\n";
                std::cout << "File path: " << result.filepath() << std::endl;
                
                if (!result.fileurl().empty()) {
                    std::cout << "File URL: " << result.fileurl() << std::endl;
                }
            } else {
                std::cout << "Export failed\n";
                if (response.has_exceptionalresult()) {
                    std::cout << "Error details available in response\n";
                }
                return 1;
            }
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}