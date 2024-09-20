from redis.commands.json.path import Path

from python import data_prepare
from python.constants import TS_PREFIX, CMD_DELIM, TRACE_EXECUTE_CMD, COMMAND_LABEL_KEY, COMMAND_TYPE_LABEL_KEY, \
    CLIENT_ID_LABEL_KEY, RETENTION_LABEL_KEY, INDEX_NAME_LABEL_KEY
from python.data_prepare import flush_db, extract_labels, TEST_INDEX_NAME
from python.manage_redis import connect_redis_with_start, connect_redis
from python.utils import escape_ts_label_value


def test_search_ft_search_query():
    producer = connect_redis_with_start()
    flush_db(producer)
    data_prepare.create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = data_prepare.generate_single_object(1000 + i, 2000 + i, "aaa")
        key = data_prepare.TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    query = ("FT.SEARCH " + data_prepare.TEST_INDEX_NAME +
             ' @User\\.PASSPORT:{aaa} RETURN 1 User.ID SORTBY User.ID LIMIT 0 3')
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace

    # check ts
    metric_key = f'{TS_PREFIX}:{client_id}:{query}'
    last_value = client.execute_command('TS.GET', metric_key)
    assert last_value

    info_response = client.execute_command("TS.INFO", metric_key)
    labels = extract_labels(info_response)

    # check expected labels
    assert labels[CLIENT_ID_LABEL_KEY] == client_id
    assert labels[COMMAND_TYPE_LABEL_KEY] == "FT.SEARCH"
    assert labels[COMMAND_LABEL_KEY] == query
    assert labels[INDEX_NAME_LABEL_KEY] == TEST_INDEX_NAME


def test_search_ft_agg_query():
    producer = connect_redis_with_start()
    flush_db(producer)
    data_prepare.create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = data_prepare.generate_single_object(1000 + i, 2000 + i, "aaa")
        key = data_prepare.TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    query = ("FT.AGGREGATE " + data_prepare.TEST_INDEX_NAME +
             ' @User\\.PASSPORT:{aaa} GROUPBY 2 @User.ID @User.PASSPORT LIMIT 0 3')
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace

    # check ts
    metric_key = f'{TS_PREFIX}:{client_id}:{query}'
    last_value = client.execute_command('TS.GET', metric_key)
    assert last_value

    info_response = client.execute_command("TS.INFO", metric_key)
    labels = extract_labels(info_response)

    # check expected labels
    assert labels[CLIENT_ID_LABEL_KEY] == client_id
    assert labels[COMMAND_TYPE_LABEL_KEY] == "FT.AGGREGATE"
    assert labels[COMMAND_LABEL_KEY] == query
    assert labels[INDEX_NAME_LABEL_KEY] == TEST_INDEX_NAME


def test_search_not_allowed_characters_ft_agg_query():
    producer = connect_redis_with_start()
    flush_db(producer)
    data_prepare.create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = data_prepare.generate_single_object(1000 + i, 2000 + i, "aaa")
        key = data_prepare.TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    command_args = [
        "FT.AGGREGATE",
        data_prepare.TEST_INDEX_NAME,
        '(@User\\.PASSPORT:{aaa}) | (@User\\.PASSPORT:{aaa})',
        "GROUPBY",
        "2",
        "@User.ID",
        "@User.PASSPORT",
        "LIMIT",
        "0",
        "3"
    ]

    arguments_string = ' '.join(map(str, command_args))

    response_no_trace = client.execute_command(*command_args)

    # check response should be the same as with no trace
    command_args_trace = [
                             TRACE_EXECUTE_CMD,
                             client_id,
                             CMD_DELIM,
                         ] + command_args

    response_with_trace = client.execute_command(*command_args_trace)
    assert response_no_trace == response_with_trace

    # check ts
    metric_key = f'{TS_PREFIX}:{client_id}:{arguments_string}'
    last_value = client.execute_command('TS.GET', metric_key)
    assert last_value

    info_response = client.execute_command("TS.INFO", metric_key)
    labels = extract_labels(info_response)

    # check expected labels
    assert labels[CLIENT_ID_LABEL_KEY] == client_id
    assert labels[COMMAND_TYPE_LABEL_KEY] == "FT.AGGREGATE"
    assert labels[COMMAND_LABEL_KEY] == escape_ts_label_value(arguments_string)
    assert labels[INDEX_NAME_LABEL_KEY] == TEST_INDEX_NAME


def test_tagvals_query():
    producer = connect_redis_with_start()
    flush_db(producer)
    data_prepare.create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = data_prepare.generate_single_object(1000 + i, 2000 + i, "aaa")
        key = data_prepare.TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    query = ("FT.TAGVALS " + data_prepare.TEST_INDEX_NAME + ' User.PASSPORT')
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace

    # check ts
    metric_key = f'{TS_PREFIX}:{client_id}:{query}'
    last_value = client.execute_command('TS.GET', metric_key)
    assert last_value

    info_response = client.execute_command("TS.INFO", metric_key)
    labels = extract_labels(info_response)

    # check expected labels
    assert labels[CLIENT_ID_LABEL_KEY] == client_id
    assert labels[COMMAND_TYPE_LABEL_KEY] == "FT.TAGVALS"
    assert labels[COMMAND_LABEL_KEY] == query
    assert labels[INDEX_NAME_LABEL_KEY] == TEST_INDEX_NAME


def test_json_get():
    producer = connect_redis_with_start()
    flush_db(producer)
    data_prepare.create_index(producer)

    d = data_prepare.generate_single_object(1000, 2000, "aaa")
    key = data_prepare.TEST_INDEX_PREFIX + str(1)
    producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    query = f"JSON.GET {key}"
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace

    # check ts
    metric_key = f'{TS_PREFIX}:{client_id}:{query}'
    last_value = client.execute_command('TS.GET', metric_key)
    assert last_value

    info_response = client.execute_command("TS.INFO", metric_key)
    labels = extract_labels(info_response)

    # check expected labels
    assert labels[CLIENT_ID_LABEL_KEY] == client_id
    assert labels[COMMAND_TYPE_LABEL_KEY] == "undefined"
    assert labels[COMMAND_LABEL_KEY] == query
    assert labels[INDEX_NAME_LABEL_KEY] == "undefined"
