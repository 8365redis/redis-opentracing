#include "monitoring_manager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

#include "json/json.hpp"

const int TRIM_BUFFER_SIZE = 10000;

// Helper function to escape special characters in JSON strings
std::string Escape_Json_String(const std::string& input) {
    std::string output;
    output.reserve(input.size());

    for (const char c : input) {
        switch (c) {
            case '\"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b";  break;
            case '\f': output += "\\f";  break;
            case '\n': output += "\\n";  break;
            case '\r': output += "\\r";  break;
            case '\t': output += "\\t";  break;
            default : output += c; break;
        }
    }

    return output;
}

std::string Create_Metric_Json_String_Stream(
    const std::string& version,
    long long metric_ts,
    const std::string& module,
    const std::string& command,
    const std::map<std::string, std::string>& tags,
    const std::string& name,
    double value,
    const std::string& value_type
) {
    // Estimate the size to reserve to reduce reallocations
    std::string json;
    json.reserve(10240);  // Adjust based on expected size

    json += '{';

    // Add basic metric fields (version, timestamp, module, command)
    json += '"'; json += METRIC_VERSION_KEY; json += "\":\""; json += Escape_Json_String(version); json += "\",";
    json += '"'; json += METRIC_TS_KEY;      json += "\":";

    json += std::to_string(metric_ts);

    json += ',';

    json += '"'; json += METRIC_MODULE_KEY; json += "\":\""; json += Escape_Json_String(module); json += "\",";
    json += '"'; json += COMMAND_KEY;       json += "\":\""; json += Escape_Json_String(command); json += "\",";

    // Add tags
    json += '"'; json += METRIC_TAGS_KEY; json += "\":{";
    for (auto it = tags.begin(); it != tags.end(); ++it) {
        json += '"'; json += Escape_Json_String(it->first); json += "\":\""; json += Escape_Json_String(it->second); json += '"';
        if (std::next(it) != tags.end()) {
            json += ',';
        }
    }
    json += "},";

    // Add metric data
    json += '"'; json += METRIC_METRIC_KEY; json += "\":{";
    json += '"'; json += METRIC_NAME_KEY;   json += "\":\""; json += Escape_Json_String(name); json += "\",";
    json += '"'; json += METRIC_VALUE_KEY;  json += "\":";

    json += std::to_string(value);

    json += ',';

    json += '"'; json += METRIC_VALUE_TYPE_KEY; json += "\":\""; json += Escape_Json_String(value_type); json += '"';
    json += '}';  // Close metric object

    json += '}';  // Close JSON object

    return json;

}

Monitoring_Manager& Monitoring_Manager::Get_Instance() {
    static Monitoring_Manager instance;
    return instance;
}

void Monitoring_Manager::Init(int monitoring_stream_cap, void(*logger)(RedisModuleCtx*, const char *, const std::string&)) {
    this->monitoring_stream_cap = monitoring_stream_cap;
    this->logger = logger ? logger : [](RedisModuleCtx*, const char*, const std::string&) {};
}

bool Monitoring_Manager::Add_Metric(RedisModuleCtx *ctx, unsigned long long metric_ts, const std::string& name, double value, const std::string& value_type, const std::string& module, const std::string& version, const std::string& command, const std::map<std::string, std::string>& tags) {
    trim_counter++;
    if (ctx == nullptr) {
        logger(ctx, REDISMODULE_LOGLEVEL_WARNING, "Add_Metric failed, context is invalid.");
        return false;
    }

    if (name.empty() || value <= 0) {
        logger(ctx, REDISMODULE_LOGLEVEL_WARNING, "Add_Metric failed, metric name or value cannot be empty.");
        return false;
    }

    if (trim_counter >= std::min(monitoring_stream_cap, TRIM_BUFFER_SIZE)) {
        // Check stream length
        RedisModuleCallReply *reply = RedisModule_Call(ctx, "XLEN", "c", OPENTRACING_STREAM_NAME.c_str());
        if (reply == nullptr || RedisModule_CallReplyType(reply) != REDISMODULE_REPLY_INTEGER) {
            logger(ctx, REDISMODULE_LOGLEVEL_WARNING, "Failed to get stream length.");
            RedisModule_FreeCallReply(reply);
            return false;
        }

        long long stream_length = RedisModule_CallReplyInteger(reply);
        RedisModule_FreeCallReply(reply);

        if (stream_length >= monitoring_stream_cap) {
            // Generate new stream name for the old stream
            unsigned long long timestamp = Get_Epoch_Time();
            std::string new_stream_name = "OPENTRACING_" + std::to_string(timestamp);

            // Rename the old stream
            reply = RedisModule_Call(ctx, "RENAME", "cc", OPENTRACING_STREAM_NAME.c_str(), new_stream_name.c_str());
            if (reply == nullptr || RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ERROR) {
                logger(ctx, REDISMODULE_LOGLEVEL_WARNING, "Failed to rename the stream.");
                return false;
            }
            RedisModule_FreeCallReply(reply);
        }
        trim_counter = 0;
    }

    // Generate the JSON string using the function
    std::string metric_str = Create_Metric_Json_String_Stream(version, metric_ts, module, command, tags, name, value, value_type);

    RedisModuleString* stream_name = RedisModule_CreateString(ctx, OPENTRACING_STREAM_NAME.c_str(), OPENTRACING_STREAM_NAME.length());
    RedisModuleKey *stream_key = RedisModule_OpenKey(ctx, stream_name, REDISMODULE_WRITE);

    const int stream_write_size_total = 2; // metric name + metric data
    RedisModuleString **xadd_params = (RedisModuleString **) RedisModule_Alloc(sizeof(RedisModuleString *) * stream_write_size_total);
    xadd_params[0] = RedisModule_CreateString(ctx, OPENTRACING_METRIC_KEY_NAME.c_str(), OPENTRACING_METRIC_KEY_NAME.length());
    xadd_params[1] = RedisModule_CreateString(ctx, metric_str.c_str(), metric_str.length());

    int stream_add_resp = RedisModule_StreamAdd(stream_key, REDISMODULE_STREAM_ADD_AUTOID, NULL, xadd_params, stream_write_size_total / 2);
    if (stream_add_resp != REDISMODULE_OK) {
        logger(ctx, REDISMODULE_LOGLEVEL_WARNING, "Add_Metric failed to add to the stream.");
        return false;
    }

    return true;
}

std::chrono::steady_clock::time_point Monitoring_Manager::Get_Start_Time() {
    return std::chrono::steady_clock::now();
}

double Monitoring_Manager::Get_Delta_Time(std::chrono::steady_clock::time_point start) {
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

unsigned long long Monitoring_Manager::Get_Epoch_Time() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}