#include "ts_tracing.h"
#include "module_config.h"
#include "logger.h"

#include <iostream>

bool Add_Metric(RedisModuleCtx *ctx,
                const unsigned long long metric_ts,
                const std::string& metric_name,
                const std::string& metric_value,
                const std::vector<std::pair<std::string,std::string>>& label_pairs) {
    if(ctx == nullptr) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Add_Metric failed , context is invalid.");
        return false;
    }

    if(metric_name.empty() || metric_value.empty()) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Add_Metric failed , metric name or value can not be empty.");
        return false;
    }

    const Module_Config &module_config = Module_Config::getInstance();

    std::vector<std::string> arguments_string_vector;
    arguments_string_vector.push_back(metric_name);
    arguments_string_vector.push_back(std::to_string(metric_ts));
    arguments_string_vector.push_back(metric_value);

    if(!label_pairs.empty()) {
        arguments_string_vector.emplace_back("LABELS");
    }
    bool added_at_least_one_label = false;
    for (const auto& pair : label_pairs) {
        if (!pair.first.empty() && !pair.second.empty()) {
            arguments_string_vector.push_back(pair.first);
            arguments_string_vector.push_back(pair.second);
            added_at_least_one_label = true;
        }
    }
    if(!added_at_least_one_label && !label_pairs.empty()) { // Remove added "LABELS" if no label is added
        arguments_string_vector.pop_back();
    }

    arguments_string_vector.emplace_back("RETENTION");
    arguments_string_vector.push_back(std::to_string(module_config.Get_Monitoring_Retention()));

    std::string arguments_string_vector_str;
    for (const auto& str : arguments_string_vector) {
        arguments_string_vector_str += str + " ";
    }

    const int new_argc = arguments_string_vector.size(); // NOLINT(*-narrowing-conversions)
    auto **new_argv = static_cast<RedisModuleString **>(RedisModule_Alloc(sizeof(RedisModuleString *) * new_argc));
    for (int i = 0; i < new_argc; ++i) {
        new_argv[i] = RedisModule_CreateString(ctx, arguments_string_vector[i].c_str(), arguments_string_vector[i].size());
    }

    RedisModuleCallReply *ts_add_reply = RedisModule_Call(ctx, "TS.ADD", "v", new_argv, new_argc);
    if (RedisModule_CallReplyType(ts_add_reply) == REDISMODULE_REPLY_ERROR) {
        size_t len;
        const char *error_msg = RedisModule_CallReplyStringPtr(ts_add_reply, &len);
        const std::string err_str(error_msg, len);
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Add_Metric failed: " + err_str);
        return false;
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