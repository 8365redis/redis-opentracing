#ifndef PARSE_UTILS_H
#define PARSE_UTILS_H
#include <string>
#include <iomanip>
#include <module_constants.h>
#include "redismodule.h"
#include "misc_utils.h"
#include <regex>

std::string ParseFtCommand(const std::string& command);
std::string ConcatArgs(RedisModuleString **argv, int argc);
std::string& EscapeTSLabelValueInPlace(std::string& label_value);
std::string EscapeTSLabelValue(const std::string& label_value);

#endif //PARSE_UTILS_H
