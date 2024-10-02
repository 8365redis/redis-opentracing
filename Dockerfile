FROM rockylinux:8.5 as builder

RUN yum check-update || true && \
    yum install -y make \
                   gcc \
                   epel-release \
                   yum-utils

RUN crb enable

RUN yum install -y libmpc-devel
RUN dnf group install -y "Development Tools"

WORKDIR /usr/src/app

COPY . .

RUN make DEBUG=1

FROM redis/redis-stack:latest

RUN apt-get update && apt-get install -y  \
    gdbserver \
    gdb  \
    gcc \
    make  \
    clang \
    clang-tools \
    cmake

COPY --from=builder /usr/src/app/bin/redisopentracing.so /usr/local/lib/redis/modules/redisopentracing.so
COPY --from=builder /usr/src/app/cfg/redis-opentracing-config.ini /usr/local/lib/redis/cfg/redis-opentracing-config.ini
COPY --from=builder /usr/src/app/cfg/redis.conf /usr/local/lib/redis/cfg/redis.conf
COPY --from=builder /usr/src/app/src /usr/src/app/src
COPY --from=builder /usr/src/app/include /usr/src/app/include

EXPOSE 6379 1234

CMD ["redis-stack-server", "/usr/local/lib/redis/cfg/redis.conf"]
