#!/bin/bash

cd $(dirname $0)
source constants.sh

./cleos.sh wallet unlock --password $(docker exec $CLEOS_CONTAINER_NAME cat wallet_password) 