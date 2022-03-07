# Lemonade LEDGIS smart contract

## Outline

This contract for `Lemonade LEDGIS smart contract` It contains normal market releated actions

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
$ git clone https://github.com/ibct-dev/lemonade-contract --recursive
or
$ git clone https://github.com/ibct-dev/lemonade-contract
$ cd contract-boilerplate
$ git submodule update --init --recursive
```

## Actions

## init

- init()**;**
    
    컨트랙트 최초 배포 이후 최초 1회만 실행한다. config 테이블에 있는 초기 설정값을 세팅한다.
    
    [init](https://www.notion.so/70392315e0d346088902ec6e68fa574d)
    

**Example**

```tsx
WIP
```

## issuelem

- issuelem()**;**
    
    LEM 토큰 신규 발행 액션, config 테이블에 저장된 `last_lem_bucket_fill` 과 현재 시간을 이용하여 신규 발행량을 계산해서 해당 수량만큼만 발행한다.
    
    [issuelem](https://www.notion.so/a0d8b4fe31094f90a52012f3d3807a36)
    

**Example**

```tsx
WIP
```

## setbtcprice

- setbtcprice()**;**
    
    config 테이블에 BTC price를 설정한다.
    
    [setbtcprice](https://www.notion.so/0af5c0e64e6448bc8eb935596fc97041)
    

**Example**

```tsx
WIP
```

## setledprice

- setledprice()**;**
    
    config2 테이블에 LED price를 설정한다.
    
    [issuelem](https://www.notion.so/43c11c76178b4bfe8a45f44f49dc0c4e)
    

**Example**

```tsx
WIP
```

## addproduct

- addproduct **(
      const name &product_name, const double &minimum_yield,
      const double &maximum_yield, const bool &has_lem_rewards,**
    
          **const bool &has_prediction, const asset &amount_per_account,
          const optional<asset> &maximum_amount_limit,
          const optional<uint32_t> &duration);**
    
    `product_name` 이름을 갖는 예치 상품을 생성한다. 해당 상품의 연이율은 `minimum_yield` 와 `maximum_yield` 사이의 값을 가지며 계정당 `amount_per_account` 의 토큰을 예치할 수 있으며 상품 전체의 제한은 `maximum_amount_limit` 이다. 또한 의무 예치기간은 `duration` 이다
    
    [addproduct](https://www.notion.so/83bbdde0dfe54b00bcf88069e374f9d9)
    

**Example**

```tsx
WIP
```

## rmproduct

- rmproduct **(const name &product_name);**
    
    `product_name` 이름을 갖는 예치 상품을 삭제한다.
    
    [rmproduct](https://www.notion.so/d41126b813284862966facbc63b8a5a5)
    

**Example**

```tsx
WIP
```

## claimled

- claimled **(const name &owner, const name &product_name);**
    
    `owner`의 `product_name` 이름을 갖는 예치 상품에 대해 현재 시간까지 쌓인 LED 보상을 수령한다. 이 액션으로 발생한 보상은 즉시 수령한다.
    
    [unstake](https://www.notion.so/8d5a66a93646434d81545cdfc0ef4a55)
    

**Example**

```tsx
WIP
```

## claimlem

- claimlem **(const name &owner, const name &product_name);**
    
    `owner`의 `product_name` 이름을 갖는 예치 상품에 대해 현재 시간까지 쌓인 LEM 보상을 수령한다. 이 액션으로 발생한 보상은 즉시 수령한다. 또한 LEM이 보상으로 주어지지 않은 상품 가입자는 보상을 받을 수 없다.
    
    [unstake](https://www.notion.so/a0a69644a17d48fab3d7a4b20b0ea891)
    

**Example**

```tsx
WIP
```

## unstake

- unstake **(const name &owner, const name &product_name);**
    
    `owner`의 `product_name` 이름을 갖는 예치 상품에 대해 unstake를 수행한다. 기존에 미리 수령하였던 LED 및 LEM 보상을 제외한 나머지 보상을 한번에 수령하며 원금과 이자를 포함한 금액은 트랜잭션 발생 이 후 일정 시간 이후에 transfer 된다.
    
    [unstake](https://www.notion.so/c3eaf7ab952e43bea1010cb72a448ed1)
    

**Example**

```tsx
WIP
```

## changeyield

- changeyield **(const name &owner,
                      const name &product_name,
                      double &yield, const string &memo);**
    
    `owner`의 `product_name` 이름을 갖는 예치 상품에 대해 연 이율을 `yield`를 변경한다. `yield`는 상품 자체의 최대 및 최소 연이율을 초과할 수 없다.
    
    [unstake](https://www.notion.so/65ef806a04aa452ab1f879a21ac83a0b)
    

**Example**

```tsx
WIP
```

## createbet

- createbet **(const uint32_t &started_at,
                   const uint32_t &betting_ended_at,
                   const uint32_t &ended_at);**
    
    `started_at` 에 시작하며 `betting_ended_at` 에 베팅이 종료되고 `ended_at`에 게임이 종료되는 게임을 생성한다.
    
    [createbet](https://www.notion.so/5253c71a1e9741d1b62c38be887e4734)
    

**Example**

```tsx
WIP
```

## setbet

- setbet **(const uint64_t bet_id, const uint8_t &status,const optional<double> &base_price, const optional<double> &final_price);**
    
    `bet_id` 을 갖는 게임의 상태를 변경한다.
    
    [rmbet](https://www.notion.so/7325dc5160e246b6b3f3b0ce224e1a67)
    

**Example**

```tsx
WIP
```

## rmbet

- rmbet **(const uint64_t bet_id);**
    
    `bet_id` 을 갖는 게임을 삭제한다. 베팅 인원이 1명이라도 있다면 삭제가 불가능하다.
    
    [rmbet](https://www.notion.so/8fc2106c6890479abbfc0051f5ee9de2)
    

**Example**

```tsx
WIP
```

## claimbet

- claimbet **(const uint64_t bet_id);**
    
    `bet_id` 을 갖는 게임에 대해 정산을 수행한다. 승리 포지션은 `win_position` 이다.
    
    [rmbet](https://www.notion.so/57efa43375c246158541f98c2098e9a7)
    

**Example**

```tsx
WIP
```

## inittoken

- inittoken **(const name user, const symbol new_symbol, const extended_asset initial_pool1, const extended_asset initial_pool2);**
    
    새로운 Pool을 생성한다. 토큰 쌍 및 유동성은`initial_pool1`,`initial_pool2` 으로 설정하며 LP 토큰의 이름은 `new_symbol` 이 된다. 
    
    [inittoken](https://www.notion.so/74f3987061984de19acadcb985f1da89)
    

**Example**

```tsx
WIP
```

## openext

- openext **(const name& user, const extended_symbol& ext_symbol);**
    
    `user`의 accounts 테이블에 `ext_symbol`의 심볼명을 갖는 row를 0의 잔액으로 설정한다.  
    
    [openext](https://www.notion.so/f5db7fa8a0894e9589738a7cdd929ebe)
    

**Example**

```tsx
WIP
```

## closeext

- closeext **(const name& user, const name& to, const extended_symbol& ext_symbol, string memo);**
    
    `user`의 accounts 테이블에 `ext_symbol`의 심볼명을 갖는 row를 삭제한다. 잔액이 존재할 경우 그 토큰은 `to` 에게 전송된다.
    
    [closeext](https://www.notion.so/e8f25e8954e34311a4ab293c71b3ea65)
    

**Example**

```tsx
WIP
```

## withdraw

- withdraw **(name user, name to, extended_asset to_withdraw, string memo);**
    
    `user`의 accounts 테이블에 `ext_symbol`의 심볼명을 갖는 row의 잔액을`to` 에게 전송된다.
    
    [withdraw](https://www.notion.so/e837d6254517452aa778f047374ad384)
    

**Example**

```tsx
WIP
```

## addliquidity

- addliquidity **(name user, asset to_buy, asset max_asset1, asset max_asset2);**
    
    `user` 가 `to_buy` LP토큰 만큼 유동성을 공급하고자 함, 유저는 최대 `max_asset1` `max_asset2` 만큼 토큰을 공급하고자 함
    
    [addliquidity](https://www.notion.so/ee3a6704dcf741a19ed0102f756ffc11)
    

**Example**

```tsx
WIP
```

## rmliquidity

- rmliquidity **(name user, asset to_sell, asset min_asset1, asset min_asset2);**
    
    `user` 가 `to_sell` LP토큰 만큼 유동성을 회수하고자 함, 유저는 최소 `min_asset1` `min_asset2` 만큼 토큰을 회수하고자 함
    
    [rmliquidity](https://www.notion.so/3cac856529c8461f8b92c1cf624c44df)
    

**Example**

```tsx
WIP
```

## exchange

- exchange **(name user, symbol_code pair_token, extended_asset ext_asset_in, asset min_expected);**
    
    `user` 가 `pair_token` 의 거래쌍을 이용해 스왑을 시도함, `ext_asset_in` 을 넣고 `min_expected` 를 받을 것이라 기대함
    
    [exchange](https://www.notion.so/abf37e02dd294b40bd92068508e097fa)
    

**Example**

```tsx
WIP
```

## clmpoolreward

- clmpoolreward **(name user, string pair_token_symbol);**
    
    `user` 는 본인이 예치한 `pair_token_symbol` 거래쌍의 중간 보상을 받음
    
    [clmpoolreward](https://www.notion.so/5e3df48f87784de09d9eb6a24d18f8e1)
    

**Example**

```tsx
WIP
```

---

# Table Reference

## **configs**

설정값을 저장하고 있는 table

**Example**

```tsx
const result = await rpc.get_table_rows({
    json: true,               // Get the response as json
    code: 'lemonade.c',       // Contract that we target
    scope: `lemonade.c`,      // Account that owns the data
    table: 'configs',        // Table name
    limit: 1,              // Maximum number of rows that we want to get
  });
console.log(result)
```

## **configs2**

설정값을 저장하고 있는 두번째 table

**Example**

```tsx
const result = await rpc.get_table_rows({
    json: true,               // Get the response as json
    code: 'lemonade.c',       // Contract that we target
    scope: `lemonade.c`,      // Account that owns the data
    table: 'configs2',        // Table name
    limit: 1,              // Maximum number of rows that we want to get
  });
console.log(result)
```

## **products**

예치 상품들을 저장하고 있는 table

**Example**

```tsx
const result = await rpc.get_table_rows({
    json: true,               // Get the response as json
    code: 'lemonade.c',       // Contract that we target
    scope: `lemonade.c`,      // Account that owns the data
    table: 'products',        // Table name
    limit: 1000,              // Maximum number of rows that we want to get
  });
console.log(result)
```

## stakings

예치한 계좌 목록

**Example**

```tsx
const result = await rpc.get_table_rows({
    json: true,               // Get the response as json
    code: 'lemonade.c',       // Contract that we target
    scope: `${account_name}`, // Account that owns the data
    table: 'stakings',        // Table name
    limit: 1000,              // Maximum number of rows that we want to get
  });
console.log(result)
```

## bettings

betting 게임 목록

**Example**

```tsx
const result = await rpc.get_table_rows({
    json: true,               // Get the response as json
    code: 'lemonade.c',       // Contract that we target
    scope: 'lemonade.c',      // Account that owns the data
    table: 'bettings',        // Table name
    limit: 1000,              // Maximum number of rows that we want to get
  });
console.log(result)
```

## accounts

계정의 토큰 목록, led.token의 accounts와는 별개

**Example**

```tsx
const result = await rpc.get_table_rows({
    json: true,               // Get the response as json
    code: 'lemonade.c',       // Contract that we target
    scope: ${account_name},      // Account that owns the data
    table: 'accounts',        // Table name
    limit: 1000,              // Maximum number of rows that we want to get
  });
console.log(result)
```

## stats

거래쌍 목록

**Example**

```tsx
const result = await rpc.get_table_rows({
    json: true,               // Get the response as json
    code: 'lemonade.c',       // Contract that we target
    scope: 'lemonade.c',      // Account that owns the data
    table: 'stats',        // Table name
    limit: 1000,              // Maximum number of rows that we want to get
  });
console.log(result)
```

## dexacnts

레모네이드 계정에 예치된 토큰 테이블

**Example**

```tsx
const result = await rpc.get_table_rows({
    json: true,               // Get the response as json
    code: 'lemonade.c',       // Contract that we target
    scope: ${account_name},      // Account that owns the data
    table: 'dexacnts',        // Table name
    limit: 1000,              // Maximum number of rows that we want to get
  });
console.log(result)
```

# How to use Transfer Action

## staking

> memo schema: stake/<product_name>/<prediction_position>
> 

```json
cleos push action led.token transfer '["myaccount", "lemonade.c", "1.0000 LED", "stake/normal"]' -p myaccount

cleos push action led.token transfer '["myaccount", "lemonade.c", "1.0000 LED", "stake/fixed2/long"]' -p myaccount
```

## bettings

> memo schema: bet/<bet_id>/<prediction_position>
> 

```json
cleos push action led.token transfer '["myaccount", "lemonade.c", "1.0000 LED", "bet/0/long"]' -p myaccount
```

## deposit

> memo schema: deposit
> 

```json
cleos push action led.token transfer '["myaccount", "lemonade.c", "1.0000 LED", "deposit"]' -p myaccount
```

## exchange

> memo schema: exchange/<lp_token_symbol>/<min_expected>
> 

```json
cleos push action led.token transfer '["myaccount", "lemonade.c", "1.0000 LED", "exchange/LEDLEM/1.0000 LEM"]' -p myaccount
```