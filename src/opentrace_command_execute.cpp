#include "opentrace_command_execute.h"

int TRACE_Execute_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, const int argc) {
    RedisModule_AutoMemory(ctx);

    const auto latency_metric_start = Get_Start_Time();
    const auto latency_metric_start_epoch = Get_Epoch_Time();

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

    const RedisModuleString *client_id_str = argv[client_id_idx];

    std::string client_id = RedisModule_StringPtrLen(client_id_str, nullptr);

    std::string command_type = UNDEFINED_LABEL_VALUE;
    std::string index_name = UNDEFINED_LABEL_VALUE;

    const Module_Config &module_config = Module_Config::getInstance();
    const std::string &redis_command = RedisModule_StringPtrLen(argv[cmd_idx], nullptr);

    if (module_config.Get_Parse_Ft_Queries()) {
        const std::string ft_query_cmd = ParseFtCommand(redis_command);
        if (!ft_query_cmd.empty()) {
            index_name = RedisModule_StringPtrLen(argv[cmd_args_idx], nullptr);  // Assuming the index name is the next argument
            command_type = ft_query_cmd;
        }
    }

    RedisModuleCallReply *reply = RedisModule_Call(ctx, redis_command.c_str(), "v",
        &argv[cmd_args_idx],
        argc - cmd_args_idx);

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

    const auto command_and_args = concatenateArguments(argv + cmd_idx, argc - cmd_idx);

    const std::vector<std::pair<std::string, std::string>> label_pairs = {
        {CLIENT_ID_LABEL_KEY, client_id},
        {COMMAND_TYPE_LABEL_KEY, command_type},
        {INDEX_NAME_LABEL_KEY, index_name},
        {COMMAND_TYPE_LABEL_KEY, command_and_args}
    };

    const auto latency_metric = Get_Delta_Time(latency_metric_start);
    const bool metric_added = Add_Metric(ctx, latency_metric_start_epoch,
        (TS_PREFIX + client_id + TS_DELIMITER + command_and_args),
        std::to_string(latency_metric),
        label_pairs);
    if (!metric_added) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING, "TRACE_Execute_RedisCommand failed to add metric.");
    }

    return RedisModule_ReplyWithCallReply(ctx, reply);
}


