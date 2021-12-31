#!/bin/bash

cd $(dirname $0)
source ../common/constants.sh
../containers/cleos_run.sh
cd ../common

./import_keys.sh

## create system accounts
SYSTEM_ACCOUNTS=(lemonade.c led.ram led.ramfee led.stake led.msig led.bios)
for sa in "${SYSTEM_ACCOUNTS[@]}"
do
    ./cleos.sh create account led $sa EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S -p led
done
## ===

## setting service accounts
./cleos.sh create account led p EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S -p led
./cleos.sh set account permission p service EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S
./cleos.sh set action permission p lemonade.c transfer service
./cleos.sh set action permission p led newaccount service
./cleos.sh set action permission p led buyram service
./cleos.sh set action permission p led delegatebw service

./cleos.sh create account led c EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S -p led
./cleos.sh set account permission c service EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S
./cleos.sh set action permission c lemonade.c transfer service
./cleos.sh set action permission c led newaccount service
./cleos.sh set action permission c led buyram service
./cleos.sh set action permission c led delegatebw service
## ===

## deploy system contracts
curl -X POST http://localhost:$NODEOS_PORT/v1/producer/schedule_protocol_feature_activations -d '{"protocol_features_to_activate": ["0ec7e080177b2c02b278d5088611686b49d739925a92d9bfcacd7fc6b74053bd"]}'
sleep 1

./cleos.sh set contract lemonade.c contracts/lemonade.c -p lemonade.c
./cleos.sh set contract led.msig contracts/led.msig -p led.msig
./cleos.sh set contract led contracts/led.bios -p led
./cleos.sh set contract led contracts/led.system -p led
## ===

## setting contracts
./cleos.sh push action lemonade.c create '["led", "0.0000 LED"]' -p lemonade.c
./cleos.sh push action lemonade.c issue '["led", "1000000000.0000 LED", "init"]' -p led
./cleos.sh push action lemonade.c transfer '["led", "p", "10000000.0000 LED", "init"]' -p led
./cleos.sh push action lemonade.c transfer '["led", "c", "10000000.0000 LED", "init"]' -p led
./cleos.sh push action led setpriv '["led.msig",1]' -p led
./cleos.sh push action led init '[0, "4,LED"]' -p led 
## ===


## update auth of system contracts
for sa in "${SYSTEM_ACCOUNTS[@]}"
do
    account="$sa"
    controller="led"
    ./cleos.sh push action led updateauth '{"account": "'$account'", "permission": "owner",  "parent": "",  "auth": { "threshold": 1, "keys": [], "waits": [], "accounts": [{ "weight": 1, "permission": {"actor": "'$controller'", "permission": "active"} }] } } ' -p $account@owner
    ./cleos.sh push action led updateauth '{"account": "'$account'", "permission": "active",  "parent": "owner",  "auth": { "threshold": 1, "keys": [], "waits": [], "accounts": [{ "weight": 1, "permission": {"actor": "'$controller'", "permission": "active"} }] } }' -p $account@active
done
## ===
