FROM gcc:9 as builder

RUN apt-get update && apt-get install -y \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/app

COPY . .

RUN make DEBUG=1

FROM redis/redis-stack:latest

RUN apt-get update && apt-get install -y  \
    gdbserver \
    gdb  \
    make  \
    g++ \
    cmake && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/src/app/bin/redisopentracing.so /usr/local/lib/redis/modules/redisopentracing.so
COPY --from=builder /usr/src/app/cfg/redis-opentracing-config.ini /usr/local/lib/redis/cfg/redis-opentracing-config.ini
COPY --from=builder /usr/src/app/cfg/redis.conf /usr/local/lib/redis/cfg/redis.conf
COPY --from=builder /usr/src/app/src /usr/src/app/src
COPY --from=builder /usr/src/app/include /usr/src/app/include

EXPOSE 6379 1234

CMD ["redis-stack-server", "/usr/local/lib/redis/cfg/redis.conf"]
