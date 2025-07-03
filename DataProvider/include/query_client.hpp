#ifdef QUERY_CLIENT_HPP
#define QUERY_CLIENT_HPP

#include <string>
#include <vector>
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
using QueryPVMetadataRequest = dp::service::query::QueryPVMetadataRequest;
using QueryPVMetadataResponse = dp::serivce::query::QueryPVMetadataResponse;
using QueryProviderRequest = dp::service::query::QueryProviderRequest;
using QueryProviderResponse = dp::service::query::QueryProviderResponse;
using QueryProviderMetadataRequest = dp::service::query::QueryProviderMetadataRequest;
using QueryProviderMetadataRequest = dp::service::query::QueryProviderMetadataResponse;

class QueryClient {
    public:
        explicit QueryClient(const std::string& server_address);
        QueryDataResponse queryData(const QueryDataRequest& request);
        QueryTableResponse queryTable(const QueryTableRequest& request);
        QueryPVMetadataResponse queryPVMetadata(const QueryPVMetadataRequest& request);
        QueryProviderResponse queryProvider(const QueryProviderRequest& request);
        QueryProviderMetadataResponse queryProviderMetadata(const QUeryProviderMetadataRequest& request);
        
        std::vector<QueryDataResponse> queryDataStream(const QueryDataRequest& request);
        
private:
    std::unique_ptr<dp::service::query::DpQueryService::Stub> stub_;

};

//Helper function declarations
Timestamp makeTimestamp(uint64_t epochSeconds, uint64_t nanoseconds);
QueryDataRequest makeQueryDataRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, const Timestamp& endTime, bool useSerializedDataColumns);
QueryTableRequest makeQueryTableRequest(const std::vector<std::string>& pvNames, const Timestamp& beginTime, const Timestamp& endTime, dp::service::query::QueryTableRequest::TableResultFormat format);
QueryPVMetadataRequest makeQueryPVMetadataRequest(const std::vector<std::string>& pvNames);
QueryPVMetadataRequest makeQueryPVMetadataRequestWithPattern(const std::string& pattern);
QueryProvidersRequest makeQueryProvidersRequest(const std::string& textSearch);
QueryProviderMetadataRequest makeQueryProviderMetadataRequest(const std::string& providerId);

#endif // QUERY_CLIENT_HPP

