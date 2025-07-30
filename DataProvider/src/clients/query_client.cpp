#include "query_client.hpp"
#include <grpcpp/create_channel.h>
#include <grpcpp/client_context.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <regex>
#include <algorithm>

using json = nlohmann::json;

// ===================== TimeRange Implementation =====================

Timestamp TimeRange::getStartTimestamp() const {
    return makeTimestamp(start_epoch_sec, start_nano);
}

Timestamp TimeRange::getEndTimestamp() const {
    return makeTimestamp(end_epoch_sec, end_nano);
}

// ===================== QueryClientConfig Implementation =====================

QueryClientConfig QueryClientConfig::fromConfigFile(const std::string& config_path) {
    QueryClientConfig config;
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open config file: " << config_path 
                      << ". Using defaults." << std::endl;
            return config;
        }
        
        json j;
        file >> j;
        return fromJson(j);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error parsing config file: " << e.what() 
                  << ". Using defaults." << std::endl;
        return config;
    }
}

QueryClientConfig QueryClientConfig::fromJson(const json& j) {
    QueryClientConfig config;
    
    if (j.contains("server_connections")) {
        const auto& conn = j["server_connections"];
        if (conn.contains("query_server")) {
            config.server_address = conn["query_server"];
        }
        if (conn.contains("connection_timeout_seconds")) {
            config.connection_timeout_seconds = conn["connection_timeout_seconds"];
        }
        if (conn.contains("max_message_size_mb")) {
            config.max_message_size_mb = conn["max_message_size_mb"];
        }
    }
    
    if (j.contains("spatial_enrichment")) {
        const auto& spatial = j["spatial_enrichment"];
        if (spatial.contains("enabled")) {
            config.enable_spatial_queries = spatial["enabled"];
        }
        if (spatial.contains("dictionaries_path")) {
            config.dictionaries_path = spatial["dictionaries_path"];
        }
    }
    
    return config;
}

// ===================== QueryClient Implementation =====================

QueryClient::QueryClient(const QueryClientConfig& config) 
    : config_(config), spatial_queries_enabled_(config.enable_spatial_queries) {
    initializeConnection();
    if (config_.enable_spatial_queries) {
        initializeSpatialEngine();
    }
}

QueryClient::QueryClient(const std::string& server_address) 
    : QueryClient(QueryClientConfig{}) {
    config_.server_address = server_address;
    initializeConnection();
}

QueryClient::QueryClient(const std::string& config_path, bool is_config_file) 
    : QueryClient(is_config_file ? 
                   QueryClientConfig::fromConfigFile(config_path) : 
                   QueryClientConfig{}) {
    if (!is_config_file) {
        config_.server_address = config_path;
        initializeConnection();
    }
}

QueryClient::~QueryClient() = default;

void QueryClient::initializeConnection() {
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(config_.max_message_size_mb * 1024 * 1024);
    args.SetMaxSendMessageSize(config_.max_message_size_mb * 1024 * 1024);

    auto channel = grpc::CreateCustomChannel(
        config_.server_address,
        grpc::InsecureChannelCredentials(),
        args
    );

    if (!channel->WaitForConnected(
        std::chrono::system_clock::now() + 
        std::chrono::seconds(config_.connection_timeout_seconds))) {
        std::string error = "Failed to connect to MLDP server at " + config_.server_address;
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error("gRPC channel connection timeout");
    }

    stub_ = dp::service::query::DpQueryService::NewStub(channel);
}

void QueryClient::initializeSpatialEngine() {
    try {
        spatial_engine_ = std::make_unique<SpatialQueryEngine>(config_.dictionaries_path);
        if (!spatial_engine_->loadDictionaries()) {
            std::cerr << "Warning: Failed to load spatial dictionaries. "
                      << "Spatial queries will be disabled." << std::endl;
            spatial_queries_enabled_ = false;
            spatial_engine_.reset();
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error initializing spatial engine: " << e.what()
                  << ". Spatial queries disabled." << std::endl;
        spatial_queries_enabled_ = false;
        spatial_engine_.reset();
    }
}

// ===================== Basic Query Methods (existing) =====================

QueryDataResponse QueryClient::queryData(const QueryDataRequest& request) {
    QueryDataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryData(&context, request, &response);

    if (!status.ok()) {
        std::string error = "QueryData RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

QueryTableResponse QueryClient::queryTable(const QueryTableRequest& request) {
    QueryTableResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryTable(&context, request, &response);

    if (!status.ok()) {
        std::string error = "QueryTable RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

QueryPvMetadataResponse QueryClient::queryPvMetadata(const QueryPvMetadataRequest& request) {
    QueryPvMetadataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryPvMetadata(&context, request, &response);

    if (!status.ok()) {
        std::string error = "QueryPvMetadata RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

QueryProvidersResponse QueryClient::queryProviders(const QueryProvidersRequest& request) {
    QueryProvidersResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryProviders(&context, request, &response);

    if (!status.ok()) {
        std::string error = "QueryProviders RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

QueryProviderMetadataResponse QueryClient::queryProviderMetadata(const QueryProviderMetadataRequest& request) {
    QueryProviderMetadataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryProviderMetadata(&context, request, &response);

    if (!status.ok()) {
        std::string error = "QueryProviderMetadata RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

std::vector<QueryDataResponse> QueryClient::queryDataStream(const QueryDataRequest& request) {
    std::vector<QueryDataResponse> responses;
    grpc::ClientContext context;

    std::unique_ptr<grpc::ClientReader<QueryDataResponse>> reader(
        stub_->queryDataStream(&context, request)
    );

    QueryDataResponse response;
    while (reader->Read(&response)) {
        responses.push_back(response);
    }

    grpc::Status status = reader->Finish();

    if (!status.ok()) {
        std::string error = "QueryDataStream RPC failed: " + std::to_string(status.error_code()) + 
                           ": " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }

    return responses;
}

// ===================== Spatial-Aware Query Methods (new) =====================

QueryResult QueryClient::queryByArea(const std::string& area, const TimeRange& time_range, 
                                     const std::vector<std::string>& pv_patterns) {
    SpatialQueryParams params;
    params.area = area;
    return spatialQuery(params, time_range, pv_patterns);
}

QueryResult QueryClient::queryByBeamPath(const std::string& beam_path, const TimeRange& time_range,
                                         const std::vector<std::string>& pv_patterns) {
    SpatialQueryParams params;
    params.beam_path = beam_path;
    return spatialQuery(params, time_range, pv_patterns);
}

QueryResult QueryClient::queryByDeviceClass(const std::string& device_class, const TimeRange& time_range,
                                            const std::vector<std::string>& pv_patterns) {
    SpatialQueryParams params;
    params.device_class = device_class;
    return spatialQuery(params, time_range, pv_patterns);
}

QueryResult QueryClient::queryByZRange(double min_z, double max_z, const TimeRange& time_range,
                                       const std::vector<std::string>& pv_patterns) {
    SpatialQueryParams params;
    params.z_range = {min_z, max_z};
    return spatialQuery(params, time_range, pv_patterns);
}

QueryResult QueryClient::queryBySequenceRange(const std::string& beam_path, int min_sequence, int max_sequence,
                                              const TimeRange& time_range, const std::vector<std::string>& pv_patterns) {
    SpatialQueryParams params;
    params.beam_path = beam_path;
    params.area_sequence_min = min_sequence;
    params.area_sequence_max = max_sequence;
    return spatialQuery(params, time_range, pv_patterns);
}

QueryResult QueryClient::spatialQuery(const SpatialQueryParams& spatial_params, const TimeRange& time_range,
                                      const std::vector<std::string>& pv_patterns) {
    QueryResult result;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Get PV names that match spatial criteria
        std::vector<std::string> pv_names = expandPvPatterns(pv_patterns, spatial_params);
        
        if (pv_names.empty()) {
            result.error_message = "No PVs found matching spatial criteria";
            return result;
        }
        
        // Create and execute query
        auto request = createQueryRequest(pv_names, time_range);
        auto responses = queryDataStream(request);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        result.processing_time_seconds = 
            std::chrono::duration<double>(end_time - start_time).count();
        
        result = processQueryResponses(responses, result.processing_time_seconds);
        result.success = true;
        
    } catch (const std::exception& e) {
        result.error_message = e.what();
        result.success = false;
    }
    
    return result;
}

// ===================== PV Discovery Methods =====================

std::vector<std::string> QueryClient::findPvsByArea(const std::string& area, const std::string& pattern) {
    if (spatial_engine_) {
        return spatial_engine_->findPvsByArea(area, pattern);
    }
    return {};
}

std::vector<std::string> QueryClient::findPvsByBeamPath(const std::string& beam_path, const std::string& pattern) {
    if (spatial_engine_) {
        return spatial_engine_->findPvsByBeamPath(beam_path, pattern);
    }
    return {};
}

std::vector<std::string> QueryClient::findPvsByDeviceClass(const std::string& device_class, const std::string& pattern) {
    if (spatial_engine_) {
        return spatial_engine_->findPvsByDeviceClass(device_class, pattern);
    }
    return {};
}

std::vector<std::string> QueryClient::getAvailableAreas() {
    if (spatial_engine_) {
        return spatial_engine_->getAvailableAreas();
    }
    return {};
}

std::vector<std::string> QueryClient::getAvailableBeamPaths() {
    if (spatial_engine_) {
        return spatial_engine_->getAvailableBeamPaths();
    }
    return {};
}

std::vector<std::string> QueryClient::getAvailableDeviceClasses() {
    if (spatial_engine_) {
        return spatial_engine_->getAvailableDeviceClasses();
    }
    return {};
}

std::vector<std::string> QueryClient::getAreasInBeamPath(const std::string& beam_path) {
    if (spatial_engine_) {
        return spatial_engine_->getAreasInBeamPath(beam_path);
    }
    return {};
}

// ===================== Configuration and Control =====================

void QueryClient::updateConfig(const QueryClientConfig& config) {
    config_ = config;
    initializeConnection();
    if (config_.enable_spatial_queries) {
        initializeSpatialEngine();
    }
}

void QueryClient::enableSpatialQueries(bool enable) {
    if (enable && !spatial_engine_) {
        initializeSpatialEngine();
    }
    spatial_queries_enabled_ = enable && (spatial_engine_ != nullptr);
}

bool QueryClient::isSpatialQueriesEnabled() const {
    return spatial_queries_enabled_ && (spatial_engine_ != nullptr);
}

bool QueryClient::isConnected() const {
    return stub_ != nullptr;
}

void QueryClient::reconnect() {
    initializeConnection();
}

// ===================== Internal Helper Methods =====================

std::vector<std::string> QueryClient::expandPvPatterns(const std::vector<std::string>& patterns, 
                                                       const SpatialQueryParams& spatial_params) {
    if (!spatial_engine_) {
        return patterns; // Return patterns as-is if no spatial engine
    }
    
    // Get PVs matching spatial criteria
    auto spatial_pvs = spatial_engine_->findPvsBySpatialParams(spatial_params);
    
    if (patterns.empty()) {
        return spatial_pvs; // Return all spatial matches if no patterns specified
    }
    
    // Filter spatial PVs by patterns
    std::vector<std::string> filtered_pvs;
    for (const auto& pattern : patterns) {
        std::regex regex_pattern(pattern);
        for (const auto& pv : spatial_pvs) {
            if (std::regex_match(pv, regex_pattern)) {
                filtered_pvs.push_back(pv);
            }
        }
    }
    
    return filtered_pvs;
}

QueryDataRequest QueryClient::createQueryRequest(const std::vector<std::string>& pv_names, 
                                                 const TimeRange& time_range, bool use_serialized) {
    return makeQueryDataRequest(pv_names, time_range.getStartTimestamp(), 
                               time_range.getEndTimestamp(), use_serialized);
}

QueryResult QueryClient::processQueryResponses(const std::vector<QueryDataResponse>& responses, 
                                               double processing_time) {
    QueryResult result;
    result.responses = responses;
    result.processing_time_seconds = processing_time;
    result.total_buckets = responses.size();
    
    for (const auto& response : responses) {
        if (response.has_querydata()) {
            for (const auto& bucket : response.querydata().databuckets()) {
                if (bucket.has_datacolumn()) {
                    result.addPvName(bucket.datacolumn().name());
                    result.total_data_points += bucket.datacolumn().datavalues_size();
                }
            }
        }
    }
    
    return result;
}

void QueryClient::notifyProgress(size_t buckets_processed, const std::string& current_pv) const {
    if (progress_callback_) {
        progress_callback_(buckets_processed, current_pv);
    }
}

// ===================== SpatialQueryEngine Stub Implementation =====================
// (Minimal implementation - would be expanded with actual spatial logic)

SpatialQueryEngine::SpatialQueryEngine(const std::string& dictionaries_path)
    : dictionaries_path_(dictionaries_path) {
}

bool SpatialQueryEngine::loadDictionaries() {
    // Stub implementation - would load actual dictionary files
    dictionaries_loaded_ = true;
    return true;
}

std::vector<std::string> SpatialQueryEngine::findPvsByArea(const std::string& area, const std::string& pattern) const {
    // Stub implementation - would return actual PVs for the area
    return {"BPMS:" + area + ":502:TMITBR", "QUAD:" + area + ":620:BCTRL"};
}

std::vector<std::string> SpatialQueryEngine::findPvsByBeamPath(const std::string& beam_path, const std::string& pattern) const {
    // Stub implementation - would return actual PVs for the beam path
    return {"BPMS:DMPH:502:TMITBR", "BPMS:LTUH:755:X", "BPMS:DMPS:693:Y"};
}

std::vector<std::string> SpatialQueryEngine::findPvsByDeviceClass(const std::string& device_class, const std::string& pattern) const {
    // Stub implementation - would return actual PVs for the device class
    if (device_class == "beam_position_monitor") {
        return {"BPMS:DMPH:502:TMITBR", "BPMS:LTUH:755:X", "BPMS:DMPS:693:Y"};
    }
    return {};
}

std::vector<std::string> SpatialQueryEngine::findPvsBySpatialParams(const SpatialQueryParams& params, const std::string& pattern) const {
    // Stub implementation - would combine all spatial criteria
    std::vector<std::string> result;
    
    if (!params.area.empty()) {
        auto area_pvs = findPvsByArea(params.area, pattern);
        result.insert(result.end(), area_pvs.begin(), area_pvs.end());
    }
    
    if (!params.beam_path.empty()) {
        auto beam_pvs = findPvsByBeamPath(params.beam_path, pattern);
        result.insert(result.end(), beam_pvs.begin(), beam_pvs.end());
    }
    
    if (!params.device_class.empty()) {
        auto class_pvs = findPvsByDeviceClass(params.device_class, pattern);
        result.insert(result.end(), class_pvs.begin(), class_pvs.end());
    }
    
    // Remove duplicates
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    
    return result;
}

std::vector<std::string> SpatialQueryEngine::getAvailableAreas() const {
    return {"DMPH", "LTUH", "DMPS", "LI23", "LI24"};
}

std::vector<std::string> SpatialQueryEngine::getAvailableBeamPaths() const {
    return {"CU_HXR", "SC_SXR", "CU_SXR"};
}

std::vector<std::string> SpatialQueryEngine::getAvailableDeviceClasses() const {
    return {"beam_position_monitor", "klystron", "quadrupole", "wire_scanner"};
}

std::vector<std::string> SpatialQueryEngine::getAreasInBeamPath(const std::string& beam_path) const {
    if (beam_path == "CU_HXR") {
        return {"LI20", "LTUH", "DMPH", "DMPS"};
    }
    return {};
}

// ===================== Helper Functions =====================

Timestamp makeTimestamp(uint64_t epoch, uint64_t nano) {
    Timestamp ts;
    ts.set_epochseconds(epoch);
    ts.set_nanoseconds(nano);
    return ts;
}

QueryDataRequest makeQueryDataRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, const Timestamp& endTime, bool useSerializedDataColumns) {
    QueryDataRequest request;
    auto* querySpec = request.mutable_queryspec();

    *querySpec->mutable_begintime() = beginTime;
    *querySpec->mutable_endtime() = endTime;
    querySpec->set_useserializeddatacolumns(useSerializedDataColumns);

    for (const auto& pvName : pvNames) {
        querySpec->add_pvnames(pvName);
    }

    return request;
}

QueryTableRequest makeQueryTableRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, const Timestamp& endTime, dp::service::query::QueryTableRequest::TableResultFormat format) {
    QueryTableRequest request;
    request.set_format(format);
    *request.mutable_begintime() = beginTime;
    *request.mutable_endtime() = endTime;

    auto* pvNameList = request.mutable_pvnamelist();
    for (const auto& pvName : pvNames) {
        pvNameList->add_pvnames(pvName);
    }

    return request;
}

QueryPvMetadataRequest makeQueryPvMetadataRequest(const std::vector<std::string>& pvNames) {
    QueryPvMetadataRequest request;
    auto* pvNameList = request.mutable_pvnamelist();

    for (const auto& pvName : pvNames) {
        pvNameList->add_pvnames(pvName);
    }

    return request;
}

QueryPvMetadataRequest makeQueryPvMetadataRequestWithPattern(const std::string& pattern) {
    QueryPvMetadataRequest request;
    auto* pvNamePattern = request.mutable_pvnamepattern();
    pvNamePattern->set_pattern(pattern);
    return request;
}

QueryProvidersRequest makeQueryProvidersRequest(const std::string& textSearch) {
    QueryProvidersRequest request;

    if (!textSearch.empty()) {
        auto* criterion = request.add_criteria();
        auto* textCriterion = criterion->mutable_textcriterion();
        textCriterion->set_text(textSearch);
    }

    return request;
}

QueryProviderMetadataRequest makeQueryProviderMetadataRequest(const std::string& providerId) {
    QueryProviderMetadataRequest request;
    request.set_providerid(providerId);
    return request;
}

// ===================== Utility Functions =====================

namespace QueryUtils {
    TimeRange createTimeRange(uint64_t start_epoch_sec, uint64_t end_epoch_sec, 
                             uint64_t start_nano, uint64_t end_nano) {
        return TimeRange(start_epoch_sec, start_nano, end_epoch_sec, end_nano);
    }

    TimeRange createTimeRangeFromDuration(uint64_t start_epoch_sec, uint64_t duration_seconds,
                                         uint64_t start_nano) {
        return TimeRange(start_epoch_sec, start_nano, 
                        start_epoch_sec + duration_seconds, start_nano);
    }

    uint64_t getCurrentEpochSeconds() {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    }

    TimeRange createRecentTimeRange(uint64_t duration_seconds) {
        uint64_t now = getCurrentEpochSeconds();
        return TimeRange(now - duration_seconds, 0, now, 0);
    }

    SpatialQueryParams createAreaQuery(const std::string& area) {
        SpatialQueryParams params;
        params.area = area;
        return params;
    }

    SpatialQueryParams createBeamPathQuery(const std::string& beam_path) {
        SpatialQueryParams params;
        params.beam_path = beam_path;
        return params;
    }

    SpatialQueryParams createDeviceClassQuery(const std::string& device_class) {
        SpatialQueryParams params;
        params.device_class = device_class;
        return params;
    }

    SpatialQueryParams createZRangeQuery(double min_z, double max_z) {
        SpatialQueryParams params;
        params.z_range = {min_z, max_z};
        return params;
    }

    SpatialQueryParams createSequenceQuery(const std::string& beam_path, int min_seq, int max_seq) {
        SpatialQueryParams params;
        params.beam_path = beam_path;
        params.area_sequence_min = min_seq;
        params.area_sequence_max = max_seq;
        return params;
    }

    SpatialQueryParams combineSpatialParams(const SpatialQueryParams& base, const SpatialQueryParams& additional) {
        SpatialQueryParams combined = base;
        
        if (!additional.beam_path.empty()) combined.beam_path = additional.beam_path;
        if (!additional.area.empty()) combined.area = additional.area;
        if (!additional.device_class.empty()) combined.device_class = additional.device_class;
        if (!additional.z_range.empty()) combined.z_range = additional.z_range;
        if (additional.area_sequence_min != -1) combined.area_sequence_min = additional.area_sequence_min;
        if (additional.area_sequence_max != -1) combined.area_sequence_max = additional.area_sequence_max;
        
        combined.areas.insert(combined.areas.end(), additional.areas.begin(), additional.areas.end());
        
        return combined;
    }
}
