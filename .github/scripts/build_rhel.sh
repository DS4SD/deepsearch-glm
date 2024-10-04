#!/bin/bash

set -e  # trigger failure on error - do not remove!
set -x  # display command on output

# Build the Python package with Poetry
poetry build -f sdist

# Docker build
docker build -f - . <<EOF
    FROM quay.io/centos/centos:stream9
    RUN dnf config-manager --set-enabled crb
    RUN dnf copr -y enable cheimes/deepsearch-glm rhel-9-x86_64
    RUN dnf install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm \
        && dnf clean all
    RUN dnf install -y --nodocs \
            gcc gcc-c++ git make cmake pkgconfig ninja-build glibc-devel \
            python3.11 python3.11-pip python3.11-devel \
            libjpeg-turbo-devel libpng-devel zlib-devel \
            cxxopts-devel fasttext-devel fmt-devel json-schema-validator-devel \
            loguru-devel sentencepiece-devel pcre2-devel utf8cpp-devel \
        && dnf clean all

    RUN mkdir /src
    COPY ./dist/*.tar.gz /src/

    RUN USE_SYSTEM_DEPS=ON pip3.11 install /src/deepsearch_glm*.tar.gz

    COPY ./tests /src/tests
    RUN cd /src \
        && pip3.11 install pytest \
        && pytest ./tests/test_structs.py -v \
        && pytest ./tests/test_nlp.py -v \
        && pytest ./tests/test_glm.py -v \
        || { echo "Tests failed"; exit 1; }
EOF
