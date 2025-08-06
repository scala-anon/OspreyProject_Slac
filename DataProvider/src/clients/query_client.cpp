#include "query_client.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>

// ========== StreamQuerySession Implementation ==========

class QueryClient::StreamQuerySession::Impl
{
public:
    std::shared_ptr<grpc::ClientReader<QueryDataResponse>> reader;
    std::shared_ptr<grpc::ClientContext> context;
    std::atomic<bool> done{false};

    Impl(std::shared_ptr<grpc::ClientReader<QueryDataResponse>> r,
         std::shared_ptr<grpc::ClientContext> ctx)
        : reader(r), context(ctx) {}
};

QueryClient::StreamQuerySession::StreamQuerySession(
    std::shared_ptr<grpc::ClientReader<QueryDataResponse>> reader,
    std::shared_ptr<grpc::ClientContext> context)
    : pImpl(std::make_unique<Impl>(reader, context)) {}

std::optional<QueryDataResponse> QueryClient::StreamQuerySession::ReadNext()
{
    if (pImpl->done || !pImpl->reader)
        return std::nullopt;

    QueryDataResponse response;
    if (pImpl->reader->Read(&response))
    {
        return response;
    }

    pImpl->done = true;
    return std::nullopt;
}

std::vector<QueryDataResponse> QueryClient::StreamQuerySession::ReadAll()
{
    std::vector<QueryDataResponse> responses;

    while (auto response = ReadNext())
    {
        responses.push_back(response.value());
    }

    return responses;
}

std::vector<DataBucket> QueryClient::StreamQuerySession::ReadAllDataBuckets()
{
    std::vector<DataBucket> all_buckets;

    while (auto response = ReadNext())
    {
        if (response->has_querydata())
        {
            const auto &query_data = response->querydata();
            for (const auto &bucket : query_data.databuckets())
            {
                all_buckets.push_back(bucket);
            }
        }
    }

    return all_buckets;
}

void QueryClient::StreamQuerySession::Cancel()
{
    if (pImpl->context)
    {
        pImpl->context->TryCancel();
    }
    pImpl->done = true;
}

bool QueryClient::StreamQuerySession::IsDone() const
{
    return pImpl->done;
}

// ========== BidiQuerySession Implementation ==========

class QueryClient::BidiQuerySession::Impl
{
public:
    std::shared_ptr<grpc::ClientReaderWriter<QueryDataRequest, QueryDataResponse>> stream;
    std::shared_ptr<grpc::ClientContext> context;
    std::atomic<bool> sending_closed{false};
    bool initial_query_sent{false};

    Impl(std::shared_ptr<grpc::ClientReaderWriter<QueryDataRequest, QueryDataResponse>> s,
         std::shared_ptr<grpc::ClientContext> ctx)
        : stream(s), context(ctx) {}
};

QueryClient::BidiQuerySession::BidiQuerySession(
    std::shared_ptr<grpc::ClientReaderWriter<QueryDataRequest, QueryDataResponse>> stream,
    std::shared_ptr<grpc::ClientContext> context)
    : pImpl(std::make_unique<Impl>(stream, context)) {}

bool QueryClient::BidiQuerySession::SendQuery(const QuerySpec &spec)
{
    if (!pImpl->stream || pImpl->initial_query_sent)
        return false;

    QueryDataRequest request;
    *request.mutable_queryspec() = spec;

    bool success = pImpl->stream->Write(request);
    if (success)
    {
        pImpl->initial_query_sent = true;
    }
    return success;
}

bool QueryClient::BidiQuerySession::RequestNext()
{
    if (!pImpl->stream || !pImpl->initial_query_sent || pImpl->sending_closed)
        return false;

    QueryDataRequest request;
    auto *cursor_op = request.mutable_cursorop();
    cursor_op->set_cursoroperationtype(CursorOperationType::QueryDataRequest_CursorOperation_CursorOperationType_CURSOR_OP_NEXT);

    return pImpl->stream->Write(request);
}

std::optional<QueryDataResponse> QueryClient::BidiQuerySession::ReadResponse()
{
    if (!pImpl->stream)
        return std::nullopt;

    QueryDataResponse response;
    if (pImpl->stream->Read(&response))
    {
        return response;
    }
    return std::nullopt;
}

std::optional<QueryDataResponse> QueryClient::BidiQuerySession::GetNext()
{
    if (RequestNext())
    {
        return ReadResponse();
    }
    return std::nullopt;
}

void QueryClient::BidiQuerySession::CloseSending()
{
    if (pImpl->stream && !pImpl->sending_closed)
    {
        pImpl->stream->WritesDone();
        pImpl->sending_closed = true;
    }
}

void QueryClient::BidiQuerySession::Cancel()
{
    if (pImpl->context)
    {
        pImpl->context->TryCancel();
    }
}

// ========== QueryClient Implementation ==========

class QueryClient::Impl
{
public:
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<DpQueryService::Stub> stub;
    CommonClient common_client;

    int default_timeout_seconds = 30;
    std::string last_error;
    ClientStats stats;

    Impl(std::shared_ptr<grpc::Channel> ch)
        : channel(ch), stub(DpQueryService::NewStub(ch)) {}
};

QueryClient::QueryClient(std::shared_ptr<grpc::Channel> channel)
    : pImpl(std::make_unique<Impl>(channel)) {}

QueryClient::QueryClient(const std::string &server_address)
    : pImpl(std::make_unique<Impl>(
          grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()))) {}

QueryClient::~QueryClient() = default;

// ========== Time Series Data Query (Unary) ==========

std::vector<DataBucket> QueryClient::QueryData(
    const Timestamp &begin_time,
    const Timestamp &end_time,
    const std::vector<std::string> &pv_names,
    bool use_serialized)
{

    auto response = QueryDataWithResponse(begin_time, end_time, pv_names, use_serialized);

    if (response.has_querydata())
    {
        std::vector<DataBucket> buckets;
        for (const auto &bucket : response.querydata().databuckets())
        {
            buckets.push_back(bucket);
            pImpl->stats.total_buckets_received++;
        }
        return buckets;
    }

    if (response.has_exceptionalresult())
    {
        pImpl->last_error = response.exceptionalresult().message();
        pImpl->stats.errors++;
    }

    return {};
}

QueryDataResponse QueryClient::QueryDataWithResponse(
    const Timestamp &begin_time,
    const Timestamp &end_time,
    const std::vector<std::string> &pv_names,
    bool use_serialized)
{

    auto spec = CreateQuerySpec(begin_time, end_time, pv_names, use_serialized);
    return QueryDataWithSpec(spec);
}

QueryDataResponse QueryClient::QueryDataWithSpec(const QuerySpec &spec)
{
    QueryDataRequest request;
    *request.mutable_queryspec() = spec;

    QueryDataResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->queryData(&context, request, &response);

    pImpl->stats.queries_executed++;

    if (!status.ok())
    {
        pImpl->last_error = status.error_message();
        pImpl->stats.errors++;

        if (!response.has_exceptionalresult())
        {
            auto *exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }

    return response;
}

QuerySpec QueryClient::CreateQuerySpec(
    const Timestamp &begin_time,
    const Timestamp &end_time,
    const std::vector<std::string> &pv_names,
    bool use_serialized)
{

    QuerySpec spec;
    *spec.mutable_begintime() = begin_time;
    *spec.mutable_endtime() = end_time;

    for (const auto &pv : pv_names)
    {
        spec.add_pvnames(pv);
    }

    spec.set_useserializeddatacolumns(use_serialized);

    return spec;
}

// ========== Time Series Data Query (Server Streaming) ==========

std::unique_ptr<QueryClient::StreamQuerySession> QueryClient::QueryDataStream(
    const Timestamp &begin_time,
    const Timestamp &end_time,
    const std::vector<std::string> &pv_names,
    bool use_serialized)
{

    auto spec = CreateQuerySpec(begin_time, end_time, pv_names, use_serialized);

    QueryDataRequest request;
    *request.mutable_queryspec() = spec;

    auto context = std::make_shared<grpc::ClientContext>();
    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds * 10); // Longer timeout for streams
    context->set_deadline(deadline);

    auto reader = std::shared_ptr<grpc::ClientReader<QueryDataResponse>>(
        pImpl->stub->queryDataStream(context.get(), request));

    pImpl->stats.stream_queries++;

    return std::make_unique<StreamQuerySession>(reader, context);
}

// ========== Time Series Data Query (Bidirectional Streaming) ==========

std::unique_ptr<QueryClient::BidiQuerySession> QueryClient::QueryDataBidiStream()
{
    auto context = std::make_shared<grpc::ClientContext>();

    auto stream = std::shared_ptr<grpc::ClientReaderWriter<QueryDataRequest, QueryDataResponse>>(
        pImpl->stub->queryDataBidiStream(context.get()));

    pImpl->stats.stream_queries++;

    return std::make_unique<BidiQuerySession>(stream, context);
}

// ========== Tabular Query ==========

std::optional<ColumnTable> QueryClient::QueryTableColumns(
    const Timestamp &begin_time,
    const Timestamp &end_time,
    const std::vector<std::string> &pv_names)
{

    QueryTableRequest request;
    request.set_format(TableResultFormat::QueryTableRequest_TableResultFormat_TABLE_FORMAT_COLUMN);
    *request.mutable_begintime() = begin_time;
    *request.mutable_endtime() = end_time;

    auto *pv_list = request.mutable_pvnamelist();
    for (const auto &pv : pv_names)
    {
        pv_list->add_pvnames(pv);
    }

    auto response = QueryTable(request);

    if (response.has_tableresult() && response.tableresult().has_columntable())
    {
        return response.tableresult().columntable();
    }

    return std::nullopt;
}

std::optional<RowMapTable> QueryClient::QueryTableRows(
    const Timestamp &begin_time,
    const Timestamp &end_time,
    const std::vector<std::string> &pv_names)
{

    QueryTableRequest request;
    request.set_format(TableResultFormat::QueryTableRequest_TableResultFormat_TABLE_FORMAT_ROW_MAP);
    *request.mutable_begintime() = begin_time;
    *request.mutable_endtime() = end_time;

    auto *pv_list = request.mutable_pvnamelist();
    for (const auto &pv : pv_names)
    {
        pv_list->add_pvnames(pv);
    }

    auto response = QueryTable(request);

    if (response.has_tableresult() && response.tableresult().has_rowmaptable())
    {
        return response.tableresult().rowmaptable();
    }

    return std::nullopt;
}

QueryTableResponse QueryClient::QueryTableWithPattern(
    const Timestamp &begin_time,
    const Timestamp &end_time,
    const std::string &pv_pattern,
    TableResultFormat format)
{

    QueryTableRequest request;
    request.set_format(format);
    *request.mutable_begintime() = begin_time;
    *request.mutable_endtime() = end_time;
    request.mutable_pvnamepattern()->set_pattern(pv_pattern);

    return QueryTable(request);
}

QueryTableResponse QueryClient::QueryTable(const QueryTableRequest &request)
{
    QueryTableResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->queryTable(&context, request, &response);

    pImpl->stats.table_queries++;

    if (!status.ok())
    {
        pImpl->last_error = status.error_message();
        pImpl->stats.errors++;

        if (!response.has_exceptionalresult())
        {
            auto *exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }

    return response;
}

// ========== PV Metadata Query ==========

std::vector<PvInfo> QueryClient::QueryPvMetadata(const std::vector<std::string> &pv_names)
{
    QueryPvMetadataRequest request;
    auto *pv_list = request.mutable_pvnamelist();
    for (const auto &pv : pv_names)
    {
        pv_list->add_pvnames(pv);
    }

    auto response = QueryPvMetadataWithResponse(request);

    if (response.has_metadataresult())
    {
        std::vector<PvInfo> infos;
        for (const auto &info : response.metadataresult().pvinfos())
        {
            infos.push_back(info);
        }
        return infos;
    }

    return {};
}

std::vector<PvInfo> QueryClient::QueryPvMetadataWithPattern(const std::string &pattern)
{
    QueryPvMetadataRequest request;
    request.mutable_pvnamepattern()->set_pattern(pattern);

    auto response = QueryPvMetadataWithResponse(request);

    if (response.has_metadataresult())
    {
        std::vector<PvInfo> infos;
        for (const auto &info : response.metadataresult().pvinfos())
        {
            infos.push_back(info);
        }
        return infos;
    }

    return {};
}

QueryPvMetadataResponse QueryClient::QueryPvMetadataWithResponse(
    const QueryPvMetadataRequest &request)
{

    QueryPvMetadataResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->queryPvMetadata(&context, request, &response);

    pImpl->stats.metadata_queries++;

    if (!status.ok())
    {
        pImpl->last_error = status.error_message();
        pImpl->stats.errors++;

        if (!response.has_exceptionalresult())
        {
            auto *exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }

    return response;
}

// ========== Provider Query ==========

std::vector<ProviderInfo> QueryClient::QueryAllProviders()
{
    auto response = QueryProviders({}); // Empty criteria returns all
    if (response.has_providersresult())
    {
        std::vector<ProviderInfo> infos;
        for (const auto &info : response.providersresult().providerinfos())
        {
            infos.push_back(info);
        }
        return infos;
    }
    return {};
}

std::vector<ProviderInfo> QueryClient::QueryProviderById(const std::string &provider_id)
{
    std::vector<ProviderCriterion> criteria;
    criteria.push_back(CreateIdCriterion(provider_id));

    auto response = QueryProviders(criteria);

    if (response.has_providersresult())
    {
        std::vector<ProviderInfo> infos;
        for (const auto &info : response.providersresult().providerinfos())
        {
            infos.push_back(info);
        }
        return infos;
    }

    return {};
}

std::vector<ProviderInfo> QueryClient::QueryProvidersByText(const std::string &search_text)
{
    std::vector<ProviderCriterion> criteria;
    criteria.push_back(CreateTextCriterion(search_text));

    auto response = QueryProviders(criteria);

    if (response.has_providersresult())
    {
        std::vector<ProviderInfo> infos;
        for (const auto &info : response.providersresult().providerinfos())
        {
            infos.push_back(info);
        }
        return infos;
    }

    return {};
}

std::vector<ProviderInfo> QueryClient::QueryProvidersByTag(const std::string &tag)
{
    std::vector<ProviderCriterion> criteria;
    criteria.push_back(CreateTagsCriterion(tag));

    auto response = QueryProviders(criteria);

    if (response.has_providersresult())
    {
        std::vector<ProviderInfo> infos;
        for (const auto &info : response.providersresult().providerinfos())
        {
            infos.push_back(info);
        }
        return infos;
    }

    return {};
}

std::vector<ProviderInfo> QueryClient::QueryProvidersByAttribute(
    const std::string &key, const std::string &value)
{

    std::vector<ProviderCriterion> criteria;
    criteria.push_back(CreateAttributesCriterion(key, value));

    auto response = QueryProviders(criteria);

    if (response.has_providersresult())
    {
        std::vector<ProviderInfo> infos;
        for (const auto &info : response.providersresult().providerinfos())
        {
            infos.push_back(info);
        }
        return infos;
    }

    return {};
}

QueryProvidersResponse QueryClient::QueryProviders(
    const std::vector<ProviderCriterion> &criteria)
{

    QueryProvidersRequest request;
    for (const auto &criterion : criteria)
    {
        *request.add_criteria() = criterion;
    }

    QueryProvidersResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->queryProviders(&context, request, &response);

    pImpl->stats.provider_queries++;

    if (!status.ok())
    {
        pImpl->last_error = status.error_message();
        pImpl->stats.errors++;

        if (!response.has_exceptionalresult())
        {
            auto *exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }

    return response;
}

ProviderCriterion QueryClient::CreateIdCriterion(const std::string &id)
{
    ProviderCriterion criterion;
    criterion.mutable_idcriterion()->set_id(id);
    return criterion;
}

ProviderCriterion QueryClient::CreateTextCriterion(const std::string &text)
{
    ProviderCriterion criterion;
    criterion.mutable_textcriterion()->set_text(text);
    return criterion;
}

ProviderCriterion QueryClient::CreateTagsCriterion(const std::string &tag)
{
    ProviderCriterion criterion;
    criterion.mutable_tagscriterion()->set_tagvalue(tag);
    return criterion;
}

ProviderCriterion QueryClient::CreateAttributesCriterion(
    const std::string &key, const std::string &value)
{

    ProviderCriterion criterion;
    auto *attr_criterion = criterion.mutable_attributescriterion();
    attr_criterion->set_key(key);
    attr_criterion->set_value(value);
    return criterion;
}

// ========== Provider Metadata Query ==========

std::optional<ProviderMetadata> QueryClient::QueryProviderMetadata(
    const std::string &provider_id)
{

    auto response = QueryProviderMetadataWithResponse(provider_id);

    if (response.has_metadataresult() &&
        response.metadataresult().providermetadatas_size() > 0)
    {
        return response.metadataresult().providermetadatas(0);
    }

    return std::nullopt;
}

QueryProviderMetadataResponse QueryClient::QueryProviderMetadataWithResponse(
    const std::string &provider_id)
{

    QueryProviderMetadataRequest request;
    request.set_providerid(provider_id);

    QueryProviderMetadataResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->queryProviderMetadata(&context, request, &response);

    pImpl->stats.metadata_queries++;

    if (!status.ok())
    {
        pImpl->last_error = status.error_message();
        pImpl->stats.errors++;

        if (!response.has_exceptionalresult())
        {
            auto *exceptional = response.mutable_exceptionalresult();
            exceptional->set_exceptionalresultstatus(
                ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR);
            exceptional->set_message(status.error_message());
        }
    }

    return response;
}

// ========== Utility Methods ==========

std::vector<DataValue> QueryClient::ExtractDataValues(const std::vector<DataBucket> &buckets)
{
    std::vector<DataValue> all_values;

    for (const auto &bucket : buckets)
    {
        DataColumn column;

        if (bucket.has_datacolumn())
        {
            column = bucket.datacolumn();
        }
        else if (bucket.has_serializeddatacolumn())
        {
            column = pImpl->common_client.DeserializeDataColumn(bucket.serializeddatacolumn());
        }

        auto values = pImpl->common_client.ExtractDataValues(column);
        all_values.insert(all_values.end(), values.begin(), values.end());
    }

    return all_values;
}

std::vector<Timestamp> QueryClient::ExtractTimestamps(const std::vector<DataBucket> &buckets)
{
    std::vector<Timestamp> all_timestamps;

    for (const auto &bucket : buckets)
    {
        auto timestamps = pImpl->common_client.ExtractAllTimestamps(bucket.datatimestamps());
        all_timestamps.insert(all_timestamps.end(), timestamps.begin(), timestamps.end());
    }

    return all_timestamps;
}

std::map<std::string, std::vector<DataValue>> QueryClient::ColumnTableToMap(
    const ColumnTable &table)
{

    std::map<std::string, std::vector<DataValue>> result;

    for (const auto &column : table.datacolumns())
    {
        result[column.name()] = pImpl->common_client.ExtractDataValues(column);
    }

    return result;
}

std::vector<std::map<std::string, DataValue>> QueryClient::RowTableToVector(
    const RowMapTable &table)
{

    std::vector<std::map<std::string, DataValue>> result;

    for (const auto &row : table.rows())
    {
        std::map<std::string, DataValue> row_map;
        for (const auto &[key, value] : row.columnvalues())
        {
            row_map[key] = value;
        }
        result.push_back(row_map);
    }

    return result;
}

DataColumn QueryClient::DeserializeDataBucket(const DataBucket &bucket)
{
    if (bucket.has_datacolumn())
    {
        return bucket.datacolumn();
    }
    else if (bucket.has_serializeddatacolumn())
    {
        return pImpl->common_client.DeserializeDataColumn(bucket.serializeddatacolumn());
    }
    return DataColumn();
}

// ========== Connection and Error Management ==========

bool QueryClient::IsConnected() const
{
    return pImpl->channel->GetState(false) == GRPC_CHANNEL_READY;
}

grpc_connectivity_state QueryClient::GetChannelState() const
{
    return pImpl->channel->GetState(false);
}

bool QueryClient::WaitForConnection(int timeout_seconds)
{
    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(timeout_seconds);
    return pImpl->channel->WaitForConnected(deadline);
}

void QueryClient::SetDefaultTimeout(int seconds)
{
    pImpl->default_timeout_seconds = seconds;
}

int QueryClient::GetDefaultTimeout() const
{
    return pImpl->default_timeout_seconds;
}

std::string QueryClient::GetLastError() const
{
    return pImpl->last_error;
}

void QueryClient::ClearLastError()
{
    pImpl->last_error.clear();
}

QueryClient::ClientStats QueryClient::GetStats() const
{
    return pImpl->stats;
}

void QueryClient::ResetStats()
{
    pImpl->stats = ClientStats{};
}

CommonClient &QueryClient::GetCommonClient()
{
    return pImpl->common_client;
}
