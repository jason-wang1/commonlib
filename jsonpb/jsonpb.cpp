#include "jsonpb.h"
#include <exception>
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"

using google::protobuf::Descriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::MessageFactory;
using google::protobuf::Reflection;

class jsonpb_error : public std::exception
{
    std::string _error;

public:
    jsonpb_error(const std::string &e) : _error(e) {}
    jsonpb_error(const FieldDescriptor *field, const std::string &e) : _error(field->name() + ": " + e) {}
    virtual ~jsonpb_error() throw(){};
    virtual const char *what() const throw() { return _error.c_str(); };
};

static void _json2pb(Message &msg, const JsonValue &JsonValue);
static void _json2field(Message &msg, const FieldDescriptor *field, const JsonValue &jsonValue);

static void _pb2json(JsonWriter &jsonWriter, const Message &msg);
static void _field2json(JsonWriter &jsonWriter, const Message &msg, const FieldDescriptor *field, size_t index);

static void _pb2json(JsonWriter &jsonWriter, const Message &msg)
{
    const Descriptor *desc = msg.GetDescriptor();
    const Reflection *ref = msg.GetReflection();
    if (!desc || !ref)
    {
        throw jsonpb_error("No descriptor or reflection");
        return;
    }

    // 通过 ListFields 方法遍历有效字段(值非0, 非空)
    // std::vector<const FieldDescriptor *> fields;
    // ref->ListFields(msg, &fields);
    // for (size_t i = 0; i != fields.size(); i++)

    // 通过 field_count 遍历获取所有字段
    for (int i = 0, field_count = desc->field_count(); i != field_count; i++)
    {
        const FieldDescriptor *field = desc->field(i);

        // 获取字段名
        bool is_extension = field->is_extension();
        const std::string &name = is_extension ? field->full_name()
                                               : field->name();

        bool is_repeated = field->is_repeated();
        if (is_repeated)
        {
            jsonWriter.Key(name.c_str(), name.size());
            jsonWriter.StartArray();
            size_t field_size = ref->FieldSize(msg, field);
            for (size_t j = 0; j < field_size; j++)
            {
                _field2json(jsonWriter, msg, field, j);
            }
            jsonWriter.EndArray();
        }
        else
        {
            if (ref->HasField(msg, field))
            {
                jsonWriter.Key(name.c_str(), name.size());
                _field2json(jsonWriter, msg, field, 0);
            }
            else
            {
                // 如果值为0或者空, 也转为Json
                jsonWriter.Key(name.c_str(), name.size());
                _field2json(jsonWriter, msg, field, 0);
            }
        }
    }
}

static void _field2json(JsonWriter &jsonWriter, const Message &msg, const FieldDescriptor *field, size_t index)
{
    const Reflection *ref = msg.GetReflection();
    const bool repeated = field->is_repeated();

    switch (field->cpp_type())
    {
#define _CONVERT(pbtype, type, set, func, repFunc)                      \
    case FieldDescriptor::pbtype:                                       \
    {                                                                   \
        const type value = (repeated) ? ref->repFunc(msg, field, index) \
                                      : ref->func(msg, field);          \
        jsonWriter.set(value);                                          \
        break;                                                          \
    }

        _CONVERT(CPPTYPE_INT32, int32_t, Int, GetInt32, GetRepeatedInt32);
        _CONVERT(CPPTYPE_INT64, int64_t, Int64, GetInt64, GetRepeatedInt64);
        _CONVERT(CPPTYPE_UINT32, uint32_t, Uint, GetUInt32, GetRepeatedUInt32);
        _CONVERT(CPPTYPE_UINT64, uint64_t, Uint64, GetUInt64, GetRepeatedUInt64);
        _CONVERT(CPPTYPE_DOUBLE, double, Double, GetDouble, GetRepeatedDouble);
        _CONVERT(CPPTYPE_FLOAT, float, Double, GetFloat, GetRepeatedFloat);
        _CONVERT(CPPTYPE_BOOL, bool, Bool, GetBool, GetRepeatedBool);
#undef _CONVERT

    case FieldDescriptor::CPPTYPE_ENUM:
    {
        // 枚举类型
        const EnumValueDescriptor *ef = (repeated) ? ref->GetRepeatedEnum(msg, field, index)
                                                   : ref->GetEnum(msg, field);

        // 用int值
        jsonWriter.Int(ef->number());

        // 用string名称
        // const std::string &name = ef->name();
        // jsonWriter.String(name.c_str(), name.size());
        break;
    }
    case FieldDescriptor::CPPTYPE_STRING:
    {
        std::string scratch;
        const std::string &value = (repeated) ? ref->GetRepeatedStringReference(msg, field, index, &scratch)
                                              : ref->GetStringReference(msg, field, &scratch);

        if (field->type() == FieldDescriptor::TYPE_BYTES)
        {
            // 对二进制字符串特殊处理
            jsonWriter.String(value.c_str(), value.size());
        }
        else
        {
            jsonWriter.String(value.c_str(), value.size());
        }
        break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE:
    {
        // 嵌套结构解析
        const Message &msgField = (repeated) ? ref->GetRepeatedMessage(msg, field, index)
                                             : ref->GetMessage(msg, field);
        jsonWriter.StartObject();
        _pb2json(jsonWriter, msgField);
        jsonWriter.EndObject();
        break;
    }
    default:
        break;
    }
}

static void _json2pb(Message &msg, const JsonValue &jsonValue)
{
    const Descriptor *desc = msg.GetDescriptor();
    const Reflection *ref = msg.GetReflection();
    if (!desc || !ref)
    {
        throw jsonpb_error("No descriptor or reflection");
        return;
    }

    for (auto it = jsonValue.MemberBegin(); it != jsonValue.MemberEnd(); it++)
    {
        // 兼容参数为null的情况 20230331 tkxiong
        const JsonValue &value = it->value;
        if (value.IsNull())
        {
            continue;
        }

        std::string name(it->name.GetString(), it->name.GetStringLength());
        const FieldDescriptor *field = desc->FindFieldByName(name);
        if (field == nullptr)
        {
            field = ref->FindKnownExtensionByName(name);
            // field = desc->file()->FindExtensionByName(name);
        }

        if (field == nullptr)
        {
            // pb 不关注的字段
            continue;
        }

        if (field->is_repeated())
        {
            if (!value.IsArray())
            {
                throw jsonpb_error(field, "Not array");
                continue;
            }

            int array_size = value.Size();
            for (int array_idx = 0; array_idx < array_size; array_idx++)
            {
                const JsonValue &valueField = value[array_idx];
                _json2field(msg, field, valueField);
            }
        }
        else
        {
            _json2field(msg, field, value);
        }
    }
}

static void _json2field(Message &msg, const FieldDescriptor *field, const JsonValue &jsonValue)
{
    const Reflection *ref = msg.GetReflection();
    const bool repeated = field->is_repeated();

    switch (field->cpp_type())
    {
#define _SET_OR_ADD(sfunc, afunc, value)    \
    {                                       \
        if (repeated)                       \
        {                                   \
            ref->afunc(&msg, field, value); \
        }                                   \
        else                                \
        {                                   \
            ref->sfunc(&msg, field, value); \
        }                                   \
    }

#define _CONVERT(pbtype, type, check, get, sfunc, afunc) \
    case FieldDescriptor::pbtype:                        \
    {                                                    \
        if (jsonValue.check())                           \
        {                                                \
            type value = jsonValue.get();                \
            _SET_OR_ADD(sfunc, afunc, value);            \
        }                                                \
        else                                             \
        {                                                \
            throw jsonpb_error(field, "check failed.");  \
        }                                                \
        break;                                           \
    }

        _CONVERT(CPPTYPE_INT32, int32_t, IsInt, GetInt, SetInt32, AddInt32);
        _CONVERT(CPPTYPE_INT64, int32_t, IsInt64, GetInt64, SetInt64, AddInt64);
        _CONVERT(CPPTYPE_UINT32, uint32_t, IsUint, GetUint, SetUInt32, AddUInt32);
        _CONVERT(CPPTYPE_UINT64, uint64_t, IsUint64, GetUint64, SetUInt64, AddUInt64);
        _CONVERT(CPPTYPE_DOUBLE, double, IsDouble, GetDouble, SetDouble, AddDouble);
        _CONVERT(CPPTYPE_FLOAT, float, IsFloat, GetFloat, SetFloat, AddFloat);
        _CONVERT(CPPTYPE_BOOL, bool, IsBool, GetBool, SetBool, AddBool);
#undef _CONVERT

    case FieldDescriptor::CPPTYPE_STRING:
    {
        if (!jsonValue.IsString())
        {
            throw jsonpb_error(field, "Not a string");
        }

        std::string value(jsonValue.GetString(), jsonValue.GetStringLength());
        if (field->type() == FieldDescriptor::TYPE_BYTES)
        {
            // 二进制特殊处理
            _SET_OR_ADD(SetString, AddString, value);
        }
        else
        {
            _SET_OR_ADD(SetString, AddString, value);
        }
        break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE:
    {
        Message *msgField = (repeated) ? ref->AddMessage(&msg, field)
                                       : ref->MutableMessage(&msg, field);
        _json2pb(*msgField, jsonValue);
        break;
    }
    case FieldDescriptor::CPPTYPE_ENUM:
    {
        const EnumDescriptor *ed = field->enum_type();
        const EnumValueDescriptor *ev = 0;

        if (jsonValue.IsInt())
        {
            ev = ed->FindValueByNumber(jsonValue.GetInt());
        }
        else if (jsonValue.IsString())
        {
            std::string name(jsonValue.GetString(), jsonValue.GetStringLength());
            ev = ed->FindValueByName(name);
        }
        else
        {
            throw jsonpb_error(field, "Not an integer or string");
        }

        if (!ev)
        {
            throw jsonpb_error(field, "Enum value not found");
        }

        _SET_OR_ADD(SetEnum, AddEnum, ev);
        break;
    }
    default:
        break;
    }
}

void json2pb(google::protobuf::Message &msg, const std::string &json)
{
    rapidjson::Document doc;
    if (doc.Parse(json.data()).HasParseError())
    {
        auto errCode = doc.GetParseError();
        throw jsonpb_error("Parse JSON data error, errCode = " + std::to_string(errCode));
    }
    _json2pb(msg, doc);
}

void pb2json(std::string &json, const google::protobuf::Message &msg)
{
    json.clear();
    rapidjson::StringBuffer jsonBuf;
    rapidjson::Writer<rapidjson::StringBuffer> jsonWriter(jsonBuf);
    jsonWriter.StartObject();
    _pb2json(jsonWriter, msg);
    jsonWriter.EndObject();
    json.append(jsonBuf.GetString(), jsonBuf.GetSize());
}

std::string pb2json(const google::protobuf::Message &msg)
{
    std::string json;
    pb2json(json, msg);
    return json;
}

void pb2json_raw(JsonWriter &jsonWriter, const google::protobuf::Message &msg)
{
    jsonWriter.StartObject();
    _pb2json(jsonWriter, msg);
    jsonWriter.EndObject();
}

void str2json(std::string &json, const std::string &str)
{
    json.clear();
    rapidjson::StringBuffer jsonBuf;
    rapidjson::Writer<rapidjson::StringBuffer> jsonWriter(jsonBuf);
    jsonWriter.String(str.c_str(), str.size());
    json.append(jsonBuf.GetString(), jsonBuf.GetSize());
}
