#ifndef OPT_MODULE_CONSTANTS_H
#define OPT_MODULE_CONSTANTS_H
#include <string>

const std::string FT_SEARCH_CMD = "FT.SEARCH";
const std::string FT_AGGREGATE_CMD = "FT.AGGREGATE";
const std::string FT_TAGVALS_CMS = "FT.TAGVALS";

const std::string CMD_DELIMITER = "CMD";

const std::string TS_PREFIX = "OPENTRACING:";
const std::string TS_DELIMITER = ":";

const std::string CLIENT_ID_LABEL_KEY = "client_id";
const std::string COMMAND_TYPE_LABEL_KEY = "command_type";
const std::string INDEX_NAME_LABEL_KEY = "index_name";
const std::string COMMAND_LABEL_KEY = "command";

const std::string UNDEFINED_LABEL_VALUE = "undefined";

const std::string LABEL_QUOTE_REPLACEMENT = "[q]";
const std::string LABEL_COMMA_REPLACEMENT = "[c]";
const std::string LABEL_OPEN_PARANTHESIS_REPLACEMENT = "[op]";
const std::string LABEL_CLOSED_PARANTHESIS_REPLACEMENT = "[cp]";

#endif //OPT_MODULE_CONSTANTS_H
