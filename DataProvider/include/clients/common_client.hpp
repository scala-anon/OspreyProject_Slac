#ifndef COMMON_CLIENT_HPP
#define COMMON_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <map>
#include <chrono>
#include <cstdint>
#include "common.pb.h"

// Type aliases for cleaner code
using Attribute = ::Attribute;
using EventMetadata = ::EventMetadata;
using Timestamp = ::Timestamp;
using TimestampList = ::TimestampList;
using SamplingClock = ::SamplingClock;
using DataTimestamps = ::DataTimestamps;
using ExceptionalResult = ::ExceptionalResult;
using DataColumn = ::DataColumn;
using DataValue = ::DataValue;
using Structure = ::Structure;
using Array = ::Array;
using Image = ::Image;
using SerializedDataColumn = ::SerializedDataColumn;

// Nested types and enums
using StructureField = ::Structure_Field;
using ValueStatus = ::DataValue_ValueStatus;
using ExceptionalResultStatus = ::ExceptionalResult_ExceptionalResultStatus;
using ImageFileType = ::Image_FileType;

class CommonClient {
public:
    CommonClient();
    ~CommonClient();

    // ========== Attribute Operations ==========
    Attribute CreateAttribute(const std::string& name, const std::string& value);
    std::map<std::string, std::string> AttributesToMap(const std::vector<Attribute>& attributes);
    std::vector<Attribute> MapToAttributes(const std::map<std::string, std::string>& map);
    std::optional<std::string> FindAttributeValue(const std::vector<Attribute>& attributes, 
                                                  const std::string& name);
    void UpdateOrAddAttribute(std::vector<Attribute>& attributes, 
                             const std::string& name, const std::string& value);
    
    // ========== Timestamp Operations ==========
    Timestamp CreateTimestamp(uint64_t epochSeconds, uint64_t nanoseconds = 0);
    Timestamp GetCurrentTimestamp();
    Timestamp FromChronoTimePoint(const std::chrono::system_clock::time_point& tp);
    std::chrono::system_clock::time_point ToChronoTimePoint(const Timestamp& ts);
    double TimestampToSeconds(const Timestamp& ts);
    Timestamp SecondsToTimestamp(double seconds);
    int64_t TimestampDifferenceMs(const Timestamp& start, const Timestamp& end);
    std::string TimestampToISOString(const Timestamp& ts);
    bool IsTimestampValid(const Timestamp& ts);
    
    // ========== EventMetadata Operations ==========
    EventMetadata CreateEventMetadata(const std::string& description,
                                      const Timestamp& start,
                                      const Timestamp& stop);
    EventMetadata CreateEventMetadata(const std::string& description);  // Current time for start/stop
    uint64_t GetEventDurationMs(const EventMetadata& event);
    bool IsEventActive(const EventMetadata& event, const Timestamp& currentTime);
    
    // ========== TimestampList Operations ==========
    TimestampList CreateTimestampList(const std::vector<Timestamp>& timestamps);
    void AddTimestamp(TimestampList& list, const Timestamp& ts);
    std::vector<Timestamp> ExtractTimestamps(const TimestampList& list);
    TimestampList CreateRegularTimestamps(const Timestamp& start, uint64_t intervalMs, size_t count);
    
    // ========== SamplingClock Operations ==========
    SamplingClock CreateSamplingClock(const Timestamp& startTime, 
                                      uint64_t periodNanos, 
                                      uint32_t count);
    std::vector<Timestamp> GenerateTimestampsFromClock(const SamplingClock& clock);
    double GetSamplingFrequencyHz(const SamplingClock& clock);
    uint64_t GetTotalDurationNanos(const SamplingClock& clock);
    
    // ========== DataTimestamps Operations ==========
    DataTimestamps CreateDataTimestampsFromClock(const SamplingClock& clock);
    DataTimestamps CreateDataTimestampsFromList(const TimestampList& list);
    bool HasSamplingClock(const DataTimestamps& dt);
    bool HasTimestampList(const DataTimestamps& dt);
    std::vector<Timestamp> ExtractAllTimestamps(const DataTimestamps& dt);
    size_t GetTimestampCount(const DataTimestamps& dt);
    
    // ========== ExceptionalResult Operations ==========
    ExceptionalResult CreateExceptionalResult(ExceptionalResultStatus status, 
                                             const std::string& message);
    ExceptionalResult CreateRejectResult(const std::string& message);
    ExceptionalResult CreateErrorResult(const std::string& message);
    ExceptionalResult CreateEmptyResult(const std::string& message = "No data available");
    ExceptionalResult CreateNotReadyResult(const std::string& message = "Service not ready");
    bool IsError(const ExceptionalResult& result);
    bool IsEmpty(const ExceptionalResult& result);
    std::string GetExceptionalResultDescription(const ExceptionalResult& result);
    
    // ========== DataValue Operations ==========
    // Factory methods for different value types
    DataValue CreateStringValue(const std::string& value);
    DataValue CreateBoolValue(bool value);
    DataValue CreateIntValue(int32_t value);
    DataValue CreateLongValue(int64_t value);
    DataValue CreateFloatValue(float value);
    DataValue CreateDoubleValue(double value);
    DataValue CreateUIntValue(uint32_t value);
    DataValue CreateULongValue(uint64_t value);
    DataValue CreateByteArrayValue(const std::vector<uint8_t>& bytes);
    DataValue CreateTimestampValue(const Timestamp& ts);
    DataValue CreateArrayValue(const Array& array);
    DataValue CreateStructureValue(const Structure& structure);
    DataValue CreateImageValue(const Image& image);
    
    // Value extraction
    std::optional<std::string> GetStringValue(const DataValue& dv);
    std::optional<bool> GetBoolValue(const DataValue& dv);
    std::optional<double> GetNumericValue(const DataValue& dv);  // Converts any numeric type to double
    std::optional<Timestamp> GetTimestampValue(const DataValue& dv);
    
    // Value type checking
    bool IsStringValue(const DataValue& dv);
    bool IsNumericValue(const DataValue& dv);
    bool IsBoolValue(const DataValue& dv);
    bool IsArrayValue(const DataValue& dv);
    bool IsStructureValue(const DataValue& dv);
    bool IsImageValue(const DataValue& dv);
    bool IsTimestampValue(const DataValue& dv);
    
    // Status operations
    void SetValueStatus(DataValue& dv, const ValueStatus& status);
    bool HasValueStatus(const DataValue& dv);
    std::optional<ValueStatus> GetValueStatus(const DataValue& dv);
    
    // ========== DataColumn Operations ==========
    DataColumn CreateDataColumn(const std::string& name, 
                               const std::vector<DataValue>& values);
    DataColumn CreateEmptyDataColumn(const std::string& name);
    void AddDataValue(DataColumn& column, const DataValue& value);
    std::vector<DataValue> ExtractDataValues(const DataColumn& column);
    DataColumn MergeDataColumns(const std::vector<DataColumn>& columns);
    DataColumn SliceDataColumn(const DataColumn& column, size_t start, size_t end);
    std::optional<DataColumn> FilterDataColumn(const DataColumn& column,
                                               std::function<bool(const DataValue&)> predicate);
    size_t GetColumnSize(const DataColumn& column);
    
    // ========== Structure Operations ==========
    Structure CreateStructure();
    void AddField(Structure& structure, const std::string& name, const DataValue& value);
    std::optional<DataValue> GetField(const Structure& structure, const std::string& name);
    bool HasField(const Structure& structure, const std::string& name);
    std::vector<std::string> GetFieldNames(const Structure& structure);
    std::map<std::string, DataValue> StructureToMap(const Structure& structure);
    Structure MapToStructure(const std::map<std::string, DataValue>& map);
    
    // ========== Array Operations ==========
    Array CreateArray(const std::vector<DataValue>& values);
    Array CreateEmptyArray();
    void AddValue(Array& array, const DataValue& value);
    std::vector<DataValue> ExtractValues(const Array& array);
    size_t GetArraySize(const Array& array);
    std::optional<DataValue> GetArrayElement(const Array& array, size_t index);
    Array SliceArray(const Array& array, size_t start, size_t end);
    
    // ========== Image Operations ==========
    Image CreateImage(const std::vector<uint8_t>& imageData, ImageFileType fileType);
    Image CreateImage(const std::string& imageData, ImageFileType fileType);
    std::vector<uint8_t> GetImageBytes(const Image& image);
    std::string GetImageMimeType(const Image& image);
    std::string GetImageFileExtension(const Image& image);
    size_t GetImageSize(const Image& image);
    bool IsValidImageType(const Image& image);
    
    // ========== SerializedDataColumn Operations ==========
    SerializedDataColumn SerializeDataColumn(const DataColumn& column);
    DataColumn DeserializeDataColumn(const SerializedDataColumn& serialized);
    std::string GetSerializedColumnName(const SerializedDataColumn& serialized);
    size_t GetSerializedSize(const SerializedDataColumn& serialized);
    
    // ========== Utility Operations ==========
    // General protobuf serialization
    std::string SerializeToString(const google::protobuf::Message& message);
    bool ParseFromString(const std::string& data, google::protobuf::Message& message);
    
    // JSON conversion (using protobuf's JSON utilities)
    std::string MessageToJson(const google::protobuf::Message& message);
    bool JsonToMessage(const std::string& json, google::protobuf::Message& message);
    
    // Validation utilities
    bool ValidateTimestamp(const Timestamp& ts);
    bool ValidateDataColumn(const DataColumn& column);
    bool ValidateDataValue(const DataValue& value);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // COMMON_CLIENT_HPP