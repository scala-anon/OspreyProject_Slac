#include "ingest_client.hpp"
#include "PacketParser.h"
#include <chrono>
#include <iostream>

int main() {
    std::string server_address = "localhost:50051";
    OspreyClient client(server_address);

    // --- Time setup ---
    uint64_t nowSec = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    uint64_t nowNano = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::system_clock::now().time_since_epoch()).count();

    // --- Provider Registration ---
    auto regReq = makeRegisterProviderRequest("Nick", {}, nowSec, nowNano);
    auto regResp = client.sendRegisterProvider(regReq);

    if (!regResp.has_registrationresult()) {
        std::cerr << "Registration failed." << std::endl;
        return 1;
    }
    std::string providerId = regResp.registrationresult().providerid();

    // --- Parse .dat file ---
    PacketParser parser("data/mic1-8-CH17-20240511-121442.dat");
    PacketParser::ParsedChunk chunk;
    if (!parser.nextChunk(chunk)) {
        std::cerr << "No data chunk found or failed to read chunk.\n";
        return 1;
    }

    // --- Extract ADC values from chunk ---
    std::vector<int32_t> adcValues;
    const auto& body = chunk.rawBody;
    for (size_t i = 0; i + 2 < body.size(); i += 3) {
        int32_t val = (static_cast<int32_t>(body[i]) << 16) |
                      (static_cast<int32_t>(body[i + 1]) << 8) |
                      (static_cast<int32_t>(body[i + 2]));
        if (val & 0x800000) val |= 0xFF000000; // Sign extension
        adcValues.push_back(val);
    }

    // --- Build Clock & Data ---
    auto clock = makeSamplingClock(chunk.seconds, chunk.nanoseconds, 4000, adcValues.size());

    std::vector<DataValue> adcData, timestampData;
    for (size_t i = 0; i < adcValues.size(); ++i) {
        adcData.push_back(makeDataValueWithSInt32(adcValues[i]));
        timestampData.push_back(makeDataValueWithTimestamp(chunk.seconds, chunk.nanoseconds + i * 4000));
    }

    std::vector<DataColumn> columns = {
        makeDataColumn("ADC", adcData),
        makeDataColumn("Timestamps", timestampData)
    };

    auto metadata = makeEventMetadata("example event", chunk.seconds, chunk.nanoseconds,
                                      chunk.seconds + 1, chunk.nanoseconds + 1000);

    auto ingestRequest = makeIngestDataRequest(providerId, "0002", {}, {}, metadata, clock, columns);

    client.ingestData(ingestRequest);

    return 0;
}

