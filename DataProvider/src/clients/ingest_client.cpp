#include "ingest_client.hpp"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <future>
#include <algorithm>

using json = nlohmann::json;

// ===================== IngestClientConfig Implementation =====================

IngestClientConfig IngestClientConfig::fromConfigFile(const std::string& config_path) {
    IngestClientConfig config;
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            return config;
        }
        json j;
        file >> j;
        return fromJson(j);
    } catch (const std::exception&) {
        return config;
    }
}

IngestClientConfig IngestClientConfig::fromJson(const nlohmann::json& config) {
    IngestClientConfig result;
    
    if (config.contains("server_connections")) {
        const auto& conn = config["server_connections"];
        if (conn.contains("ingestion_server")) {
            result.server_address = conn["ingestion_server"];
        }
        if (conn.contains("connection_timeout_seconds")) {
            result.connection_timeout_seconds = conn["connection_timeout_seconds"];
        }
        if (conn.contains("max_message_size_mb")) {
            result.max_message_size_mb = conn["max_message_size_mb"];
        }
    }
    
    if (config.contains("spatial_enrichment")) {
        const auto& spatial = config["spatial_enrichment"];
        if (spatial.contains("enabled")) {
            result.enable_spatial_enrichment = spatial["enabled"];
        }
        if (spatial.contains("dictionaries_path")) {
            result.dictionaries_path = spatial["dictionaries_path"];
        }
    }
    
    if (config.contains("ingestion_settings")) {
        const auto& ingest = config["ingestion_settings"];
        if (ingest.contains("default_batch_size")) {
            result.default_batch_size = ingest["default_batch_size"];
        }
        if (ingest.contains("max_batch_size")) {
            result.max_batch_size = ingest["max_batch_size"];
        }
        if (ingest.contains("streaming_preferred")) {
            result.streaming_preferred = ingest["streaming_preferred"];
        }
        if (ingest.contains("retry_attempts")) {
            result.retry_attempts = ingest["retry_attempts"];
        }
        if (ingest.contains("retry_delay_ms")) {
            result.retry_delay_ms = ingest["retry_delay_ms"];
        }
        if (ingest.contains("enable_progress_monitoring")) {
            result.enable_progress_monitoring = ingest["enable_progress_monitoring"];
        }
    }
    
    return result;
}

// ===================== IngestClient Implementation =====================

IngestClient::IngestClient(const IngestClientConfig& config) 
    : config_(config), spatial_enrichment_enabled_(config.enable_spatial_enrichment) {
    initializeConnection();
    if (config_.enable_spatial_enrichment) {
        initializeSpatialAnalyzer();
    }
}

IngestClient::IngestClient(const std::string& config_file_path) 
    : IngestClient(IngestClientConfig::fromConfigFile(config_file_path)) {}

IngestClient::~IngestClient() = default;

void IngestClient::initializeConnection() {
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(config_.max_message_size_mb * 1024 * 1024);
    args.SetMaxSendMessageSize(config_.max_message_size_mb * 1024 * 1024);

    auto channel = grpc::CreateCustomChannel(config_.server_address, grpc::InsecureChannelCredentials(), args);

    if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(config_.connection_timeout_seconds))) {
        throw std::runtime_error("Failed to connect to server at " + config_.server_address);
    }

    stub_ = dp::service::ingestion::DpIngestionService::NewStub(channel);
}

void IngestClient::initializeSpatialAnalyzer() {
    try {
        spatial_analyzer_ = std::make_unique<SpatialAnalyzer>(std::thread::hardware_concurrency());
        if (!spatial_analyzer_->loadDictionaries(config_.dictionaries_path)) {
            spatial_enrichment_enabled_ = false;
            spatial_analyzer_.reset();
        }
    } catch (const std::exception&) {
        spatial_enrichment_enabled_ = false;
        spatial_analyzer_.reset();
    }
}

RegisterProviderResponse IngestClient::registerProvider(const RegisterProviderRequest& request) {
    RegisterProviderResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->registerProvider(&context, request, &response);

    if (!status.ok()) {
        throw std::runtime_error("RegisterProvider RPC failed: " + status.error_message());
    }
    return response;
}

RegisterProviderResponse IngestClient::registerProvider(const std::string& name,
                                                       const std::vector<Attribute>& attributes,
                                                       const std::vector<std::string>& tags) {
    RegisterProviderRequest request;
    request.set_providername(name);
    
    for (const auto& attr : attributes) {
        *request.add_attributes() = attr;
    }
    
    for (const auto& tag : tags) {
        request.add_tags(tag);
    }
    
    return registerProvider(request);
}

std::string IngestClient::ingestData(const IngestDataRequest& request) {
    IngestDataRequest enriched_request = spatial_enrichment_enabled_ ? enrichRequest(request) : request;
    
    try {
        dp::service::ingestion::IngestDataResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->ingestData(&context, enriched_request, &response);

        if (status.ok()) {
            return "Success: Data ingested";
        } else {
            return "IngestData RPC failed: " + status.error_message();
        }
    } catch (const std::exception& e) {
        return "Exception during ingest: " + std::string(e.what());
    }
}

IngestionResult IngestClient::ingestBatch(const std::vector<IngestDataRequest>& requests, const std::string& provider_id) {
    auto start_time = std::chrono::high_resolution_clock::now();

    IngestionResult result;
    result.total_requests = requests.size();
    result.provider_id = provider_id;

    if (requests.empty()) {
        result.success = true;
        return result;
    }

    // Parallel spatial enrichment if enabled
    std::vector<IngestDataRequest> enriched_requests;
    if (spatial_enrichment_enabled_) {
        enriched_requests = enrichRequestsBatch(requests);
    } else {
        enriched_requests = requests;
    }

    // Use optimized chunking and streaming
    auto chunks = chunkRequestsToVector(enriched_requests, config_.default_batch_size);

    for (const auto& chunk : chunks) {
        std::string chunk_result;
        
        if (config_.streaming_preferred && chunk.size() > 1) {
            chunk_result = sendBatchToServerStream(chunk);
        } else {
            // Process individually for small batches
            for (const auto& request : chunk) {
                chunk_result = ingestData(request);
                if (chunk_result.find("Success") == std::string::npos) {
                    break;
                }
            }
        }

        if (chunk_result.find("Success") != std::string::npos) {
            result.successful_requests += chunk.size();
        } else {
            result.failed_requests += chunk.size();
            result.error_messages.push_back("Batch failed: " + chunk_result);
        }

        if (config_.enable_progress_monitoring) {
            notifyProgress(result.successful_requests + result.failed_requests, result.total_requests, result.successful_requests);
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.processing_time_seconds = std::chrono::duration<double>(end_time - start_time).count();
    result.success = (result.successful_requests > 0);

    return result;
}

std::string IngestClient::sendBatchToServerStream(const std::vector<IngestDataRequest>& batch) {
    if (batch.empty()) {
        return "Success: Empty batch";
    }
    
    try {
        grpc::ClientContext context;
        dp::service::ingestion::IngestDataStreamResponse response;
        
        auto stream = stub_->ingestDataStream(&context, &response);
        
        for (const auto& request : batch) {
            if (!stream->Write(request)) {
                return "Error: Failed to write request to stream";
            }
        }
        
        stream->WritesDone();
        grpc::Status status = stream->Finish();
        
        if (status.ok()) {
            return "Success: Batch of " + std::to_string(batch.size()) + " requests processed";
        } else {
            return "Error: " + status.error_message();
        }
        
    } catch (const std::exception& e) {
        return "Exception: " + std::string(e.what());
    }
}

IngestDataRequest IngestClient::enrichRequest(const IngestDataRequest& request) const {
    if (!spatial_analyzer_) {
        return request;
    }
    
    IngestDataRequest enriched = request;
    
    try {
        std::string pv_name;
        if (request.has_ingestiondataframe() && 
            request.ingestiondataframe().datacolumns_size() > 0) {
            pv_name = request.ingestiondataframe().datacolumns(0).name();
        } else {
            return enriched;
        }
        
        auto spatialData = spatial_analyzer_->analyzePV(pv_name);
        
        if (spatialData.isValid) {
            auto* attr = enriched.add_attributes();
            attr->set_name("z_position");
            attr->set_value(std::to_string(spatialData.zPosition));
            
            attr = enriched.add_attributes();
            attr->set_name("beam_path");
            attr->set_value(spatialData.beamPath);
            
            attr = enriched.add_attributes();
            attr->set_name("device_type");
            attr->set_value(spatialData.deviceType);
            
            attr = enriched.add_attributes();
            attr->set_name("area");
            attr->set_value(spatialData.area);
            
            enriched.add_tags("spatial_enriched");
            for (const auto& tag : spatialData.tags) {
                enriched.add_tags("spatial_" + tag);
            }
        }
    } catch (const std::exception&) {
        // Silent fallback to original request
    }
    
    return enriched;
}

std::vector<IngestDataRequest> IngestClient::enrichRequestsBatch(const std::vector<IngestDataRequest>& requests) const {
    if (!spatial_analyzer_ || requests.empty()) {
        return requests;
    }
    
    const size_t num_threads = std::thread::hardware_concurrency();
    const size_t batch_size = std::max(size_t(1), requests.size() / num_threads);
    std::vector<std::future<std::vector<IngestDataRequest>>> futures;
    
    for (size_t i = 0; i < requests.size(); i += batch_size) {
        size_t end = std::min(i + batch_size, requests.size());
        std::vector<IngestDataRequest> batch(requests.begin() + i, requests.begin() + end);
        
        auto future = std::async(std::launch::async, [this, batch]() {
            std::vector<IngestDataRequest> enriched_batch;
            enriched_batch.reserve(batch.size());
            
            for (const auto& request : batch) {
                enriched_batch.push_back(enrichRequest(request));
            }
            
            return enriched_batch;
        });
        
        futures.push_back(std::move(future));
    }
    
    std::vector<IngestDataRequest> all_enriched;
    all_enriched.reserve(requests.size());
    
    for (auto& future : futures) {
        auto batch_results = future.get();
        all_enriched.insert(all_enriched.end(), 
                           std::make_move_iterator(batch_results.begin()),
                           std::make_move_iterator(batch_results.end()));
    }
    
    return all_enriched;
}

std::vector<std::vector<IngestDataRequest>> IngestClient::chunkRequestsToVector(const std::vector<IngestDataRequest>& requests, size_t chunk_size) const {
    std::vector<std::vector<IngestDataRequest>> chunks;
    chunks.reserve((requests.size() + chunk_size - 1) / chunk_size);

    for (size_t i = 0; i < requests.size(); i += chunk_size) {
        size_t end = std::min(i + chunk_size, requests.size());
        chunks.emplace_back(requests.begin() + i, requests.begin() + end);
    }

    return chunks;
}

void IngestClient::enableSpatialEnrichment(bool enable) {
    if (enable && !spatial_analyzer_) {
        initializeSpatialAnalyzer();
    }
    spatial_enrichment_enabled_ = enable && (spatial_analyzer_ != nullptr);
}

bool IngestClient::isSpatialEnrichmentEnabled() const {
    return spatial_enrichment_enabled_ && (spatial_analyzer_ != nullptr);
}

SpatialContext IngestClient::enrichPvName(const std::string& pv_name) const {
    SpatialContext context;
    if (spatial_analyzer_) {
        auto metadata = spatial_analyzer_->analyzePV(pv_name);
        context.beam_path = metadata.beamPath;
        context.area = metadata.area;
        context.device_type = metadata.deviceType;
        context.device_location = metadata.position;
        context.device_attribute = metadata.attribute;
        context.full_pv_name = pv_name;
        context.z_coordinates = {metadata.zPosition};
    }
    return context;
}

bool IngestClient::isConnected() const {
    return stub_ != nullptr;
}

void IngestClient::reconnect() {
    initializeConnection();
}

void IngestClient::updateConfig(const IngestClientConfig& config) {
    config_ = config;
    initializeConnection();
    if (config_.enable_spatial_enrichment) {
        initializeSpatialAnalyzer();
    }
}

void IngestClient::notifyProgress(size_t processed, size_t total, size_t successful) const {
    if (progress_callback_) {
        progress_callback_(processed, total, successful);
    }
}

void IngestClient::notifyError(const std::string& error, const std::string& context) const {
    if (error_callback_) {
        error_callback_(error, context);
    }
}

// ===================== Helper Functions Implementation =====================

Timestamp makeTimeStamp(uint64_t epoch, uint64_t nano) {
    Timestamp ts;
    ts.set_epochseconds(epoch);
    ts.set_nanoseconds(nano);
    return ts;
}

Attribute makeAttribute(const std::string& name, const std::string& value) {
    Attribute attr;
    attr.set_name(name);
    attr.set_value(value);
    return attr;
}

EventMetadata makeEventMetadata(const std::string& desc, uint64_t startEpoch, uint64_t startNano,
                                uint64_t endEpoch, uint64_t endNano) {
    EventMetadata meta;
    meta.set_description(desc);
    *meta.mutable_starttimestamp() = makeTimeStamp(startEpoch, startNano);
    *meta.mutable_stoptimestamp() = makeTimeStamp(endEpoch, endNano);
    return meta;
}

SamplingClock makeSamplingClock(uint64_t epoch, uint64_t nano, uint64_t periodNanos, uint32_t count) {
    SamplingClock clk;
    *clk.mutable_starttime() = makeTimeStamp(epoch, nano);
    clk.set_periodnanos(periodNanos);
    clk.set_count(count);
    return clk;
}

DataValue makeDataValueWithSInt32(int val) {
    DataValue dv;
    dv.set_intvalue(val);
    return dv;
}

DataValue makeDataValueWithUInt64(uint64_t val) {
    DataValue dv;
    dv.set_ulongvalue(val);
    return dv;
}

DataValue makeDataValueWithDouble(double val){
    DataValue dv;
    dv.set_doublevalue(val);
    return dv;
}

DataValue makeDataValueWithTimestamp(uint64_t sec, uint64_t nano) {
    DataValue dv;
    *dv.mutable_timestampvalue() = makeTimeStamp(sec, nano);
    return dv;
}

DataColumn makeDataColumn(const std::string& name, const std::vector<DataValue>& values) {
    DataColumn col;
    col.set_name(name);
    for (const auto& v : values)
        *col.add_datavalues() = v;
    return col;
}

IngestDataRequest makeIngestDataRequest(const std::string& providerId,
                                        const std::string& clientRequestId,
                                        const std::vector<Attribute>& attributes,
                                        const std::vector<std::string>& tags,
                                        const EventMetadata& metadata,
                                        const SamplingClock& clock,
                                        const std::vector<DataColumn>& columns) {
    IngestDataRequest req;
    req.set_providerid(providerId);
    req.set_clientrequestid(clientRequestId);
    for (const auto& attr : attributes)
        *req.add_attributes() = attr;
    for (const auto& tag : tags)
        req.add_tags(tag);
    *req.mutable_eventmetadata() = metadata;

    auto* frame = req.mutable_ingestiondataframe();
    *frame->mutable_datatimestamps()->mutable_samplingclock() = clock;
    for (const auto& c : columns)
        *frame->add_datacolumns() = c;
    return req;
}

// ===================== Utility Functions =====================

namespace IngestUtils {
    uint64_t getCurrentEpochSeconds() {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    }

    uint64_t getCurrentEpochNanos() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::string generateRequestId(const std::string& prefix) {
        return prefix + "_" + std::to_string(getCurrentEpochSeconds()) + "_" +
               std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count() % 1000000);
    }
}