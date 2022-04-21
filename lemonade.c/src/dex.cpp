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

void lemonade::exchangeall(name user, symbol_code pair_token,
                           extended_symbol asset_in) {
    check(is_dex_open, "DEX is not open");
    require_auth(user);
    stats statstable(get_self(), pair_token.raw());
    const auto& token = statstable.find(pair_token.raw());
    check(token != statstable.end(), "pair token does not exist");

    extended_asset ext_asset_in;
    asset min_expected;
    extended_symbol ext_asset_sym;
    if (token->pool1.get_extended_symbol() == asset_in) {
        ext_asset_sym = token->pool1.get_extended_symbol();
        min_expected = token->pool2.quantity;
    } else if (token->pool2.get_extended_symbol() == asset_in) {
        ext_asset_sym = token->pool2.get_extended_symbol();
        min_expected = token->pool2.quantity;
    } else {
        check(false, "doesn't have such asset in dexacnts");
    }

    dexacnts acnts(get_self(), user.value);
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find(make128key(
        ext_asset_sym.get_contract().value, ext_asset_sym.get_symbol().raw()));
    check(acnt_balance != index.end(), "User does not have such token");
    ext_asset_in = acnt_balance->balance;

    exchange(user, pair_token, ext_asset_in, min_expected);
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

    // Get a fee
    const auto fee = ext_asset_in.quantity.amount * swap_fee;
    if (ext_asset_in.quantity.symbol == token->fee1.quantity.symbol) {
        statstable.modify(token, same_payer,
                          [&](auto& a) { a.fee1.quantity.amount += fee / 2; });
    } else if (ext_asset_in.quantity.symbol == token->fee2.quantity.symbol) {
        statstable.modify(token, same_payer,
                          [&](auto& a) { a.fee2.quantity.amount += fee / 2; });
    }
    ext_asset_in.quantity.amount -= fee;
    auto ext_asset_out = process_exch(pair_token, ext_asset_in, min_expected);

    add_signed_ext_balance(user, -ext_asset_in);
    add_signed_ext_balance(user, ext_asset_out);
}

void lemonade::clmpoolreward(string pair_token_symbol, double total,
                             double pool) {
    require_auth(get_self());
    issue_lem();
    symbol_code pair_token = symbol_code(pair_token_symbol);
    pool_rewards poolrewardstable(get_self(), get_self().value);
    const auto poolrewards = poolrewardstable.find(pair_token.raw());

    configs config_table(get_self(), get_self().value);
    auto existing_config = config_table.find(0);
    check(existing_config != config_table.end(), "contract not initialized");

    stats statstable(get_self(), pair_token.raw());
    const auto token = statstable.find(pair_token.raw());
    check(token != statstable.end(), "pair token does not exist");

    auto current = now();
    if (poolrewards == poolrewardstable.end()) {
        poolrewardstable.emplace(get_self(), [&](auto& a) {
            a.pair_symbol_code = pair_token;
            a.last_reward = now();
        });
    } else {
        check(current - poolrewards->last_reward >= (secondsPerDay - 3600),
              "today reward already claimed");
        poolrewardstable.modify(poolrewards, same_payer,
                                [&](auto& a) { a.last_reward = now(); });
    }

    const double ratio = pool / total;

    user_infos userinfostable(get_self(), pair_token.raw());
    for (auto k : userinfostable) {
        if (k.balance.amount == 0) {
            continue;
        }
        const double user_ratio = k.balance.amount / token->supply.amount;
        const asset fee1_reward =
            asset(token->fee1.quantity.amount * user_ratio,
                  token->fee1.quantity.symbol);
        const asset fee2_reward =
            asset(token->fee2.quantity.amount * user_ratio,
                  token->fee2.quantity.symbol);
        const asset fee3_reward =
            asset(DEX_LEM_REWARD /
                      (uint32_t)pow(2, existing_config->half_life_count) *
                      ratio * user_ratio,
                  token->fee3.quantity.symbol);
        if (fee1_reward.amount > 0) {
            action(permission_level{get_self(), "active"_n}, "led.token"_n,
                   "transfer"_n,
                   make_tuple(get_self(), k.account, fee1_reward,
                              string("Pool rewards for providing liquidity")))
                .send();
        }
        if (fee2_reward.amount > 0) {
            action(permission_level{get_self(), "active"_n}, "led.token"_n,
                   "transfer"_n,
                   make_tuple(get_self(), k.account, fee2_reward,
                              string("Pool rewards for providing liquidity")))
                .send();
        }
        if (fee3_reward.amount > 0) {
            action(permission_level{get_self(), "active"_n}, "led.token"_n,
                   "transfer"_n,
                   make_tuple(get_self(), k.account, fee3_reward,
                              string("Pool rewards for providing liquidity")))
                .send();
        }
    }
    auto fee1 = token->pool1;
    auto fee2 = token->pool2;
    fee1.quantity.amount = 0;
    fee2.quantity.amount = 0;
    statstable.modify(token, same_payer, [&](auto& a) {
        a.fee1 = fee1;
        a.fee2 = fee2;
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
    stats statstable(get_self(), pair_token.raw());
    const auto& token = statstable.find(pair_token.raw());
    check(token != statstable.end(), "pair token does not exist");

    // Get a fee
    const auto fee = ext_asset_in.quantity.amount * swap_fee;
    if (ext_asset_in.quantity.symbol == token->fee1.quantity.symbol) {
        statstable.modify(token, same_payer,
                          [&](auto& a) { a.fee1.quantity.amount += fee / 2; });
    } else if (ext_asset_in.quantity.symbol == token->fee2.quantity.symbol) {
        statstable.modify(token, same_payer,
                          [&](auto& a) { a.fee2.quantity.amount += fee / 2; });
    }
    ext_asset_in.quantity.amount -= fee;
    auto ext_asset_out = process_exch(pair_token, ext_asset_in, min_expected);

    action(permission_level{get_self(), "active"_n}, ext_asset_out.contract,
           "transfer"_n,
           std::make_tuple(
               get_self(), user, ext_asset_out.quantity,
               std::string("exchange ") + ext_asset_in.quantity.to_string() +
                   std::string(" to ") + ext_asset_out.quantity.to_string()))
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

    auto fee1 = initial_pool1;
    auto fee2 = initial_pool2;
    fee1.quantity.amount = 0;
    fee2.quantity.amount = 0;
    statstable.emplace(user, [&](auto& a) {
        a.supply = new_token;
        a.max_supply = asset{MAX, new_symbol};
        a.issuer = user;
        a.pool1 = initial_pool1;
        a.pool2 = initial_pool2;
        a.fee1 = fee1;
        a.fee2 = fee2;
        a.fee3 = extended_asset(asset(0, symbol("LEM", 4)), "led.token"_n);
    });

    user_infos userinfostable(get_self(), new_symbol.code().raw());
    const auto& infos = userinfostable.find(user.value);
    check(infos == userinfostable.end(), "user already has token");
    userinfostable.emplace(get_self(), [&](auto& a) {
        a.account = user;
        a.balance = new_token;
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
        a.symbol1 = extended_symbol(initial_pool1.quantity.symbol,
                                    initial_pool1.contract);
        a.symbol2 = extended_symbol(initial_pool2.quantity.symbol,
                                    initial_pool2.contract);
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

// void lemonade::test(name owner, string pair_token_symbol) {
//     symbol_code pair_token = symbol_code(pair_token_symbol);
//     accounts from_acnts(get_self(), owner.value);

//     const auto& from =
//         from_acnts.get(pair_token.raw(), "no balance object found");

//     user_infos userinfostable(get_self(), pair_token.raw());
//     userinfostable.emplace(get_self(), [&](auto& a) {
//         a.account = owner;
//         a.balance = from.balance;
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
