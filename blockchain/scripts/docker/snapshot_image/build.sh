#!/bin/bash

cd $(dirname $0)
source ../../common/constants.sh

echo "start make snapshot"

if [ -d "./data" ]; then
    echo "snapshot already exist"
    rm -rf ./data
    rm -rf ./snapshot.name
fi
DOCKER_INITIAL_IMAGE_CHECK=$(docker images | grep $BUILD_INITIAL_IMAGE_NAME | grep $BUILD_INITIAL_IMAGE_VERSION)
if [ "$DOCKER_INITIAL_IMAGE_CHECK" == "" ]; then
    echo "$IMAGE_NAME is not found"
    echo "you have to build initial docker image"
    ../intial_image/build.sh
fi
mkdir data
docker rm -f $NODEOS_CONTAINER_NAME
docker run -it -d --name $NODEOS_CONTAINER_NAME \
    -v $(pwd)/data:/data \
    -p 8888:8888 -p 8080:8080 -p 9876:9876 \
    $BUILD_INITIAL_IMAGE_FULL \
	# $PRODUCTION_INITIAL_IMAGE_FULL \
    nodeos --genesis-json /etc/nodeos/genesis.json --data-dir /data --config-dir /etc/nodeos --disable-replay-opts

../../contract/boot.sh # boostraping system contract 

curl -X POST http://localhost:$NODEOS_PORT/v1/producer/create_snapshot # make snapshot

docker rm -f $NODEOS_CONTAINER_NAME

rm -rf data/blocks/reversible
rm -rf data/blocks/blocks.index
rm -rf data/state
rm -rf data/state-history
ls data/snapshots > ./snapshot.name

docker image rm $BUILD_SNAPSHOT_IMAGE_FULL
docker build -t $BUILD_SNAPSHOT_IMAGE_FULL -f snapshot.dockerfile .



