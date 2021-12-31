#!/bin/bash
cd $(dirname $0)

read -p "Endpoint of the target blockchain (ex. http://localhost:8888): " NODEOS_HOST
read -p "Name of the contract you implemented: " CONTRACT_NAME
read -p "Name of an account: " ACCOUNT_NAME
read -p "PrivateKey for the account: " PRIVATE_KEY

read -p "Do you want to create the account? (y/n): " ANSWER

if [ "$ANSWER" = "y" ];then
	read -p "PublicKey for the account: " PUBLIC_KEY
fi

CLEOS_CONTAINER_NAME=deploy-cleos
yarn test # to compile

docker rm -f $CLEOS_CONTAINER_NAME
docker run -it -d --name $CLEOS_CONTAINER_NAME \
	--network host \
	-v $PWD/../$CONTRACT_NAME/build/$CONTRACT_NAME:/root/contracts/$CONTRACT_NAME \
	ibct/ledgis:2.0.7

docker exec $CLEOS_CONTAINER_NAME keosd &
sleep 1
docker exec $CLEOS_CONTAINER_NAME cleos wallet create --to-console

docker exec $CLEOS_CONTAINER_NAME cleos wallet import --private-key $PRIVATE_KEY

if [ "$ANSWER" = "y" ];then
	# LED (Import 7 keys)
	for var in {1..7}
	do
		read -p "Input the privateKey of LED account $var: " LED_PRIVATE_KEY
		echo $LED_PRIVATE_KEY
		docker exec $CLEOS_CONTAINER_NAME cleos wallet import --private-key $LED_PRIVATE_KEY
	done
	
	docker exec $CLEOS_CONTAINER_NAME cleos -u $NODEOS_HOST system newaccount --stake-net "10.0000 LED" --stake-cpu "30.0000 LED" --buy-ram "100.0000 LED" led $ACCOUNT_NAME $PUBLIC_KEY $PUBLIC_KEY

	read -p "Input the privatekey of event.ibct account: " EVENT_IBCT_PRIVATE_KEY
	docker exec $CLEOS_CONTAINER_NAME cleos wallet import --private-key $EVENT_IBCT_PRIVATE_KEY
	docker exec $CLEOS_CONTAINER_NAME cleos -u $NODEOS_HOST push action lemonade.c transfer '{"from": "event.ibct", "to": "'$ACCOUNT_NAME'", "quantity": "100.0000 LED", "memo": "TEST"}' -p event.ibct@active
fi

docker exec $CLEOS_CONTAINER_NAME cleos -u $NODEOS_HOST set contract $ACCOUNT_NAME contracts/$CONTRACT_NAME --clear
docker exec $CLEOS_CONTAINER_NAME cleos -u $NODEOS_HOST set contract $ACCOUNT_NAME contracts/$CONTRACT_NAME -p $ACCOUNT_NAME

# rlcmanager.p
# # NEW
# 5JBb5znnmNVfLnmAmDaw42pxFQnXMj8yCaMPSW9JkUw3wfyD68H
# EOS7GgQnAKCVbBahBi3iLq7jmj41xHG5pK9PrWVDF7AJ9yuHEnfSA

# # led
# 5KbPfhLE2e2ZhfnU98vmSrMs8mbu9Xu8QciM2gHrgpYkRAachGN
# 5J2cLHeu3cVMsi9Kmb6zJeonpkkug9tnYufprQRwZQYithEveFa
# 5JL2Y2URis9oHHwCYYPRLVuTvxRB1sTcTpxELHB1M4MfrVjwFQa
# 5J7Bj6xik6fBaYPoP95JSVmZn6PuTfXxzLZ5obyoYBiycwgygTi
# 5Km1S4B4jFiUv81NBinPNMsFJjRn1MumCsdwjoXsU96gDiPY57R
# 5J1AiUcby5FX6p7HFhUE5MUzx5VWeGsECSPgQXsStQSSR78qoTy
# 5HuMBLBbcWo1mm2F7DekiWEyNMmnJjMJkyEzo2eXhFozcqcnhvG

# # event.ibct
# 5JRpAsWWUwTM9dBctgx1XGgCzknMTitWziYXsYx9sEfvdqTF6GX

# # Password
# PW5KAWBQ4PavBccLEvW6QwQKxD7pF5pCFUUwspUYBxasXDiTwj15h