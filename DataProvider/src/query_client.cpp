#include "query_client.hpp"
#include <grcpp/create_channel.h>
#include <grpcpp/client_context.h>
#include <iostream>

// --- QueryClient Implementation ---
QueryClient::QueryClient(const std::string& server_address){
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());

    // Block until the channel is ready
    if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(5))){
        std::cerr << "Failed to connect to MLDP server at " << server_address << std::endl;
        throw std::runtime_error("gRPC channel connection timeout");
    }
    stub_ = dp::service::query::DpQueryService::NewStub(channel);
}

QueryDataResponse QueryCLient::queryData(const QueryDataRequest& request){
    QueryDataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryData(&context, request, &response)

    if(!status.ok()) {
        std::cerr << "QueryData RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryData RPC failed");
    }
    return response
}

QueryTableResponse QueryClient::queryTable(const QueryTableRequest& request){
    QueryTableResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryTable(&context, request, &response);
    
    if(!status.ok()){
        std::cerr << "QueryTable RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryTable RPC failed");
    }
    return response;
}

QueryPVMetadataResponse QueryClient::queryPVMetadata(const QueryPVMetadataRequest& request){
    QueryPVMetadatResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryPVMetadata(&context, request, &response);

    if(!status.ok()){
        std::cerr << "QueryPVMetadata RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryPVMetadata RPC failed");
    }
    return response;
}

QueryProviderResponse QueryClient::queryProvider(const QueryProviderRequest& request){
    QueryProviderResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryProviders(&context, request, &response);

    if(!status.ok()){
        std::cerr << "QueryProvider RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryProvider RPC failed");
    }
    return response;
}

QueryProviderMetadataResponse QueryClient::queryProviderMetadata(const QueryProviderMetadataRequest& request){
    QueryProviderMetadataResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->queryProviderMetadata(&context, request, &response);

    if(!status.ok()){
        std::cerr << "QueryProviderMetadata RPC failed: " << status.error_message() << std::endl;
        throw std::runtime_error("QueryProviderMetadata RPC failed");
    }
    return response'
}

std::vector<QueryDataResponse> QueryClient::queryDataStream(const QueryDataRequest& request){
    std::vector<QueryDataResponse> responses;
    grpc::ClientContext context;

    // Create the read for the server-side stream
    std::unique_ptr<grpc::ClientReader<QueryDataResponse>> reader(
        stub_->queryDataStream(&context, request);
    );

    QueryDataResponse response;
    while(reader->Read(&response)){
        responses.push_back(response);
    }

    grpc::Status status = reader->Finish();

    if(!status.ok()){
        std::cerr << "QueryDataStream RPC failed: " << status.error_code() << ": " << status.error_message() << std::endl;
        throw std::runtime_error("QueryDataStream RPC failed");
    }
    std::cout << "QueryDataStream Success: received " << response.size() << " response" << std::endl;
    return responses;

}

// --- Helper Function Implementations ---
Timestamp makeTimeStamp(uint64_t epoch, uint64_t nano) {
    Timestamp ts;
    ts.set_epochseconds(epoch);
    ts.set_nanoseconds(nano);
    return ts;
}

QueryDataRequest makeQueryDataRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, const Timestamp& endTime, bool useSerializedDataColumns){
    QueryDataRequest request;
    auto* querySpec = request.mutable_queryspec();
    
    *querySpec->mutable_begintime() = beginTime;
    *querySpec->mutable_endtime() = endTime;
    querySpec->set_useserializeddatacolumns(useSerializedDataColumns);

    for(const auto& pvName : pvNames){
        querySpec ->add_pvnames(pvName);
    }
    
    return request;
}

QueryTableRequest makeQueryTableRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, const Timestamp& endTime, dp::service::query::QueryTableRequest::TableResultFormat format){
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

QueryPVMetadataRequest makeQueryPVMetadataRequest(const std::vector<std::string>& pvNames){
    QueryPVMetadataRequest request;
    auto* pvNameList = request.mutable_pvnamelist();

    for (const auto& pvName : pvNames){
        pvNameList->add_pvnames(pvName);
    }
    
    return request
}

QueryPVMetadataRequest makeQueryPVMetadataRequestWithPattern(const std::string& pattern){
    QueryPVMetadataRequest request'
    auto* pvNamePattern = request.mutable_pvnamepattern();
    pvNamePatter->set_pattern(pattern);
    return request;
}

QueryProviderRequest makeQueryProvidersRequest(const std::string& textSearch){
    QueryProviderRequest request;

    if(!textSearch.empty()){
        auto* criterion = request.add_criteria();
        auto* textCriterion = criterion->mutable_textcriterion();
        textCriterion->set_text(textSearch);
    }

    return request;
}

QueryProviderMetadataRequest makeQueryProviderMetadataRequest(const std::string& providerId){
    QueryProviderMetadataRequest request;
    request.set_providerid(providerid);
    return request;
}

