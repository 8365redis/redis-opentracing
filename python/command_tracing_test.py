from redis.commands.json.path import Path
import pytest
from constants import TS_PREFIX, CMD_DELIM, TRACE_EXECUTE_CMD, COMMAND_LABEL_KEY, COMMAND_TYPE_LABEL_KEY, \
    CLIENT_ID_LABEL_KEY, RETENTION_LABEL_KEY, INDEX_NAME_LABEL_KEY, OPENTRACING_STREAM_NAME, OPENTRACING_KEY_NAME, METRIC_TAGS_KEY, \
    INDEX_NAME_LABEL_KEY

from data_prepare import flush_db, extract_labels, TEST_INDEX_NAME, create_index, generate_single_object, TEST_INDEX_PREFIX
from manage_redis import connect_redis_with_start, connect_redis, kill_redis
from utils import escape_ts_label_value
import json

@pytest.fixture(autouse=True)
def before_and_after_test():
    print("Start")
    yield
    kill_redis()
    print("End")


def test_search_ft_search_query():
    producer = connect_redis_with_start()
    flush_db(producer)
    create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = generate_single_object(1000 + i, 2000 + i, "aaa")
        key = TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    query = ("FT.SEARCH " + TEST_INDEX_NAME +
             ' @User\\.PASSPORT:{aaa} RETURN 1 User.ID SORTBY User.ID LIMIT 0 3')
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace

    # CHECK MONITORING STREAM
    from_stream = client.xread( streams={OPENTRACING_STREAM_NAME:0} )
    #print(from_stream)
    json_data = str(from_stream[0][1][0][1][OPENTRACING_KEY_NAME])
    d = json.loads(json_data)
    #print(json_data)
    assert d[COMMAND_LABEL_KEY] == '''FT.SEARCH usersJsonIdx @User\\.PASSPORT:{aaa} RETURN 1 User.ID SORTBY User.ID LIMIT 0 3'''
    assert d[METRIC_TAGS_KEY][CLIENT_ID_LABEL_KEY] == '''test_search_latency_metric_is_added'''
    assert d[METRIC_TAGS_KEY][COMMAND_TYPE_LABEL_KEY] == '''FT.SEARCH'''
    assert d[METRIC_TAGS_KEY][INDEX_NAME_LABEL_KEY] == '''usersJsonIdx'''


def test_search_ft_agg_query():
    producer = connect_redis_with_start()
    flush_db(producer)
    create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = generate_single_object(1000 + i, 2000 + i, "aaa")
        key = TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    query = ("FT.AGGREGATE " + TEST_INDEX_NAME +
             ' @User\\.PASSPORT:{aaa} GROUPBY 2 @User.ID @User.PASSPORT LIMIT 0 3')
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace

    # CHECK MONITORING STREAM
    from_stream = client.xread( streams={OPENTRACING_STREAM_NAME:0} )
    #print(from_stream)
    json_data = str(from_stream[0][1][0][1][OPENTRACING_KEY_NAME])
    d = json.loads(json_data)
    #print(json_data)
    assert d[COMMAND_LABEL_KEY] == '''FT.AGGREGATE usersJsonIdx @User\\.PASSPORT:{aaa} GROUPBY 2 @User.ID @User.PASSPORT LIMIT 0 3'''
    assert d[METRIC_TAGS_KEY][CLIENT_ID_LABEL_KEY] == '''test_search_latency_metric_is_added'''
    assert d[METRIC_TAGS_KEY][COMMAND_TYPE_LABEL_KEY] == '''FT.AGGREGATE'''
    assert d[METRIC_TAGS_KEY][INDEX_NAME_LABEL_KEY] == '''usersJsonIdx'''


def test_search_not_allowed_characters_ft_agg_query():
    producer = connect_redis_with_start()
    flush_db(producer)
    create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = generate_single_object(1000 + i, 2000 + i, "aaa")
        key = TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    command_args = [
        "FT.AGGREGATE",
        TEST_INDEX_NAME,
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

    # CHECK MONITORING STREAM
    from_stream = client.xread( streams={OPENTRACING_STREAM_NAME:0} )
    #print(from_stream)
    json_data = str(from_stream[0][1][0][1][OPENTRACING_KEY_NAME])
    d = json.loads(json_data)
    #print(json_data)
    assert d[COMMAND_LABEL_KEY] == '''FT.AGGREGATE usersJsonIdx (@User\\.PASSPORT:{aaa}) | (@User\\.PASSPORT:{aaa}) GROUPBY 2 @User.ID @User.PASSPORT LIMIT 0 3'''
    assert d[METRIC_TAGS_KEY][CLIENT_ID_LABEL_KEY] == '''test_search_latency_metric_is_added'''
    assert d[METRIC_TAGS_KEY][COMMAND_TYPE_LABEL_KEY] == '''FT.AGGREGATE'''
    assert d[METRIC_TAGS_KEY][INDEX_NAME_LABEL_KEY] == '''usersJsonIdx'''


def test_tagvals_query():
    producer = connect_redis_with_start()
    flush_db(producer)
    create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = generate_single_object(1000 + i, 2000 + i, "aaa")
        key = TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    query = ("FT.TAGVALS " + TEST_INDEX_NAME + ' User.PASSPORT')
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace

    # CHECK MONITORING STREAM
    from_stream = client.xread( streams={OPENTRACING_STREAM_NAME:0} )
    #print(from_stream)
    json_data = str(from_stream[0][1][0][1][OPENTRACING_KEY_NAME])
    d = json.loads(json_data)
    #print(json_data)
    assert d[COMMAND_LABEL_KEY] == '''FT.TAGVALS usersJsonIdx User.PASSPORT'''
    assert d[METRIC_TAGS_KEY][CLIENT_ID_LABEL_KEY] == '''test_search_latency_metric_is_added'''
    assert d[METRIC_TAGS_KEY][COMMAND_TYPE_LABEL_KEY] == '''FT.TAGVALS'''
    assert d[METRIC_TAGS_KEY][INDEX_NAME_LABEL_KEY] == '''usersJsonIdx'''

def test_json_get():
    producer = connect_redis_with_start()
    flush_db(producer)
    create_index(producer)

    d = generate_single_object(1000, 2000, "aaa")
    key = TEST_INDEX_PREFIX + str(1)
    producer.json().set(key, Path.root_path(), d)

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"

    query = f"JSON.GET {key}"
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace

    # CHECK MONITORING STREAM
    from_stream = client.xread( streams={OPENTRACING_STREAM_NAME:0} )
    #print(from_stream)
    json_data = str(from_stream[0][1][0][1][OPENTRACING_KEY_NAME])
    d = json.loads(json_data)
    #print(json_data)
    assert d[COMMAND_LABEL_KEY] == '''JSON.GET users:1'''
    assert d[METRIC_TAGS_KEY][CLIENT_ID_LABEL_KEY] == '''test_search_latency_metric_is_added'''
    assert d[METRIC_TAGS_KEY][COMMAND_TYPE_LABEL_KEY] == '''undefined'''
    assert d[METRIC_TAGS_KEY][INDEX_NAME_LABEL_KEY] == '''undefined'''
