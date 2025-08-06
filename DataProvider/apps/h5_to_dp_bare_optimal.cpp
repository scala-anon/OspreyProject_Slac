#include "clients/ingest_client.hpp"
#include "clients/common_client.hpp"
#include <H5Cpp.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <limits>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <sys/times.h>
#include <unistd.h>

// Basic optimized settings
constexpr size_t WORKER_THREADS = 4;
constexpr size_t BATCH_SIZE = 200;  // Sweet spot - 6x larger than original but not overwhelming
constexpr size_t IO_BUFFER_SIZE = 4 * 1024 * 1024; // 4MB

// Thread-safe HDF5 global mutex (critical for non-thread-safe HDF5)
static std::mutex hdf5_global_mutex_;

// Simple thread pool for file processing
class SimpleThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};

public:
    SimpleThreadPool() {
        size_t thread_count = std::min(WORKER_THREADS, static_cast<size_t>(std::thread::hardware_concurrency()));

        for (size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this] {
                while (!stop_) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

                        if (stop_ && tasks_.empty()) return;

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::forward<F>(f));
        }
        condition_.notify_one();
    }

    ~SimpleThreadPool() {
        stop_ = true;
        condition_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) worker.join();
        }
    }
};

// Statistics tracking
struct Stats {
    std::atomic<size_t> files_processed{0};
    std::atomic<size_t> files_failed{0};
    std::atomic<size_t> signals_processed{0};
    std::chrono::steady_clock::time_point start_time;

    Stats() : start_time(std::chrono::steady_clock::now()) {}
};

// Optimized data reading with NaN preservation
std::vector<double> readSignalData(H5::H5File& file, const std::string& signal_name) {
    std::vector<double> data;

    try {
        H5::DataSet dataset = file.openDataSet(signal_name);
        H5::DataSpace dataspace = dataset.getSpace();

        hsize_t dims[1];
        dataspace.getSimpleExtentDims(dims);

        if (dims[0] == 0 || dims[0] > 10000000) return data; // Sanity check

        data.resize(dims[0]);

        // Try double first
        try {
            dataset.read(data.data(), H5::PredType::NATIVE_DOUBLE);
        } catch (...) {
            // Try float
            try {
                std::vector<float> float_data(dims[0]);
                dataset.read(float_data.data(), H5::PredType::NATIVE_FLOAT);
                for (size_t i = 0; i < dims[0]; i++) {
                    data[i] = static_cast<double>(float_data[i]);
                }
            } catch (...) {
                // Fill with NaN if all else fails
                std::fill(data.begin(), data.end(), std::numeric_limits<double>::quiet_NaN());
            }
        }

        dataset.close();
        dataspace.close();

    } catch (...) {
        data.clear();
    }

    return data;
}

// Optimized timestamp reading
std::vector<uint64_t> readTimestamps(H5::H5File& file) {
    std::vector<uint64_t> timestamps;

    try {
        if (file.nameExists("secondsPastEpoch")) {
            H5::DataSet seconds_ds = file.openDataSet("secondsPastEpoch");
            H5::DataSpace space = seconds_ds.getSpace();

            hsize_t dims[1];
            space.getSimpleExtentDims(dims);

            if (dims[0] > 0 && dims[0] <= 10000000) {
                timestamps.resize(dims[0]);
                seconds_ds.read(timestamps.data(), H5::PredType::NATIVE_UINT64);
            }

            seconds_ds.close();
            space.close();
        }
    } catch (...) {
        timestamps.clear();
    }

    return timestamps;
}

// Process single H5 file with thread safety
bool processFile(const std::string& filepath, const std::string& provider_id,
                IngestionClient* client, Stats& stats) {
    try {
        // CRITICAL: All HDF5 operations must be serialized
        std::lock_guard<std::mutex> hdf5_lock(hdf5_global_mutex_);

        H5::H5File file(filepath, H5F_ACC_RDONLY);

        // Read timestamps
        auto timestamps = readTimestamps(file);
        if (timestamps.empty()) {
            file.close();
            stats.files_failed++;
            return false;
        }

        // Get signal names
        std::vector<std::string> signal_names;
        try {
            H5::Group root = file.openGroup("/");
            hsize_t num_objects = root.getNumObjs();

            for (hsize_t i = 0; i < num_objects; i++) {
                if (root.getObjTypeByIdx(i) == H5G_DATASET) {
                    std::string obj_name = root.getObjnameByIdx(i);
                    if (obj_name != "secondsPastEpoch" && obj_name != "nanoseconds") {
                        signal_names.push_back(obj_name);
                    }
                }
            }
            root.close();
        } catch (...) {
            file.close();
            stats.files_failed++;
            return false;
        }

        if (signal_names.empty()) {
            file.close();
            stats.files_failed++;
            return false;
        }

        // Get common client for helper functions
        auto& common = client->GetCommonClient();

        // Calculate timing parameters using CommonClient
        uint64_t start_sec = timestamps[0];
        uint64_t period_nanos = (timestamps.size() > 1) ?
            ((timestamps[1] - timestamps[0]) * 1000000000ULL) : 1000000000ULL;

        // Validate period (100 Hz to 10 MHz range)
        if (period_nanos < 100 || period_nanos > 10000000000ULL) {
            period_nanos = 1000000000ULL; // 1 Hz fallback
        }

        // Create timestamps using CommonClient
        auto start_ts = common.CreateTimestamp(start_sec, 0);

        // Process signals in batches
        for (size_t batch_start = 0; batch_start < signal_names.size(); batch_start += BATCH_SIZE) {
            size_t batch_end = std::min(batch_start + BATCH_SIZE, signal_names.size());

            for (size_t i = batch_start; i < batch_end; i++) {
                const auto& signal_name = signal_names[i];

                // Read signal data
                auto values = readSignalData(file, signal_name);
                if (values.empty()) continue;

                // Create request ID
                std::string requestId = signal_name + "_" + std::to_string(i) + "_" +
                                      std::to_string(std::time(nullptr));

                // Create data values using CommonClient (preserving NaNs)
                std::vector<DataValue> dataValues;
                dataValues.reserve(values.size());
                for (double value : values) {
                    dataValues.push_back(common.CreateDoubleValue(value));
                }

                // Create data column using CommonClient
                auto dataColumn = common.CreateDataColumn(signal_name, dataValues);

                // Create sampling clock using CommonClient
                auto samplingClock = common.CreateSamplingClock(
                    start_ts, period_nanos, static_cast<uint32_t>(values.size()));

                // Create data timestamps
                auto dataTimestamps = common.CreateDataTimestampsFromClock(samplingClock);

                // Create ingestion data frame
                auto dataFrame = client->CreateDataFrame(dataTimestamps, {dataColumn});

                // Send single request using new client API
                bool success = client->IngestData(
                    provider_id,
                    requestId,
                    {dataColumn},
                    dataTimestamps,
                    {"h5_data", "optimized"},  // tags
                    {}  // attributes
                );

                if (success) {
                    stats.signals_processed++;
                }
            }
        }

        file.close();
        stats.files_processed++;
        return true;

    } catch (...) {
        stats.files_failed++;
        return false;
    }
}

// Recursive directory traversal for year/month/day structure OR direct directory
std::vector<std::string> findH5Files(const std::string& root_directory) {
    std::vector<std::string> h5_files;

    try {
        // First, check if there are H5 files directly in the provided directory
        for (const auto& entry : std::filesystem::directory_iterator(root_directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".h5") {
                h5_files.push_back(entry.path().string());
            }
        }

        // If we found H5 files directly, return them
        if (!h5_files.empty()) {
            return h5_files;
        }

        // Otherwise, assume year/month/day structure
        for (const auto& year_entry : std::filesystem::directory_iterator(root_directory)) {
            if (!year_entry.is_directory()) continue;

            for (const auto& month_entry : std::filesystem::directory_iterator(year_entry.path())) {
                if (!month_entry.is_directory()) continue;

                for (const auto& day_entry : std::filesystem::directory_iterator(month_entry.path())) {
                    if (!day_entry.is_directory()) continue;

                    // Find H5 files in day directory
                    for (const auto& file_entry : std::filesystem::directory_iterator(day_entry.path())) {
                        if (file_entry.is_regular_file() &&
                            file_entry.path().extension() == ".h5") {
                            h5_files.push_back(file_entry.path().string());
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directories: " << e.what() << std::endl;
    }

    return h5_files;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory> [--collection-suffix=YYYY_MM]" << std::endl;
        std::cout << "Supports: Direct directory with .h5 files OR year/month/day structure" << std::endl;
        return 1;
    }

    std::string root_directory = argv[1];
    std::string collection_suffix = "";

    // Parse command line arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.substr(0, 20) == "--collection-suffix=") {
            collection_suffix = arg.substr(20);
        }
    }

    // Capture start times for detailed timing
    auto wall_start = std::chrono::high_resolution_clock::now();
    struct tms tms_start;
    clock_t start_times = times(&tms_start);

    try {
        // Create timestamped provider name
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "ingest_data_optimized_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        std::string provider_name = ss.str();

        // Setup client using new structure
        IngestionClient client("localhost:50051");
        auto& common = client.GetCommonClient();

        std::vector<Attribute> attrs;
        attrs.push_back(common.CreateAttribute("source", "optimized_h5_parser"));
        attrs.push_back(common.CreateAttribute("threading", "file_level_parallel"));

        std::vector<std::string> tags = {"h5_data", "optimized"};
        
        auto provider_id = client.RegisterProvider(provider_name, 
                                                   "Optimized H5 data processor", 
                                                   tags, attrs);

        if (!provider_id.has_value()) {
            std::cerr << "Failed to register provider" << std::endl;
            return 1;
        }

        std::cout << "Provider registered: " << provider_id.value() << std::endl;

        // Find all H5 files
        std::cout << "Scanning directory structure..." << std::endl;
        auto h5_files = findH5Files(root_directory);

        if (h5_files.empty()) {
            std::cout << "No H5 files found!" << std::endl;
            return 0;
        }

        std::cout << "Found " << h5_files.size() << " H5 files" << std::endl;

        // Sort by size (smaller files first for faster initial progress)
        std::sort(h5_files.begin(), h5_files.end(), [](const std::string& a, const std::string& b) {
            try {
                return std::filesystem::file_size(a) < std::filesystem::file_size(b);
            } catch (...) {
                return a < b;
            }
        });

        // Process files with thread pool
        Stats stats;
        SimpleThreadPool thread_pool;
        std::atomic<size_t> completed{0};

        std::cout << "Processing with " << WORKER_THREADS << " threads..." << std::endl;

        for (const auto& filepath : h5_files) {
            thread_pool.enqueue([&, filepath]() {
                processFile(filepath, provider_id.value(), &client, stats);

                size_t count = completed.fetch_add(1) + 1;

                if (count % 10 == 0 || count == h5_files.size()) {
                    auto elapsed = std::chrono::steady_clock::now() - stats.start_time;
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
                    double rate = seconds > 0 ? static_cast<double>(count) / seconds : 0.0;

                    std::cout << "\rProgress: " << count << "/" << h5_files.size()
                              << " (" << std::fixed << std::setprecision(1)
                              << (100.0 * count / h5_files.size()) << "%) "
                              << "Rate: " << std::setprecision(1) << rate << " files/s "
                              << "Signals: " << stats.signals_processed.load()
                              << " Failed: " << stats.files_failed.load() << std::flush;
                }
            });
        }

        // Wait for completion
        while (completed.load() < h5_files.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Final stats with detailed timing
        auto wall_end = std::chrono::high_resolution_clock::now();
        struct tms tms_end;
        clock_t end_times = times(&tms_end);

        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(wall_end - wall_start);
        auto total_seconds = std::chrono::duration_cast<std::chrono::seconds>(total_duration).count();

        // Calculate CPU times
        long ticks_per_sec = sysconf(_SC_CLK_TCK);
        double user_time = static_cast<double>(tms_end.tms_utime - tms_start.tms_utime) / ticks_per_sec;
        double sys_time = static_cast<double>(tms_end.tms_stime - tms_start.tms_stime) / ticks_per_sec;
        double real_time = static_cast<double>(total_duration.count()) / 1000.0;

        std::cout << "\n\nCompleted!" << std::endl;
        std::cout << "Files processed: " << stats.files_processed.load() << std::endl;
        std::cout << "Files failed: " << stats.files_failed.load() << std::endl;
        std::cout << "Total signals: " << stats.signals_processed.load() << std::endl;
        std::cout << "Processing time: " << total_seconds << " seconds" << std::endl;

        if (total_seconds > 0) {
            std::cout << "Average rate: " << std::setprecision(2)
                      << static_cast<double>(stats.files_processed.load()) / total_seconds
                      << " files/second" << std::endl;
        }

        // Print timing in same format as `time` command
        std::cout << "\nDetailed Timing:" << std::endl;
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "real\t" << real_time << "s" << std::endl;
        std::cout << "user\t" << user_time << "s" << std::endl;
        std::cout << "sys\t" << sys_time << "s" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
