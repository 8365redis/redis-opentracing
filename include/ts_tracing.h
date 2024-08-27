#ifndef TS_TRACING_H
#define TS_TRACING_H

#include <chrono>
#include <string>
#include <vector>
#include "redismodule.h"


bool Add_Metric(RedisModuleCtx *ctx, unsigned long long metric_ts,
    const std::string& metric_name,
    const std::string& metric_value,
    const std::vector<std::pair<std::string,std::string>>& label_pairs);

std::chrono::steady_clock::time_point Get_Start_Time();
double Get_Delta_Time(std::chrono::steady_clock::time_point start);
unsigned long long Get_Epoch_Time();

std::vector<std::pair<std::string,std::string>> Create_Latency_Labels(std::string client,
    std::string search_type);


#endif //TS_TRACING_H
