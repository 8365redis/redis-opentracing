#include "opentrace_command_execute.h"

int TRACE_Execute_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);

    const auto latency_metric_start = Get_Start_Time();
    const auto latency_metric_start_epoch = Get_Epoch_Time();

    if (argc < 3) {
        return RedisModule_WrongArity(ctx);
    }

    const RedisModuleString *command_str = argv[1];
    const RedisModuleString *client_id_str = argv[2];

    std::string client_id = RedisModule_StringPtrLen(client_id_str, nullptr);

    const std::string command_and_args = RedisModule_StringPtrLen(command_str, nullptr);

    const std::vector<std::string> parsed_args = TokenizeCommandString(command_and_args);

    if (parsed_args.empty()) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING, "TRACE_Execute_RedisCommand failed, command parsing error.");
        return RedisModule_ReplyWithError(ctx, "ERR Command parsing failed");
    }

    std::string command_type = UNDEFINED_LABEL_VALUE;
    std::string index_name = UNDEFINED_LABEL_VALUE;

    const Module_Config &module_config = Module_Config::getInstance();

    if (module_config.Get_Parse_Ft_Queries()) {
        const std::string &command = parsed_args[0];
        const std::string ft_query_cmd = ParseFtCommand(command);
        if (!ft_query_cmd.empty()) {
            if (parsed_args.size() > 1) {
                index_name = parsed_args[1];  // Assuming the index name is the second argument
            }
            command_type = ft_query_cmd;
        }
    }

    std::vector<RedisModuleString*> redis_args;
    for (const auto& arg : parsed_args) {
        redis_args.push_back(RedisModule_CreateString(ctx, arg.c_str(), arg.size()));
    }

    RedisModuleCallReply *reply = RedisModule_Call(ctx, parsed_args[0].c_str(), "v", redis_args.data(), redis_args.size());
    if (reply == nullptr || RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ERROR) {
        LOG(ctx, REDISMODULE_LOGLEVEL_WARNING, "TRACE_Execute_RedisCommand failed to execute command.");
        return RedisModule_ReplyWithError(ctx, "ERR Command execution failed");
    }

    const std::vector<std::pair<std::string, std::string>> label_pairs = {
        {CLIENT_ID_LABEL_KEY, client_id},
        {COMMAND_TYPE_LABEL_KEY, command_type},
        {INDEX_NAME_LABEL_KEY, index_name}
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


