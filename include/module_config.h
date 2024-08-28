#ifndef MODULE_CONFIG_H
#define MODULE_CONFIG_H

#include <string>
#include "redismodule.h"
#include "misc_utils.h"

constexpr unsigned long DEFAULT_RETENTION = 1000 * 60 * 60 * 24 * 7; // Default retention 7 days
constexpr bool DEFAULT_PARSE_FT_COMMANDS = true;

const std::string DEFAULT_CONFIG_FILE_NAME  = "redis-opentracing-config.ini";
const std::string DEFAULT_CONFIG_SECTION  = "redis-opentracing";

const std::string MONITORING_RETENTION_CONFIG = "MONITORING_RETENTION_HOURS";
const std::string EXTRACT_FT_DATA = "IS_EXTRACT_FT_DATA";


class Module_Config {
public:
    static Module_Config& getInstance(){
        static Module_Config instance; 
        return instance;
    }
private:
    Module_Config() {}
    unsigned long long monitoring_retention = DEFAULT_RETENTION;
    bool parse_ft_queries = DEFAULT_PARSE_FT_COMMANDS;
public:
    Module_Config(Module_Config const&) = delete;
    void operator=(Module_Config const&) = delete;
    void Read_Module_Config(RedisModuleCtx *ctx, const std::string& config_file_path_str);
    unsigned long long Get_Monitoring_Retention() const { return monitoring_retention; }
    bool Get_Parse_Ft_Queries() const { return parse_ft_queries; }
};
#endif /* MODULE_CONFIG_H */