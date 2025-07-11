#ifndef QUERY_CLIENT_HPP
#define QUERY_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "common.pb.h"
#include "query.pb.h"
#include "query.grpc.pb.h"

using Timestamp = ::Timestamp;
using Attribute = ::Attribute;
using DataColumn = ::DataColumn;
using DataValue = ::DataValue;
using QueryDataRequest = dp::service::query::QueryDataRequest;
using QueryDataResponse = dp::service::query::QueryDataResponse;
using QueryTableRequest = dp::service::query::QueryTableRequest;
using QueryTableResponse = dp::service::query::QueryTableResponse;
using QueryPvMetadataRequest = dp::service::query::QueryPvMetadataRequest;
using QueryPvMetadataResponse = dp::service::query::QueryPvMetadataResponse;
using QueryProvidersRequest = dp::service::query::QueryProvidersRequest;
using QueryProvidersResponse = dp::service::query::QueryProvidersResponse;
using QueryProviderMetadataRequest = dp::service::query::QueryProviderMetadataRequest;
using QueryProviderMetadataResponse = dp::service::query::QueryProviderMetadataResponse;

class QueryClient {
public:
    explicit QueryClient(const std::string& server_address = "localhost:50052");
    QueryDataResponse queryData(const QueryDataRequest& request);
    QueryTableResponse queryTable(const QueryTableRequest& request);
    QueryPvMetadataResponse queryPvMetadata(const QueryPvMetadataRequest& request);
    QueryProvidersResponse queryProviders(const QueryProvidersRequest& request);
    QueryProviderMetadataResponse queryProviderMetadata(const QueryProviderMetadataRequest& request);
    
    std::vector<QueryDataResponse> queryDataStream(const QueryDataRequest& request);
    
private:
    std::unique_ptr<dp::service::query::DpQueryService::Stub> stub_;
};

// Helper function declarations
Timestamp makeTimestamp(uint64_t epochSeconds, uint64_t nanoseconds);
QueryDataRequest makeQueryDataRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, const Timestamp& endTime, bool useSerializedDataColumns);
QueryTableRequest makeQueryTableRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, const Timestamp& endTime, dp::service::query::QueryTableRequest::TableResultFormat format);
QueryPvMetadataRequest makeQueryPvMetadataRequest(const std::vector<std::string>& pvNames);
QueryPvMetadataRequest makeQueryPvMetadataRequestWithPattern(const std::string& pattern);
QueryProvidersRequest makeQueryProvidersRequest(const std::string& textSearch);
QueryProviderMetadataRequest makeQueryProviderMetadataRequest(const std::string& providerId);

#endif // QUERY_CLIENT_HPP
