#ifndef MONITORING_H
#define MONITORING_H

#include <chrono>
#include <string>
#include <vector>
#include <map>
#include "redismodule.h"


const std::string METRIC_VALUE_TYPE_NS = "ns";
const std::string OPENTRACING_STREAM_NAME = "OPENTRACING";
const std::string OPENTRACING_PREFIX = "OPENTRACING";
const std::string OPENTRACING_DELIMETER = ":";
const std::string OPENTRACING_METRIC_KEY_NAME = "TRACING";
const std::string METRIC_TAGS_KEY = "tags";
const std::string METRIC_METRIC_KEY = "metric";
const std::string METRIC_NAME_LATENCY = "latency";
const std::string COMMAND_TYPE_KEY = "command_type";
const std::string INDEX_NAME_KEY = "index_name";
const std::string COMMAND_KEY = "command";
const std::string METRIC_NAME_KEY = "name";
const std::string METRIC_VALUE_KEY = "value";
const std::string METRIC_VALUE_TYPE_KEY = "value_type";
const std::string METRIC_TS_KEY = "timestamp";
const std::string METRIC_VERSION_KEY = "version";
const std::string METRIC_MODULE_KEY = "module";
const std::string CLIENT_KEY = "client_id";
const std::string MODULE_NAME = "tracing";


class Monitoring_Manager {
public:
    static Monitoring_Manager& Get_Instance();

    void Init(int monitoring_stream_cap, void(*logger)(RedisModuleCtx*, const char *, const std::string&));

    bool Add_Metric(RedisModuleCtx *ctx, unsigned long long metric_ts, const std::string& name, double value, const std::string& value_type, const std::string& module, const std::string& version, const std::string& command, const std::map<std::string, std::string>& tags);

    std::chrono::steady_clock::time_point Get_Start_Time();
    double Get_Delta_Time(std::chrono::steady_clock::time_point start);
    unsigned long long Get_Epoch_Time();

private:
    Monitoring_Manager() = default;
    ~Monitoring_Manager() = default;
    Monitoring_Manager(const Monitoring_Manager&) = delete;
    Monitoring_Manager& operator=(const Monitoring_Manager&) = delete;

    void(*logger)(RedisModuleCtx*, const char *, const std::string&) = [](RedisModuleCtx*, const char*, const std::string&) {};
    int trim_counter = 0;
    int monitoring_stream_cap = 100000;
};

#endif // MONITORING_H
