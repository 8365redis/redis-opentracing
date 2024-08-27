#ifndef OPENTRACE_COMMAND_EXECUTE_H
#define OPENTRACE_COMMAND_EXECUTE_H

#include "redismodule.h"
#include "module_constants.h"
#include "module_config.h"
#include "parse_utils.h"
#include <algorithm>
#include "ts_tracing.h"
#include "logger.h"

int TRACE_Execute_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif /* OPENTRACE_COMMAND_EXECUTE_H */