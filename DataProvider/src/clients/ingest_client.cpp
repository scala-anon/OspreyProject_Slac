#include "ingest_client.hpp"
#include <grpcpp/create_channel.h>
#include <grpcpp/client_context.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>

using json = nlohmann::json;

// ===================== IngestClientConfig Implementation =====================

IngestClientConfig IngestClientConfig::fromConfigFile(const std::string& config_path) {
    IngestClientConfig config;
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open config file: " << config_path << ". Using defaults." << std::endl;
            return config;
        }
        json j;
        file >> j;
        return fromJson(j);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error parsing config file: " << e.what() << ". Using defaults." << std::endl;
        return config;
    }
}

IngestClientConfig IngestClientConfig::fromJson(const json& j) {
    IngestClientConfig config;

    if (j.contains("server_connections")) {
        const auto& conn = j["server_connections"];
        if (conn.contains("ingestion_server")) config.server_address = conn["ingestion_server"];
        if (conn.contains("connection_timeout_seconds")) config.connection_timeout_seconds = conn["connection_timeout_seconds"];
        if (conn.contains("max_message_size_mb")) config.max_message_size_mb = conn["max_message_size_mb"];
    }

    if (j.contains("spatial_enrichment")) {
        const auto& spatial = j["spatial_enrichment"];
        if (spatial.contains("enabled")) config.enable_spatial_enrichment = spatial["enabled"];
        if (spatial.contains("dictionaries_path")) config.dictionaries_path = spatial["dictionaries_path"];
    }

    if (j.contains("ingestion_settings")) {
        const auto& ingest = j["ingestion_settings"];
        if (ingest.contains("default_batch_size")) config.default_batch_size = ingest["default_batch_size"];
        if (ingest.contains("max_batch_size")) config.max_batch_size = ingest["max_batch_size"];
        if (ingest.contains("streaming_preferred")) config.streaming_preferred = ingest["streaming_preferred"];
        if (ingest.contains("retry_attempts")) config.retry_attempts = ingest["retry_attempts"];
        if (ingest.contains("retry_delay_ms")) config.retry_delay_ms = ingest["retry_delay_ms"];
        if (ingest.contains("enable_progress_monitoring")) config.enable_progress_monitoring = ingest["enable_progress_monitoring"];
    }

    return config;
}

// ===================== IngestClient Implementation =====================

IngestClient::IngestClient(const IngestClientConfig& config)
    : config_(config), spatial_enrichment_enabled_(config.enable_spatial_enrichment) {
    initializeConnection();
    if (config_.enable_spatial_enrichment) {
        initializeSpatialAnalyzer();
    }
}

IngestClient::IngestClient(const std::string& server_address)
    : IngestClient(IngestClientConfig{}) {
    config_.server_address = server_address;
    initializeConnection();
}

IngestClient::IngestClient(const std::string& config_path, bool is_config_file)
    : IngestClient(is_config_file ? IngestClientConfig::fromConfigFile(config_path) : IngestClientConfig{}) {
    if (!is_config_file) {
        config_.server_address = config_path;
        initializeConnection();
    }
}

IngestClient::~IngestClient() = default;

void IngestClient::initializeConnection() {
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(config_.max_message_size_mb * 1024 * 1024);
    args.SetMaxSendMessageSize(config_.max_message_size_mb * 1024 * 1024);

    auto channel = grpc::CreateCustomChannel(config_.server_address, grpc::InsecureChannelCredentials(), args);

    if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(config_.connection_timeout_seconds))) {
        std::string error = "Failed to connect to MLDP server at " + config_.server_address;
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error("gRPC channel connection timeout");
    }

    stub_ = dp::service::ingestion::DpIngestionService::NewStub(channel);
}

void IngestClient::initializeSpatialAnalyzer() {
    try {
        spatial_analyzer_ = std::make_unique<SpatialAnalyzer>();
        if (!spatial_analyzer_->loadDictionaries(config_.dictionaries_path)) {
            std::cerr << "Warning: Failed to load spatial dictionaries. Spatial enrichment disabled." << std::endl;
            spatial_enrichment_enabled_ = false;
            spatial_analyzer_.reset();
        } else {
            std::cout << "Spatial enrichment initialized successfully" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error initializing spatial analyzer: " << e.what() << ". Spatial enrichment disabled." << std::endl;
        spatial_enrichment_enabled_ = false;
        spatial_analyzer_.reset();
    }
}

RegisterProviderResponse IngestClient::registerProvider(const RegisterProviderRequest& request) {
    RegisterProviderResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->registerProvider(&context, request, &response);

    if (!status.ok()) {
        std::string error = "RegisterProvider RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

RegisterProviderResponse IngestClient::registerProvider(const std::string& name,
                                                       const std::vector<Attribute>& attributes,
                                                       const std::vector<std::string>& tags) {
    auto request = makeRegisterProviderRequest(name, attributes, IngestUtils::getCurrentEpochSeconds(), IngestUtils::getCurrentEpochNanos());
    return registerProvider(request);
}

std::string IngestClient::ingestData(const IngestDataRequest& request) {
    IngestDataRequest enriched_request = spatial_enrichment_enabled_ ? enrichRequest(request) : request;

    dp::service::ingestion::IngestDataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->ingestData(&context, enriched_request, &response);

    if (status.ok() && response.has_ackresult()) {
        std::cout << "Ack: Rows=" << response.ackresult().numrows() << ", Cols=" << response.ackresult().numcolumns() << "\n";
        return "IngestData Success";
    } else if (status.ok()) {
        std::cerr << "No AckResult in response.\n";
        return "IngestData Failed";
    } else {
        std::string error = std::to_string(status.error_code()) + ": " + status.error_message();
        std::cerr << error << "\n";
        last_error_ = error;
        return "RPC Failed";
    }
}

std::string IngestClient::ingestDataStream(const std::vector<IngestDataRequest>& requests) {
    if (requests.empty()) {
        std::cout << "No requests to send in stream" << std::endl;
        return "IngestDataStream Empty";
    }

    std::vector<IngestDataRequest> enriched_requests;
    enriched_requests.reserve(requests.size());

    if (spatial_enrichment_enabled_) {
        for (const auto& req : requests) {
            enriched_requests.push_back(enrichRequest(req));
        }
    } else {
        enriched_requests = requests;
    }

    grpc::ClientContext context;
    dp::service::ingestion::IngestDataStreamResponse response;

    std::unique_ptr<grpc::ClientWriter<IngestDataRequest>> writer(stub_->ingestDataStream(&context, &response));

    size_t sent_count = 0;
    for (const auto& request : enriched_requests) {
        if (!writer->Write(request)) {
            std::cerr << "Failed to write request to stream after " << sent_count << " requests" << std::endl;
            break;
        }
        sent_count++;
    }

    writer->WritesDone();
    grpc::Status status = writer->Finish();

    if (status.ok()) {
        if (response.has_ingestdatastreamresult()) {
            std::cout << "Stream Success: " << response.ingestdatastreamresult().numrequests()
                      << " requests processed (sent " << sent_count << " of " << enriched_requests.size() << ")" << std::endl;

            if (response.rejectedrequestids_size() > 0) {
                std::cout << "Warning: " << response.rejectedrequestids_size() << " requests were rejected" << std::endl;
                return "IngestDataStream Partial Success";
            }
            return "IngestDataStream Success";
        } else if (response.has_exceptionalresult()) {
            std::string error = "Stream rejected: " + response.exceptionalresult().message();
            std::cerr << error << std::endl;
            last_error_ = error;
            return "IngestDataStream Rejected";
        } else {
            std::cerr << "Unexpected response format from ingestDataStream" << std::endl;
            return "IngestDataStream Unexpected Response";
        }
    } else {
        std::string error = "Stream RPC failed: " + std::to_string(status.error_code()) + ": " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        return "IngestDataStream RPC Failed";
    }
}

IngestionResult IngestClient::ingestBatch(const std::vector<IngestDataRequest>& requests, const std::string& provider_id) {
    IngestionResult result;
    result.total_requests = requests.size();

    if (requests.empty()) {
        result.success = true;
        return result;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    if (config_.streaming_preferred && requests.size() > 1) {
        auto chunks = chunkRequestsToVector(requests, config_.default_batch_size);

        for (const auto& chunk : chunks) {
            std::string stream_result = ingestDataStream(chunk);

            if (stream_result.find("Success") != std::string::npos) {
                result.successful_requests += chunk.size();
            } else {
                result.failed_requests += chunk.size();
                result.error_messages.push_back("Batch failed: " + stream_result);
            }

            if (config_.enable_progress_monitoring) {
                notifyProgress(result.successful_requests + result.failed_requests, result.total_requests, result.successful_requests);
            }
        }
    } else {
        for (const auto& request : requests) {
            std::string single_result = ingestData(request);

            if (single_result.find("Success") != std::string::npos) {
                result.successful_requests++;
            } else {
                result.failed_requests++;
                result.error_messages.push_back("Request failed: " + single_result);
            }

            if (config_.enable_progress_monitoring && (result.successful_requests + result.failed_requests) % 10 == 0) {
                notifyProgress(result.successful_requests + result.failed_requests, result.total_requests, result.successful_requests);
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.processing_time_seconds = std::chrono::duration<double>(end_time - start_time).count();

    result.success = (result.successful_requests > 0);
    result.provider_id = provider_id;

    return result;
}

IngestionResult IngestClient::ingestWithSpatialEnrichment(const std::vector<IngestDataRequest>& requests, const std::string& provider_id) {
    if (!spatial_enrichment_enabled_) {
        std::cout << "Spatial enrichment not enabled, using standard batch ingestion" << std::endl;
        return ingestBatch(requests, provider_id);
    }

    std::cout << "Starting spatial-aware batch ingestion for " << requests.size() << " requests" << std::endl;
    auto result = ingestBatch(requests, provider_id);
    std::cout << "Spatial-aware ingestion complete. Success rate: " << (result.getSuccessRate() * 100.0) << "%" << std::endl;

    return result;
}

IngestDataRequest IngestClient::enrichRequest(const IngestDataRequest& request) const {
    if (!spatial_analyzer_ || !spatial_enrichment_enabled_) {
        return request;
    }

    IngestDataRequest enriched = request;
    
    try {
        if (enriched.ingestiondataframe().datacolumns_size() > 0) {
            std::string pv_name = enriched.ingestiondataframe().datacolumns(0).name();
            
            // Convert underscore format to colon format for spatial analysis
            std::string normalized_pv = pv_name;
            std::replace(normalized_pv.begin(), normalized_pv.end(), '_', ':');

            std::cout << "  [SPATIAL] Processing: " << pv_name << " (normalized: " << normalized_pv << ")" << std::endl;

            auto spatialData = spatial_analyzer_->analyzePV(normalized_pv);

            if (spatialData.isValid) {
                // Add spatial attributes
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_device_type", spatialData.deviceType));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_area", spatialData.area));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_position", spatialData.position));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_attribute", spatialData.attribute));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_category", spatialData.category));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_function", spatialData.function));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_signal_type", spatialData.signalType));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_units", spatialData.units));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_beam_path", spatialData.beamPath));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_area_description", spatialData.areaDescription));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_z_position", std::to_string(spatialData.zPosition)));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_z_uncertainty", std::to_string(spatialData.zUncertainty)));
                enriched.add_attributes()->CopyFrom(makeAttribute("spatial_controllable", spatialData.controllable ? "true" : "false"));

                // Add spatial tags
                enriched.add_tags("spatial_enriched");
                for (const auto& tag : spatialData.tags) {
                    enriched.add_tags("spatial_" + tag);
                }

                std::cout << "  [SPATIAL] Added " << spatialData.tags.size() << " tags and spatial metadata" << std::endl;
            } else {
                std::cout << "  [SPATIAL] No enrichment data found for: " << normalized_pv << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error enriching request: " << e.what() << std::endl;
    }

    return enriched;
}

std::vector<std::vector<IngestDataRequest>> IngestClient::chunkRequestsToVector(const std::vector<IngestDataRequest>& requests, size_t chunk_size) const {
    std::vector<std::vector<IngestDataRequest>> chunks;

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
        // Map other fields as needed
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

RegisterProviderRequest makeRegisterProviderRequest(const std::string& name,
                                                    const std::vector<Attribute>& attributes,
                                                    uint64_t epoch,
                                                    uint64_t nano) {
    dp::service::ingestion::RegisterProviderRequest req;
    req.set_providername(name);
    for (const auto& attr : attributes)
        *req.add_attributes() = attr;
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

    std::vector<Attribute> mergeAttributes(const std::vector<Attribute>& base,
                                          const std::vector<Attribute>& additional) {
        std::vector<Attribute> result = base;
        result.insert(result.end(), additional.begin(), additional.end());
        return result;
    }

    std::vector<std::string> mergeTags(const std::vector<std::string>& base,
                                      const std::vector<std::string>& additional) {
        std::vector<std::string> result = base;
        result.insert(result.end(), additional.begin(), additional.end());
        return result;
    }
}
