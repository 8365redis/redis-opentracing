#include "parse_utils.h"


std::vector<std::string> TokenizeCommandString(const std::string& command_str) {
    std::vector<std::string> result;
    std::string token;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (size_t i = 0; i < command_str.length(); ++i) {
        char c = command_str[i];

        if ((c == '"' || c == '\'') && (i == 0 || command_str[i - 1] != '\\')) {
            if (inQuotes && c == quoteChar) {
                inQuotes = false;
            } else if (!inQuotes) {
                inQuotes = true;
                quoteChar = c;
            }
        } else if (c == ' ' && !inQuotes) {
            if (!token.empty()) {
                result.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }

    if (!token.empty()) {
        result.push_back(token);
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

    if (command == ft_search_suffix) {
        return FT_SEARCH_CMD;
    }
    if (command == ft_aggregate_suffix) {
        return FT_AGGREGATE_CMD;
    }
    if (command == ft_tagvals_suffix) {
        return FT_TAGVALS_CMS;
    }
    return "";
}

