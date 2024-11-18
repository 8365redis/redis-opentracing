#include "opentrace_command_execute.h"
#include "version.h"

int TRACE_Execute_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);

    const auto cct_command_prefix = "CCT2.";

    const auto latency_metric_start = Monitoring_Manager::Get_Instance().Get_Start_Time();
    const auto latency_metric_start_epoch = Monitoring_Manager::Get_Instance().Get_Epoch_Time();

    if (argc < 4) {
        return RedisModule_WrongArity(ctx);
    }

    constexpr int client_id_idx = 1;
    constexpr int cmd_delimiter_idx = client_id_idx + 1;
    constexpr int cmd_idx = cmd_delimiter_idx + 1;
    constexpr int cmd_args_idx = cmd_idx + 1;

    if (RedisModule_StringPtrLen(argv[cmd_delimiter_idx], nullptr) != CMD_DELIMITER) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING, "TRACE_Execute_RedisCommand failed to execute command.");
        return RedisModule_ReplyWithError(ctx, "Synthax exception. Expected TRACE.EXECUTE [client_id] CMD [**args]");
    }

    RedisModuleString *client_id_str = argv[client_id_idx];

    std::string client_id = RedisModule_StringPtrLen(client_id_str, nullptr);

    std::string command_type = UNDEFINED_LABEL_VALUE;
    std::string index_name = UNDEFINED_LABEL_VALUE;

    const Module_Config &module_config = Module_Config::getInstance();
    const std::string &redis_command = RedisModule_StringPtrLen(argv[cmd_idx], nullptr);

    bool is_cct_command = false;
    // Check if redis_command starts with CCT2. prefix
    if (redis_command.find(cct_command_prefix) == 0) {
        LOG(ctx, REDISMODULE_LOGLEVEL_DEBUG, "TRACE_Execute_RedisCommand is a CCT2 command.");
        is_cct_command = true;
    }

    RedisModuleString **new_argv = nullptr;
    int new_argc = 0;
    // If it is cct command, add "CLIENTNAME" and client_id to end of the argv after extending the array
    RedisModuleString *client_name_str_key = NULL;
    RedisModuleString *client_name_str = NULL;
    if (is_cct_command) {
        client_name_str_key = RedisModule_CreateString(ctx, CLIENTNAME_KEY.c_str(), CLIENTNAME_KEY.length());
        client_name_str = RedisModule_CreateString(ctx, client_id.c_str(), client_id.length());
        new_argc = 2 + (argc - cmd_args_idx);
        new_argv = (RedisModuleString **)malloc(new_argc * sizeof(RedisModuleString *));
        if (new_argv == NULL) {
            return RedisModule_ReplyWithError(ctx, "ERR memory allocation failed");
        }
        LOG(ctx, REDISMODULE_LOGLEVEL_DEBUG, "TRACE_Execute_RedisCommand new argc is : " + std::to_string(new_argc));

        int append_idx = 0;
        for (int i = cmd_args_idx; i < argc; i++) {
            LOG(ctx, REDISMODULE_LOGLEVEL_DEBUG, "TRACE_Execute_RedisCommand argv[" + std::to_string(i) + "] is : " + RedisModule_StringPtrLen(argv[i], nullptr));
            new_argv[append_idx] = argv[i];
            append_idx++;
        }
        // Append the new arguments
        new_argv[append_idx] = client_name_str_key;
        new_argv[append_idx + 1] = client_name_str;
    }

    for (int i = 0; i < new_argc; i++) {
        if(new_argv[i] != NULL){
            LOG(ctx, REDISMODULE_LOGLEVEL_DEBUG, "TRACE_Execute_RedisCommand new argv[" + std::to_string(i) + "] is : " + RedisModule_StringPtrLen(new_argv[i], nullptr));
        } else {
            LOG(ctx, REDISMODULE_LOGLEVEL_DEBUG, "TRACE_Execute_RedisCommand new argv[" + std::to_string(i) + "] is NULL");
        }
    }

    RedisModuleCallReply *reply = nullptr;
    if(is_cct_command){
        reply = RedisModule_Call(ctx, redis_command.c_str(), "v",
            new_argv,
            new_argc);        

    } else {
        reply = RedisModule_Call(ctx, redis_command.c_str(), "v",
            &argv[cmd_args_idx],
            argc - cmd_args_idx);
    }

    if (reply == nullptr) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING, "TRACE_Execute_RedisCommand failed to execute command.");
        return RedisModule_ReplyWithError(ctx, "TRACE command execution failed: NULL reply");
    }

    if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ERROR) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING, "TRACE_Execute_RedisCommand failed to execute command.");
        size_t len;
        const char *error_msg = RedisModule_CallReplyStringPtr(reply, &len);
        return RedisModule_ReplyWithError(ctx, error_msg);
    }

    
    const auto command_and_args = ConcatArgs(argv + cmd_idx, argc - cmd_idx);
    const auto latency_metric = Monitoring_Manager::Get_Instance().Get_Delta_Time(latency_metric_start);

    std::map<std::string, std::string> tags = {
        {CLIENT_KEY, client_id},
        {INDEX_NAME_KEY, index_name},
        {COMMAND_TYPE_KEY, redis_command}
    };

    std::string module_version = VersionManager::GetInstance().Get_Module_Version_Str();
    bool metric_added = Monitoring_Manager::Get_Instance().Add_Metric(ctx, latency_metric_start_epoch, METRIC_NAME_LATENCY, latency_metric, METRIC_VALUE_TYPE_NS, MODULE_NAME, module_version, command_and_args, tags);
 
    if (!metric_added) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING, "TRACE_Execute_RedisCommand failed to add metric.");
    }
    
    return RedisModule_ReplyWithCallReply(ctx, reply);
}
