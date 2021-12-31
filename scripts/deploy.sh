cd $(dirname $0)
source constant.sh

read -p "배포할 블록체인의 주소 ( ex. localhost:8888 ) > " NODEOS_HOST
read -p "계정이름 > " ACCOUNT_NAME
read -p "개인키 > " PRIVATE_KEY

CLEOS_CONTAINER_NAME=deploy-cleos

docker rm -f $CLEOS_CONTAINER_NAME

docker run -it -d --name $CLEOS_CONTAINER_NAME \
    --network host \
    -v $PWD/../$CONTRACT_NAME/build/$CONTRACT_NAME:/root/contracts/$CONTRACT_NAME \
    ibct/ledgis:2.0.7

docker exec $CLEOS_CONTAINER_NAME keosd &
sleep 1
docker exec $CLEOS_CONTAINER_NAME cleos wallet create --to-console
docker exec $CLEOS_CONTAINER_NAME cleos wallet import --private-key $PRIVATE_KEY
docker exec $CLEOS_CONTAINER_NAME cleos -u http://$NODEOS_HOST:$NODEOS_PORT set contract $ACCOUNT_NAME contracts/$CONTRACT_NAME -p $ACCOUNT_NAME
