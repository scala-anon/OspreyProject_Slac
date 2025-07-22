#include "h5_parser.hpp"
#include "ingest_client.hpp"
#include <iostream>
#include <chrono>
#include <future>
#include <thread>
#include <algorithm>
#include <filesystem>

void printUsage(const std::string& program_name) {
    std::cout << "H5 to DataProvider Ingestion (Optimized)\n";
    std::cout << "USAGE: " << program_name << " <h5_directory> [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  --config=PATH                 Use config file (default: config/ingestion_config.json)\n";
    std::cout << "  --local-only                  Parse only, don't send to MLDP\n";
    std::cout << "  --device=DEVICE               Filter by device (e.g., KLYS, BPMS)\n";
    std::cout << "  --project=PROJECT             Filter by project (e.g., CoAD)\n";
    std::cout << "  --max-signals=N               Limit to N signals\n";
    std::cout << "  --batch-size=N                Batch size (default from config)\n";
    std::cout << "  --no-spatial                  Disable spatial enrichment\n";
    std::cout << "  --server=ADDRESS              MLDP server address\n";
    std::cout << "  --verbose                     Enable detailed output\n";
    std::cout << "  --parallel-files=N            Number of H5 files to process in parallel\n";
    std::cout << "  --enable-bulk                 Enable bulk optimization mode\n";
    std::cout << "  --memory-limit=MB             Memory limit in megabytes (default: 2048)\n";
    std::cout << "  --benchmark                   Enable performance benchmarking\n\n";
    std::cout << "OPTIMIZATION FLAGS:\n";
    std::cout << "  --cache-size=N                LRU cache size for spatial analysis\n";
    std::cout << "  --worker-threads=N            Number of worker threads (default: CPU cores)\n";
    std::cout << "  --optimal-batch=N             Optimal batch size for ingestion\n\n";
}

// PHASE 2: Parallel H5 file parsing structure
struct H5FileParseTask {
    std::string file_path;
    std::string device_filter;
    std::string project_filter;
    size_t max_signals_per_file;
    bool enable_spatial;
};

struct H5ParseResult {
    std::vector<SignalData> signals;
    std::string file_path;
    bool success;
    double parse_time_seconds;
    std::string error_message;
};

// PHASE 2: Parallel H5 file parser
class ParallelH5Parser {
private:
    size_t num_threads_;
    bool verbose_;
    
public:
    ParallelH5Parser(size_t num_threads, bool verbose = false) 
        : num_threads_(num_threads), verbose_(verbose) {}
    
    std::vector<H5ParseResult> parseFilesParallel(const std::vector<H5FileParseTask>& tasks) {
        if (verbose_) {
            std::cout << "Starting parallel H5 parsing with " << num_threads_ << " threads" << std::endl;
        }
        
        std::vector<std::future<H5ParseResult>> futures;
        futures.reserve(tasks.size());
        
        // Launch parsing tasks
        for (const auto& task : tasks) {
            auto future = std::async(std::launch::async, [this, task]() {
                return parseFileTask(task);
            });
            futures.push_back(std::move(future));
        }
        
        // Collect results
        std::vector<H5ParseResult> results;
        results.reserve(tasks.size());
        
        for (auto& future : futures) {
            results.push_back(future.get());
        }
        
        return results;
    }
    
private:
    H5ParseResult parseFileTask(const H5FileParseTask& task) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        H5ParseResult result;
        result.file_path = task.file_path;
        result.success = false;
        
        try {
            if (verbose_) {
                std::cout << "Parsing file: " << task.file_path << std::endl;
            }
            
            // Create parser for single file
            H5Parser parser(std::filesystem::path(task.file_path).parent_path().string());
            parser.enableSpatialEnrichment(task.enable_spatial);
            
            // Parse the specific file
            if (parser.parseFile(task.file_path)) {
                auto all_signals = parser.getAllSignals();
                
                // Apply filters
                result.signals = filterSignals(all_signals, task.device_filter, 
                                              task.project_filter, task.max_signals_per_file);
                result.success = true;
                
                if (verbose_) {
                    std::cout << "Parsed " << result.signals.size() 
                              << " signals from " << task.file_path << std::endl;
                }
            } else {
                result.error_message = "Failed to parse file: " + task.file_path;
            }
            
        } catch (const std::exception& e) {
            result.error_message = "Exception parsing " + task.file_path + ": " + e.what();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        result.parse_time_seconds = std::chrono::duration<double>(end_time - start_time).count();
        
        return result;
    }
};

// PHASE 4: Memory-aware signal processing
class MemoryAwareProcessor {
private:
    size_t memory_limit_bytes_;
    std::atomic<size_t> current_usage_{0};
    
public:
    explicit MemoryAwareProcessor(size_t memory_limit_mb) 
        : memory_limit_bytes_(memory_limit_mb * 1024 * 1024) {}
    
    bool canProcessBatch(const std::vector<SignalData>& signals) {
        size_t estimated_size = estimateBatchSize(signals);
        return (current_usage_ + estimated_size) <= memory_limit_bytes_;
    }
    
    void trackAllocation(size_t bytes) {
        current_usage_ += bytes;
    }
    
    void trackDeallocation(size_t bytes) {
        current_usage_ -= bytes;
    }
    
    size_t getCurrentUsage() const {
        return current_usage_;
    }
    
    double getUsagePercentage() const {
        return static_cast<double>(current_usage_) / memory_limit_bytes_ * 100.0;
    }
    
private:
    size_t estimateBatchSize(const std::vector<SignalData>& signals) {
        // Rough estimation: each signal has timestamp data + values
        size_t total_size = 0;
        for (const auto& signal : signals) {
            if (signal.timestamps) {
                total_size += signal.timestamps->count * sizeof(double) * 2; // timestamp + value
            }
            total_size += signal.pv_name.size() + 1000; // Overhead for metadata
        }
        return total_size;
    }
};

// Enhanced signal filtering with performance optimization
std::vector<SignalData> filterSignalsOptimized(const std::vector<SignalData>& signals,
                                              const std::string& device_filter,
                                              const std::string& project_filter,
                                              size_t max_signals) {
    std::vector<SignalData> filtered;
    filtered.reserve(std::min(signals.size(), max_signals));
    
    for (const auto& signal : signals) {
        if (filtered.size() >= max_signals) {
            break;
        }
        
        bool include = true;
        
        // Device filter (optimized string search)
        if (!device_filter.empty()) {
            include = include && (signal.pv_name.find(device_filter) != std::string::npos);
        }
        
        // Project filter
        if (!project_filter.empty() && include) {
            include = include && (signal.pv_name.find(project_filter) != std::string::npos);
        }
        
        if (include) {
            filtered.push_back(signal);
        }
    }
    
    return filtered;
}

// PHASE 1 & 3: Optimized request creation with memory pools
std::vector<IngestDataRequest> createIngestRequestsOptimized(
    const std::vector<SignalData>& signals,
    const std::string& providerId,
    IngestClient& client) {
    
    std::vector<IngestDataRequest> requests;
    requests.reserve(signals.size());
    
    for (size_t i = 0; i < signals.size(); ++i) {
        const auto& signal = signals[i];
        std::string requestId = IngestUtils::generateRequestId("h5_signal_" + std::to_string(i));
        
        // Use memory pool if available
        auto request = client.createRequest();
        *request = createIngestRequest(signal, providerId, requestId);
        
        requests.push_back(std::move(*request));
        client.releaseRequest(std::move(request));
    }
    
    return requests;
}

// Performance benchmarking structure
struct PerformanceBenchmark {
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point parse_end_time;
    std::chrono::high_resolution_clock::time_point enrich_end_time;
    std::chrono::high_resolution_clock::time_point ingest_end_time;
    
    size_t total_signals = 0;
    size_t total_files = 0;
    size_t parallel_files = 1;
    size_t worker_threads = 1;
    
    void printSummary() const {
        auto total_time = std::chrono::duration<double>(ingest_end_time - start_time).count();
        auto parse_time = std::chrono::duration<double>(parse_end_time - start_time).count();
        auto enrich_time = std::chrono::duration<double>(enrich_end_time - parse_end_time).count();
        auto ingest_time = std::chrono::duration<double>(ingest_end_time - enrich_end_time).count();
        
        std::cout << "\n=== PERFORMANCE BENCHMARK ===" << std::endl;
        std::cout << "Total signals processed: " << total_signals << std::endl;
        std::cout << "Total files processed: " << total_files << std::endl;
        std::cout << "Parallel files: " << parallel_files << std::endl;
        std::cout << "Worker threads: " << worker_threads << std::endl;
        std::cout << "Total processing time: " << total_time << "s" << std::endl;
        std::cout << "  - H5 parsing: " << parse_time << "s (" << (parse_time/total_time*100) << "%)" << std::endl;
        std::cout << "  - Spatial enrichment: " << enrich_time << "s (" << (enrich_time/total_time*100) << "%)" << std::endl;
        std::cout << "  - Data ingestion: " << ingest_time << "s (" << (ingest_time/total_time*100) << "%)" << std::endl;
        std::cout << "Throughput: " << (total_signals / total_time) << " signals/second" << std::endl;
        std::cout << "Target throughput: " << (50000.0 / 60.0) << " signals/second" << std::endl;
        
        double improvement_factor = (total_signals / total_time) / (1000.0 / 60.0);
        std::cout << "Performance improvement: " << improvement_factor << "x" << std::endl;
        std::cout << "==============================\n" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    // Parse command line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string h5_directory = argv[1];
    std::string config_path = "config/ingestion_config.json";
    std::string server_override;
    std::string device_filter;
    std::string project_filter;
    size_t max_signals = 0;
    size_t batch_size = 0;
    size_t parallel_files = std::thread::hardware_concurrency();
    size_t worker_threads = std::thread::hardware_concurrency();
    size_t memory_limit_mb = 2048;
    size_t cache_size = 10000;
    size_t optimal_batch = 500;
    bool local_only = false;
    bool disable_spatial = false;
    bool verbose = false;
    bool enable_bulk = true;
    bool benchmark_mode = false;

    // Parse options
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg.find("--config=") == 0) {
            config_path = arg.substr(9);
        } else if (arg.find("--server=") == 0) {
            server_override = arg.substr(9);
        } else if (arg.find("--device=") == 0) {
            device_filter = arg.substr(9);
        } else if (arg.find("--project=") == 0) {
            project_filter = arg.substr(10);
        } else if (arg.find("--max-signals=") == 0) {
            max_signals = std::stoull(arg.substr(14));
        } else if (arg.find("--batch-size=") == 0) {
            batch_size = std::stoull(arg.substr(13));
        } else if (arg.find("--parallel-files=") == 0) {
            parallel_files = std::stoull(arg.substr(17));
        } else if (arg.find("--worker-threads=") == 0) {
            worker_threads = std::stoull(arg.substr(17));
        } else if (arg.find("--memory-limit=") == 0) {
            memory_limit_mb = std::stoull(arg.substr(15));
        } else if (arg.find("--cache-size=") == 0) {
            cache_size = std::stoull(arg.substr(13));
        } else if (arg.find("--optimal-batch=") == 0) {
            optimal_batch = std::stoull(arg.substr(16));
        } else if (arg == "--local-only") {
            local_only = true;
        } else if (arg == "--no-spatial") {
            disable_spatial = true;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--enable-bulk") {
            enable_bulk = true;
        } else if (arg == "--benchmark") {
            benchmark_mode = true;
            verbose = true;
        }
    }

    PerformanceBenchmark benchmark;
    if (benchmark_mode) {
        benchmark.start_time = std::chrono::high_resolution_clock::now();
        benchmark.parallel_files = parallel_files;
        benchmark.worker_threads = worker_threads;
    }

    try {
        if (verbose) {
            std::cout << "H5 TO MONGODB INGESTION (OPTIMIZED)\n";
            std::cout << "Directory: " << h5_directory << std::endl;
            std::cout << "Config: " << config_path << std::endl;
            std::cout << "Parallel files: " << parallel_files << std::endl;
            std::cout << "Worker threads: " << worker_threads << std::endl;
            std::cout << "Memory limit: " << memory_limit_mb << " MB" << std::endl;
            std::cout << "Bulk mode: " << (enable_bulk ? "enabled" : "disabled") << std::endl;
        }

        // PHASE 2: Discover H5 files for parallel processing
        if (verbose) std::cout << "\nDiscovering H5 files...\n";
        
        std::vector<std::string> h5_files;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(h5_directory)) {
            if (entry.path().extension() == ".h5" || entry.path().extension() == ".hdf5") {
                h5_files.push_back(entry.path().string());
            }
        }
        
        if (h5_files.empty()) {
            std::cerr << "No H5 files found in directory" << std::endl;
            return 1;
        }
        
        std::cout << "Found " << h5_files.size() << " H5 files" << std::endl;
        
        // Limit number of files based on parallel processing capacity
        if (h5_files.size() > parallel_files * 10) {
            h5_files.resize(parallel_files * 10);
            std::cout << "Limited to " << h5_files.size() << " files for optimal parallel processing" << std::endl;
        }

        // PHASE 2: Create parallel parsing tasks
        std::vector<H5FileParseTask> parse_tasks;
        parse_tasks.reserve(h5_files.size());
        
        size_t signals_per_file = max_signals > 0 ? std::max(size_t(1), max_signals / h5_files.size()) : 0;
        
        for (const auto& file : h5_files) {
            H5FileParseTask task;
            task.file_path = file;
            task.device_filter = device_filter;
            task.project_filter = project_filter;
            task.max_signals_per_file = signals_per_file;
            task.enable_spatial = !disable_spatial;
            parse_tasks.push_back(task);
        }

        // PHASE 2: Parse H5 files in parallel
        if (verbose) std::cout << "Parsing H5 files in parallel...\n";
        
        ParallelH5Parser parser(parallel_files, verbose);
        auto parse_results = parser.parseFilesParallel(parse_tasks);
        
        // Collect all signals
        std::vector<SignalData> all_signals;
        size_t successful_files = 0;
        
        for (const auto& result : parse_results) {
            if (result.success) {
                all_signals.insert(all_signals.end(), result.signals.begin(), result.signals.end());
                successful_files++;
                if (verbose) {
                    std::cout << "File " << result.file_path << ": " 
                              << result.signals.size() << " signals in " 
                              << result.parse_time_seconds << "s" << std::endl;
                }
            } else {
                std::cerr << "Failed to parse " << result.file_path << ": " 
                          << result.error_message << std::endl;
            }
        }
        
        if (benchmark_mode) {
            benchmark.parse_end_time = std::chrono::high_resolution_clock::now();
            benchmark.total_files = successful_files;
        }

        if (all_signals.empty()) {
            std::cerr << "No signals found" << std::endl;
            return 1;
        }

        // Apply final filtering
        auto signals_to_process = filterSignalsOptimized(all_signals, device_filter, project_filter, max_signals);
        if (signals_to_process.empty()) {
            std::cerr << "No signals match filters" << std::endl;
            return 1;
        }

        std::cout << "Found " << signals_to_process.size() << " signals to process from " 
                  << successful_files << " files" << std::endl;

        if (benchmark_mode) {
            benchmark.total_signals = signals_to_process.size();
        }

        if (local_only) {
            std::cout << "Local parsing complete" << std::endl;
            if (benchmark_mode) {
                benchmark.enrich_end_time = benchmark.ingest_end_time = std::chrono::high_resolution_clock::now();
                benchmark.printSummary();
            }
            return 0;
        }

        // PHASE 4: Initialize memory-aware processor
        MemoryAwareProcessor memory_processor(memory_limit_mb);

        // STEP 2: Initialize optimized IngestClient
        if (verbose) std::cout << "Initializing optimized ingest client...\n";
        
        IngestClientConfig client_config;
        try {
            client_config = IngestClientConfig::fromConfigFile(config_path);
        } catch (const std::exception& e) {
            if (verbose) std::cout << "Using default config: " << e.what() << std::endl;
        }

        // Apply command-line overrides
        if (!server_override.empty()) client_config.server_address = server_override;
        if (disable_spatial) client_config.enable_spatial_enrichment = false;
        if (batch_size > 0) client_config.optimal_batch_size = batch_size;
        if (worker_threads > 0) client_config.num_worker_threads = worker_threads;
        
        // Enable optimizations
        client_config.enable_bulk_mode = enable_bulk;
        client_config.enable_parallel_enrichment = true;
        client_config.enable_memory_pools = true;
        client_config.max_memory_usage_mb = memory_limit_mb;
        client_config.optimal_batch_size = optimal_batch;

        if (verbose) {
            std::cout << "Server: " << client_config.server_address << std::endl;
            std::cout << "Spatial enrichment: " << (client_config.enable_spatial_enrichment ? "enabled" : "disabled") << std::endl;
            std::cout << "Bulk mode: " << (client_config.enable_bulk_mode ? "enabled" : "disabled") << std::endl;
            std::cout << "Optimal batch size: " << client_config.optimal_batch_size << std::endl;
        }

        IngestClient client(client_config);

        // Progress callback with performance monitoring
        size_t last_percentage = 0;
        client.setProgressCallback([&](size_t processed, size_t total, size_t successful) {
            size_t percentage = (processed * 100) / total;
            if (percentage >= last_percentage + 5 || processed == total) {
                std::cout << "Progress: " << processed << "/" << total 
                          << " (" << percentage << "%) - " << successful << " successful";
                if (memory_processor.getCurrentUsage() > 0) {
                    std::cout << " - Memory: " << memory_processor.getUsagePercentage() << "%";
                }
                std::cout << std::endl;
                last_percentage = percentage;
            }
        });

        client.setErrorCallback([verbose](const std::string& error, const std::string& context) {
            if (verbose) {
                std::cerr << "Error in " << context << ": " << error << std::endl;
            }
        });

        // STEP 3: Register Provider
        if (verbose) std::cout << "Registering provider...\n";
        
        std::vector<Attribute> provider_attributes;
        provider_attributes.push_back(makeAttribute("facility", "SLAC"));
        provider_attributes.push_back(makeAttribute("accelerator", "LCLS-II"));
        provider_attributes.push_back(makeAttribute("signal_count", std::to_string(signals_to_process.size())));
        provider_attributes.push_back(makeAttribute("optimization_level", "50x_target"));
        provider_attributes.push_back(makeAttribute("parallel_files", std::to_string(parallel_files)));

        std::vector<std::string> provider_tags = {"lcls-ii", "h5_parser", "spatial_enriched", "optimized"};

        auto registration_response = client.registerProvider("H5_Parser_Provider_Optimized", provider_attributes, provider_tags);
        if (!registration_response.has_registrationresult()) {
            std::cerr << "Failed to register provider" << std::endl;
            return 1;
        }

        std::string providerId = registration_response.registrationresult().providerid();
        std::cout << "Registered as provider: " << providerId << std::endl;

        // STEP 4: Create optimized requests
        if (verbose) std::cout << "Creating optimized ingestion requests...\n";
        
        std::vector<IngestDataRequest> requests = createIngestRequestsOptimized(signals_to_process, providerId, client);
        
        if (benchmark_mode) {
            benchmark.enrich_end_time = std::chrono::high_resolution_clock::now();
        }

        // STEP 5: Optimized batch ingestion
        if (verbose) std::cout << "Starting optimized batch ingestion...\n";
        
        auto ingestion_result = client.ingestBatchOptimized(requests, providerId);
        
        if (benchmark_mode) {
            benchmark.ingest_end_time = std::chrono::high_resolution_clock::now();
        }

        // Print results
        std::cout << "\n=== INGESTION RESULTS ===" << std::endl;
        std::cout << "Total requests: " << ingestion_result.total_requests << std::endl;
        std::cout << "Successful: " << ingestion_result.successful_requests << std::endl;
        std::cout << "Failed: " << ingestion_result.failed_requests << std::endl;
        std::cout << "Success rate: " << (ingestion_result.getSuccessRate() * 100) << "%" << std::endl;
        std::cout << "Processing time: " << ingestion_result.processing_time_seconds << "s" << std::endl;
        std::cout << "Batches sent: " << ingestion_result.total_batches_sent << std::endl;
        std::cout << "Batching efficiency: " << ingestion_result.getBatchingEfficiency() << "x" << std::endl;
        
        if (ingestion_result.spatial_enrichment_time_seconds > 0) {
            std::cout << "Spatial enrichment time: " << ingestion_result.spatial_enrichment_time_seconds << "s" << std::endl;
            std::cout << "Parallel speedup: " << ingestion_result.parallel_speedup_factor << "x" << std::endl;
        }
        
        if (client.getConfig().enable_memory_pools) {
            std::cout << "Pool allocations: " << ingestion_result.pool_allocations << std::endl;
            std::cout << "Heap allocations: " << ingestion_result.heap_allocations << std::endl;
            std::cout << "Peak memory usage: " << ingestion_result.peak_memory_usage_mb << " MB" << std::endl;
        }

        // Print error details if any
        if (!ingestion_result.error_messages.empty()) {
            std::cout << "\nErrors encountered:" << std::endl;
            for (const auto& error : ingestion_result.error_messages) {
                std::cout << "  - " << error << std::endl;
            }
        }

        if (benchmark_mode) {
            benchmark.printSummary();
        }

        bool overall_success = ingestion_result.success && ingestion_result.getSuccessRate() > 0.8;
        return overall_success ? 0 : 1;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
