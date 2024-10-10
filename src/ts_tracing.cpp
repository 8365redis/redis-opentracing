#include "ts_tracing.h"
#include "module_config.h"
#include "logger.h"
#include "json/json.hpp"

#include <iostream>

bool Add_Metric(RedisModuleCtx *ctx, unsigned long long metric_ts, std::string name, double value, std::string value_type, std::string module, std::string version, std::string command, const std::map<std::string, std::string>& tags)
 {
    if(ctx == nullptr) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Add_Metric failed , context is invalid.");
        return false;
    }

    if(name.empty() || value <= 0) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Add_Metric failed , metric name or value can not be empty.");
        return false;
    }

    // Create the JSON metric object
    nlohmann::json metric;
    metric[METRIC_VERSION_KEY] = version;
    metric[METRIC_TS_KEY] = metric_ts;
    metric[METRIC_MODULE_KEY] = module;
    metric[COMMAND_KEY] = command;
    // Insert the provided tags into the metric JSON under METRIC_TAGS_KEY
    for (const auto& tag : tags) {
        metric[METRIC_TAGS_KEY][tag.first] = tag.second;
    }
    // Add metric data
    metric[METRIC_METRIC_KEY][METRIC_NAME_KEY] = name;
    metric[METRIC_METRIC_KEY][METRIC_VALUE_KEY] = value;
    metric[METRIC_METRIC_KEY][METRIC_VALUE_TYPE_KEY] = value_type;
    std::string metric_str = metric.dump();

    const int stream_write_size_total = 2; // metric name + metric data
    RedisModuleString* stream_name = RedisModule_CreateString(ctx, OPENTRACING_STREAM_NAME.c_str(), OPENTRACING_STREAM_NAME.length());
    RedisModuleKey *stream_key = RedisModule_OpenKey(ctx, stream_name, REDISMODULE_WRITE);
    RedisModuleString **xadd_params = (RedisModuleString **) RedisModule_Alloc(sizeof(RedisModuleString *) * stream_write_size_total);

    xadd_params[0] = RedisModule_CreateString(ctx, OPENTRACING_METRIC_KEY_NAME.c_str(), OPENTRACING_METRIC_KEY_NAME.length());
    xadd_params[1] = RedisModule_CreateString(ctx, metric_str.c_str(), metric_str.length());

    int stream_add_resp = RedisModule_StreamAdd( stream_key, REDISMODULE_STREAM_ADD_AUTOID, NULL, xadd_params, stream_write_size_total/2);
    if (stream_add_resp != REDISMODULE_OK) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Add_Metric failed to add to the stream.");
        return false;
    }

    Module_Config &module_config = Module_Config::getInstance();
    RedisModuleString* mon_stream_name = RedisModule_CreateString(ctx, OPENTRACING_STREAM_NAME.c_str(), OPENTRACING_STREAM_NAME.length());
    RedisModuleKey *mon_stream_key = RedisModule_OpenKey(ctx, mon_stream_name, REDISMODULE_WRITE);
    RedisModule_StreamTrimByLength(mon_stream_key, REDISMODULE_STREAM_TRIM_APPROX, module_config.Get_Monitoring_Stream_Cap());

    return true;
}

std::chrono::steady_clock::time_point Get_Start_Time() {
    return std::chrono::steady_clock::now();
}

double Get_Delta_Time(std::chrono::steady_clock::time_point start) {
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(); // NOLINT(*-narrowing-conversions)
}

unsigned long long Get_Epoch_Time(){
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}