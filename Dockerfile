FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
    build-essential \
    clang \
    valgrind \
    gdb \
    make \
    git \
    nano \
    vim \
    python3 \
    python3-pip

WORKDIR /app

CMD ["/bin/bash"]
