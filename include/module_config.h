#ifndef MODULE_CONFIG_H
#define MODULE_CONFIG_H

#include <string>
#include "redismodule.h"
#include "misc_utils.h"

constexpr unsigned long DEFAULT_MONITORING_STREAM_CAP = 1000000; // Default cap 1M
constexpr bool DEFAULT_PARSE_FT_COMMANDS = true;

const std::string DEFAULT_CONFIG_FILE_NAME  = "redis-opentracing-config.ini";
const std::string DEFAULT_CONFIG_SECTION  = "redis-opentracing";

const std::string MONITORING_STREAM_CAP_CONFIG = "MONITORING_STREAM_CAP";
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
    bool parse_ft_queries = DEFAULT_PARSE_FT_COMMANDS;
public:
    Module_Config(Module_Config const&) = delete;
    void operator=(Module_Config const&) = delete;
    void Read_Module_Config(RedisModuleCtx *ctx, const std::string& config_file_path_str);
    unsigned long long Get_Monitoring_Stream_Cap() const { return monitoring_stream_cap; }
    bool Get_Parse_Ft_Queries() const { return parse_ft_queries; }
};
#endif /* MODULE_CONFIG_H */