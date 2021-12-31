
## intial docker image
BUILD_INITIAL_IMAGE_NAME="ledgis"
BUILD_INITIAL_IMAGE_VERSION="2.0.7"
BUILD_INITIAL_IMAGE_FULL="$BUILD_INITIAL_IMAGE_NAME:$BUILD_INITIAL_IMAGE_VERSION"
PRODUCTION_INITIAL_IMAGE_NAME="ibct/ledgis"
PRODUCTION_INITIAL_IMAGE_VERSION="2.0.7"
PRODUCTION_INITIAL_IMAGE_FULL="$PRODUCTION_INITIAL_IMAGE_NAME:$PRODUCTION_INITIAL_IMAGE_VERSION"

## snapshot docker image
BUILD_SNAPSHOT_IMAGE_NAME="ledgis"
BUILD_SNAPSHOT_IMAGE_VERSION="2.0.7-snapshot"
BUILD_SNAPSHOT_IMAGE_FULL="$BUILD_SNAPSHOT_IMAGE_NAME:$BUILD_SNAPSHOT_IMAGE_VERSION"
PRODUCTION_SNAPSHOT_IMAGE_NAME="ibct/ledgis"
PRODUCTION_SNAPSHOT_IMAGE_VERSION="2.0.7-snapshot"
PRODUCTION_SNAPSHOT_IMAGE_FULL="$PRODUCTION_SNAPSHOT_IMAGE_NAME:$PRODUCTION_SNAPSHOT_IMAGE_VERSION"

## container name
CONTAINER_NAME=ledgis
NODEOS_CONTAINER_NAME=$CONTAINER_NAME-nodeos
CLEOS_CONTAINER_NAME=$CONTAINER_NAME-cleos

## cleos.sh and boot.sh 설정
NODEOS_HOST=$NODEOS_CONTAINER_NAME
NODEOS_PORT=8888
WALLET_HOST=$CLEOS_CONTAINER_NAME
WALLET_PORT=7777
WALLET_DIR="/wallet"
