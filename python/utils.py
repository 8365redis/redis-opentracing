import re

LABEL_QUOTE_REPLACEMENT = "[q]"
LABEL_COMMA_REPLACEMENT = "[c]"
LABEL_OPEN_PARANTHESIS_REPLACEMENT = "[op]"
LABEL_CLOSED_PARANTHESIS_REPLACEMENT = "[cp]"


def escape_ts_label_value(label_value):
    if not label_value:
        return "\"\""

    label_value = re.sub(r'"', LABEL_QUOTE_REPLACEMENT, label_value)
    label_value = re.sub(r'\(', LABEL_OPEN_PARANTHESIS_REPLACEMENT, label_value)
    label_value = re.sub(r'\)', LABEL_CLOSED_PARANTHESIS_REPLACEMENT, label_value)
    label_value = re.sub(r',', LABEL_COMMA_REPLACEMENT, label_value)

    return label_value
