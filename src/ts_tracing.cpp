#include "ts_tracing.h"
#include "module_config.h"
#include "logger.h"
#include "json/json.hpp"

#include <iostream>
#include <sstream>

const int TRIM_BUFFER_SIZE = 10000;

bool Add_Metric(RedisModuleCtx *ctx, unsigned long long metric_ts, std::string name, double value, std::string value_type, std::string module, std::string version, std::string command, const std::map<std::string, std::string>& tags)
 {
    static int trim_counter = 0;
    static int metric_push_counter = 0;
    trim_counter++;
    metric_push_counter++;
    if(ctx == nullptr) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Add_Metric failed , context is invalid.");
        return false;
    }

    if(name.empty() || value <= 0) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Add_Metric failed , metric name or value can not be empty.");
        return false;
    }

    std::stringstream json;
    // Open the JSON object
    json << "{";

    // Add basic metric fields (version, timestamp, module, command)
    json << "\"" << METRIC_VERSION_KEY << "\":\"" << version << "\",";
    json << "\"" << METRIC_TS_KEY << "\":" << std::to_string(metric_ts) << ",";
    json << "\"" << METRIC_MODULE_KEY << "\":\"" << module << "\",";
    json << "\"" << COMMAND_KEY << "\":\"" << command << "\",";

    // Add tags as an object inside the metric
    json << "\"" << METRIC_TAGS_KEY << "\":{";
    for (auto it = tags.begin(); it != tags.end(); ++it) {
        json << "\"" << it->first << "\":\"" << it->second << "\"";
        if (std::next(it) != tags.end()) {
            json << ",";  // Add a comma if it's not the last element
        }
    }
    json << "},";  // End of tags

    // Add metric data (name, value, value_type)
    json << "\"" << METRIC_METRIC_KEY << "\":{";
    json << "\"" << METRIC_NAME_KEY << "\":\"" << name << "\",";
    json << "\"" << METRIC_VALUE_KEY << "\":" << std::to_string(value) << ",";
    json << "\"" << METRIC_VALUE_TYPE_KEY << "\":\"" << value_type << "\"";
    json << "}";  // End of metric object

    // Close the JSON object
    json << "}";


    std::string metric_str = json.str();

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

    if(trim_counter > TRIM_BUFFER_SIZE) {
        Module_Config &module_config = Module_Config::getInstance();
        RedisModuleString* mon_stream_name = RedisModule_CreateString(ctx, OPENTRACING_STREAM_NAME.c_str(), OPENTRACING_STREAM_NAME.length());
        RedisModuleKey *mon_stream_key = RedisModule_OpenKey(ctx, mon_stream_name, REDISMODULE_WRITE);
        RedisModule_StreamTrimByLength(mon_stream_key, REDISMODULE_STREAM_TRIM_APPROX, module_config.Get_Monitoring_Stream_Cap());
        trim_counter = 0;
    }

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