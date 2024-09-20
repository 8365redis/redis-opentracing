#include "parse_utils.h"

std::string ConcatArgs(RedisModuleString **argv, const int argc) {
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

    if (equalsIgnoreCase(ft_cmd, ft_search_suffix)) {
        return FT_SEARCH_CMD;
    }
    if (equalsIgnoreCase(ft_cmd, ft_aggregate_suffix)) {
        return FT_AGGREGATE_CMD;
    }
    if (equalsIgnoreCase(ft_cmd, ft_tagvals_suffix)) {
        return FT_TAGVALS_CMS;
    }
    return "";
}

std::string EscapeTSLabelValue(const std::string& label_value) {
    if (label_value.empty()) {
        return "\"\"";
    }

    std::string escaped_value;
    escaped_value.reserve(label_value.size());

    for (const char it : label_value) {
        switch (it) {
            case '"':
                escaped_value += LABEL_QUOTE_REPLACEMENT;
                break;
            case '(':
                escaped_value += LABEL_OPEN_PARANTHESIS_REPLACEMENT;
                break;
            case ')':
                escaped_value += LABEL_CLOSED_PARANTHESIS_REPLACEMENT;
                break;
            case ',':
                escaped_value += LABEL_COMMA_REPLACEMENT;
                break;
            default:
                escaped_value += it;
        }
    }

    return escaped_value;
}

