#include "parse_utils.h"

std::string concatenateArguments(RedisModuleString **argv, int argc) {
    std::string result;

    for (int i = 0; i < argc; ++i) {
        size_t len;
        const char *arg = RedisModule_StringPtrLen(argv[i], &len);
        result.append(arg, len);

        if (i < argc - 1) {
            result.append(" ");
        }
    }

    return result;
}

std::string ParseFtCommand(const std::string& command) {
    static const std::string ft_prefix = "ft.";
    static const std::string ft_search_suffix = "search";
    static const std::string ft_aggregate_suffix = "aggregate";
    static const std::string ft_tagvals_suffix = "tagvals";

    if (command.size() < 3 || std::tolower(command[0]) != 'f' || std::tolower(command[1]) != 't' || command[2] != '.') {
        return "";
    }

    const auto ft_cmd = command.substr(3, command.size());

    if (ft_cmd == ft_search_suffix) {
        return FT_SEARCH_CMD;
    }
    if (ft_cmd == ft_aggregate_suffix) {
        return FT_AGGREGATE_CMD;
    }
    if (ft_cmd == ft_tagvals_suffix) {
        return FT_TAGVALS_CMS;
    }
    return "";
}

bool stringToBool(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned const char c) { return std::tolower(c);});

    if (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "on") {
        return true;
    }

    if (lowerStr == "false" || lowerStr == "0" || lowerStr == "no" || lowerStr == "off") {
        return false;
    }

    throw std::invalid_argument("Invalid boolean string: " + str);
}

