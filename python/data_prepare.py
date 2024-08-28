from redis.commands.json.path import Path
import random
import string
from constants import RETENTION_LABEL_KEY, COMMAND_LABEL_KEY, COMMAND_TYPE_LABEL_KEY, INDEX_NAME_LABEL_KEY, \
    CLIENT_ID_LABEL_KEY

from redis.commands.search.field import TagField
from redis.commands.search.indexDefinition import IndexDefinition, IndexType

TEST_INDEX_NAME = "usersJsonIdx"
TEST_INDEX_PREFIX = "users:"


def create_index(r):
    schema = (TagField("$.User.ID", as_name="User.ID"), TagField("$.User.PASSPORT", as_name="User.PASSPORT"),
              TagField("$.User.Address.ID", as_name="User.Address.ID"))
    r.ft(TEST_INDEX_NAME).create_index(schema,
                                       definition=IndexDefinition(prefix=[TEST_INDEX_PREFIX],
                                                                  index_type=IndexType.JSON))


def flush_db(r):
    r.flushall()


def generate_object(d, id, addr_id):
    d["User"] = {}
    d["User"]["ID"] = str(id)
    first = "".join(random.choices(string.ascii_uppercase + string.digits, k=3))
    second = "".join(random.choices(string.ascii_uppercase + string.digits, k=2))
    third = "".join(random.choices(string.ascii_uppercase + string.digits, k=2))
    d["User"]["PASSPORT"] = first + "-" + second + "-" + third
    d["User"]["Address"] = {}
    d["User"]["Address"]["ID"] = str(addr_id)


def generate_single_object(user_id, addr_id, passport):
    d = {"User": {}}
    d["User"]["ID"] = str(user_id)
    d["User"]["PASSPORT"] = passport
    d["User"]["Address"] = {}
    d["User"]["Address"]["ID"] = str(addr_id)
    return d


def extract_labels(ts_info_response):
    resp = {}
    label_data = ts_info_response[ts_info_response.index('labels') + 1]
    for label_kv in label_data:
        if label_kv[0] == CLIENT_ID_LABEL_KEY:
            resp[CLIENT_ID_LABEL_KEY] = label_kv[1]
        elif label_kv[0] == COMMAND_TYPE_LABEL_KEY:
            resp[COMMAND_TYPE_LABEL_KEY] = label_kv[1]
        elif label_kv[0] == INDEX_NAME_LABEL_KEY:
            resp[INDEX_NAME_LABEL_KEY] = label_kv[1]
        elif label_kv[0] == COMMAND_LABEL_KEY:
            resp[COMMAND_LABEL_KEY] = label_kv[1]
        elif label_kv[0] == RETENTION_LABEL_KEY:
            resp[RETENTION_LABEL_KEY] = label_kv[1]
    return resp
