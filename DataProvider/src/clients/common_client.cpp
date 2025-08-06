#include "common_client.hpp"
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/text_format.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

// Private implementation class
class CommonClient::Impl {
public:
    // Any private state can go here
};

CommonClient::CommonClient() : pImpl(std::make_unique<Impl>()) {}
CommonClient::~CommonClient() = default;

// ========== Attribute Operations ==========

Attribute CommonClient::CreateAttribute(const std::string& name, const std::string& value) {
    Attribute attr;
    attr.set_name(name);
    attr.set_value(value);
    return attr;
}

std::map<std::string, std::string> CommonClient::AttributesToMap(const std::vector<Attribute>& attributes) {
    std::map<std::string, std::string> result;
    for (const auto& attr : attributes) {
        result[attr.name()] = attr.value();
    }
    return result;
}

std::vector<Attribute> CommonClient::MapToAttributes(const std::map<std::string, std::string>& map) {
    std::vector<Attribute> result;
    result.reserve(map.size());
    for (const auto& [name, value] : map) {
        result.push_back(CreateAttribute(name, value));
    }
    return result;
}

std::optional<std::string> CommonClient::FindAttributeValue(const std::vector<Attribute>& attributes, 
                                                           const std::string& name) {
    auto it = std::find_if(attributes.begin(), attributes.end(),
                          [&name](const Attribute& attr) { return attr.name() == name; });
    if (it != attributes.end()) {
        return it->value();
    }
    return std::nullopt;
}

void CommonClient::UpdateOrAddAttribute(std::vector<Attribute>& attributes, 
                                       const std::string& name, const std::string& value) {
    auto it = std::find_if(attributes.begin(), attributes.end(),
                          [&name](const Attribute& attr) { return attr.name() == name; });
    if (it != attributes.end()) {
        it->set_value(value);
    } else {
        attributes.push_back(CreateAttribute(name, value));
    }
}

// ========== Timestamp Operations ==========

Timestamp CommonClient::CreateTimestamp(uint64_t epochSeconds, uint64_t nanoseconds) {
    Timestamp ts;
    ts.set_epochseconds(epochSeconds);
    ts.set_nanoseconds(nanoseconds);
    return ts;
}

Timestamp CommonClient::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
    
    return CreateTimestamp(seconds.count(), nanos.count());
}

Timestamp CommonClient::FromChronoTimePoint(const std::chrono::system_clock::time_point& tp) {
    auto duration = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
    
    return CreateTimestamp(seconds.count(), nanos.count());
}

std::chrono::system_clock::time_point CommonClient::ToChronoTimePoint(const Timestamp& ts) {
    auto duration = std::chrono::seconds(ts.epochseconds()) + 
                   std::chrono::nanoseconds(ts.nanoseconds());
    return std::chrono::system_clock::time_point(duration);
}

double CommonClient::TimestampToSeconds(const Timestamp& ts) {
    return ts.epochseconds() + (ts.nanoseconds() / 1e9);
}

Timestamp CommonClient::SecondsToTimestamp(double seconds) {
    uint64_t sec = static_cast<uint64_t>(seconds);
    uint64_t nanos = static_cast<uint64_t>((seconds - sec) * 1e9);
    return CreateTimestamp(sec, nanos);
}

int64_t CommonClient::TimestampDifferenceMs(const Timestamp& start, const Timestamp& end) {
    int64_t diff_seconds = end.epochseconds() - start.epochseconds();
    int64_t diff_nanos = end.nanoseconds() - start.nanoseconds();
    return (diff_seconds * 1000) + (diff_nanos / 1000000);
}

std::string CommonClient::TimestampToISOString(const Timestamp& ts) {
    std::time_t time = static_cast<std::time_t>(ts.epochseconds());
    std::tm* tm_info = std::gmtime(&time);
    
    std::stringstream ss;
    ss << std::put_time(tm_info, "%Y-%m-%dT%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(9) << ts.nanoseconds() << "Z";
    return ss.str();
}

bool CommonClient::IsTimestampValid(const Timestamp& ts) {
    // Check if timestamp is reasonable (between year 1970 and 2100)
    return ts.epochseconds() > 0 && ts.epochseconds() < 4102444800 && 
           ts.nanoseconds() < 1000000000;
}

// ========== EventMetadata Operations ==========

EventMetadata CommonClient::CreateEventMetadata(const std::string& description,
                                               const Timestamp& start,
                                               const Timestamp& stop) {
    EventMetadata event;
    event.set_description(description);
    *event.mutable_starttimestamp() = start;
    *event.mutable_stoptimestamp() = stop;
    return event;
}

EventMetadata CommonClient::CreateEventMetadata(const std::string& description) {
    auto now = GetCurrentTimestamp();
    return CreateEventMetadata(description, now, now);
}

uint64_t CommonClient::GetEventDurationMs(const EventMetadata& event) {
    if (event.has_starttimestamp() && event.has_stoptimestamp()) {
        return TimestampDifferenceMs(event.starttimestamp(), event.stoptimestamp());
    }
    return 0;
}

bool CommonClient::IsEventActive(const EventMetadata& event, const Timestamp& currentTime) {
    if (!event.has_starttimestamp() || !event.has_stoptimestamp()) {
        return false;
    }
    
    double current = TimestampToSeconds(currentTime);
    double start = TimestampToSeconds(event.starttimestamp());
    double stop = TimestampToSeconds(event.stoptimestamp());
    
    return current >= start && current <= stop;
}

// ========== TimestampList Operations ==========

TimestampList CommonClient::CreateTimestampList(const std::vector<Timestamp>& timestamps) {
    TimestampList list;
    for (const auto& ts : timestamps) {
        *list.add_timestamps() = ts;
    }
    return list;
}

void CommonClient::AddTimestamp(TimestampList& list, const Timestamp& ts) {
    *list.add_timestamps() = ts;
}

std::vector<Timestamp> CommonClient::ExtractTimestamps(const TimestampList& list) {
    std::vector<Timestamp> result;
    result.reserve(list.timestamps_size());
    for (int i = 0; i < list.timestamps_size(); i++) {
        result.push_back(list.timestamps(i));
    }
    return result;
}

TimestampList CommonClient::CreateRegularTimestamps(const Timestamp& start, uint64_t intervalMs, size_t count) {
    TimestampList list;
    Timestamp current = start;
    
    for (size_t i = 0; i < count; i++) {
        *list.add_timestamps() = current;
        
        // Add interval to current timestamp
        uint64_t totalNanos = current.nanoseconds() + (intervalMs * 1000000);
        uint64_t extraSeconds = totalNanos / 1000000000;
        
        current.set_epochseconds(current.epochseconds() + extraSeconds);
        current.set_nanoseconds(totalNanos % 1000000000);
    }
    
    return list;
}

// ========== SamplingClock Operations ==========

SamplingClock CommonClient::CreateSamplingClock(const Timestamp& startTime, 
                                               uint64_t periodNanos, 
                                               uint32_t count) {
    SamplingClock clock;
    *clock.mutable_starttime() = startTime;
    clock.set_periodnanos(periodNanos);
    clock.set_count(count);
    return clock;
}

std::vector<Timestamp> CommonClient::GenerateTimestampsFromClock(const SamplingClock& clock) {
    std::vector<Timestamp> timestamps;
    timestamps.reserve(clock.count());
    
    Timestamp current = clock.starttime();
    for (uint32_t i = 0; i < clock.count(); i++) {
        timestamps.push_back(current);
        
        // Add period to current timestamp
        uint64_t totalNanos = current.nanoseconds() + clock.periodnanos();
        uint64_t extraSeconds = totalNanos / 1000000000;
        
        current.set_epochseconds(current.epochseconds() + extraSeconds);
        current.set_nanoseconds(totalNanos % 1000000000);
    }
    
    return timestamps;
}

double CommonClient::GetSamplingFrequencyHz(const SamplingClock& clock) {
    if (clock.periodnanos() == 0) return 0.0;
    return 1e9 / clock.periodnanos();
}

uint64_t CommonClient::GetTotalDurationNanos(const SamplingClock& clock) {
    return clock.periodnanos() * (clock.count() - 1);
}

// ========== DataTimestamps Operations ==========

DataTimestamps CommonClient::CreateDataTimestampsFromClock(const SamplingClock& clock) {
    DataTimestamps dt;
    *dt.mutable_samplingclock() = clock;
    return dt;
}

DataTimestamps CommonClient::CreateDataTimestampsFromList(const TimestampList& list) {
    DataTimestamps dt;
    *dt.mutable_timestamplist() = list;
    return dt;
}

bool CommonClient::HasSamplingClock(const DataTimestamps& dt) {
    return dt.has_samplingclock();
}

bool CommonClient::HasTimestampList(const DataTimestamps& dt) {
    return dt.has_timestamplist();
}

std::vector<Timestamp> CommonClient::ExtractAllTimestamps(const DataTimestamps& dt) {
    if (dt.has_samplingclock()) {
        return GenerateTimestampsFromClock(dt.samplingclock());
    } else if (dt.has_timestamplist()) {
        return ExtractTimestamps(dt.timestamplist());
    }
    return {};
}

size_t CommonClient::GetTimestampCount(const DataTimestamps& dt) {
    if (dt.has_samplingclock()) {
        return dt.samplingclock().count();
    } else if (dt.has_timestamplist()) {
        return dt.timestamplist().timestamps_size();
    }
    return 0;
}

// ========== ExceptionalResult Operations ==========

ExceptionalResult CommonClient::CreateExceptionalResult(ExceptionalResultStatus status, 
                                                       const std::string& message) {
    ExceptionalResult result;
    result.set_exceptionalresultstatus(status);
    result.set_message(message);
    return result;
}

ExceptionalResult CommonClient::CreateRejectResult(const std::string& message) {
    return CreateExceptionalResult(ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_REJECT, message);
}

ExceptionalResult CommonClient::CreateErrorResult(const std::string& message) {
    return CreateExceptionalResult(ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR, message);
}

ExceptionalResult CommonClient::CreateEmptyResult(const std::string& message) {
    return CreateExceptionalResult(ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_EMPTY, message);
}

ExceptionalResult CommonClient::CreateNotReadyResult(const std::string& message) {
    return CreateExceptionalResult(ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_NOT_READY, message);
}

bool CommonClient::IsError(const ExceptionalResult& result) {
    return result.exceptionalresultstatus() == ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR;
}

bool CommonClient::IsEmpty(const ExceptionalResult& result) {
    return result.exceptionalresultstatus() == ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_EMPTY;
}

std::string CommonClient::GetExceptionalResultDescription(const ExceptionalResult& result) {
    std::string status;
    switch (result.exceptionalresultstatus()) {
        case ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_REJECT:
            status = "REJECTED"; break;
        case ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_ERROR:
            status = "ERROR"; break;
        case ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_EMPTY:
            status = "EMPTY"; break;
        case ExceptionalResult_ExceptionalResultStatus_RESULT_STATUS_NOT_READY:
            status = "NOT_READY"; break;
        default:
            status = "UNKNOWN"; break;
    }
    return status + ": " + result.message();
}

// ========== DataValue Operations ==========

DataValue CommonClient::CreateStringValue(const std::string& value) {
    DataValue dv;
    dv.set_stringvalue(value);
    return dv;
}

DataValue CommonClient::CreateBoolValue(bool value) {
    DataValue dv;
    dv.set_booleanvalue(value);
    return dv;
}

DataValue CommonClient::CreateIntValue(int32_t value) {
    DataValue dv;
    dv.set_intvalue(value);
    return dv;
}

DataValue CommonClient::CreateLongValue(int64_t value) {
    DataValue dv;
    dv.set_longvalue(value);
    return dv;
}

DataValue CommonClient::CreateFloatValue(float value) {
    DataValue dv;
    dv.set_floatvalue(value);
    return dv;
}

DataValue CommonClient::CreateDoubleValue(double value) {
    DataValue dv;
    dv.set_doublevalue(value);
    return dv;
}

DataValue CommonClient::CreateUIntValue(uint32_t value) {
    DataValue dv;
    dv.set_uintvalue(value);
    return dv;
}

DataValue CommonClient::CreateULongValue(uint64_t value) {
    DataValue dv;
    dv.set_ulongvalue(value);
    return dv;
}

DataValue CommonClient::CreateByteArrayValue(const std::vector<uint8_t>& bytes) {
    DataValue dv;
    dv.set_bytearrayvalue(bytes.data(), bytes.size());
    return dv;
}

DataValue CommonClient::CreateTimestampValue(const Timestamp& ts) {
    DataValue dv;
    *dv.mutable_timestampvalue() = ts;
    return dv;
}

DataValue CommonClient::CreateArrayValue(const Array& array) {
    DataValue dv;
    *dv.mutable_arrayvalue() = array;
    return dv;
}

DataValue CommonClient::CreateStructureValue(const Structure& structure) {
    DataValue dv;
    *dv.mutable_structurevalue() = structure;
    return dv;
}

DataValue CommonClient::CreateImageValue(const Image& image) {
    DataValue dv;
    *dv.mutable_imagevalue() = image;
    return dv;
}

std::optional<std::string> CommonClient::GetStringValue(const DataValue& dv) {
    if (dv.value_case() == DataValue::kStringValue) {
        return dv.stringvalue();
    }
    return std::nullopt;
}

std::optional<bool> CommonClient::GetBoolValue(const DataValue& dv) {
    if (dv.value_case() == DataValue::kBooleanValue) {
        return dv.booleanvalue();
    }
    return std::nullopt;
}

std::optional<double> CommonClient::GetNumericValue(const DataValue& dv) {
    switch (dv.value_case()) {
        case DataValue::kDoubleValue:
            return dv.doublevalue();
        case DataValue::kFloatValue:
            return static_cast<double>(dv.floatvalue());
        case DataValue::kIntValue:
            return static_cast<double>(dv.intvalue());
        case DataValue::kLongValue:
            return static_cast<double>(dv.longvalue());
        case DataValue::kUintValue:
            return static_cast<double>(dv.uintvalue());
        case DataValue::kUlongValue:
            return static_cast<double>(dv.ulongvalue());
        default:
            return std::nullopt;
    }
}

std::optional<Timestamp> CommonClient::GetTimestampValue(const DataValue& dv) {
    if (dv.value_case() == DataValue::kTimestampValue) {
        return dv.timestampvalue();
    }
    return std::nullopt;
}

bool CommonClient::IsStringValue(const DataValue& dv) {
    return dv.value_case() == DataValue::kStringValue;
}

bool CommonClient::IsNumericValue(const DataValue& dv) {
    switch (dv.value_case()) {
        case DataValue::kDoubleValue:
        case DataValue::kFloatValue:
        case DataValue::kIntValue:
        case DataValue::kLongValue:
        case DataValue::kUintValue:
        case DataValue::kUlongValue:
            return true;
        default:
            return false;
    }
}

bool CommonClient::IsBoolValue(const DataValue& dv) {
    return dv.value_case() == DataValue::kBooleanValue;
}

bool CommonClient::IsArrayValue(const DataValue& dv) {
    return dv.value_case() == DataValue::kArrayValue;
}

bool CommonClient::IsStructureValue(const DataValue& dv) {
    return dv.value_case() == DataValue::kStructureValue;
}

bool CommonClient::IsImageValue(const DataValue& dv) {
    return dv.value_case() == DataValue::kImageValue;
}

bool CommonClient::IsTimestampValue(const DataValue& dv) {
    return dv.value_case() == DataValue::kTimestampValue;
}

void CommonClient::SetValueStatus(DataValue& dv, const ValueStatus& status) {
    *dv.mutable_valuestatus() = status;
}

bool CommonClient::HasValueStatus(const DataValue& dv) {
    return dv.has_valuestatus();
}

std::optional<ValueStatus> CommonClient::GetValueStatus(const DataValue& dv) {
    if (dv.has_valuestatus()) {
        return dv.valuestatus();
    }
    return std::nullopt;
}

// ========== DataColumn Operations ==========

DataColumn CommonClient::CreateDataColumn(const std::string& name, 
                                         const std::vector<DataValue>& values) {
    DataColumn column;
    column.set_name(name);
    for (const auto& value : values) {
        *column.add_datavalues() = value;
    }
    return column;
}

DataColumn CommonClient::CreateEmptyDataColumn(const std::string& name) {
    DataColumn column;
    column.set_name(name);
    return column;
}

void CommonClient::AddDataValue(DataColumn& column, const DataValue& value) {
    *column.add_datavalues() = value;
}

std::vector<DataValue> CommonClient::ExtractDataValues(const DataColumn& column) {
    std::vector<DataValue> values;
    values.reserve(column.datavalues_size());
    for (int i = 0; i < column.datavalues_size(); i++) {
        values.push_back(column.datavalues(i));
    }
    return values;
}

DataColumn CommonClient::MergeDataColumns(const std::vector<DataColumn>& columns) {
    if (columns.empty()) {
        return DataColumn();
    }
    
    DataColumn merged;
    merged.set_name(columns[0].name());
    
    for (const auto& column : columns) {
        for (int i = 0; i < column.datavalues_size(); i++) {
            *merged.add_datavalues() = column.datavalues(i);
        }
    }
    return merged;
}

DataColumn CommonClient::SliceDataColumn(const DataColumn& column, size_t start, size_t end) {
    DataColumn sliced;
    sliced.set_name(column.name());
    
    size_t actualEnd = std::min(end, static_cast<size_t>(column.datavalues_size()));
    for (size_t i = start; i < actualEnd; i++) {
        *sliced.add_datavalues() = column.datavalues(i);
    }
    return sliced;
}

std::optional<DataColumn> CommonClient::FilterDataColumn(const DataColumn& column,
                                                        std::function<bool(const DataValue&)> predicate) {
    DataColumn filtered;
    filtered.set_name(column.name());
    
    for (int i = 0; i < column.datavalues_size(); i++) {
        if (predicate(column.datavalues(i))) {
            *filtered.add_datavalues() = column.datavalues(i);
        }
    }
    
    if (filtered.datavalues_size() > 0) {
        return filtered;
    }
    return std::nullopt;
}

size_t CommonClient::GetColumnSize(const DataColumn& column) {
    return column.datavalues_size();
}

// ========== Structure Operations ==========

Structure CommonClient::CreateStructure() {
    return Structure();
}

void CommonClient::AddField(Structure& structure, const std::string& name, const DataValue& value) {
    auto* field = structure.add_fields();
    field->set_name(name);
    *field->mutable_value() = value;
}

std::optional<DataValue> CommonClient::GetField(const Structure& structure, const std::string& name) {
    for (int i = 0; i < structure.fields_size(); i++) {
        if (structure.fields(i).name() == name) {
            return structure.fields(i).value();
        }
    }
    return std::nullopt;
}

bool CommonClient::HasField(const Structure& structure, const std::string& name) {
    for (int i = 0; i < structure.fields_size(); i++) {
        if (structure.fields(i).name() == name) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> CommonClient::GetFieldNames(const Structure& structure) {
    std::vector<std::string> names;
    names.reserve(structure.fields_size());
    for (int i = 0; i < structure.fields_size(); i++) {
        names.push_back(structure.fields(i).name());
    }
    return names;
}

std::map<std::string, DataValue> CommonClient::StructureToMap(const Structure& structure) {
    std::map<std::string, DataValue> map;
    for (int i = 0; i < structure.fields_size(); i++) {
        map[structure.fields(i).name()] = structure.fields(i).value();
    }
    return map;
}

Structure CommonClient::MapToStructure(const std::map<std::string, DataValue>& map) {
    Structure structure;
    for (const auto& [name, value] : map) {
        AddField(structure, name, value);
    }
    return structure;
}

// ========== Array Operations ==========

Array CommonClient::CreateArray(const std::vector<DataValue>& values) {
    Array array;
    for (const auto& value : values) {
        *array.add_datavalues() = value;
    }
    return array;
}

Array CommonClient::CreateEmptyArray() {
    return Array();
}

void CommonClient::AddValue(Array& array, const DataValue& value) {
    *array.add_datavalues() = value;
}

std::vector<DataValue> CommonClient::ExtractValues(const Array& array) {
    std::vector<DataValue> values;
    values.reserve(array.datavalues_size());
    for (int i = 0; i < array.datavalues_size(); i++) {
        values.push_back(array.datavalues(i));
    }
    return values;
}

size_t CommonClient::GetArraySize(const Array& array) {
    return array.datavalues_size();
}

std::optional<DataValue> CommonClient::GetArrayElement(const Array& array, size_t index) {
    if (index < static_cast<size_t>(array.datavalues_size())) {
        return array.datavalues(index);
    }
    return std::nullopt;
}

Array CommonClient::SliceArray(const Array& array, size_t start, size_t end) {
    Array sliced;
    size_t actualEnd = std::min(end, static_cast<size_t>(array.datavalues_size()));
    for (size_t i = start; i < actualEnd; i++) {
        *sliced.add_datavalues() = array.datavalues(i);
    }
    return sliced;
}

// ========== Image Operations ==========

Image CommonClient::CreateImage(const std::vector<uint8_t>& imageData, ImageFileType fileType) {
    Image image;
    image.set_image(imageData.data(), imageData.size());
    image.set_filetype(fileType);
    return image;
}

Image CommonClient::CreateImage(const std::string& imageData, ImageFileType fileType) {
    Image image;
    image.set_image(imageData);
    image.set_filetype(fileType);
    return image;
}

std::vector<uint8_t> CommonClient::GetImageBytes(const Image& image) {
    const std::string& data = image.image();
    return std::vector<uint8_t>(data.begin(), data.end());
}

std::string CommonClient::GetImageMimeType(const Image& image) {
    switch (image.filetype()) {
        case Image_FileType_JPEG: return "image/jpeg";
        case Image_FileType_PNG: return "image/png";
        case Image_FileType_GIF: return "image/gif";
        case Image_FileType_BMP: return "image/bmp";
        case Image_FileType_TIFF: return "image/tiff";
        case Image_FileType_SVG: return "image/svg+xml";
        case Image_FileType_EPS: return "application/postscript";
        case Image_FileType_PDF: return "application/pdf";
        case Image_FileType_RAW: 
        default: return "application/octet-stream";
    }
}

std::string CommonClient::GetImageFileExtension(const Image& image) {
    switch (image.filetype()) {
        case Image_FileType_JPEG: return ".jpg";
        case Image_FileType_PNG: return ".png";
        case Image_FileType_GIF: return ".gif";
        case Image_FileType_BMP: return ".bmp";
        case Image_FileType_TIFF: return ".tiff";
        case Image_FileType_SVG: return ".svg";
        case Image_FileType_EPS: return ".eps";
        case Image_FileType_PDF: return ".pdf";
        case Image_FileType_RAW: 
        default: return ".raw";
    }
}

size_t CommonClient::GetImageSize(const Image& image) {
    return image.image().size();
}

bool CommonClient::IsValidImageType(const Image& image) {
    return image.filetype() >= Image_FileType_RAW && 
           image.filetype() <= Image_FileType_PDF;
}

// ========== SerializedDataColumn Operations ==========

SerializedDataColumn CommonClient::SerializeDataColumn(const DataColumn& column) {
    SerializedDataColumn serialized;
    serialized.set_columnname(column.name());
    
    std::string serializedData;
    column.SerializeToString(&serializedData);
    serialized.set_serializeddata(serializedData);
    
    return serialized;
}

DataColumn CommonClient::DeserializeDataColumn(const SerializedDataColumn& serialized) {
    DataColumn column;
    column.ParseFromString(serialized.serializeddata());
    // Override name with the one stored in SerializedDataColumn if different
    if (!serialized.columnname().empty()) {
        column.set_name(serialized.columnname());
    }
    return column;
}

std::string CommonClient::GetSerializedColumnName(const SerializedDataColumn& serialized) {
    return serialized.columnname();
}

size_t CommonClient::GetSerializedSize(const SerializedDataColumn& serialized) {
    return serialized.serializeddata().size();
}

// ========== Utility Operations ==========

std::string CommonClient::SerializeToString(const google::protobuf::Message& message) {
    std::string output;
    message.SerializeToString(&output);
    return output;
}

bool CommonClient::ParseFromString(const std::string& data, google::protobuf::Message& message) {
    return message.ParseFromString(data);
}

std::string CommonClient::MessageToJson(const google::protobuf::Message& message) {
    std::string json_string;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    
    auto status = google::protobuf::util::MessageToJsonString(message, &json_string, options);
    if (status.ok()) {
        return json_string;
    }
    return "{}";
}

bool CommonClient::JsonToMessage(const std::string& json, google::protobuf::Message& message) {
    auto status = google::protobuf::util::JsonStringToMessage(json, &message);
    return status.ok();
}

bool CommonClient::ValidateTimestamp(const Timestamp& ts) {
    return IsTimestampValid(ts);
}

bool CommonClient::ValidateDataColumn(const DataColumn& column) {
    return !column.name().empty();
}

bool CommonClient::ValidateDataValue(const DataValue& value) {
    // Check that at least one value type is set
    return value.value_case() != DataValue::VALUE_NOT_SET;
}
