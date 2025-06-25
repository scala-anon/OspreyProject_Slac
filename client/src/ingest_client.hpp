#ifndef INGEST_CLIENT_HPP
#define INGEST_CLIENT_HPP

#include <string>
#include <vector>
#include "common.pb.h"
#include "ingestion.pb.h"
#include "ingestion.grpc.pb.h"

using Timestamp = ::Timestamp;
using Attribute = ::Attribute;
using EventMetadata = ::EventMetadata;
using SamplingClock = ::SamplingClock;
using DataValue = ::DataValue;
using DataColumn = ::DataColumn;
using IngestDataRequest = dp::service::ingestion::IngestDataRequest;
using RegisterProviderRequest = dp::service::ingestion::RegisterProviderRequest;
using RegisterProviderResponse = dp::service::ingestion::RegisterProviderResponse;

class OspreyClient {
public:
    explicit OspreyClient(const std::string& server_address);
    RegisterProviderResponse sendRegisterProvider(const RegisterProviderRequest& request);
    std::string ingestData(const IngestDataRequest& request);

private:
    std::unique_ptr<dp::service::ingestion::DpIngestionService::Stub> stub_;
};

// Helper function declarations here...
Timestamp makeTimeStamp(uint64_t epoch, uint64_t nano);
Attribute makeAttribute(const std::string& name, const std::string& value);
EventMetadata makeEventMetadata(const std::string& desc, uint64_t startEpoch, uint64_t startNano, uint64_t endEpoch, uint64_t endNano);
SamplingClock makeSamplingClock(uint64_t epoch, uint64_t nano, uint64_t periodNanos, uint32_t count);
DataValue makeDataValueWithSInt32(int val);
DataValue makeDataValueWithUInt64(uint64_t val);
DataValue makeDataValueWithTimestamp(uint64_t sec, uint64_t nano);
DataColumn makeDataColumn(const std::string& name, const std::vector<DataValue>& values);
IngestDataRequest makeIngestDataRequest(const std::string& providerId, const std::string& clientRequestId, const std::vector<Attribute>& attributes, const std::vector<std::string>& tags, const EventMetadata& metadata, const SamplingClock& samplingClock, const std::vector<DataColumn>& dataColumns);
RegisterProviderRequest makeRegisterProviderRequest(const std::string& providerName, const std::vector<Attribute>& attributes, uint64_t epoch, uint64_t nano);

#endif // INGEST_CLIENT_HPP

