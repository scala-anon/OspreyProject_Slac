#include "query_client.hpp"
#include <grpcpp/create_channel.h>
#include <grpcpp/client_context.h>
#include <iostream>
#include <chrono>

// --- QueryClient Implementation ---
QueryClient::QueryClient(const std::string& server_address) {
    
    // Create channel with increased message size limits
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(67108864);
    args.SetMaxSendMessageSize(67108864);

    auto channel = grpc::CreateCustomChannel(
        server_address,
        grpc::InsecureChannelCredentials(),
        args
    );

    // Block until the channel is ready
    if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(5))) {
        std::cerr << "Failed to connect to MLDP server at " << server_address << std::endl;
        throw std::runtime_error("gRPC channel connection timeout");
    }
    
    stub_ = dp::service::query::DpQueryService::NewStub(channel);
}

QueryDataResponse QueryClient::queryData(const QueryDataRequest& request) {
    QueryDataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryData(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "QueryData RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryData RPC failed");
    }
    return response;
}

QueryTableResponse QueryClient::queryTable(const QueryTableRequest& request) {
    QueryTableResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryTable(&context, request, &response);
    
    if (!status.ok()) {
        std::cerr << "QueryTable RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryTable RPC failed");
    }
    return response;
}

QueryPvMetadataResponse QueryClient::queryPvMetadata(const QueryPvMetadataRequest& request) {
    QueryPvMetadataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryPvMetadata(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "QueryPvMetadata RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryPvMetadata RPC failed");
    }
    return response;
}

QueryProvidersResponse QueryClient::queryProviders(const QueryProvidersRequest& request) {
    QueryProvidersResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryProviders(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "QueryProviders RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryProviders RPC failed");
    }
    return response;
}

QueryProviderMetadataResponse QueryClient::queryProviderMetadata(const QueryProviderMetadataRequest& request) {
    QueryProviderMetadataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryProviderMetadata(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "QueryProviderMetadata RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryProviderMetadata RPC failed");
    }
    return response;
}

std::vector<QueryDataResponse> QueryClient::queryDataStream(const QueryDataRequest& request) {
    std::vector<QueryDataResponse> responses;
    grpc::ClientContext context;

    // Create the reader for the server-side stream
    std::unique_ptr<grpc::ClientReader<QueryDataResponse>> reader(
        stub_->queryDataStream(&context, request)
    );

    QueryDataResponse response;
    while (reader->Read(&response)) {
        responses.push_back(response);
    }

    grpc::Status status = reader->Finish();

    if (!status.ok()) {
        std::cerr << "QueryDataStream RPC failed: " << status.error_code() 
                  << ": " << status.error_message() << std::endl;
        throw std::runtime_error("QueryDataStream RPC failed");
    }
    
    std::cout << "QueryDataStream Success: received " << responses.size() << " responses" << std::endl;
    return responses;
}

// --- Helper Function Implementations ---
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
