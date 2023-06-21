#pragma once
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "google/protobuf/message.h"

using JsonWriter = rapidjson::Writer<rapidjson::StringBuffer>;
using JsonReader = rapidjson::Document;
using JsonValue = rapidjson::Value;

void json2pb(google::protobuf::Message &msg, const std::string &json);

void pb2json(std::string &json, const google::protobuf::Message &msg);
std::string pb2json(const google::protobuf::Message &msg);
void pb2json_raw(JsonWriter &json_writer, const google::protobuf::Message &msg);

void str2json(std::string &json, const std::string &str);