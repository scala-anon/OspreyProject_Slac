#include "metadata_recovery.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

// ===================== MetadataRecoveryEngine Implementation =====================

bool MetadataRecoveryEngine::discoverAllAvailablePvs() {
    try {
        // First, discover all providers
        if (!discoverAllProviders()) {
            std::cerr << "Failed to discover providers" << std::endl;
            return false;
        }

        // Get PVs from each provider
        std::set<std::string> all_pvs;
        for (const auto& [provider_id, provider_info] : provider_info_) {
            auto provider_pvs = getPvsFromProvider(provider_id);
            all_pvs.insert(provider_pvs.begin(), provider_pvs.end());
        }

        // Convert to vector and process in batches
        std::vector<std::string> pv_list(all_pvs.begin(), all_pvs.end());
        auto batches = createBatches(pv_list);

        std::cout << "Discovering metadata for " << pv_list.size()
                  << " PVs in " << batches.size() << " batches..." << std::endl;

        for (size_t i = 0; i < batches.size(); ++i) {
            std::cout << "Processing batch " << (i + 1) << "/" << batches.size() << std::endl;
            processPvBatch(batches[i]);
        }

        // Enrich discovered metadata
        enrichMetadataFromNaming();

        std::cout << "Discovery complete. Found metadata for "
                  << discovered_metadata_.size() << " PVs" << std::endl;

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error during PV discovery: " << e.what() << std::endl;
        return false;
    }
}

bool MetadataRecoveryEngine::validateExistingMetadata(const std::vector<std::string>& known_pvs) {
    missing_pvs_.clear();

    // Check each known PV against the query service
    auto batches = createBatches(known_pvs);

    for (const auto& batch : batches) {
        auto request = makeQueryPvMetadataRequest(batch);

        try {
            auto response = query_client_.queryPvMetadata(request);

            if (response.has_exceptionalresult()) {
                std::cerr << "Error validating batch: "
                          << response.exceptionalresult().message() << std::endl;
                continue;
            }

            if (response.has_metadataresult()) {
                // Track which PVs were found
                std::set<std::string> found_pvs;
                for (const auto& pv_info : response.metadataresult().pvinfos()) {
                    found_pvs.insert(pv_info.pvname());
                }

                // Identify missing PVs
                for (const auto& pv : batch) {
                    if (found_pvs.find(pv) == found_pvs.end()) {
                        missing_pvs_.push_back(pv);
                    }
                }

                processPvMetadataResponse(response);
            }

        } catch (const std::exception& e) {
            std::cerr << "Error validating PV batch: " << e.what() << std::endl;
            // Add entire batch to missing list as fallback
            missing_pvs_.insert(missing_pvs_.end(), batch.begin(), batch.end());
        }
    }

    std::cout << "Validation complete. " << missing_pvs_.size()
              << " PVs are missing or invalid" << std::endl;

    return missing_pvs_.empty();
}

std::vector<std::string> MetadataRecoveryEngine::discoverPvsByPattern(const std::string& pattern) {
    std::vector<std::string> discovered_pvs;

    try {
        auto request = makeQueryPvMetadataRequestWithPattern(pattern);
        auto response = query_client_.queryPvMetadata(request);

        if (response.has_metadataresult()) {
            for (const auto& pv_info : response.metadataresult().pvinfos()) {
                discovered_pvs.push_back(pv_info.pvname());
            }
            processPvMetadataResponse(response);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error discovering PVs by pattern '" << pattern
                  << "': " << e.what() << std::endl;
    }

    return discovered_pvs;
}

std::vector<std::string> MetadataRecoveryEngine::discoverPvsByArea(const std::string& area) {
    // Stub implementation - would use spatial engine if available
    std::string pattern = area + ":.*:.*:.*";
    return discoverPvsByPattern(pattern);
}

std::vector<std::string> MetadataRecoveryEngine::discoverPvsByDeviceType(const std::string& device_type) {
    // Map device types to PV prefixes
    std::string pattern;
    if (device_type == "beam_position_monitor") {
        pattern = "BPMS:.*:.*:.*";
    } else if (device_type == "quadrupole") {
        pattern = "QUAD:.*:.*:.*";
    } else if (device_type == "klystron") {
        pattern = "KLYS:.*:.*:.*";
    } else {
        pattern = ".*:.*:.*:.*"; // Default to all
    }
    
    return discoverPvsByPattern(pattern);
}

bool MetadataRecoveryEngine::discoverAllProviders() {
    try {
        auto request = makeQueryProvidersRequest(); // Empty request gets all providers
        auto response = query_client_.queryProviders(request);

        if (response.has_exceptionalresult()) {
            std::cerr << "Error discovering providers: "
                      << response.exceptionalresult().message() << std::endl;
            return false;
        }

        processProviderResponse(response);

        // Get detailed metadata for each provider
        for (const auto& [provider_id, provider_info] : provider_info_) {
            auto metadata_request = makeQueryProviderMetadataRequest(provider_id);
            auto metadata_response = query_client_.queryProviderMetadata(metadata_request);

            if (metadata_response.has_metadataresult()) {
                processProviderMetadata(metadata_response);
            }
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error discovering providers: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> MetadataRecoveryEngine::getPvsFromProvider(const std::string& provider_id) {
    auto it = provider_info_.find(provider_id);
    if (it != provider_info_.end()) {
        return it->second.pv_names;
    }
    return {};
}

bool MetadataRecoveryEngine::enrichMetadataFromNaming() {
    for (auto& [pv_name, metadata] : discovered_metadata_) {
        metadata.area = inferAreaFromPvName(pv_name);
        metadata.device_type = inferDeviceTypeFromPvName(pv_name);
        metadata.signal_type = inferSignalTypeFromPvName(pv_name);
        metadata.beam_path = inferBeamPathFromArea(metadata.area);
    }
    return true;
}

bool MetadataRecoveryEngine::inferSpatialInformation() {
    // Stub implementation
    return true;
}

bool MetadataRecoveryEngine::categorizeByDeviceType() {
    // Stub implementation
    return true;
}

bool MetadataRecoveryEngine::recoverMissingMetadata(const std::vector<std::string>& missing_pvs) {
    // Stub implementation
    return true;
}

bool MetadataRecoveryEngine::importExistingMetadata(const std::string& input_file) {
    // Stub implementation
    return true;
}

std::vector<std::string> MetadataRecoveryEngine::findPvsWithInconsistentData() {
    // Stub implementation
    return {};
}

std::map<std::string, std::vector<std::string>> MetadataRecoveryEngine::groupPvsByProvider() {
    std::map<std::string, std::vector<std::string>> grouped;
    
    for (const auto& [provider_id, provider_info] : provider_info_) {
        grouped[provider_id] = provider_info.pv_names;
    }
    
    return grouped;
}

std::vector<std::string> MetadataRecoveryEngine::findInactivePvs(uint64_t cutoff_hours) {
    inactive_pvs_.clear();
    uint64_t cutoff_time = QueryUtils::getCurrentEpochSeconds() - (cutoff_hours * 3600);

    for (const auto& [pv_name, metadata] : discovered_metadata_) {
        if (!metadata.hasRecentData(cutoff_time)) {
            inactive_pvs_.push_back(pv_name);
        }
    }

    return inactive_pvs_;
}

bool MetadataRecoveryEngine::exportDiscoveredMetadata(const std::string& output_file, const std::string& format) {
    try {
        if (format == "json") {
            nlohmann::json output;

            for (const auto& [pv_name, metadata] : discovered_metadata_) {
                nlohmann::json pv_json;
                pv_json["pv_name"] = metadata.pv_name;
                pv_json["area"] = metadata.area;
                pv_json["device_type"] = metadata.device_type;
                pv_json["signal_type"] = metadata.signal_type;
                pv_json["beam_path"] = metadata.beam_path;
                pv_json["data_type"] = metadata.data_type_name;
                pv_json["num_buckets"] = metadata.num_buckets;
                pv_json["sample_count"] = metadata.sample_count;
                pv_json["first_data_time"] = metadata.first_data_timestamp.epochseconds();
                pv_json["last_data_time"] = metadata.last_data_timestamp.epochseconds();

                output["pvs"][pv_name] = pv_json;
            }

            output["metadata"]["total_pvs"] = discovered_metadata_.size();
            output["metadata"]["discovery_time"] = QueryUtils::getCurrentEpochSeconds();
            output["metadata"]["inactive_pvs"] = inactive_pvs_.size();

            std::ofstream file(output_file);
            file << output.dump(2);
            return true;
        }

        return false;

    } catch (const std::exception& e) {
        std::cerr << "Error exporting metadata: " << e.what() << std::endl;
        return false;
    }
}

// ===================== Private Helper Methods =====================

void MetadataRecoveryEngine::processPvMetadataResponse(const QueryPvMetadataResponse& response) {
    if (!response.has_metadataresult()) return;

    for (const auto& pv_info : response.metadataresult().pvinfos()) {
        DiscoveredPvMetadata metadata;
        metadata.pv_name = pv_info.pvname();
        metadata.last_bucket_id = pv_info.lastbucketid();
        metadata.data_type_case = pv_info.lastbucketdatatypecase();
        metadata.data_type_name = pv_info.lastbucketdatatype();
        metadata.timestamps_case = pv_info.lastbucketdatatimestampscase();
        metadata.timestamps_type = pv_info.lastbucketdatatimestampstype();
        metadata.sample_count = pv_info.lastbucketsamplecount();
        metadata.sample_period_nanos = pv_info.lastbucketsampleperiod();
        metadata.first_data_timestamp = pv_info.firstdatatimestamp();
        metadata.last_data_timestamp = pv_info.lastdatatimestamp();
        metadata.num_buckets = pv_info.numbuckets();

        discovered_metadata_[metadata.pv_name] = metadata;
    }
}

void MetadataRecoveryEngine::processProviderResponse(const QueryProvidersResponse& response) {
    if (!response.has_providersresult()) return;

    for (const auto& provider_info : response.providersresult().providerinfos()) {
        ProviderInfo info;
        info.id = provider_info.id();
        info.name = provider_info.name();
        info.description = provider_info.description();

        for (const auto& tag : provider_info.tags()) {
            info.tags.push_back(tag);
        }

        for (const auto& attr : provider_info.attributes()) {
            info.attributes.emplace_back(attr.name(), attr.value());
        }

        provider_info_[info.id] = info;
    }
}

void MetadataRecoveryEngine::processProviderMetadata(const QueryProviderMetadataResponse& response) {
    if (!response.has_metadataresult()) return;

    for (const auto& provider_metadata : response.metadataresult().providermetadatas()) {
        auto it = provider_info_.find(provider_metadata.id());
        if (it != provider_info_.end()) {
            for (const auto& pv_name : provider_metadata.pvnames()) {
                it->second.pv_names.push_back(pv_name);
            }
            it->second.first_bucket_time = provider_metadata.firstbuckettime();
            it->second.last_bucket_time = provider_metadata.lastbuckettime();
            it->second.num_buckets = provider_metadata.numbuckets();
        }
    }
}

std::string MetadataRecoveryEngine::inferAreaFromPvName(const std::string& pv_name) {
    // Common EPICS naming patterns: AREA:DEVICE:INSTANCE:SIGNAL
    std::regex area_pattern(R"(^([A-Z0-9]+):)");
    std::smatch match;

    if (std::regex_search(pv_name, match, area_pattern)) {
        std::string potential_area = match[1].str();

        // Check against known area patterns
        std::vector<std::string> known_areas = {
            "BPMS", "QUAD", "KLYS", "TCAV", "RFOF", "ACCL", "BEND", "XCOR", "YCOR",
            "DMPH", "LTUH", "DMPS", "LI20", "LI21", "LI22", "LI23", "LI24", "LI25"
        };

        for (const auto& area : known_areas) {
            if (potential_area.find(area) != std::string::npos) {
                return potential_area;
            }
        }
    }

    return "UNKNOWN";
}

std::string MetadataRecoveryEngine::inferDeviceTypeFromPvName(const std::string& pv_name) {
    if (pv_name.find("BPMS:") == 0) return "beam_position_monitor";
    if (pv_name.find("QUAD:") == 0) return "quadrupole";
    if (pv_name.find("KLYS:") == 0) return "klystron";
    if (pv_name.find("TCAV:") == 0) return "transverse_cavity";
    if (pv_name.find("BEND:") == 0) return "dipole";
    if (pv_name.find("XCOR:") == 0 || pv_name.find("YCOR:") == 0) return "corrector";
    if (pv_name.find("WIRE:") == 0) return "wire_scanner";
    if (pv_name.find("TORO:") == 0) return "toroidal_current_monitor";

    return "unknown";
}

std::string MetadataRecoveryEngine::inferSignalTypeFromPvName(const std::string& pv_name) {
    if (pv_name.find(":X") != std::string::npos) return "position_x";
    if (pv_name.find(":Y") != std::string::npos) return "position_y";
    if (pv_name.find(":TMIT") != std::string::npos) return "charge";
    if (pv_name.find(":BCTRL") != std::string::npos) return "magnetic_field";
    if (pv_name.find(":BACT") != std::string::npos) return "magnetic_field_readback";
    if (pv_name.find(":EDES") != std::string::npos) return "energy_setpoint";
    if (pv_name.find(":EACT") != std::string::npos) return "energy_readback";
    if (pv_name.find(":PWR") != std::string::npos) return "rf_power";
    if (pv_name.find(":PHAS") != std::string::npos) return "rf_phase";

    return "unknown";
}

std::string MetadataRecoveryEngine::inferBeamPathFromArea(const std::string& area) {
    // Stub implementation - would use spatial dictionaries
    if (area.find("LTUH") != std::string::npos) return "CU_HXR";
    if (area.find("DMPH") != std::string::npos) return "CU_HXR";
    if (area.find("DMPS") != std::string::npos) return "SC_SXR";
    
    return "UNKNOWN";
}

void MetadataRecoveryEngine::processPvBatch(const std::vector<std::string>& pv_batch) {
    auto request = makeQueryPvMetadataRequest(pv_batch);

    try {
        auto response = query_client_.queryPvMetadata(request);

        if (response.has_exceptionalresult()) {
            std::cerr << "Batch error: " << response.exceptionalresult().message() << std::endl;
            return;
        }

        processPvMetadataResponse(response);

    } catch (const std::exception& e) {
        std::cerr << "Error processing batch: " << e.what() << std::endl;
    }
}

std::vector<std::vector<std::string>> MetadataRecoveryEngine::createBatches(const std::vector<std::string>& pvs) {
    std::vector<std::vector<std::string>> batches;

    for (size_t i = 0; i < pvs.size(); i += batch_size_) {
        size_t end = std::min(i + batch_size_, pvs.size());
        batches.emplace_back(pvs.begin() + i, pvs.begin() + end);
    }

    return batches;
}
