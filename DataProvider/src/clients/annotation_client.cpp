#include "annotation_client.hpp"
#include <grpcpp/create_channel.h>
#include <grpcpp/client_context.h>
#include <iostream>
#include <fstream>
#include <chrono>

using json = nlohmann::json;

// ===================== TimeRange Implementation =====================

Timestamp TimeRange::getStartTimestamp() const {
    return makeTimestamp(start_epoch_sec, start_nano);
}

Timestamp TimeRange::getEndTimestamp() const {
    return makeTimestamp(end_epoch_sec, end_nano);
}

// ===================== AnnotationClientConfig Implementation =====================

AnnotationClientConfig AnnotationClientConfig::fromConfigFile(const std::string& config_path) {
    AnnotationClientConfig config;
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

AnnotationClientConfig AnnotationClientConfig::fromJson(const json& j) {
    AnnotationClientConfig config;

    if (j.contains("server_connections")) {
        const auto& conn = j["server_connections"];
        if (conn.contains("annotation_server")) {
            config.server_address = conn["annotation_server"];
        }
        if (conn.contains("connection_timeout_seconds")) {
            config.connection_timeout_seconds = conn["connection_timeout_seconds"];
        }
        if (conn.contains("max_message_size_mb")) {
            config.max_message_size_mb = conn["max_message_size_mb"];
        }
    }

    return config;
}

// ===================== AnnotationClient Implementation =====================

AnnotationClient::AnnotationClient(const AnnotationClientConfig& config)
    : config_(config) {
    initializeConnection();
}

AnnotationClient::AnnotationClient(const std::string& server_address)
    : AnnotationClient(AnnotationClientConfig{}) {
    config_.server_address = server_address;
    initializeConnection();
}

AnnotationClient::~AnnotationClient() = default;

void AnnotationClient::initializeConnection() {
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
        std::string error = "Failed to connect to annotation server at " + config_.server_address;
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error("gRPC channel connection timeout");
    }

    stub_ = dp::service::annotation::DpAnnotationService::NewStub(channel);
}

// ===================== Core RPC Methods =====================

CreateDataSetResponse AnnotationClient::createDataSet(const CreateDataSetRequest& request) {
    CreateDataSetResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->createDataSet(&context, request, &response);

    if (!status.ok()) {
        std::string error = "CreateDataSet RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

QueryDataSetsResponse AnnotationClient::queryDataSets(const QueryDataSetsRequest& request) {
    QueryDataSetsResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryDataSets(&context, request, &response);

    if (!status.ok()) {
        std::string error = "QueryDataSets RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

CreateAnnotationResponse AnnotationClient::createAnnotation(const CreateAnnotationRequest& request) {
    CreateAnnotationResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->createAnnotation(&context, request, &response);

    if (!status.ok()) {
        std::string error = "CreateAnnotation RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

QueryAnnotationsResponse AnnotationClient::queryAnnotations(const QueryAnnotationsRequest& request) {
    QueryAnnotationsResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryAnnotations(&context, request, &response);

    if (!status.ok()) {
        std::string error = "QueryAnnotations RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

ExportDataResponse AnnotationClient::exportData(const ExportDataRequest& request) {
    ExportDataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->exportData(&context, request, &response);

    if (!status.ok()) {
        std::string error = "ExportData RPC failed: " + status.error_message();
        std::cerr << error << std::endl;
        last_error_ = error;
        throw std::runtime_error(error);
    }
    return response;
}

// ===================== Connection Management =====================

bool AnnotationClient::isConnected() const {
    return stub_ != nullptr;
}

void AnnotationClient::reconnect() {
    initializeConnection();
}

void AnnotationClient::updateConfig(const AnnotationClientConfig& config) {
    config_ = config;
    initializeConnection();
}

// ===================== Helper Functions Implementation =====================

Timestamp makeTimestamp(uint64_t epoch, uint64_t nano) {
    Timestamp ts;
    ts.set_epochseconds(epoch);
    ts.set_nanoseconds(nano);
    return ts;
}

DataSet makeDataSet(const std::string& name, const std::string& owner_id, 
                   const std::string& description, const std::vector<DataBlock>& data_blocks) {
    DataSet dataset;
    dataset.set_name(name);
    dataset.set_ownerid(owner_id);
    dataset.set_description(description);
    for (const auto& block : data_blocks) {
        *dataset.add_datablocks() = block;
    }
    return dataset;
}

DataBlock makeDataBlock(const TimeRange& time_range, const std::vector<std::string>& pv_names) {
    DataBlock block;
    *block.mutable_begintime() = time_range.getStartTimestamp();
    *block.mutable_endtime() = time_range.getEndTimestamp();
    for (const auto& pv : pv_names) {
        block.add_pvnames(pv);
    }
    return block;
}

CreateDataSetRequest makeCreateDataSetRequest(const DataSet& dataset) {
    CreateDataSetRequest request;
    *request.mutable_dataset() = dataset;
    return request;
}

CreateAnnotationRequest makeCreateAnnotationRequest(const std::string& owner_id, const std::string& name,
                                                   const std::vector<std::string>& dataset_ids) {
    CreateAnnotationRequest request;
    request.set_ownerid(owner_id);
    request.set_name(name);
    for (const auto& id : dataset_ids) {
        request.add_datasetids(id);
    }
    return request;
}

QueryDataSetsRequest makeQueryDataSetsByOwner(const std::string& owner_id) {
    QueryDataSetsRequest request;
    auto* criterion = request.add_criteria();
    auto* ownerCriterion = criterion->mutable_ownercriterion();
    ownerCriterion->set_ownerid(owner_id);
    return request;
}

QueryDataSetsRequest makeQueryDataSetsByText(const std::string& text) {
    QueryDataSetsRequest request;
    auto* criterion = request.add_criteria();
    auto* textCriterion = criterion->mutable_textcriterion();
    textCriterion->set_text(text);
    return request;
}

QueryDataSetsRequest makeQueryDataSetsByPvName(const std::string& pv_name) {
    QueryDataSetsRequest request;
    auto* criterion = request.add_criteria();
    auto* pvCriterion = criterion->mutable_pvnamecriterion();
    pvCriterion->set_name(pv_name);
    return request;
}

QueryDataSetsRequest makeQueryDataSetsById(const std::string& dataset_id) {
    QueryDataSetsRequest request;
    auto* criterion = request.add_criteria();
    auto* idCriterion = criterion->mutable_idcriterion();
    idCriterion->set_id(dataset_id);
    return request;
}

QueryAnnotationsRequest makeQueryAnnotationsByOwner(const std::string& owner_id) {
    QueryAnnotationsRequest request;
    auto* criterion = request.add_criteria();
    auto* ownerCriterion = criterion->mutable_ownercriterion();
    ownerCriterion->set_ownerid(owner_id);
    return request;
}

QueryAnnotationsRequest makeQueryAnnotationsByDataSet(const std::string& dataset_id) {
    QueryAnnotationsRequest request;
    auto* criterion = request.add_criteria();
    auto* datasetCriterion = criterion->mutable_datasetscriterion();
    datasetCriterion->set_datasetid(dataset_id);
    return request;
}

QueryAnnotationsRequest makeQueryAnnotationsByText(const std::string& text) {
    QueryAnnotationsRequest request;
    auto* criterion = request.add_criteria();
    auto* textCriterion = criterion->mutable_textcriterion();
    textCriterion->set_text(text);
    return request;
}

QueryAnnotationsRequest makeQueryAnnotationsByTag(const std::string& tag) {
    QueryAnnotationsRequest request;
    auto* criterion = request.add_criteria();
    auto* tagCriterion = criterion->mutable_tagscriterion();
    tagCriterion->set_tagvalue(tag);
    return request;
}

QueryAnnotationsRequest makeQueryAnnotationsById(const std::string& annotation_id) {
    QueryAnnotationsRequest request;
    auto* criterion = request.add_criteria();
    auto* idCriterion = criterion->mutable_idcriterion();
    idCriterion->set_id(annotation_id);
    return request;
}

ExportDataRequest makeExportDataRequest(const std::string& dataset_id, 
                                       ExportDataRequest::ExportOutputFormat format) {
    ExportDataRequest request;
    request.set_datasetid(dataset_id);
    request.set_outputformat(format);
    return request;
}

// ===================== Utility Functions =====================

namespace AnnotationUtils {
    TimeRange createTimeRange(uint64_t start_epoch_sec, uint64_t end_epoch_sec,
                             uint64_t start_nano, uint64_t end_nano) {
        return TimeRange(start_epoch_sec, start_nano, end_epoch_sec, end_nano);
    }

    uint64_t getCurrentEpochSeconds() {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    }

    TimeRange createRecentTimeRange(uint64_t duration_seconds) {
        uint64_t now = getCurrentEpochSeconds();
        return TimeRange(now - duration_seconds, 0, now, 0);
    }
}
