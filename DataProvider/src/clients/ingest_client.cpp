#include "ingest_client.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>

// ========== StreamIngestionSession Implementation ==========

class IngestionClient::StreamIngestionSession::Impl {
public:
    std::shared_ptr<grpc::ClientWriter<IngestDataRequest>> writer;
    std::shared_ptr<grpc::ClientContext> context;
    std::vector<std::string> sent_request_ids;
    
    Impl(std::shared_ptr<grpc::ClientWriter<IngestDataRequest>> w,
         std::shared_ptr<grpc::ClientContext> ctx)
        : writer(w), context(ctx) {}
};

IngestionClient::StreamIngestionSession::StreamIngestionSession(
    std::shared_ptr<grpc::ClientWriter<IngestDataRequest>> writer,
    std::shared_ptr<grpc::ClientContext> context)
    : pImpl(std::make_unique<Impl>(writer, context)) {}

bool IngestionClient::StreamIngestionSession::SendData(const IngestDataRequest& request) {
    if (!pImpl->writer) return false;
    
    bool success = pImpl->writer->Write(request);
    if (success) {
        pImpl->sent_request_ids.push_back(request.clientrequestid());
    }
    return success;
}

bool IngestionClient::StreamIngestionSession::SendData(
    const std::string& provider_id,
    const std::string& client_request_id,
    const IngestionDataFrame& data_frame) {
    
    IngestDataRequest request;
    request.set_providerid(provider_id);
    request.set_clientrequestid(client_request_id);
    *request.mutable_ingestiondataframe() = data_frame;
    
    return SendData(request);
}

IngestDataStreamResponse IngestionClient::StreamIngestionSession::Finish() {
    IngestDataStreamResponse response;
    
    if (pImpl->writer) {
        pImpl->writer->WritesDone();
        grpc::Status status = pImpl->writer->Finish();
        
        // Response should be populated by the server
        // If not, we create a basic response
        if (!status.ok()) {
            auto* exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }
    
    return response;
}

void IngestionClient::StreamIngestionSession::Cancel() {
    if (pImpl->context) {
        pImpl->context->TryCancel();
    }
}

// ========== BidiStreamIngestionSession Implementation ==========

class IngestionClient::BidiStreamIngestionSession::Impl {
public:
    std::shared_ptr<grpc::ClientReaderWriter<IngestDataRequest, IngestDataResponse>> stream;
    std::shared_ptr<grpc::ClientContext> context;
    std::atomic<bool> sending_closed{false};
    
    Impl(std::shared_ptr<grpc::ClientReaderWriter<IngestDataRequest, IngestDataResponse>> s,
         std::shared_ptr<grpc::ClientContext> ctx)
        : stream(s), context(ctx) {}
};

IngestionClient::BidiStreamIngestionSession::BidiStreamIngestionSession(
    std::shared_ptr<grpc::ClientReaderWriter<IngestDataRequest, IngestDataResponse>> stream,
    std::shared_ptr<grpc::ClientContext> context)
    : pImpl(std::make_unique<Impl>(stream, context)) {}

bool IngestionClient::BidiStreamIngestionSession::SendData(const IngestDataRequest& request) {
    if (!pImpl->stream || pImpl->sending_closed) return false;
    return pImpl->stream->Write(request);
}

std::optional<IngestDataResponse> IngestionClient::BidiStreamIngestionSession::ReadResponse() {
    if (!pImpl->stream) return std::nullopt;
    
    IngestDataResponse response;
    if (pImpl->stream->Read(&response)) {
        return response;
    }
    return std::nullopt;
}

bool IngestionClient::BidiStreamIngestionSession::WaitForResponse(
    IngestDataResponse& response, int timeout_ms) {
    // Note: gRPC doesn't have built-in timeout for Read, so this is a simplified version
    // In production, you might want to use async operations with deadlines
    if (!pImpl->stream) return false;
    return pImpl->stream->Read(&response);
}

void IngestionClient::BidiStreamIngestionSession::CloseSending() {
    if (pImpl->stream && !pImpl->sending_closed) {
        pImpl->stream->WritesDone();
        pImpl->sending_closed = true;
    }
}

std::vector<IngestDataResponse> IngestionClient::BidiStreamIngestionSession::ReadAllResponses() {
    std::vector<IngestDataResponse> responses;
    IngestDataResponse response;
    
    while (pImpl->stream && pImpl->stream->Read(&response)) {
        responses.push_back(response);
    }
    
    return responses;
}

// ========== SubscriptionSession Implementation ==========

class IngestionClient::SubscriptionSession::Impl {
public:
    std::shared_ptr<grpc::ClientReaderWriter<SubscribeDataRequest, SubscribeDataResponse>> stream;
    std::shared_ptr<grpc::ClientContext> context;
    std::atomic<bool> active{true};
    std::atomic<bool> async_reading{false};
    std::thread async_reader;
    
    Impl(std::shared_ptr<grpc::ClientReaderWriter<SubscribeDataRequest, SubscribeDataResponse>> s,
         std::shared_ptr<grpc::ClientContext> ctx)
        : stream(s), context(ctx) {}
    
    ~Impl() {
        StopAsyncReading();
    }
    
    void StopAsyncReading() {
        async_reading = false;
        if (async_reader.joinable()) {
            async_reader.join();
        }
    }
};

IngestionClient::SubscriptionSession::SubscriptionSession(
    std::shared_ptr<grpc::ClientReaderWriter<SubscribeDataRequest, SubscribeDataResponse>> stream,
    std::shared_ptr<grpc::ClientContext> context)
    : pImpl(std::make_unique<Impl>(stream, context)) {}

bool IngestionClient::SubscriptionSession::Subscribe(const std::vector<std::string>& pv_names) {
    if (!pImpl->stream || !pImpl->active) return false;
    
    SubscribeDataRequest request;
    auto* new_sub = request.mutable_newsubscription();
    for (const auto& pv : pv_names) {
        new_sub->add_pvnames(pv);
    }
    
    return pImpl->stream->Write(request);
}

bool IngestionClient::SubscriptionSession::CancelSubscription() {
    if (!pImpl->stream || !pImpl->active) return false;
    
    SubscribeDataRequest request;
    request.mutable_cancelsubscription();  // Creates empty CancelSubscription message
    
    bool result = pImpl->stream->Write(request);
    pImpl->active = false;
    return result;
}

std::optional<SubscribeDataResponse> IngestionClient::SubscriptionSession::ReadResponse() {
    if (!pImpl->stream || !pImpl->active) return std::nullopt;
    
    SubscribeDataResponse response;
    if (pImpl->stream->Read(&response)) {
        return response;
    }
    return std::nullopt;
}

void IngestionClient::SubscriptionSession::StartAsyncReading(
    DataCallback on_data, ErrorCallback on_error) {
    
    if (pImpl->async_reading) return;
    
    pImpl->async_reading = true;
    pImpl->async_reader = std::thread([this, on_data, on_error]() {
        while (pImpl->async_reading && pImpl->active) {
            auto response = ReadResponse();
            if (response.has_value()) {
                if (response->has_subscribedataresult() && on_data) {
                    on_data(response->subscribedataresult());
                } else if (response->has_exceptionalresult() && on_error) {
                    on_error(response->exceptionalresult());
                }
            } else {
                // Stream ended or error
                pImpl->active = false;
                break;
            }
        }
    });
}

void IngestionClient::SubscriptionSession::StopAsyncReading() {
    pImpl->StopAsyncReading();
}

bool IngestionClient::SubscriptionSession::IsActive() const {
    return pImpl->active;
}

// ========== IngestionClient Implementation ==========

class IngestionClient::Impl {
public:
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<DpIngestionService::Stub> stub;
    CommonClient common_client;
    
    int default_timeout_seconds = 30;
    std::string last_error;
    ClientStats stats;
    
    Impl(std::shared_ptr<grpc::Channel> ch) 
        : channel(ch), stub(DpIngestionService::NewStub(ch)) {}
};

IngestionClient::IngestionClient(std::shared_ptr<grpc::Channel> channel)
    : pImpl(std::make_unique<Impl>(channel)) {}

IngestionClient::IngestionClient(const std::string& server_address)
    : pImpl(std::make_unique<Impl>(
        grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()))) {}

IngestionClient::~IngestionClient() = default;

// ========== Provider Registration ==========

std::optional<std::string> IngestionClient::RegisterProvider(
    const std::string& provider_name,
    const std::string& description,
    const std::vector<std::string>& tags,
    const std::vector<Attribute>& attributes) {
    
    auto response = RegisterProviderWithDetails(provider_name, description, tags, attributes);
    
    if (response.has_registrationresult()) {
        pImpl->stats.providers_registered++;
        return response.registrationresult().providerid();
    }
    
    if (response.has_exceptionalresult()) {
        pImpl->last_error = response.exceptionalresult().message();
        pImpl->stats.errors++;
    }
    
    return std::nullopt;
}

RegisterProviderResponse IngestionClient::RegisterProviderWithDetails(
    const std::string& provider_name,
    const std::string& description,
    const std::vector<std::string>& tags,
    const std::vector<Attribute>& attributes) {
    
    RegisterProviderRequest request;
    request.set_providername(provider_name);
    if (!description.empty()) {
        request.set_description(description);
    }
    
    for (const auto& tag : tags) {
        request.add_tags(tag);
    }
    
    for (const auto& attr : attributes) {
        *request.add_attributes() = attr;
    }
    
    RegisterProviderResponse response;
    grpc::ClientContext context;
    
    auto deadline = std::chrono::system_clock::now() + 
                   std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);
    
    grpc::Status status = pImpl->stub->registerProvider(&context, request, &response);
    
    if (!status.ok()) {
        pImpl->last_error = status.error_message();
        pImpl->stats.errors++;
        
        // Create exceptional result if not already present
        if (!response.has_exceptionalresult()) {
            auto* exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }
    
    return response;
}

// ========== Data Ingestion ==========

bool IngestionClient::IngestData(
    const std::string& provider_id,
    const std::string& client_request_id,
    const std::vector<DataColumn>& columns,
    const DataTimestamps& timestamps,
    const std::vector<std::string>& tags,
    const std::vector<Attribute>& attributes,
    const std::optional<EventMetadata>& event) {
    
    auto data_frame = CreateDataFrame(timestamps, columns);
    auto response = IngestDataWithResponse(provider_id, client_request_id, 
                                          data_frame, tags, attributes, event);
    
    if (response.has_ackresult()) {
        pImpl->stats.data_ingested++;
        return true;
    }
    
    if (response.has_exceptionalresult()) {
        pImpl->last_error = response.exceptionalresult().message();
        pImpl->stats.errors++;
    }
    
    return false;
}

IngestDataResponse IngestionClient::IngestDataWithResponse(
    const std::string& provider_id,
    const std::string& client_request_id,
    const IngestionDataFrame& data_frame,
    const std::vector<std::string>& tags,
    const std::vector<Attribute>& attributes,
    const std::optional<EventMetadata>& event) {
    
    IngestDataRequest request;
    request.set_providerid(provider_id);
    request.set_clientrequestid(client_request_id);
    *request.mutable_ingestiondataframe() = data_frame;
    
    for (const auto& tag : tags) {
        request.add_tags(tag);
    }
    
    for (const auto& attr : attributes) {
        *request.add_attributes() = attr;
    }
    
    if (event.has_value()) {
        *request.mutable_eventmetadata() = event.value();
    }
    
    IngestDataResponse response;
    grpc::ClientContext context;
    
    auto deadline = std::chrono::system_clock::now() + 
                   std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);
    
    grpc::Status status = pImpl->stub->ingestData(&context, request, &response);
    
    if (!status.ok()) {
        pImpl->last_error = status.error_message();
        pImpl->stats.errors++;
        
        if (!response.has_exceptionalresult()) {
            auto* exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }
    
    return response;
}

IngestionDataFrame IngestionClient::CreateDataFrame(
    const DataTimestamps& timestamps,
    const std::vector<DataColumn>& columns) {
    
    IngestionDataFrame frame;
    *frame.mutable_datatimestamps() = timestamps;
    
    for (const auto& column : columns) {
        *frame.add_datacolumns() = column;
    }
    
    return frame;
}

IngestionDataFrame IngestionClient::CreateDataFrameFromClock(
    const SamplingClock& clock,
    const std::vector<DataColumn>& columns) {
    
    DataTimestamps timestamps = pImpl->common_client.CreateDataTimestampsFromClock(clock);
    return CreateDataFrame(timestamps, columns);
}

// ========== Streaming Ingestion ==========

std::unique_ptr<IngestionClient::StreamIngestionSession> 
IngestionClient::CreateStreamIngestionSession() {
    auto context = std::make_shared<grpc::ClientContext>();
    
    auto deadline = std::chrono::system_clock::now() + 
                   std::chrono::seconds(pImpl->default_timeout_seconds * 10); // Longer timeout for streams
    context->set_deadline(deadline);
    
    IngestDataStreamResponse response;
    auto writer = std::shared_ptr<grpc::ClientWriter<IngestDataRequest>>(
        pImpl->stub->ingestDataStream(context.get(), &response));
    
    pImpl->stats.stream_sessions++;
    
    return std::make_unique<StreamIngestionSession>(writer, context);
}

std::unique_ptr<IngestionClient::BidiStreamIngestionSession> 
IngestionClient::CreateBidiStreamIngestionSession() {
    auto context = std::make_shared<grpc::ClientContext>();
    
    auto stream = std::shared_ptr<grpc::ClientReaderWriter<IngestDataRequest, IngestDataResponse>>(
        pImpl->stub->ingestDataBidiStream(context.get()));
    
    pImpl->stats.stream_sessions++;
    
    return std::make_unique<BidiStreamIngestionSession>(stream, context);
}

// ========== Request Status Query ==========

std::vector<RequestStatus> IngestionClient::QueryRequestStatusByProviderId(
    const std::string& provider_id) {
    
    std::vector<QueryRequestStatusCriterion> criteria;
    criteria.push_back(CreateProviderIdCriterion(provider_id));
    
    auto response = QueryRequestStatus(criteria);
    
    if (response.has_requeststatusresult()) {
        std::vector<RequestStatus> results;
        for (const auto& status : response.requeststatusresult().requeststatus()) {
            results.push_back(status);
        }
        return results;
    }
    
    return {};
}

std::vector<RequestStatus> IngestionClient::QueryRequestStatusByProviderName(
    const std::string& provider_name) {
    
    std::vector<QueryRequestStatusCriterion> criteria;
    criteria.push_back(CreateProviderNameCriterion(provider_name));
    
    auto response = QueryRequestStatus(criteria);
    
    if (response.has_requeststatusresult()) {
        std::vector<RequestStatus> results;
        for (const auto& status : response.requeststatusresult().requeststatus()) {
            results.push_back(status);
        }
        return results;
    }
    
    return {};
}

std::vector<RequestStatus> IngestionClient::QueryRequestStatusByRequestId(
    const std::string& request_id) {
    
    std::vector<QueryRequestStatusCriterion> criteria;
    criteria.push_back(CreateRequestIdCriterion(request_id));
    
    auto response = QueryRequestStatus(criteria);
    
    if (response.has_requeststatusresult()) {
        std::vector<RequestStatus> results;
        for (const auto& status : response.requeststatusresult().requeststatus()) {
            results.push_back(status);
        }
        return results;
    }
    
    return {};
}

std::vector<RequestStatus> IngestionClient::QueryRequestStatusByStatus(
    IngestionRequestStatus status) {
    
    std::vector<QueryRequestStatusCriterion> criteria;
    criteria.push_back(CreateStatusCriterion(status));
    
    auto response = QueryRequestStatus(criteria);
    
    if (response.has_requeststatusresult()) {
        std::vector<RequestStatus> results;
        for (const auto& status : response.requeststatusresult().requeststatus()) {
            results.push_back(status);
        }
        return results;
    }
    
    return {};
}

std::vector<RequestStatus> IngestionClient::QueryRequestStatusByTimeRange(
    const Timestamp& start_time,
    const Timestamp& end_time) {
    
    std::vector<QueryRequestStatusCriterion> criteria;
    criteria.push_back(CreateTimeRangeCriterion(start_time, end_time));
    
    auto response = QueryRequestStatus(criteria);
    
    if (response.has_requeststatusresult()) {
        std::vector<RequestStatus> results;
        for (const auto& status : response.requeststatusresult().requeststatus()) {
            results.push_back(status);
        }
        return results;
    }
    
    return {};
}

QueryRequestStatusResponse IngestionClient::QueryRequestStatus(
    const std::vector<QueryRequestStatusCriterion>& criteria) {
    
    QueryRequestStatusRequest request;
    for (const auto& criterion : criteria) {
        *request.add_criteria() = criterion;
    }
    
    QueryRequestStatusResponse response;
    grpc::ClientContext context;
    
    auto deadline = std::chrono::system_clock::now() + 
                   std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);
    
    grpc::Status status = pImpl->stub->queryRequestStatus(&context, request, &response);
    
    if (!status.ok()) {
        pImpl->last_error = status.error_message();
        pImpl->stats.errors++;
        
        if (!response.has_exceptionalresult()) {
            auto* exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }
    
    return response;
}

QueryRequestStatusCriterion IngestionClient::CreateProviderIdCriterion(
    const std::string& provider_id) {
    
    QueryRequestStatusCriterion criterion;
    criterion.mutable_provideridcriterion()->set_providerid(provider_id);
    return criterion;
}

QueryRequestStatusCriterion IngestionClient::CreateProviderNameCriterion(
    const std::string& provider_name) {
    
    QueryRequestStatusCriterion criterion;
    criterion.mutable_providernamecriterion()->set_providername(provider_name);
    return criterion;
}

QueryRequestStatusCriterion IngestionClient::CreateRequestIdCriterion(
    const std::string& request_id) {
    
    QueryRequestStatusCriterion criterion;
    criterion.mutable_requestidcriterion()->set_requestid(request_id);
    return criterion;
}

QueryRequestStatusCriterion IngestionClient::CreateStatusCriterion(
    IngestionRequestStatus status) {
    
    QueryRequestStatusCriterion criterion;
    criterion.mutable_statuscriterion()->add_status(status);
    return criterion;
}

QueryRequestStatusCriterion IngestionClient::CreateTimeRangeCriterion(
    const Timestamp& start, const Timestamp& end) {
    
    QueryRequestStatusCriterion criterion;
    auto* time_range = criterion.mutable_timerangecriterion();
    *time_range->mutable_begintime() = start;
    *time_range->mutable_endtime() = end;
    return criterion;
}

// ========== Data Subscription ==========

std::unique_ptr<IngestionClient::SubscriptionSession> 
IngestionClient::CreateSubscriptionSession() {
    
    auto context = std::make_shared<grpc::ClientContext>();
    
    auto stream = std::shared_ptr<grpc::ClientReaderWriter<SubscribeDataRequest, SubscribeDataResponse>>(
        pImpl->stub->subscribeData(context.get()));
    
    pImpl->stats.subscriptions++;
    
    return std::make_unique<SubscriptionSession>(stream, context);
}

std::future<void> IngestionClient::SubscribeToData(
    const std::vector<std::string>& pv_names,
    std::function<void(const SubscribeDataResult&)> callback,
    std::function<void(const std::string&)> error_callback) {
    
    return std::async(std::launch::async, [this, pv_names, callback, error_callback]() {
        auto session = CreateSubscriptionSession();
        
        if (!session->Subscribe(pv_names)) {
            if (error_callback) {
                error_callback("Failed to subscribe to PVs");
            }
            return;
        }
        
        // Read acknowledgment
        auto ack = session->ReadResponse();
        if (!ack.has_value() || !ack->has_ackresult()) {
            if (error_callback) {
                error_callback("Failed to receive subscription acknowledgment");
            }
            return;
        }
        
        // Start reading data
        while (session->IsActive()) {
            auto response = session->ReadResponse();
            if (response.has_value()) {
                if (response->has_subscribedataresult()) {
                    callback(response->subscribedataresult());
                } else if (response->has_exceptionalresult() && error_callback) {
                    error_callback(response->exceptionalresult().message());
                    break;
                }
            } else {
                break;  // Stream ended
            }
        }
    });
}

// ========== Utility Methods ==========

bool IngestionClient::IsConnected() const {
    return pImpl->channel->GetState(false) == GRPC_CHANNEL_READY;
}

grpc_connectivity_state IngestionClient::GetChannelState() const {
    return pImpl->channel->GetState(false);
}

bool IngestionClient::WaitForConnection(int timeout_seconds) {
    auto deadline = std::chrono::system_clock::now() + 
                   std::chrono::seconds(timeout_seconds);
    return pImpl->channel->WaitForConnected(deadline);
}

void IngestionClient::SetDefaultTimeout(int seconds) {
    pImpl->default_timeout_seconds = seconds;
}

int IngestionClient::GetDefaultTimeout() const {
    return pImpl->default_timeout_seconds;
}

std::string IngestionClient::GetLastError() const {
    return pImpl->last_error;
}

void IngestionClient::ClearLastError() {
    pImpl->last_error.clear();
}

IngestionClient::ClientStats IngestionClient::GetStats() const {
    return pImpl->stats;
}

void IngestionClient::ResetStats() {
    pImpl->stats = ClientStats{};
}

CommonClient& IngestionClient::GetCommonClient() {
    return pImpl->common_client;
}
