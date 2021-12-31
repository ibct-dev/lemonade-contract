#!/bin/bash

source constants.sh

docker exec -it $CLEOS_CONTAINER_NAME cleos -u http://$NODEOS_HOST:$NODEOS_PORT "$@"
