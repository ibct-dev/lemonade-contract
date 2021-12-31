# Realcollection LEDGIS smart contract

## Outline

This contract for `Realcollection LEDGIS smart contract` It contains normal market releated actions

## Prerequisite

#### Public

1. [docker](https://docs.docker.com/get-started/)
2. [eosio.cdt](https://developers.eos.io/manuals/eosio.cdt/latest/installation)

#### Private

3. `npm login` on `@ibct-dev` to get `@ibct-dev/sclest` package : [Our guide](./docs/Github_Registry_Login.md)
4. `docker login` on `ibct` to get `ledgis-nodeos` image

## Get Started

### Clone

```bash
$ git clone https://github.com/ibct-dev/rlcservice --recursive
or
$ git clone https://github.com/ibct-dev/rlcservice
$ cd contract-boilerplate
$ git submodule update --init --recursive
```

## Actions

1. auction
    - auction 등록 액션
2. rmauction
    - auction 삭제 액션
3. sale
    - sale 등록 액션
4. rmsale
    - sale 삭제 액션
5. approve
    - sale 시 highest_offer을 승인하는 액션
6. claimtoken
    - auction 에서 highest_bidder에게 판매하는 액션
7. clearresidue
    - 시간 지난 거래 일괄삭제



