#!/bin/bash

cd $(dirname $0)
source ../../common/constants.sh
filename=$(ls data/snapshots)
docker rm -f $NODEOS_CONTAINER_NAME
docker run -it -d --name $NODEOS_CONTAINER_NAME \
    -p 8888:8888 -p 8080:8080 -p 9876:9876 \
    ledgis:2.0.7-snapshot \
    nodeos --snapshots $filename.bin --genesis-json /etc/nodeos/genesis.json --data-dir /data --config-dir /etc/nodeos --disable-replay-opts --replay-blockchain
