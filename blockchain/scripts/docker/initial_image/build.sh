#!/bin/bash

cd $(dirname $0)
source ../../common/constants.sh

docker rm -f $NODEOS_CONTAINER_NAME

docker image rm -f $BUILD_INITIAL_IMAGE_FULL
# docker image rm -f $PRODUCTION_INITIAL_IMAGE_FULL
docker build -t $BUILD_INITIAL_IMAGE_FULL .
# docker build -t $PRODUCTION_INITIAL_IMAGE_FULL .