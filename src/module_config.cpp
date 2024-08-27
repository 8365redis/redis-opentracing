#include <dlfcn.h>
#include "module_config.h"
#include "ini.h"
#include "logger.h"

static std::string GetModulePath() {
    Dl_info dl_info;
    dladdr(reinterpret_cast<void*>(&GetModulePath), &dl_info);
    const std::string module_lib_path = dl_info.dli_fname;
    const size_t found = module_lib_path.find_last_of('/');
    return module_lib_path.substr(0, found + 1);
}

void Module_Config::Read_Module_Config(RedisModuleCtx *ctx, const std::string& config_file_path_str) {
    std::string config_file_full_path;
    if (config_file_path_str.empty()) {
        config_file_full_path = GetModulePath() + DEFAULT_CONFIG_FILE_NAME;
    } else {
        config_file_full_path = config_file_path_str;
    }
    const mINI::INIFile file(config_file_full_path);
    mINI::INIStructure config;
    const bool config_read = file.read(config);

    if(config_read == false) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config file is not loaded (using default values) : " + config_file_full_path);
    } else {
        if(config_file_path_str.empty()) {
            LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config file is read from default config file." );
        } else {
            LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config file provided as argument." );
        }
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config values read from file: " + config_file_full_path);

        if(config[DEFAULT_CONFIG_SECTION].size() == 0) {
            LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config section " + DEFAULT_CONFIG_SECTION + " is missing. Using default values.");
            return;
        }

        if(!config[DEFAULT_CONFIG_SECTION][MONITORING_RETENTION_CONFIG].empty() ) {
            try{
                const int monitoring_retention_hours = std::stoi(config[DEFAULT_CONFIG_SECTION][MONITORING_RETENTION_CONFIG]);
                monitoring_retention = monitoring_retention_hours * 60 * 60 * 1000;
            } catch (std::invalid_argument const& _) {
                LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config value " + MONITORING_RETENTION_CONFIG  + " has failed to read value.");
            }
            LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config value [MONITORING_RETENTION_CONFIG] : " + std::to_string(monitoring_retention));
        } else {
            LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config value [MONITORING_RETENTION_CONFIG] is not found in config file using default.");
        }

        if(!config[DEFAULT_CONFIG_SECTION][EXTRACT_FT_DATA].empty() ) {
            try{
                const int monitoring_retention_hours = stringToBool(config[DEFAULT_CONFIG_SECTION][EXTRACT_FT_DATA]);
                parse_ft_queries = monitoring_retention_hours * 60 * 60 * 1000;
            } catch (std::invalid_argument const& _) {
                LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config value " + EXTRACT_FT_DATA  + " has failed to read value.");
            }
            LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config value [EXTRACT_FT_DATA] : " + std::to_string(parse_ft_queries));
        } else {
            LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "Module config value [EXTRACT_FT_DATA] is not found in config file using default.");
        }
    }
}