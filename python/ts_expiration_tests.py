import time

import pytest

from python.constants import CMD_DELIM, TRACE_EXECUTE_CMD, TS_PREFIX
from python.data_prepare import flush_db
from python.manage_redis import connect_redis_with_start, kill_redis


@pytest.fixture(autouse=True)
def before_and_after_test():
    print("Start")
    yield
    kill_redis()
    print("End")


def test_ts_key_should_have_and_refresh_ttl():
    print("entered")
    client = connect_redis_with_start()
    flush_db(client)
    client_id = "ts_key_should_have_and_refresh_ttl"
    command = "PING"
    response_with_trace = client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {command}')
    assert response_with_trace == "PONG"

    metric_key = f'{TS_PREFIX}:{client_id}:{command}'
    ttl = client.execute_command('TTL', metric_key)
    assert int(ttl) > 0

    time.sleep(1)

    new_ttl = client.execute_command('TTL', metric_key)
    assert int(new_ttl) < int(ttl)

    client.execute_command(f'{TRACE_EXECUTE_CMD} {client_id} {CMD_DELIM} {command}')
    new_ttl = client.execute_command('TTL', metric_key)
    assert int(new_ttl) >= int(ttl)
