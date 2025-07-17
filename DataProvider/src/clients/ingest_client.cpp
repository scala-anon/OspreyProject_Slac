#include "ingest_client.hpp"
#include <grpcpp/create_channel.h>
#include <grpcpp/client_context.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <regex>
#include <algorithm>

using json = nlohmann::json;

// ===================== IngestClientConfig Implementation =====================

IngestClientConfig IngestClientConfig::fromConfigFile(const std::string& config_path) {
    IngestClientConfig config;
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

IngestClientConfig IngestClientConfig::fromJson(const json& j) {
    IngestClientConfig config;
    
    if (j.contains("server_connections")) {
        const auto& conn = j["server_connections"];
        if (conn.contains("ingestion_server")) {
            config.server_address = conn["ingestion_server"];
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
            config.enable_spatial_enrichment = spatial["enabled"];
        }
        if (spatial.contains("dictionaries_path")) {
            config.dictionaries_path = spatial["dictionaries_path"];
        }
    }
    
    if (j.contains("ingestion_settings")) {
        const auto& ingest = j["ingestion_settings"];
        if (ingest.contains("default_batch_size")) {
            config.default_batch_size = ingest["default_batch_size"];
        }
        if (ingest.contains("max_batch_size")) {
            config.max_batch_size = ingest["max_batch_size"];
        }
        if (ingest.contains("streaming_preferred")) {
            config.streaming_preferred = ingest["streaming_preferred"];
        }
        if (ingest.contains("retry_attempts")) {
            config.retry_attempts = ingest["retry_attempts"];
        }
        if (ingest.contains("retry_delay_ms")) {
            config.retry_delay_ms = ingest["retry_delay_ms"];
        }
        if (ingest.contains("enable_progress_monitoring")) {
            config.enable_progress_monitoring = ingest["enable_progress_monitoring"];
        }
    }
    
    return config;
}

// ===================== IngestClient Implementation =====================

IngestClient::IngestClient(const IngestClientConfig& config) 
    : config_(config), spatial_enrichment_enabled_(config.enable_spatial_enrichment) {
    initializeConnection();
    if (config_.enable_spatial_enrichment) {
        initializeSpatialEngine();
    }
}

IngestClient::IngestClient(const std::string& server_address) 
    : IngestClient(IngestClientConfig{}) {
    config_.server_address = server_address;
    initializeConnection();
}

IngestClient::IngestClient(const std::string& config_path, bool is_config_file) 
    : IngestClient(is_config_file ? 
                   IngestClientConfig::fromConfigFile(config_path) : 
                   IngestClientConfig{}) {
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

    stub_ = dp::service::ingestion::DpIngestionService::NewStub(channel);
}

void IngestClient::initializeSpatialEngine() {
    try {
        spatial_engine_ = std::make_unique<SpatialEnrichmentEngine>(config_.dictionaries_path);
        if (!spatial_engine_->loadDictionaries()) {
            std::cerr << "Warning: Failed to load spatial dictionaries. "
                      << "Spatial enrichment will be disabled." << std::endl;
            spatial_enrichment_enabled_ = false;
            spatial_engine_.reset();
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error initializing spatial engine: " << e.what()
                  << ". Spatial enrichment disabled." << std::endl;
        spatial_enrichment_enabled_ = false;
        spatial_engine_.reset();
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
    auto request = makeRegisterProviderRequest(name, attributes, 
                                             IngestUtils::getCurrentEpochSeconds(),
                                             IngestUtils::getCurrentEpochNanos());
    return registerProvider(request);
}

std::string IngestClient::ingestData(const IngestDataRequest& request) {
    IngestDataRequest enriched_request = spatial_enrichment_enabled_ ? 
                                        enrichRequest(request) : request;
    
    dp::service::ingestion::IngestDataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->ingestData(&context, enriched_request, &response);

    if (status.ok() && response.has_ackresult()) {
        std::cout << "Ack: Rows=" << response.ackresult().numrows()
                  << ", Cols=" << response.ackresult().numcolumns() << "\n";
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

    // Enrich requests if spatial enrichment is enabled
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

    std::unique_ptr<grpc::ClientWriter<IngestDataRequest>> writer(
        stub_->ingestDataStream(&context, &response)
    );

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
        std::string error = "Stream RPC failed: " + std::to_string(status.error_code()) + 
                           ": " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        return "IngestDataStream RPC Failed";
    }
}

IngestionResult IngestClient::ingestBatch(const std::vector<IngestDataRequest>& requests,
                                         const std::string& provider_id) {
    IngestionResult result;
    result.total_requests = requests.size();
    
    if (requests.empty()) {
        result.success = true;
        return result;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    if (config_.streaming_preferred && requests.size() > 1) {
        // Use streaming for multiple requests - process in chunks
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
                notifyProgress(result.successful_requests + result.failed_requests, 
                             result.total_requests, result.successful_requests);
            }
        }
    } else {
        // Use individual requests
        for (const auto& request : requests) {
            std::string single_result = ingestData(request);
            
            if (single_result.find("Success") != std::string::npos) {
                result.successful_requests++;
            } else {
                result.failed_requests++;
                result.error_messages.push_back("Request failed: " + single_result);
            }
            
            if (config_.enable_progress_monitoring && 
                (result.successful_requests + result.failed_requests) % 10 == 0) {
                notifyProgress(result.successful_requests + result.failed_requests, 
                             result.total_requests, result.successful_requests);
            }
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    result.processing_time_seconds = 
        std::chrono::duration<double>(end_time - start_time).count();
    
    result.success = (result.successful_requests > 0);
    result.provider_id = provider_id;
    
    return result;
}

IngestionResult IngestClient::ingestWithSpatialEnrichment(const std::vector<IngestDataRequest>& requests,
                                                         const std::string& provider_id) {
    if (!spatial_enrichment_enabled_) {
        std::cout << "Spatial enrichment not enabled, using standard batch ingestion" << std::endl;
        return ingestBatch(requests, provider_id);
    }
    
    std::cout << "Starting spatial-aware batch ingestion for " << requests.size() << " requests" << std::endl;
    
    // All requests will be enriched automatically in ingestBatch via enrichRequest()
    auto result = ingestBatch(requests, provider_id);
    
    std::cout << "Spatial-aware ingestion complete. Success rate: " 
              << (result.getSuccessRate() * 100.0) << "%" << std::endl;
    
    return result;
}

// Replace the enrichRequest method in your ingest_client.cpp (around line 290)
// Find this method and replace it completely:

// Replace the enrichRequest method in your ingest_client.cpp (around line 290)
// Find this method and replace it completely:

IngestDataRequest IngestClient::enrichRequest(const IngestDataRequest& request) const {
    if (!spatial_engine_ || !spatial_enrichment_enabled_) {
        return request;
    }

    IngestDataRequest enriched = request;
    std::string pv_name; // Move this OUTSIDE the try block

    try {
        // Get the PV name from the first data column (this is how your current code works)
        if (enriched.ingestiondataframe().datacolumns_size() > 0) {
            pv_name = enriched.ingestiondataframe().datacolumns(0).name();
        }
        
        if (pv_name.empty()) {
            return enriched; // Can't enrich without PV name
        }

        // CRITICAL FIX: Normalize PV name format for spatial enrichment
        // Convert underscore format to colon format for processing
        std::string normalized_pv = pv_name;
        std::replace(normalized_pv.begin(), normalized_pv.end(), '_', ':');
        
        // Debug output (simple version since config doesn't have logging structure)
        std::cout << "  [SPATIAL] Processing: " << pv_name 
                  << " (normalized: " << normalized_pv << ")" << std::endl;
        
        // Get spatial context using the normalized PV name
        SpatialContext context = spatial_engine_->enrichPvName(normalized_pv);
        
        if (context.isValid()) {
            // Generate spatial attributes and add them to the request
            auto spatial_attrs = spatial_engine_->generateSpatialAttributes(context);
            for (const auto& attr : spatial_attrs) {
                *enriched.add_attributes() = attr;
            }
            
            // Generate spatial tags and add them to the request
            auto spatial_tags = spatial_engine_->generateSpatialTags(context);
            for (const auto& tag : spatial_tags) {
                enriched.add_tags(tag);
            }
            
            std::cout << "  [SPATIAL] Added " << spatial_attrs.size() 
                      << " attributes and " << spatial_tags.size() << " tags" << std::endl;
        } else {
            std::cout << "  [SPATIAL] No enrichment data found for: " << normalized_pv << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error enriching PV name " << pv_name << ": " << e.what() << std::endl;
        // Continue without spatial enrichment - don't fail the request
    }
    
    return enriched;
}

std::vector<std::vector<IngestDataRequest>> IngestClient::chunkRequestsToVector(const std::vector<IngestDataRequest>& requests, 
                                                                                size_t chunk_size) const {
    std::vector<std::vector<IngestDataRequest>> chunks;
    
    for (size_t i = 0; i < requests.size(); i += chunk_size) {
        size_t end = std::min(i + chunk_size, requests.size());
        chunks.emplace_back(requests.begin() + i, requests.begin() + end);
    }
    
    return chunks;
}

void IngestClient::enableSpatialEnrichment(bool enable) {
    if (enable && !spatial_engine_) {
        initializeSpatialEngine();
    }
    spatial_enrichment_enabled_ = enable && (spatial_engine_ != nullptr);
}

bool IngestClient::isSpatialEnrichmentEnabled() const {
    return spatial_enrichment_enabled_ && (spatial_engine_ != nullptr);
}

SpatialContext IngestClient::enrichPvName(const std::string& pv_name) const {
    if (spatial_engine_) {
        return spatial_engine_->enrichPvName(pv_name);
    }
    return SpatialContext{};
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
        initializeSpatialEngine();
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

// ===================== SpatialEnrichmentEngine Implementation =====================

SpatialEnrichmentEngine::SpatialEnrichmentEngine(const std::string& dictionaries_path)
    : dictionaries_path_(dictionaries_path) {
}

bool SpatialEnrichmentEngine::loadDictionaries() {
    try {
        // Load beamline boundaries
        std::ifstream boundaries_file(dictionaries_path_ + "/lcls_beamline_boundaries.json");
        if (boundaries_file.is_open()) {
            boundaries_file >> beamline_boundaries_;
            boundaries_file.close();
        } else {
            std::cerr << "Warning: Could not load beamline boundaries dictionary" << std::endl;
        }
        
        // Load device classifications
        std::ifstream device_file(dictionaries_path_ + "/device_classifications.json");
        if (device_file.is_open()) {
            device_file >> device_classifications_;
            device_file.close();
        } else {
            std::cerr << "Warning: Could not load device classifications dictionary" << std::endl;
        }
        
        // Load coordinate systems
        std::ifstream coord_file(dictionaries_path_ + "/coordinate_systems.json");
        if (coord_file.is_open()) {
            coord_file >> coordinate_systems_;
            coord_file.close();
        } else {
            std::cerr << "Warning: Could not load coordinate systems dictionary" << std::endl;
        }
        
        // Load EPICS records
        std::ifstream epics_file(dictionaries_path_ + "/epics_record_types.json");
        if (epics_file.is_open()) {
            epics_file >> epics_records_;
            epics_file.close();
        } else {
            std::cerr << "Warning: Could not load EPICS records dictionary" << std::endl;
        }
        
        dictionaries_loaded_ = true;
        std::cout << "Spatial enrichment dictionaries loaded successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading spatial dictionaries: " << e.what() << std::endl;
        dictionaries_loaded_ = false;
        return false;
    }
}

SpatialContext SpatialEnrichmentEngine::enrichPvName(const std::string& pv_name) const {
    SpatialContext context;
    
    if (!dictionaries_loaded_) {
        return context;
    }
    
    try {
        // Normalize PV name - convert underscores to colons for parsing
        std::string normalized_pv = pv_name;
        std::replace(normalized_pv.begin(), normalized_pv.end(), '_', ':');
        
        // Extract all components from normalized PV name (e.g., "BPMS:DMPH:502:TMITBR")
        std::string device = extractDevice(normalized_pv);                    // "BPMS"
        std::string device_area = extractDeviceArea(normalized_pv);          // "DMPH" 
        std::string device_location = extractDeviceLocation(normalized_pv);  // "502"
        std::string device_attribute = extractDeviceAttribute(normalized_pv); // "TMITBR"
        
        // Store all parsed components
        context.device_type = device;
        context.device_location = device_location;
        context.device_attribute = device_attribute;
        context.full_pv_name = pv_name; // Store original format
        
        // Find beam path from area
        context.beam_path = findBeamPath(device_area);
        context.area = device_area;
        
        // Get coordinates (use specific location if available, otherwise area range)
        context.z_coordinates = getZCoordinates(device_area, device_location);
        
        // Get area sequence index
        context.area_sequence_index = getAreaSequenceIndex(device_area, context.beam_path);
        
        // Set device class from device type
        if (device_classifications_.contains("device_types") &&
            device_classifications_["device_types"].contains(device)) {
            context.device_class = device_classifications_["device_types"][device]["class"];
        }
        
        // Set area description
        if (beamline_boundaries_.contains("areas") &&
            beamline_boundaries_["areas"].contains(device_area)) {
            context.area_description = beamline_boundaries_["areas"][device_area]["description"];
        }
        
        // Extract sector information if available (from location or area)
        context.sector = extractSector(device_area, device_location);
        
    } catch (const std::exception& e) {
        std::cerr << "Error enriching PV name " << pv_name << ": " << e.what() << std::endl;
    }
    
    return context;
}

std::vector<Attribute> SpatialEnrichmentEngine::generateSpatialAttributes(const SpatialContext& context) const {
    std::vector<Attribute> attributes;
    
    // Core spatial attributes
    if (!context.beam_path.empty()) {
        attributes.push_back(makeAttribute("beam_path", context.beam_path));
    }
    if (!context.area.empty()) {
        attributes.push_back(makeAttribute("area", context.area));
    }
    if (context.area_sequence_index >= 0) {
        attributes.push_back(makeAttribute("area_sequence_index", std::to_string(context.area_sequence_index)));
    }
    if (!context.device_class.empty()) {
        attributes.push_back(makeAttribute("device_class", context.device_class));
    }
    if (!context.area_description.empty()) {
        attributes.push_back(makeAttribute("area_description", context.area_description));
    }
    if (!context.sector.empty()) {
        attributes.push_back(makeAttribute("sector", context.sector));
    }
    
    // Detailed PV component attributes
    if (!context.device_type.empty()) {
        attributes.push_back(makeAttribute("device_type", context.device_type));
    }
    if (!context.device_location.empty()) {
        attributes.push_back(makeAttribute("device_location", context.device_location));
        attributes.push_back(makeAttribute("location_id", context.getLocationId()));
    }
    if (!context.device_attribute.empty()) {
        attributes.push_back(makeAttribute("device_attribute", context.device_attribute));
    }
    
    // Coordinate information
    if (!context.z_coordinates.empty()) {
        std::string coords_str = std::to_string(context.z_coordinates[0]);
        if (context.z_coordinates.size() > 1) {
            coords_str += "," + std::to_string(context.z_coordinates[1]);
        }
        attributes.push_back(makeAttribute("z_coordinates", coords_str));
        
        // Add individual coordinate attributes for easier querying
        attributes.push_back(makeAttribute("z_min", std::to_string(context.z_coordinates[0])));
        if (context.z_coordinates.size() > 1) {
            attributes.push_back(makeAttribute("z_max", std::to_string(context.z_coordinates[1])));
        }
    }
    
    return attributes;
}

std::vector<std::string> SpatialEnrichmentEngine::generateSpatialTags(const SpatialContext& context) const {
    std::vector<std::string> tags;
    
    tags.push_back("spatial_enriched");
    
    if (!context.beam_path.empty()) {
        tags.push_back("beam_path_" + context.beam_path);
    }
    if (!context.area.empty()) {
        tags.push_back("area_" + context.area);
    }
    if (!context.device_class.empty()) {
        tags.push_back("device_class_" + context.device_class);
    }
    if (!context.device_type.empty()) {
        tags.push_back("device_type_" + context.device_type);
    }
    if (!context.device_location.empty()) {
        tags.push_back("location_" + context.device_location);
        tags.push_back("device_location_" + context.getLocationId());
    }
    if (!context.device_attribute.empty()) {
        tags.push_back("attribute_" + context.device_attribute);
    }
    if (!context.sector.empty()) {
        tags.push_back("sector_" + context.sector);
    }
    
    return tags;
}

// Internal parsing methods
std::string SpatialEnrichmentEngine::extractDevice(const std::string& pv_name) const {
    std::regex device_regex(R"(^([A-Z]+):)");
    std::smatch match;
    if (std::regex_search(pv_name, match, device_regex) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

std::string SpatialEnrichmentEngine::extractDeviceArea(const std::string& pv_name) const {
    std::regex area_regex(R"(^[A-Z]+:([^:]+):)");
    std::smatch match;
    if (std::regex_search(pv_name, match, area_regex) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

std::string SpatialEnrichmentEngine::extractDeviceLocation(const std::string& pv_name) const {
    std::regex location_regex(R"(^[A-Z]+:[^:]+:([^:]+):)");
    std::smatch match;
    if (std::regex_search(pv_name, match, location_regex) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

std::string SpatialEnrichmentEngine::extractDeviceAttribute(const std::string& pv_name) const {
    std::regex attr_regex(R"(:([^:]+)$)");
    std::smatch match;
    if (std::regex_search(pv_name, match, attr_regex) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

std::string SpatialEnrichmentEngine::findBeamPath(const std::string& device_area) const {
    if (!beamline_boundaries_.contains("areas") || 
        !beamline_boundaries_["areas"].contains(device_area)) {
        return "";
    }
    
    const auto& area_info = beamline_boundaries_["areas"][device_area];
    if (area_info.contains("beam_path")) {
        return area_info["beam_path"];
    }
    
    return "";
}

std::vector<double> SpatialEnrichmentEngine::getZCoordinates(const std::string& area, 
                                                           const std::string& location) const {
    std::vector<double> coords;
    
    if (!beamline_boundaries_.contains("areas") || 
        !beamline_boundaries_["areas"].contains(area)) {
        return coords;
    }
    
    const auto& area_info = beamline_boundaries_["areas"][area];
    
    // First, try to get specific device location coordinates
    if (!location.empty() && area_info.contains("devices") && 
        area_info["devices"].contains(location)) {
        const auto& device_info = area_info["devices"][location];
        if (device_info.contains("z_position")) {
            double z_pos = device_info["z_position"];
            coords.push_back(z_pos);
            // For specific devices, we might have uncertainty or range
            if (device_info.contains("z_uncertainty")) {
                double uncertainty = device_info["z_uncertainty"];
                coords.push_back(z_pos + uncertainty);
            }
            return coords;
        }
    }
    
    // Fallback to area range coordinates
    if (area_info.contains("z_range") && area_info["z_range"].is_array() && 
        area_info["z_range"].size() >= 2) {
        coords.push_back(area_info["z_range"][0]);
        coords.push_back(area_info["z_range"][1]);
    }
    
    return coords;
}

// Add new method for sector extraction
std::string SpatialEnrichmentEngine::extractSector(const std::string& area, 
                                                   const std::string& location) const {
    // Try to extract sector from area mapping first
    if (beamline_boundaries_.contains("areas") && 
        beamline_boundaries_["areas"].contains(area)) {
        const auto& area_info = beamline_boundaries_["areas"][area];
        if (area_info.contains("sector")) {
            return area_info["sector"];
        }
    }
    
    // Try to extract from location-specific information
    if (!location.empty() && coordinate_systems_.contains("sectors")) {
        // Look for sector patterns in location (e.g., "502" might map to sector "92H")
        const auto& sectors = coordinate_systems_["sectors"];
        for (const auto& [sector_name, sector_info] : sectors.items()) {
            if (sector_info.contains("location_range")) {
                const auto& range = sector_info["location_range"];
                if (range.is_array() && range.size() >= 2) {
                    int loc_num = std::stoi(location);
                    int min_loc = range[0];
                    int max_loc = range[1];
                    if (loc_num >= min_loc && loc_num <= max_loc) {
                        return sector_name;
                    }
                }
            }
        }
    }
    
    return "";
}

int SpatialEnrichmentEngine::getAreaSequenceIndex(const std::string& area, 
                                                 const std::string& beam_path) const {
    if (!beamline_boundaries_.contains("beam_paths") || 
        !beamline_boundaries_["beam_paths"].contains(beam_path)) {
        return -1;
    }
    
    const auto& beam_info = beamline_boundaries_["beam_paths"][beam_path];
    if (beam_info.contains("sequence") && beam_info["sequence"].is_array()) {
        const auto& sequence = beam_info["sequence"];
        for (size_t i = 0; i < sequence.size(); ++i) {
            if (sequence[i] == area) {
                return static_cast<int>(i);
            }
        }
    }
    
    return -1;
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
