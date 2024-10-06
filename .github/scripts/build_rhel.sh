#!/bin/bash

set -e  # trigger failure on error - do not remove!
set -x  # display command on output

# Build the Python package with Poetry
poetry build -f sdist

USE_SYSTEM_DEPS="ON"

USE_TEST_LOGURU="ON"

docker build --progress=plain \
             --build-arg USE_SYSTEM_DEPS="$USE_SYSTEM_DEPS" \
             --build-arg USE_TEST_LOGURU="$USE_TEST_LOGURU" \
             -f - . <<EOF
    # syntax=docker/dockerfile:1

    FROM quay.io/centos/centos:stream9

    RUN dnf config-manager --set-enabled crb
    
    RUN dnf copr -y enable cheimes/deepsearch-glm rhel-9-x86_64
    
    RUN dnf install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm \
        && dnf clean all
    
    RUN dnf install -y --nodocs \
            autoconf automake binutils cmake gcc gcc-c++ git glibc-devel glibc-headers glibc-static kernel-devel libtool libstdc++-devel make ninja-build pkgconfig zlib-devel \
            python3.11 python3.11-pip python3.11-devel \
            cxxopts-devel fasttext-devel fmt-devel json-schema-validator-devel pcre2-devel sentencepiece-devel utf8cpp-devel \
        && dnf clean all

    ## USE_TEST_LOGURU

    # RUN dns install -y --nodocs \
    #         loguru-devel \
    #     && dnf clean all

    RUN dnf install -y --nodocs \
            gperftools gperftools-devel gperftools-libs protobuf protobuf-c protobuf-lite protobuf-lite-devel \
            && dnf clean all

    ARG USE_TEST_LOGURU

    RUN if [ "\$USE_TEST_LOGURU" = "ON" ]; then \
        git clone https://github.com/emilk/loguru.git /loguru \
        && cd /loguru \
        && mkdir build && cd build \
        && cmake .. \
        && make \
        && make install \
        && cp /loguru/loguru.cpp /usr/local/include/loguru/ \
        && ls /usr/local/include/loguru/; \
    fi

    RUN ldconfig -p | grep pthread

    ## USE_TEST_LOGURU

    RUN mkdir /src

    COPY ./dist/*.tar.gz /src/

    ARG USE_SYSTEM_DEPS

    RUN USE_SYSTEM_DEPS=\$USE_SYSTEM_DEPS pip3.11 install /src/deepsearch_glm*.tar.gz

    COPY ./tests /src/tests
    
    RUN cd /src \
        && pip3.11 install pytest \
        && pytest ./tests/test_structs.py -v \
        && pytest ./tests/test_nlp.py -v \
        && pytest ./tests/test_glm.py -v
EOF
