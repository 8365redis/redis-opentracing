#ifndef PARSE_UTILS_H
#define PARSE_UTILS_H
#include <string>
#include <iomanip>
#include <module_constants.h>
#include "redismodule.h"
#include "misc_utils.h"

std::string ConcatArgs(RedisModuleString **argv, int argc);
std::string EscapeTSLabelValue(const std::string& label_value);

#endif /* PARSE_UTILS_H */
