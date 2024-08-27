#ifndef PARSE_UTILS_H
#define PARSE_UTILS_H
#include <string>
#include <iomanip>
#include <module_constants.h>
#include "redismodule.h"
#include <algorithm>

std::string ParseFtCommand(const std::string& command);
std::string concatenateArguments(RedisModuleString **argv, int argc);
bool stringToBool(const std::string& str);

#endif //PARSE_UTILS_H
