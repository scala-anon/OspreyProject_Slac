#include "annotation_client.hpp"
#include <chrono>
#include <sstream>

// ========== AnnotationClient Implementation ==========

class AnnotationClient::Impl
{
public:
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<DpAnnotationService::Stub> stub;
    CommonClient common_client;

    int default_timeout_seconds = 30;
    std::string last_error;
    ClientStats stats;

    Impl(std::shared_ptr<grpc::Channel> ch)
        : channel(ch), stub(DpAnnotationService::NewStub(ch)) {}
};

AnnotationClient::AnnotationClient(std::shared_ptr<grpc::Channel> channel)
    : pImpl(std::make_unique<Impl>(channel)) {}

AnnotationClient::AnnotationClient(const std::string &server_address)
    : pImpl(std::make_unique<Impl>(
          grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()))) {}

AnnotationClient::~AnnotationClient() = default;

// ========== DataSet Operations ==========

std::optional<std::string> AnnotationClient::CreateDataSet(
    const std::string &name,
    const std::string &owner_id,
    const std::vector<DataBlock> &data_blocks,
    const std::string &description)
{

    auto dataset = CreateDataSetObject(name, owner_id, data_blocks, description);
    auto response = CreateDataSetWithResponse(dataset);

    if (response.has_createdatasetresult())
    {
        pImpl->stats.datasets_created++;
        return response.createdatasetresult().datasetid();
    }

    if (response.has_exceptionalresult())
    {
        pImpl->last_error = response.exceptionalresult().message();
        pImpl->stats.errors++;
    }

    return std::nullopt;
}

CreateDataSetResponse AnnotationClient::CreateDataSetWithResponse(const DataSet &dataset)
{
    CreateDataSetRequest request;
    *request.mutable_dataset() = dataset;

    CreateDataSetResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->createDataSet(&context, request, &response);

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

DataBlock AnnotationClient::CreateDataBlock(
    const Timestamp &begin_time,
    const Timestamp &end_time,
    const std::vector<std::string> &pv_names)
{

    DataBlock block;
    *block.mutable_begintime() = begin_time;
    *block.mutable_endtime() = end_time;

    for (const auto &pv : pv_names)
    {
        block.add_pvnames(pv);
    }

    return block;
}

DataSet AnnotationClient::CreateDataSetObject(
    const std::string &name,
    const std::string &owner_id,
    const std::vector<DataBlock> &data_blocks,
    const std::string &description)
{

    DataSet dataset;
    dataset.set_name(name);
    dataset.set_ownerid(owner_id);

    if (!description.empty())
    {
        dataset.set_description(description);
    }

    for (const auto &block : data_blocks)
    {
        *dataset.add_datablocks() = block;
    }

    return dataset;
}

// ========== DataSet Query Operations ==========

std::vector<DataSet> AnnotationClient::QueryAllDataSets()
{
    auto response = QueryDataSets({}); // Empty criteria returns all
    if (response.has_datasetsresult())
    {
        std::vector<DataSet> datasets;
        for (const auto &ds : response.datasetsresult().datasets())
        {
            datasets.push_back(ds);
        }
        return datasets;
    }
    return {};
}

std::vector<DataSet> AnnotationClient::QueryDataSetById(const std::string &dataset_id)
{
    std::vector<QueryDataSetsCriterion> criteria;
    criteria.push_back(CreateDataSetIdCriterion(dataset_id));

    auto response = QueryDataSets(criteria);

    if (response.has_datasetsresult())
    {
        std::vector<DataSet> datasets;
        for (const auto &ds : response.datasetsresult().datasets())
        {
            datasets.push_back(ds);
        }
        return datasets;
    }

    return {};
}

std::vector<DataSet> AnnotationClient::QueryDataSetsByOwner(const std::string &owner_id)
{
    std::vector<QueryDataSetsCriterion> criteria;
    criteria.push_back(CreateDataSetOwnerCriterion(owner_id));

    auto response = QueryDataSets(criteria);

    if (response.has_datasetsresult())
    {
        std::vector<DataSet> datasets;
        for (const auto &ds : response.datasetsresult().datasets())
        {
            datasets.push_back(ds);
        }
        return datasets;
    }

    return {};
}

std::vector<DataSet> AnnotationClient::QueryDataSetsByText(const std::string &search_text)
{
    std::vector<QueryDataSetsCriterion> criteria;
    criteria.push_back(CreateDataSetTextCriterion(search_text));

    auto response = QueryDataSets(criteria);

    if (response.has_datasetsresult())
    {
        std::vector<DataSet> datasets;
        for (const auto &ds : response.datasetsresult().datasets())
        {
            datasets.push_back(ds);
        }
        return datasets;
    }

    return {};
}

std::vector<DataSet> AnnotationClient::QueryDataSetsByPvName(const std::string &pv_name)
{
    std::vector<QueryDataSetsCriterion> criteria;
    criteria.push_back(CreateDataSetPvNameCriterion(pv_name));

    auto response = QueryDataSets(criteria);

    if (response.has_datasetsresult())
    {
        std::vector<DataSet> datasets;
        for (const auto &ds : response.datasetsresult().datasets())
        {
            datasets.push_back(ds);
        }
        return datasets;
    }

    return {};
}

QueryDataSetsResponse AnnotationClient::QueryDataSets(
    const std::vector<QueryDataSetsCriterion> &criteria)
{

    QueryDataSetsRequest request;
    for (const auto &criterion : criteria)
    {
        *request.add_criteria() = criterion;
    }

    QueryDataSetsResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->queryDataSets(&context, request, &response);

    pImpl->stats.datasets_queried++;

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

QueryDataSetsCriterion AnnotationClient::CreateDataSetIdCriterion(const std::string &id)
{
    QueryDataSetsCriterion criterion;
    criterion.mutable_idcriterion()->set_id(id);
    return criterion;
}

QueryDataSetsCriterion AnnotationClient::CreateDataSetOwnerCriterion(const std::string &owner_id)
{
    QueryDataSetsCriterion criterion;
    criterion.mutable_ownercriterion()->set_ownerid(owner_id);
    return criterion;
}

QueryDataSetsCriterion AnnotationClient::CreateDataSetTextCriterion(const std::string &text)
{
    QueryDataSetsCriterion criterion;
    criterion.mutable_textcriterion()->set_text(text);
    return criterion;
}

QueryDataSetsCriterion AnnotationClient::CreateDataSetPvNameCriterion(const std::string &pv_name)
{
    QueryDataSetsCriterion criterion;
    criterion.mutable_pvnamecriterion()->set_name(pv_name);
    return criterion;
}

// ========== Annotation Operations ==========

std::optional<std::string> AnnotationClient::CreateAnnotation(
    const std::string &owner_id,
    const std::vector<std::string> &dataset_ids,
    const std::string &name,
    const std::string &comment,
    const std::vector<std::string> &tags,
    const std::vector<Attribute> &attributes)
{

    CreateAnnotationRequest request;
    request.set_ownerid(owner_id);
    request.set_name(name);

    for (const auto &id : dataset_ids)
    {
        request.add_datasetids(id);
    }

    if (!comment.empty())
    {
        request.set_comment(comment);
    }

    for (const auto &tag : tags)
    {
        request.add_tags(tag);
    }

    for (const auto &attr : attributes)
    {
        *request.add_attributes() = attr;
    }

    auto response = CreateAnnotationWithResponse(request);

    if (response.has_createannotationresult())
    {
        pImpl->stats.annotations_created++;
        return response.createannotationresult().annotationid();
    }

    if (response.has_exceptionalresult())
    {
        pImpl->last_error = response.exceptionalresult().message();
        pImpl->stats.errors++;
    }

    return std::nullopt;
}

std::optional<std::string> AnnotationClient::CreateAnnotationWithCalculations(
    const std::string &owner_id,
    const std::vector<std::string> &dataset_ids,
    const std::string &name,
    const Calculations &calculations,
    const std::string &comment,
    const std::vector<std::string> &tags,
    const std::vector<Attribute> &attributes)
{

    CreateAnnotationRequest request;
    request.set_ownerid(owner_id);
    request.set_name(name);
    *request.mutable_calculations() = calculations;

    for (const auto &id : dataset_ids)
    {
        request.add_datasetids(id);
    }

    if (!comment.empty())
    {
        request.set_comment(comment);
    }

    for (const auto &tag : tags)
    {
        request.add_tags(tag);
    }

    for (const auto &attr : attributes)
    {
        *request.add_attributes() = attr;
    }

    auto response = CreateAnnotationWithResponse(request);

    if (response.has_createannotationresult())
    {
        pImpl->stats.annotations_created++;
        return response.createannotationresult().annotationid();
    }

    if (response.has_exceptionalresult())
    {
        pImpl->last_error = response.exceptionalresult().message();
        pImpl->stats.errors++;
    }

    return std::nullopt;
}

CreateAnnotationResponse AnnotationClient::CreateAnnotationWithResponse(
    const CreateAnnotationRequest &request)
{

    CreateAnnotationResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->createAnnotation(&context, request, &response);

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

Calculations AnnotationClient::CreateCalculations(
    const std::string &id,
    const std::vector<CalculationsDataFrame> &frames)
{

    Calculations calcs;
    calcs.set_id(id);

    for (const auto &frame : frames)
    {
        *calcs.add_calculationdataframes() = frame;
    }

    return calcs;
}

CalculationsDataFrame AnnotationClient::CreateCalculationsDataFrame(
    const std::string &name,
    const DataTimestamps &timestamps,
    const std::vector<DataColumn> &columns)
{

    CalculationsDataFrame frame;
    frame.set_name(name);
    *frame.mutable_datatimestamps() = timestamps;

    for (const auto &column : columns)
    {
        *frame.add_datacolumns() = column;
    }

    return frame;
}

// ========== Annotation Query Operations ==========

std::vector<Annotation> AnnotationClient::QueryAllAnnotations()
{
    auto response = QueryAnnotations({}); // Empty criteria returns all
    if (response.has_annotationsresult())
    {
        std::vector<Annotation> annotations;
        for (const auto &ann : response.annotationsresult().annotations())
        {
            annotations.push_back(ann);
        }
        return annotations;
    }
    return {};
}

std::vector<Annotation> AnnotationClient::QueryAnnotationById(const std::string &annotation_id)
{
    std::vector<QueryAnnotationsCriterion> criteria;
    criteria.push_back(CreateAnnotationIdCriterion(annotation_id));

    auto response = QueryAnnotations(criteria);

    if (response.has_annotationsresult())
    {
        std::vector<Annotation> annotations;
        for (const auto &ann : response.annotationsresult().annotations())
        {
            annotations.push_back(ann);
        }
        return annotations;
    }

    return {};
}

std::vector<Annotation> AnnotationClient::QueryAnnotationsByOwner(const std::string &owner_id)
{
    std::vector<QueryAnnotationsCriterion> criteria;
    criteria.push_back(CreateAnnotationOwnerCriterion(owner_id));

    auto response = QueryAnnotations(criteria);

    if (response.has_annotationsresult())
    {
        std::vector<Annotation> annotations;
        for (const auto &ann : response.annotationsresult().annotations())
        {
            annotations.push_back(ann);
        }
        return annotations;
    }

    return {};
}

std::vector<Annotation> AnnotationClient::QueryAnnotationsByDataSet(const std::string &dataset_id)
{
    std::vector<QueryAnnotationsCriterion> criteria;
    criteria.push_back(CreateAnnotationDataSetCriterion(dataset_id));

    auto response = QueryAnnotations(criteria);

    if (response.has_annotationsresult())
    {
        std::vector<Annotation> annotations;
        for (const auto &ann : response.annotationsresult().annotations())
        {
            annotations.push_back(ann);
        }
        return annotations;
    }

    return {};
}

std::vector<Annotation> AnnotationClient::QueryAnnotationsByAnnotation(const std::string &annotation_id)
{
    std::vector<QueryAnnotationsCriterion> criteria;
    criteria.push_back(CreateAnnotationAnnotationCriterion(annotation_id));

    auto response = QueryAnnotations(criteria);

    if (response.has_annotationsresult())
    {
        std::vector<Annotation> annotations;
        for (const auto &ann : response.annotationsresult().annotations())
        {
            annotations.push_back(ann);
        }
        return annotations;
    }

    return {};
}

std::vector<Annotation> AnnotationClient::QueryAnnotationsByText(const std::string &search_text)
{
    std::vector<QueryAnnotationsCriterion> criteria;
    criteria.push_back(CreateAnnotationTextCriterion(search_text));

    auto response = QueryAnnotations(criteria);

    if (response.has_annotationsresult())
    {
        std::vector<Annotation> annotations;
        for (const auto &ann : response.annotationsresult().annotations())
        {
            annotations.push_back(ann);
        }
        return annotations;
    }

    return {};
}

std::vector<Annotation> AnnotationClient::QueryAnnotationsByTag(const std::string &tag)
{
    std::vector<QueryAnnotationsCriterion> criteria;
    criteria.push_back(CreateAnnotationTagCriterion(tag));

    auto response = QueryAnnotations(criteria);

    if (response.has_annotationsresult())
    {
        std::vector<Annotation> annotations;
        for (const auto &ann : response.annotationsresult().annotations())
        {
            annotations.push_back(ann);
        }
        return annotations;
    }

    return {};
}

std::vector<Annotation> AnnotationClient::QueryAnnotationsByAttribute(
    const std::string &key, const std::string &value)
{

    std::vector<QueryAnnotationsCriterion> criteria;
    criteria.push_back(CreateAnnotationAttributeCriterion(key, value));

    auto response = QueryAnnotations(criteria);

    if (response.has_annotationsresult())
    {
        std::vector<Annotation> annotations;
        for (const auto &ann : response.annotationsresult().annotations())
        {
            annotations.push_back(ann);
        }
        return annotations;
    }

    return {};
}

QueryAnnotationsResponse AnnotationClient::QueryAnnotations(
    const std::vector<QueryAnnotationsCriterion> &criteria)
{

    QueryAnnotationsRequest request;
    for (const auto &criterion : criteria)
    {
        *request.add_criteria() = criterion;
    }

    QueryAnnotationsResponse response;
    grpc::ClientContext context;

    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->queryAnnotations(&context, request, &response);

    pImpl->stats.annotations_queried++;

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

QueryAnnotationsCriterion AnnotationClient::CreateAnnotationIdCriterion(const std::string &id)
{
    QueryAnnotationsCriterion criterion;
    criterion.mutable_idcriterion()->set_id(id);
    return criterion;
}

QueryAnnotationsCriterion AnnotationClient::CreateAnnotationOwnerCriterion(const std::string &owner_id)
{
    QueryAnnotationsCriterion criterion;
    criterion.mutable_ownercriterion()->set_ownerid(owner_id);
    return criterion;
}

QueryAnnotationsCriterion AnnotationClient::CreateAnnotationDataSetCriterion(const std::string &dataset_id)
{
    QueryAnnotationsCriterion criterion;
    criterion.mutable_datasetscriterion()->set_datasetid(dataset_id);
    return criterion;
}

QueryAnnotationsCriterion AnnotationClient::CreateAnnotationAnnotationCriterion(const std::string &annotation_id)
{
    QueryAnnotationsCriterion criterion;
    criterion.mutable_annotationscriterion()->set_annotationid(annotation_id);
    return criterion;
}

QueryAnnotationsCriterion AnnotationClient::CreateAnnotationTextCriterion(const std::string &text)
{
    QueryAnnotationsCriterion criterion;
    criterion.mutable_textcriterion()->set_text(text);
    return criterion;
}

QueryAnnotationsCriterion AnnotationClient::CreateAnnotationTagCriterion(const std::string &tag)
{
    QueryAnnotationsCriterion criterion;
    criterion.mutable_tagscriterion()->set_tagvalue(tag);
    return criterion;
}

QueryAnnotationsCriterion AnnotationClient::CreateAnnotationAttributeCriterion(
    const std::string &key, const std::string &value)
{

    QueryAnnotationsCriterion criterion;
    auto *attr_criterion = criterion.mutable_attributescriterion();
    attr_criterion->set_key(key);
    attr_criterion->set_value(value);
    return criterion;
}

// ========== Export Operations ==========

std::optional<std::string> AnnotationClient::ExportDataSetToFile(
    const std::string &dataset_id,
    ExportOutputFormat format)
{

    auto response = ExportData(dataset_id, format);

    if (response.has_exportdataresult())
    {
        pImpl->stats.exports_performed++;
        return response.exportdataresult().filepath();
    }

    if (response.has_exceptionalresult())
    {
        pImpl->last_error = response.exceptionalresult().message();
        pImpl->stats.errors++;
    }

    return std::nullopt;
}

std::optional<std::string> AnnotationClient::ExportDataSetToUrl(
    const std::string &dataset_id,
    ExportOutputFormat format)
{

    auto response = ExportData(dataset_id, format);

    if (response.has_exportdataresult())
    {
        pImpl->stats.exports_performed++;
        return response.exportdataresult().fileurl();
    }

    if (response.has_exceptionalresult())
    {
        pImpl->last_error = response.exceptionalresult().message();
        pImpl->stats.errors++;
    }

    return std::nullopt;
}

ExportDataResponse AnnotationClient::ExportData(
    const std::string &dataset_id,
    ExportOutputFormat format)
{

    ExportDataRequest request;
    request.set_datasetid(dataset_id);
    request.set_outputformat(format);

    ExportDataResponse response;
    grpc::ClientContext context;

    // Export might take longer
    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(pImpl->default_timeout_seconds * 3);
    context.set_deadline(deadline);

    grpc::Status status = pImpl->stub->exportData(&context, request, &response);

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

std::vector<std::string> AnnotationClient::ExtractPvNamesFromDataSet(const DataSet &dataset)
{
    std::vector<std::string> all_pvs;

    for (const auto &block : dataset.datablocks())
    {
        for (int i = 0; i < block.pvnames_size(); i++)
        {
            all_pvs.push_back(block.pvnames(i));
        }
    }

    return all_pvs;
}

std::pair<Timestamp, Timestamp> AnnotationClient::ExtractTimeRangeFromDataSet(const DataSet &dataset)
{
    if (dataset.datablocks_size() == 0)
    {
        return {Timestamp(), Timestamp()};
    }

    Timestamp earliest = dataset.datablocks(0).begintime();
    Timestamp latest = dataset.datablocks(0).endtime();

    for (const auto &block : dataset.datablocks())
    {
        if (pImpl->common_client.TimestampToSeconds(block.begintime()) <
            pImpl->common_client.TimestampToSeconds(earliest))
        {
            earliest = block.begintime();
        }
        if (pImpl->common_client.TimestampToSeconds(block.endtime()) >
            pImpl->common_client.TimestampToSeconds(latest))
        {
            latest = block.endtime();
        }
    }

    return {earliest, latest};
}

bool AnnotationClient::ValidateDataSet(const DataSet &dataset)
{
    // Check required fields
    if (dataset.name().empty() || dataset.ownerid().empty())
    {
        return false;
    }

    // Check that there's at least one data block
    if (dataset.datablocks_size() == 0)
    {
        return false;
    }

    // Validate each data block
    for (const auto &block : dataset.datablocks())
    {
        if (!block.has_begintime() || !block.has_endtime())
        {
            return false;
        }
        if (block.pvnames_size() == 0)
        {
            return false;
        }
    }

    return true;
}

bool AnnotationClient::ValidateAnnotationRequest(const CreateAnnotationRequest &request)
{
    // Check required fields
    if (request.ownerid().empty() || request.name().empty())
    {
        return false;
    }

    // Check that there's at least one dataset
    if (request.datasetids_size() == 0)
    {
        return false;
    }

    return true;
}

bool AnnotationClient::DataSetExists(const std::string &dataset_id)
{
    auto datasets = QueryDataSetById(dataset_id);
    return !datasets.empty();
}

bool AnnotationClient::AnnotationExists(const std::string &annotation_id)
{
    auto annotations = QueryAnnotationById(annotation_id);
    return !annotations.empty();
}

std::optional<DataSet> AnnotationClient::GetDataSetMetadata(const std::string &dataset_id)
{
    auto datasets = QueryDataSetById(dataset_id);
    if (!datasets.empty())
    {
        return datasets[0];
    }
    return std::nullopt;
}

std::optional<Annotation> AnnotationClient::GetAnnotationMetadata(const std::string &annotation_id)
{
    auto annotations = QueryAnnotationById(annotation_id);
    if (!annotations.empty())
    {
        return annotations[0];
    }
    return std::nullopt;
}

// ========== Connection and Error Management ==========

bool AnnotationClient::IsConnected() const
{
    return pImpl->channel->GetState(false) == GRPC_CHANNEL_READY;
}

grpc_connectivity_state AnnotationClient::GetChannelState() const
{
    return pImpl->channel->GetState(false);
}

bool AnnotationClient::WaitForConnection(int timeout_seconds)
{
    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::seconds(timeout_seconds);
    return pImpl->channel->WaitForConnected(deadline);
}

void AnnotationClient::SetDefaultTimeout(int seconds)
{
    pImpl->default_timeout_seconds = seconds;
}

int AnnotationClient::GetDefaultTimeout() const
{
    return pImpl->default_timeout_seconds;
}

std::string AnnotationClient::GetLastError() const
{
    return pImpl->last_error;
}

void AnnotationClient::ClearLastError()
{
    pImpl->last_error.clear();
}

AnnotationClient::ClientStats AnnotationClient::GetStats() const
{
    return pImpl->stats;
}

void AnnotationClient::ResetStats()
{
    pImpl->stats = ClientStats{};
}

CommonClient &AnnotationClient::GetCommonClient()
{
    return pImpl->common_client;
}
