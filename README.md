# redis-opentracing

## Development Environment

### Local

* IDE : [Visual Studio Code with WSL](https://code.visualstudio.com/docs/cpp/config-wsl) or [CLion](https://www.jetbrains.com/clion/)
* g++ 11.4
* WSL : 5.15.90.1-microsoft-standard-WSL2 (Ubuntu Ubuntu 22.04.2 LTS)
* Redis server v=Redis 7.2.1 (Latest Stable Package)
* Redis Modules :
    * [Redis Search] (https://github.com/RediSearch/RediSearch)
    * [Redis JSON] (https://github.com/RedisJSON/RedisJSON)

### Containerized
* [Docker](https://www.docker.com/products/docker-desktop/) or [Podman](https://podman.io/docs/installation)

## Building

### Local

After preparing the build environment just run :

```
make
```

Or if you want the Debug version which has lots of logs to standard output (don't run heavy stress tests) or to run the tests you can use:

```
make DEBUG=1
```

#### Load

To load the development module you can use these commands:

```
redis-server --loadmodule ./bin/cct-view.so
```

or

```
make load
```

### Containerized

#### Docker
After preparing the build environment just run :

```
docker-compose build
```

#### Load

To load the development module you can use these commands:

```
docker-compose up
```

#### Podman
After preparing the build environment just run :

```
podman-compose build
```

## Load

To load the development module you can use these commands:

```
podman-compose up
```

## Debugging
* Install gdbserver (if local development)
* Load the module in debug mode (make DEBUG=1). On containerized build this is done as default
* Find pid of Redis (done on container if containerized development)
  ```
  pgrep -l redis-server
  ```
* Attach gdb server (done on container if containerized development)
  ```
  gdbserver :1234 --attach {pid}
  ```
* Create remote debug run configuration on localhost:1234 in the IDE
* Run debug configuration

## Test
Development is done by `Python 3.10.2` but any version after `3.6.X` should be fine. To install needed python packages just run (generated by `pipreqs`):

```
pip install -r python/requirements.txt
```

To run a specific test file you can run (example):

```
pytest -rP python/command_tracing_tests.py
```

or to run a specific test case you can run (example):

```
pytest -rP python/command_tracing_tests.py::test_trace_ft_search_simple
```

## API Contract

```
TRACE.EXECUTE [client_id] CMD [**cmdargs]
```

The module would create stream key in this format:
```
OPENTRACING
```
with values:
```
[
  ['OPENTRACING', [
    ('1728652869653-0', {
      'TRACING': '{
        "command":"JSON.GET users:1",
        "metric":{
          "name":"latency",
          "value":17416.0,
          "value_type":"ns"
        },
        "module":"tracing",
        "tags":{
          "client_id":"test_search_latency_metric_is_added,
          "command_type":"undefined",
          "index_name":"undefined"
          
        },
        "timestamp":1728652869653,
        "version":"1.0.0"
      }'
    }
    )]
  ]
]
```

## Configuration
Module configuration file is by default is in the same directory with binary. You can also set a specific directory or name for the configuration file with passing configuration file as a parameter to module (full path).
Example format of the configuration :
```
[redis-opentracing]
IS_EXTRACT_FT_DATA = true | if true, the module will also add command_type and index_name labels
MONITORING_RETENTION_HOURS = 24
```