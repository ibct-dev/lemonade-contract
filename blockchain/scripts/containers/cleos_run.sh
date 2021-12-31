#!/bin/bash

cd $(dirname $0)
source ../common/constants.sh

## run cleos containter
docker rm -f $CLEOS_CONTAINER_NAME
docker run -it -d --name $CLEOS_CONTAINER_NAME \
    --link $NODEOS_CONTAINER_NAME \
    -v "$(pwd)"/../../contracts/:/root/contracts \
    $PRODUCTION_INITIAL_IMAGE_FULL
## ===

docker exec $CLEOS_CONTAINER_NAME keosd &
sleep 1
docker exec $CLEOS_CONTAINER_NAME cleos wallet create --file wallet_password
docker exec $CLEOS_CONTAINER_NAME cat wallet_password & echo -e "\n"