#include "lemonade.c/lemonade.c.hpp"

void lemonade::openext(const name& user, const extended_symbol& ext_symbol) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    dexacnts acnts(get_self(), user.value);
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find(make128key(
        ext_symbol.get_contract().value, ext_symbol.get_symbol().raw()));
    if (acnt_balance == index.end()) {
        acnts.emplace(user, [&](auto& a) {
            a.balance = extended_asset{0, ext_symbol};
            a.id = acnts.available_primary_key();
        });
    }
}

void lemonade::closeext(const name& user, const name& to,
                        const extended_symbol& ext_symbol, string memo) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    dexacnts acnts(get_self(), user.value);
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find(make128key(
        ext_symbol.get_contract().value, ext_symbol.get_symbol().raw()));
    check(acnt_balance != index.end(), "User does not have such token");
    auto ext_balance = acnt_balance->balance;
    if (ext_balance.quantity.amount > 0) {
        action(permission_level{get_self(), "active"_n}, ext_balance.contract,
               "transfer"_n,
               std::make_tuple(get_self(), to, ext_balance.quantity, memo))
            .send();
    }
    index.erase(acnt_balance);
}

void lemonade::withdraw(name user, name to, extended_asset to_withdraw,
                        string memo) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    check(to_withdraw.quantity.amount > 0, "quantity must be positive");
    add_signed_ext_balance(user, -to_withdraw);
    action(permission_level{get_self(), "active"_n}, to_withdraw.contract,
           "transfer"_n,
           std::make_tuple(get_self(), to, to_withdraw.quantity, memo))
        .send();
}

void lemonade::addliquidity(name user, asset to_buy, asset max_asset1,
                            asset max_asset2) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    stats statstable(get_self(), to_buy.symbol.code().raw());
    const auto& token = statstable.find(to_buy.symbol.code().raw());
    check(token != statstable.end(), "pair token does not exist");
    check((to_buy.amount > 0), "to_buy amount must be positive");
    check((max_asset1.amount >= 0) && (max_asset2.amount >= 0),
          "assets must be nonnegative");
    // TODO: when fee policy is fixed, need to change amount
    int64_t amount = 1;
    asset ledfee = asset(amount, symbol("LED", 4));
    extended_asset fee = extended_asset{ledfee, led_token_contract};
    add_signed_ext_balance(user, -fee);
    statstable.modify(token, same_payer, [&](auto& a) { a.fee += ledfee; });

    add_signed_liq(user, to_buy, true, max_asset1, max_asset2);
}

void lemonade::rmliquidity(name user, asset to_sell, asset min_asset1,
                           asset min_asset2) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    check(to_sell.amount > 0, "to_sell amount must be positive");
    check((min_asset1.amount >= 0) && (min_asset2.amount >= 0),
          "assets must be nonnegative");
    add_signed_liq(user, -to_sell, false, -min_asset1, -min_asset2);
}

void lemonade::exchange(name user, symbol_code pair_token,
                        extended_asset ext_asset_in, asset min_expected) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    stats statstable(get_self(), pair_token.raw());
    const auto& token = statstable.find(pair_token.raw());
    check(token != statstable.end(), "pair token does not exist");
    check(
        ((ext_asset_in.quantity.amount > 0) && (min_expected.amount >= 0)) ||
            ((ext_asset_in.quantity.amount < 0) && (min_expected.amount <= 0)),
        "ext_asset_in must be nonzero and min_expected must have same sign or "
        "be zero");

    auto ext_asset_out = process_exch(pair_token, ext_asset_in, min_expected);

    // TODO: fee policy is 0.2% LED
    int64_t amount = 1;
    if (ext_asset_in.quantity.symbol == symbol("LED", 4)) {
        amount = ext_asset_in.quantity.amount * 0.002 > 0
                     ? ext_asset_in.quantity.amount * 0.002
                     : 1;
    } else if (ext_asset_out.quantity.symbol == symbol("LED", 4)) {
        amount = ext_asset_out.quantity.amount * 0.002 > 0
                     ? ext_asset_out.quantity.amount * 0.002
                     : 1;
    }
    asset ledfee = asset(amount, symbol("LED", 4));
    extended_asset fee = extended_asset{ledfee, led_token_contract};
    add_signed_ext_balance(user, -fee);

    statstable.modify(token, same_payer, [&](auto& a) { a.fee += ledfee; });

    add_signed_ext_balance(user, -ext_asset_in);
    add_signed_ext_balance(user, ext_asset_out);
}

void lemonade::clmpoolreward(name user, string pair_token_symbol) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    symbol_code pair_token = symbol_code(pair_token_symbol);
    stats statstable(get_self(), pair_token.raw());
    const auto token = statstable.find(pair_token.raw());
    check(token != statstable.end(), "pair token does not exist");

    accounts acnts(get_self(), user.value);
    auto it = acnts.find(pair_token.raw());
    check(it != acnts.end(), "user does not have pair token");

    // TODO: when reward policy is fixed, need to change amount
    asset led_reward =
        asset(token->fee.amount * it->balance.amount / token->supply.amount,
              symbol("LED", 4));
    asset lem_reward =
        asset(it->balance.amount / token->supply.amount, symbol("LEM", 4));

    check(led_reward.amount > 0, "led reward must be positive");
    check(lem_reward.amount > 0, "lem reward must be positive");

    action(permission_level{get_self(), "active"_n}, led_token_contract,
           "transfer"_n,
           std::make_tuple(get_self(), user, led_reward,
                           std::string("claim pool reward LED")))
        .send();
    action(permission_level{get_self(), "active"_n}, led_token_contract,
           "transfer"_n,
           std::make_tuple(get_self(), user, lem_reward,
                           std::string("claim pool reward LEM")))
        .send();

    statstable.modify(token, same_payer, [&](auto& a) {
        a.fee -= led_reward;
        check(a.fee.amount >= 0, "fee must be non negative");
    });
}

extended_asset lemonade::process_exch(symbol_code pair_token,
                                      extended_asset ext_asset_in,
                                      asset min_expected) {
    stats statstable(get_self(), pair_token.raw());
    const auto token = statstable.find(pair_token.raw());
    check(token != statstable.end(), "pair token does not exist");
    bool in_first;
    if ((token->pool1.get_extended_symbol() ==
         ext_asset_in.get_extended_symbol()) &&
        (token->pool2.quantity.symbol == min_expected.symbol)) {
        in_first = true;
    } else if ((token->pool1.quantity.symbol == min_expected.symbol) &&
               (token->pool2.get_extended_symbol() ==
                ext_asset_in.get_extended_symbol())) {
        in_first = false;
    } else
        check(false, "extended_symbol mismatch");
    int64_t P_in, P_out;
    if (in_first) {
        P_in = token->pool1.quantity.amount;
        P_out = token->pool2.quantity.amount;
    } else {
        P_in = token->pool2.quantity.amount;
        P_out = token->pool1.quantity.amount;
    }
    auto A_in = ext_asset_in.quantity.amount;
    int64_t A_out = compute(-A_in, P_out, P_in + A_in);
    check(min_expected.amount <= -A_out, "available is less than expected");
    extended_asset ext_asset1, ext_asset2, ext_asset_out;
    if (in_first) {
        ext_asset1 = ext_asset_in;
        ext_asset2 = extended_asset{A_out, token->pool2.get_extended_symbol()};
        ext_asset_out = -ext_asset2;
    } else {
        ext_asset1 = extended_asset{A_out, token->pool1.get_extended_symbol()};
        ext_asset2 = ext_asset_in;
        ext_asset_out = -ext_asset1;
    }
    statstable.modify(token, same_payer, [&](auto& a) {
        a.pool1 += ext_asset1;
        a.pool2 += ext_asset2;
    });
    return ext_asset_out;
}

// computes x * y / z
int64_t lemonade::compute(int64_t x, int64_t y, int64_t z) {
    check((x != 0) && (y > 0) && (z > 0), "invalid parameters");
    int128_t prod = int128_t(x) * int128_t(y);
    int128_t tmp = 0;
    if (x > 0) {
        tmp = 1 + (prod - 1) / int128_t(z);
        check((tmp <= MAX), "computation overflow");
    } else {
        tmp = prod / int128_t(z);
        check((tmp >= -MAX), "computation underflow");
    }
    return int64_t(tmp);
}

void lemonade::memoexchange(name user, extended_asset ext_asset_in,
                            symbol_code pair_token, asset min_expected) {
    check(is_dex_open, "DEX is not open");
    check(min_expected.amount >= 0,
          "min_expected must be expressed with a positive amount");
    auto ext_asset_out = process_exch(pair_token, ext_asset_in, min_expected);
    stats statstable(get_self(), pair_token.raw());
    const auto& token = statstable.find(pair_token.raw());
    check(token != statstable.end(), "pair token does not exist");
    // TODO: fee policy is 0.2% LED
    int64_t amount = 1;
    if (ext_asset_in.quantity.symbol == symbol("LED", 4)) {
        amount = ext_asset_in.quantity.amount * 0.002 > 0
                     ? ext_asset_in.quantity.amount * 0.002
                     : 1;
    } else if (ext_asset_out.quantity.symbol == symbol("LED", 4)) {
        amount = ext_asset_out.quantity.amount * 0.002 > 0
                     ? ext_asset_out.quantity.amount * 0.002
                     : 1;
    }
    asset ledfee = asset(amount, symbol("LED", 4));
    extended_asset fee = extended_asset{ledfee, led_token_contract};
    add_signed_ext_balance(user, -fee);

    statstable.modify(token, same_payer, [&](auto& a) { a.fee += ledfee; });

    action(permission_level{get_self(), "active"_n}, ext_asset_out.contract,
           "transfer"_n,
           std::make_tuple(
               get_self(), user, ext_asset_out.quantity,
               std::string("exchange ") + ext_asset_in.quantity.to_string() +
                   std::string("to ") + ext_asset_out.quantity.to_string()))
        .send();
}

void lemonade::inittoken(name user, symbol new_symbol,
                         extended_asset initial_pool1,
                         extended_asset initial_pool2) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    check((initial_pool1.quantity.amount > 0) &&
              (initial_pool2.quantity.amount > 0),
          "Both assets must be positive");
    check((initial_pool1.quantity.amount < INIT_MAX) &&
              (initial_pool2.quantity.amount < INIT_MAX),
          "Initial amounts must be less than 10^15");
    uint8_t new_precision = (initial_pool1.quantity.symbol.precision() +
                             initial_pool2.quantity.symbol.precision()) /
                            2;
    check(new_symbol.precision() == new_precision,
          "new_symbol precision must be (precision1 + precision2) / 2");
    int128_t geometric_mean = sqrt(int128_t(initial_pool1.quantity.amount) *
                                   int128_t(initial_pool2.quantity.amount));
    auto new_token = asset{int64_t(geometric_mean), new_symbol};
    check(initial_pool1.get_extended_symbol() !=
              initial_pool2.get_extended_symbol(),
          "extended symbols must be different");

    if (user != "lemonade.c"_n) {
        auto create_pool_fee = extended_asset{asset(200'0000, symbol("LEM", 4)),
                                              led_token_contract};
        add_signed_ext_balance(user, -create_pool_fee);
    }

    stats statstable(get_self(), new_symbol.code().raw());
    const auto& token = statstable.find(new_symbol.code().raw());
    check(token == statstable.end(), "token symbol already exists");

    statstable.emplace(user, [&](auto& a) {
        a.supply = new_token;
        a.max_supply = asset{MAX, new_symbol};
        a.issuer = get_self();
        a.pool1 = initial_pool1;
        a.pool2 = initial_pool2;
        a.fee = asset(0, symbol("LED", 4));
    });

    swap_lists listtable1(get_self(),
                          initial_pool1.quantity.symbol.code().raw());
    const auto& token1 =
        listtable1.find(initial_pool2.quantity.symbol.code().raw());
    check(token1 == listtable1.end(), "token symbol already exists");
    listtable1.emplace(get_self(), [&](auto& a) {
        a.pair_symbol = initial_pool2.quantity.symbol;
    });

    swap_lists listtable2(get_self(),
                          initial_pool2.quantity.symbol.code().raw());
    const auto& token2 =
        listtable2.find(initial_pool1.quantity.symbol.code().raw());
    check(token2 == listtable2.end(), "token symbol already exists");
    listtable2.emplace(get_self(), [&](auto& a) {
        a.pair_symbol = initial_pool1.quantity.symbol;
    });

    pool_lists poollisttable(get_self(), get_self().value);
    const auto& poollist = poollisttable.find(new_symbol.code().raw());
    check(poollist == poollisttable.end(), "token symbol already exists");
    poollisttable.emplace(get_self(), [&](auto& a) {
        a.lp_symbol = new_symbol;
        a.symbol1 = initial_pool1.quantity.symbol;
        a.symbol2 = initial_pool2.quantity.symbol;
    });

    placeindex(user, new_symbol, initial_pool1, initial_pool2);
    add_balance(user, new_token, user);
    add_signed_ext_balance(user, -initial_pool1);
    add_signed_ext_balance(user, -initial_pool2);
}

void lemonade::indexpair(name user, symbol lp_symbol) {
    check(is_dex_open, "DEX is not open");
    stats statstable(get_self(), lp_symbol.code().raw());
    const auto& token = statstable.find(lp_symbol.code().raw());
    check(token != statstable.end(), "token symbol does not exist");
    auto pool1 = token->pool1;
    auto pool2 = token->pool2;
    placeindex(user, lp_symbol, pool1, pool2);
}

void lemonade::placeindex(name user, symbol lp_symbol, extended_asset pool1,
                          extended_asset pool2) {
    auto id_256 = make256key(pool1.contract.value, pool1.quantity.symbol.raw(),
                             pool2.contract.value, pool2.quantity.symbol.raw());
    lemindexes indextable(get_self(), get_self().value);
    auto index = indextable.get_index<"extended"_n>();
    const auto& info = index.find(id_256);
    check(info == index.end(), "the pool is already indexed");
    indextable.emplace(user, [&](auto& a) {
        a.lp_symbol = lp_symbol;
        a.id_256 = id_256;
    });
}

void lemonade::add_signed_liq(name user, asset to_add, bool is_buying,
                              asset max_asset1, asset max_asset2) {
    check(to_add.is_valid(), "invalid asset");
    stats statstable(get_self(), to_add.symbol.code().raw());
    const auto& token = statstable.find(to_add.symbol.code().raw());
    check(token != statstable.end(), "pair token does not exist");
    auto A = token->supply.amount;
    auto P1 = token->pool1.quantity.amount;
    auto P2 = token->pool2.quantity.amount;

    auto to_pay1 = extended_asset{
        asset{compute(to_add.amount, P1, A), token->pool1.quantity.symbol},
        token->pool1.contract};
    auto to_pay2 = extended_asset{
        asset{compute(to_add.amount, P2, A), token->pool2.quantity.symbol},
        token->pool2.contract};
    check((to_pay1.quantity.symbol == max_asset1.symbol) &&
              (to_pay2.quantity.symbol == max_asset2.symbol),
          "incorrect symbol");
    check((to_pay1.quantity.amount <= max_asset1.amount) &&
              (to_pay2.quantity.amount <= max_asset2.amount),
          "available is less than expected");

    add_signed_ext_balance(user, -to_pay1);
    add_signed_ext_balance(user, -to_pay2);
    (to_add.amount > 0) ? add_balance(user, to_add, user)
                        : sub_balance(user, -to_add);
    statstable.modify(token, same_payer, [&](auto& a) {
        a.supply += to_add;
        a.pool1 += to_pay1;
        a.pool2 += to_pay2;
    });
    check(token->supply.amount != 0, "the pool cannot be left empty");
}

void lemonade::add_signed_ext_balance(const name& user,
                                      const extended_asset& to_add) {
    check(to_add.quantity.is_valid(), "invalid asset");
    dexacnts acnts(get_self(), user.value);
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find(
        make128key(to_add.contract.value, to_add.quantity.symbol.raw()));
    if (acnt_balance == index.end()) {
        check(to_add.quantity.amount >= 0, "to_add must be positive");
        acnts.emplace(user, [&](auto& a) {
            a.balance = to_add;
            a.id = acnts.available_primary_key();
        });
    } else {
        index.modify(acnt_balance, same_payer, [&](auto& a) {
            a.balance += to_add;
            check(a.balance.quantity.amount >= 0, "insufficient funds");
        });
    }
}

// void lemonade::test() {
//     auto symbol1 = symbol("LED", 4);
//     auto symbol2 = symbol("LEM", 4);
//     auto symbol3 = symbol("LEDLEM", 4);

//     swap_lists listtable1(get_self(), symbol1.code().raw());
//     const auto& token1 = listtable1.find(symbol2.code().raw());
//     check(token1 == listtable1.end(), "token symbol already exists");
//     listtable1.emplace(get_self(), [&](auto& a) { a.pair_symbol = symbol2; });

//     swap_lists listtable2(get_self(), symbol2.code().raw());
//     const auto& token2 = listtable2.find(symbol1.code().raw());
//     check(token2 == listtable2.end(), "token symbol already exists");
//     listtable2.emplace(get_self(), [&](auto& a) { a.pair_symbol = symbol1; });

//     pool_lists tt(get_self(), get_self().value);
//     const auto& aa = tt.find(symbol3.code().raw());
//     check(aa == tt.end(), "token symbol already exists");
//     tt.emplace(get_self(), [&](auto& a) {
//         a.lp_symbol = symbol3;
//         a.symbol1 = symbol1;
//         a.symbol2 = symbol2;
//     });
// }

uint128_t lemonade::make128key(uint64_t a, uint64_t b) {
    uint128_t aa = a;
    uint128_t bb = b;
    return (aa << 64) + bb;
}

checksum256 lemonade::make256key(uint64_t a, uint64_t b, uint64_t c,
                                 uint64_t d) {
    if (make128key(a, b) < make128key(c, d))
        return checksum256::make_from_word_sequence<uint64_t>(a, b, c, d);
    else
        return checksum256::make_from_word_sequence<uint64_t>(c, d, a, b);
}
