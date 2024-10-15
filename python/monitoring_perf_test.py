from manage_redis import kill_redis, connect_redis_with_start, connect_redis
from data_prepare import flush_db, create_index, TEST_INDEX_NAME, TEST_INDEX_PREFIX, generate_single_object
from constants import TRACE_EXECUTE_CMD, CMD_DELIM, OPENTRACING_STREAM_NAME
import pytest
from redis.commands.json.path import Path
from redis.commands.search.query import GeoFilter, NumericFilter, Query
from redis.commands.search.result import Result
import time
import json
import numpy as np


def test_monitoring_latency():
    producer = connect_redis_with_start()
    flush_db(producer) # clean all db first
    create_index(producer)
    time.sleep(5)

    producer.execute_command("FT.CONFIG SET MAXSEARCHRESULTS 1000000")
    producer.execute_command("FT.CONFIG SET MAXAGGREGATERESULTS 1000000")

    print("Test starts")
    total = 10000
    #ADD INITIAL DATA
    for i in range(total):
        passport = "aaa"
        if i % 3 == 0 :
            passport = "bbb"
        elif i % 3 == 1 :
            passport = "ccc"
        d = generate_single_object(100000 + i , 2000000 - i, passport)
        key = TEST_INDEX_PREFIX + str(i)
        producer.json().set(key, Path.root_path(), d)

    print("Adding data finished")

    time.sleep(5)

    latency_values = []
    query_cnt = 200
    total_query = 0

    client = connect_redis()
    client_id = "test_search_latency_metric_is_added"
    offset = 0
    num = 10


    for _ in range(query_cnt):
        query = ("FT.SEARCH " + TEST_INDEX_NAME + ' @User\\.PASSPORT:{aaa} RETURN 1 User.ID SORTBY User.ID LIMIT ' + str(offset) + ' ' + str(num))
        start_set_time = time.perf_counter_ns()
        client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {query}')
        end_set_time = time.perf_counter_ns()
        latency_values.append(end_set_time-start_set_time)
        total_query += 1


    time.sleep(3)


    max_value = np.max(latency_values)
    min_value = np.min(latency_values)
    mean_value = np.mean(latency_values)  # Mean and average are the same in this context
    average_value = np.mean(latency_values)
    p50 = np.percentile(latency_values, 50)  # 50th percentile (median)
    p99 = np.percentile(latency_values, 99)  # 99th percentile
    p999 = np.percentile(latency_values, 99.9)  # 99.9th percentile

    # Print the results
    print(f"Total key count: {total}")
    print(f"Total query: {total_query}")
    print(f"Total entry in stream: {len(latency_values)}")
    print(f"Max: {max_value}")
    print(f"Min: {min_value}")
    print(f"Mean: {mean_value}")
    print(f"Average: {average_value}")
    print(f"P50 (Median): {p50}")
    print(f"P99: {p99}")
    print(f"P99.9: {p999}")

    print(client.execute_command("INFO latencystats"))


    '''
    time.sleep(2)
    
    latency_values.clear()

    for _ in range(query_cnt):
        query = ("FT.SEARCH " + TEST_INDEX_NAME + ' @User\\.PASSPORT:{aaa} RETURN 1 User.ID SORTBY User.ID LIMIT ' + str(offset) + ' ' + str(num))
        start_set_time = time.perf_counter_ns()
        client.execute_command(query)
        end_set_time = time.perf_counter_ns()
        latency_values.append(end_set_time-start_set_time)
        total_query += 1

    max_value = np.max(latency_values)
    min_value = np.min(latency_values)
    mean_value = np.mean(latency_values)  # Mean and average are the same in this context
    average_value = np.mean(latency_values)
    p50 = np.percentile(latency_values, 50)  # 50th percentile (median)
    p99 = np.percentile(latency_values, 99)  # 99th percentile
    p999 = np.percentile(latency_values, 99.9)  # 99.9th percentile

    # Print the results
    print(f"Total key count: {total}")
    print(f"Total query: {total_query}")
    print(f"Total entry in stream: {len(latency_values)}")
    print(f"Max: {max_value}")
    print(f"Min: {min_value}")
    print(f"Mean: {mean_value}")
    print(f"Average: {average_value}")
    print(f"P50 (Median): {p50}")
    print(f"P99: {p99}")
    print(f"P99.9: {p999}")
    '''
    
    kill_redis()


if __name__=="__main__":
    test_monitoring_latency()