#include "redismodule.h"
#include "logger.h"
#include "module_config.h"

#include "opentrace_command_execute.h"

#ifndef OPENTRACING_MODULE_VERSION
#define OPENTRACING_MODULE_VERSION "unknown"
#endif

#ifdef __cplusplus
extern "C" {
#endif


RedisModuleCtx *rdts_staticCtx;

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    
    if (RedisModule_Init(ctx,"OPENTRACING",1,REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }
    
    const char* version_string = { OPENTRACING_MODULE_VERSION " compiled at " __TIME__ " "  __DATE__  };
    LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "OPENTRACING_MODULE_VERSION : " + std::string(version_string));

    #ifdef _DEBUG
    LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "THIS IS A DEBUG BUILD." );
    #endif
    #ifdef NDEBUG
    LOG(ctx, REDISMODULE_LOGLEVEL_WARNING , "THIS IS A RELEASE BUILD." );
    #endif

    rdts_staticCtx = RedisModule_GetDetachedThreadSafeContext(ctx);

    if (RedisModule_CreateCommand(ctx,"TRACE.EXECUTE", TRACE_Execute_RedisCommand , "admin write", 0, 0, 0) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }
    LOG(ctx, REDISMODULE_LOGLEVEL_DEBUG, "TRACE.EXECUTE command created successfully.");

    Module_Config &module_config = Module_Config::getInstance();
    std::string config_file_path_str;
    if (argc > 0) {
        config_file_path_str = RedisModule_StringPtrLen(argv[0], nullptr);
    }
    #ifdef _DEBUG
        config_file_path_str = "non-existing-file";
    #endif
        module_config.Read_Module_Config(ctx, config_file_path_str);

    return REDISMODULE_OK;
}


#ifdef __cplusplus
}
#endif

