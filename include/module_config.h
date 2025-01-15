#ifndef MODULE_CONFIG_H
#define MODULE_CONFIG_H

#include <string>
#include "redismodule.h"
#include "misc_utils.h"

constexpr unsigned long DEFAULT_MONITORING_STREAM_CAP = 100000; // Default cap 100k
constexpr unsigned long DEFAULT_MONITORING_TRIM_BUFFER_SIZE = 10000;

const std::string DEFAULT_CONFIG_FILE_NAME  = "redis-opentracing-config.ini";
const std::string DEFAULT_CONFIG_SECTION  = "redis-opentracing";

const std::string MONITORING_STREAM_CAP_CONFIG = "MONITORING_STREAM_CAP";
const std::string MONITORING_BUFFER_SIZE_CONFIG = "MONITORING_TRIM_BUFFER_SIZE";
const std::string EXTRACT_FT_DATA = "IS_EXTRACT_FT_DATA";


class Module_Config {
public:
    static Module_Config& getInstance(){
        static Module_Config instance; 
        return instance;
    }
private:
    Module_Config() {}
    unsigned long long monitoring_stream_cap = DEFAULT_MONITORING_STREAM_CAP;
    unsigned long long monitoring_trim_buffer_size = DEFAULT_MONITORING_TRIM_BUFFER_SIZE;
public:
    Module_Config(Module_Config const&) = delete;
    void operator=(Module_Config const&) = delete;
    void Read_Module_Config(RedisModuleCtx *ctx, const std::string& config_file_path_str);
    unsigned long long Get_Monitoring_Stream_Cap() const { return monitoring_stream_cap; }
    unsigned long long Get_Monitoring_Trim_Buffer_Size() const { return monitoring_trim_buffer_size; }
};
#endif /* MODULE_CONFIG_H */