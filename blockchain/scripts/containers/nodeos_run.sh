#!/bin/bash

cd $(dirname $0)
source ../common/constants.sh

function usage() {
    printf "Usage: $0 OPTION...
    -i          custom initial nodeos image
    -s          custom snapshot nodeos image (NAME is Snapshots File Name )
    -S          public snapshot nodeos image
    \\n" "$0" 1>&2
    exit 1
}

[ $# -ne 1 ] && usage

CONTAINER_EXISTS=$(docker rm -f $NODEOS_CONTAINER_NAME) 
echo $CONTAINER_EXISTS
if [ "$CONTAINER_EXISTS" != "" ]; then
    CHECK_PORT=$(netstat -anv | grep -E "\b8888\b|\b8080\b|\b9876\b")
    while [ "$CHECK_PORT" != "" ]; do
        echo "Wating until previous docker container is completely removed"
        sleep 1
        CHECK_PORT=$(netstat -anv | grep -E "\b8888\b|\b8080\b|\b9876\b")
    done
fi

if [ "$CHECK_PORT" != "" ]; then
    echo "*** WARNING: already use port 8888 or 8080 or 9876 ***"
fi

while getopts "isS" opt; do
    case $opt in
    i)
            DOCKER_IMAGE_CHECK=$(docker images | grep $BUILD_INITIAL_IMAGE_NAME | grep $BUILD_INITIAL_IMAGE_VERSION)
            if [ "$DOCKER_IMAGE_CHECK" == "" ]; then
                echo "$IMAGE_NAME is not found"
                echo "start to build docker image"
                ../docker/intial_image/build.sh
            fi
            IMAGE_NAME=$BUILD_INITIAL_IMAGE_FULL
			# IMAGE_NAME=$PRODUCTION_INITIAL_IMAGE_FULL
            COMMAND="nodeos --genesis-json=/etc/nodeos/genesis.json --data-dir=/data --config-dir=/etc/nodeos --disable-replay-opts"
        ;;
    s)
            DOCKER_IMAGE_CHECK=$(docker images | grep -E "\b$BUILD_SNAPSHOT_IMAGE_NAME\b" | grep -E "\b$BUILD_SNAPSHOT_IMAGE_VERSION\b")
            IMAGE_NAME=$BUILD_SNAPSHOT_IMAGE_FULL
            echo "$DOCKER_IMAGE_CHECK"
            echo "$IMAGE_NAME"
            if [ "$DOCKER_IMAGE_CHECK" == "" ]; then
                echo "$IMAGE_NAME is not found"
                echo "start to build docker image..."
                ../docker/snapshot_image/build.sh
                echo "Retry this script with snapshot name"
            fi
            COMMAND="nodeos --snapshots $(cat ../docker/snapshot_image/snapshot.name) --genesis-json /etc/nodeos/genesis.json --data-dir /data --config-dir /etc/nodeos --disable-replay-opts --replay-blockchain"
            echo $COMMAND
        ;;
    S)
            IMAGE_NAME=$PRODUCTION_SNAPSHOT_IMAGE_FULL
            COMMAND="nodeos -e -p eosio --plugin eosio::chain_api_plugin --plugin eosio::history_api_plugin --snapshots snapshot-0000004364cf0f57feb48b41c5a4d4d96a92668adb518d4f2fd482dfb94f64ef.bin --genesis-json /etc/nodeos/genesis.json --data-dir /data --config-dir /etc/nodeos --disable-replay-opts --replay-blockchain"
        ;;
    ?)
        echo "Invalid Option!" 1>&2
        usage
        ;;
    :)
        echo "Invalid Option: -${OPTARG} requires an argument." 1>&2
        usage
        ;;
    *)
        usage
        ;;
    esac
done


docker run -it -d --name $NODEOS_CONTAINER_NAME \
    -p 8888:8888 -p 8080:8080 -p 9876:9876 \
    $IMAGE_NAME \
    $COMMAND