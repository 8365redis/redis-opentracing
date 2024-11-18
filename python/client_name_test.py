from redis.commands.json.path import Path
import pytest
from constants import TS_PREFIX, CMD_DELIM, TRACE_EXECUTE_CMD, COMMAND_LABEL_KEY, COMMAND_TYPE_LABEL_KEY, \
    CLIENT_ID_LABEL_KEY, RETENTION_LABEL_KEY, INDEX_NAME_LABEL_KEY, OPENTRACING_STREAM_NAME, OPENTRACING_KEY_NAME, METRIC_TAGS_KEY, \
    INDEX_NAME_LABEL_KEY

from data_prepare import flush_db, extract_labels, TEST_INDEX_NAME, create_index, generate_single_object, TEST_INDEX_PREFIX
from manage_redis import connect_redis_with_start, connect_redis, kill_redis
from utils import escape_ts_label_value
import json
import time

@pytest.fixture(autouse=True)
def before_and_after_test():
    print("Start")
    yield
    kill_redis()
    print("End")

@pytest.mark.skipif(True ,
                    reason="Need to be run manually after both CCT2 and opentracing module are loaded")
def test_search_ft_search_query_with_client_name():
    producer = connect_redis()
    flush_db(producer)
    create_index(producer)

    total_count = 100

    for i in range(1, total_count):
        d = generate_single_object(1000 + i, 2000 + i, "aaa")
        key = TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    client = connect_redis()

    client_id = "test_search_latency_metric_is_added"

    client.execute_command("CCT2.REGISTER " + client_id + " CLIENTNAME" + client_id)

    query = ("CCT2.FT.SEARCH " + TEST_INDEX_NAME +
             ' @User\\.PASSPORT:{aaa} ')
    response_no_trace = client.execute_command(query)

    # check response should be the same as with no trace
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
    assert response_no_trace == response_with_trace


@pytest.mark.skipif(True ,
                    reason="Need to be run manually after both CCT2 and opentracing module are loaded")
def test_search_ft_search_query_with_client_name_update():
    producer = connect_redis()
    flush_db(producer)
    create_index(producer)

    
    d = generate_single_object(1000 , 2000 , "aaa")
    key = TEST_INDEX_PREFIX + str(1)
    producer.json().set(key, Path.root_path(), d)

    client = connect_redis()

    client_id = "test_search_ft_search_query_with_client_name_update"
    client_id_2 = "test_search_ft_search_query_with_client_name_update_2"

    register_command = "CCT2.REGISTER " + client_id 
    res = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {register_command}')
    assert res == "OK"

    register_command_2 = "CCT2.REGISTER " + client_id_2
    res = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id_2} {CMD_DELIM} {register_command_2}')
    assert res == "OK"

    search_command = "CCT2.FT.SEARCH " + TEST_INDEX_NAME + " @User\\.PASSPORT:{aaa} "
    res = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {search_command}')
    #print(res)

    search_command_2 = "CCT2.FT.SEARCH " + TEST_INDEX_NAME + " @User\\.PASSPORT:{aaa} "
    res = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id_2} {CMD_DELIM} {search_command_2}')
    #print(res)

    #UPDATE DATA
    d = generate_single_object(9999 , 9999, "aaa")
    producer.json().set(key, Path.root_path(), d)

    time.sleep(0.1)

    #CHECK STREAMS
    from_stream = client.xread(streams={client_id:0} )
    #print(from_stream)
    assert '''users:1''' in str(from_stream)

    from_stream = client.xread(streams={client_id_2:0} )
    #print(from_stream)
    assert '''users:1''' in str(from_stream)
