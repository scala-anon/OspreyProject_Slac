#include "ingest_client.hpp"
#include <grpcpp/create_channel.h>
#include <grpcpp/client_context.h>
#include <iostream>

// --- IngestClient Implementation ---
IngestClient::IngestClient(const std::string& server_address) {
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    
    // Block until the channel is ready
    if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(5))) {
        std::cerr << "Failed to connect to MLDP server at " << server_address << std::endl;
        throw std::runtime_error("gRPC channel connection timeout");
    }
    
    stub_ = dp::service::ingestion::DpIngestionService::NewStub(channel);
}

dp::service::ingestion::RegisterProviderResponse
IngestClient::sendRegisterProvider(const RegisterProviderRequest& request) {
    RegisterProviderResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->registerProvider(&context, request, &response);
    
    if (!status.ok()) {
        std::cerr << "RegisterProvider RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("RegisterProvider RPC failed");
    }
    return response;
}

std::string IngestClient::ingestData(const IngestDataRequest& request) {
    dp::service::ingestion::IngestDataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->ingestData(&context, request, &response);
    
    if (status.ok() && response.has_ackresult()) {
        std::cout << "Ack: Rows=" << response.ackresult().numrows()
                  << ", Cols=" << response.ackresult().numcolumns() << "\n";
        return "IngestData Success";
    } else if (status.ok()) {
        std::cerr << "No AckResult in response.\n";
        return "IngestData Failed";
    } else {
        std::cerr << status.error_code() << ": " << status.error_message() << "\n";
        return "RPC Failed";
    }
}

std::string IngestClient::ingestDataStream(const std::vector<IngestDataRequest>& requests) {
    if (requests.empty()) {
        std::cout << "No requests to send in stream" << std::endl;
        return "IngestDataStream Empty";
    }
    
    grpc::ClientContext context;
    dp::service::ingestion::IngestDataStreamResponse response;
    
    // Create the writer for the client-side stream
    std::unique_ptr<grpc::ClientWriter<IngestDataRequest>> writer(
        stub_->ingestDataStream(&context, &response)
    );
    
    // Send all requests in the stream
    size_t sent_count = 0;
    for (const auto& request : requests) {
        if (!writer->Write(request)) {
            std::cerr << "Failed to write request to stream after " << sent_count << " requests" << std::endl;
            break; // Stream closed or error
        }
        sent_count++;
    }
    
    // Close the stream and get response
    writer->WritesDone();
    grpc::Status status = writer->Finish();
    
    if (status.ok()) {
        if (response.has_ingestdatastreamresult()) {
            std::cout << "Stream Success: " << response.ingestdatastreamresult().numrequests() 
                      << " requests processed (sent " << sent_count << " of " << requests.size() << ")" << std::endl;
            
            // Show all client request IDs that were received
            if (response.clientrequestids_size() > 0) {
                std::cout << "Received " << response.clientrequestids_size() << " client request IDs" << std::endl;
            }
            
            // Print any rejected requests
            if (response.rejectedrequestids_size() > 0) {
                std::cout << "Warning: " << response.rejectedrequestids_size() << " requests were rejected:" << std::endl;
                for (int i = 0; i < response.rejectedrequestids_size(); i++) {
                    std::cout << "  - " << response.rejectedrequestids(i) << std::endl;
                }
                return "IngestDataStream Partial Success";
            }
            
            return "IngestDataStream Success";
            
        } else if (response.has_exceptionalresult()) {
            std::cerr << "Stream rejected: " << response.exceptionalresult().message() << std::endl;
            
            // Still show rejected request IDs if available
            if (response.rejectedrequestids_size() > 0) {
                std::cerr << "Rejected request IDs (" << response.rejectedrequestids_size() << "): ";
                for (int i = 0; i < response.rejectedrequestids_size(); i++) {
                    std::cerr << response.rejectedrequestids(i);
                    if (i < response.rejectedrequestids_size() - 1) std::cerr << ", ";
                }
                std::cerr << std::endl;
            }
            
            return "IngestDataStream Rejected";
        } else {
            std::cerr << "Unexpected response format from ingestDataStream" << std::endl;
            return "IngestDataStream Unexpected Response";
        }
    } else {
        std::cerr << "Stream RPC failed: " << status.error_code() 
                  << ": " << status.error_message() << std::endl;
        return "IngestDataStream RPC Failed";
    }
}

// --- Helper Function Implementations ---
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

dp::service::ingestion::IngestDataRequest::IngestionDataFrame
makeIngestionDataFrame(const SamplingClock& clock, const std::vector<DataColumn>& cols) {
    dp::service::ingestion::IngestDataRequest::IngestionDataFrame frame;
    *frame.mutable_datatimestamps()->mutable_samplingclock() = clock;
    for (const auto& c : cols)
        *frame.add_datacolumns() = c;
    return frame;
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
    *req.mutable_ingestiondataframe() = makeIngestionDataFrame(clock, columns);
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
