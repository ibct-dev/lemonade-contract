#include "lemonade.c/lemonade.c.hpp"

#include "utils.hpp"

void lemonade::init() {
    require_auth(get_self());

    configs config_table(get_self(), get_self().value);
    auto existing_config = config_table.find(0);
    auto current = now();
    if (existing_config == config_table.end()) {
        config_table.emplace(get_self(), [&](config &a) {
            a.id = config_table.available_primary_key();
            a.is_active = true;
            a.last_lem_bucket_fill = current;
            a.last_half_life_updated.push_back(current);
            a.last_half_life_updated.push_back(current + secondsPerYear * 2);
            a.last_half_life_updated.push_back(current + secondsPerYear * 4);
            a.last_half_life_updated.push_back(current + secondsPerYear * 6);
            a.half_life_count = 0;
            a.btc_price = 0;
        });
    }

    configs2 config2_table(get_self(), get_self().value);
    auto config2Idx = config2_table.get_index<eosio::name("bysymbol")>();
    auto existing_config2 = config2Idx.find("led"_n.value);
    if (existing_config2 == config2Idx.end()) {
        config2_table.emplace(get_self(), [&](config2 &a) {
            a.id = config2_table.available_primary_key();
            a.symbol = "led"_n;
            a.price = 0;
        });
    }
}

void lemonade::issuelem() {
    require_auth(get_self());
    issue_lem();
}

void lemonade::setbtcprice(const double &price) {
    require_auth(get_self());

    configs config_table(get_self(), get_self().value);
    auto existing_config = config_table.find(0);
    check(existing_config != config_table.end(), "contract not initialized");
    config_table.modify(existing_config, same_payer,
                        [&](config &a) { a.btc_price = price; });
}

void lemonade::setledprice(const double &price) {
    require_auth(get_self());

    configs2 config2_table(get_self(), get_self().value);
    auto config2Idx = config2_table.get_index<eosio::name("bysymbol")>();
    auto existing_config2 = config2Idx.find("led"_n.value);
    check(existing_config2 != config2Idx.end(), "led price not exist");
    config2Idx.modify(existing_config2, same_payer,
                      [&](config2 &a) { a.price = price; });
}

void lemonade::addproduct(const name &product_name, const double &minimum_yield,
                          const double &maximum_yield,
                          const bool &has_lem_rewards,
                          const bool &has_prediction,
                          const asset &amount_per_account,
                          const optional<asset> &maximum_amount_limit,
                          const optional<uint32_t> &duration) {
    require_auth(get_self());

    products products_table(get_self(), get_self().value);
    auto productIdx = products_table.get_index<eosio::name("byname")>();
    auto existing_product = productIdx.find(product_name.value);
    check(existing_product == productIdx.end(), "product name already exist");

    check(minimum_yield <= maximum_yield,
          "maximum yield must be same or bigger "
          "then minimum yield");

    asset zero = asset(0, amount_per_account.symbol);

    asset limit = zero;
    uint32_t dur = 0;

    if (maximum_amount_limit.has_value()) {
        check(amount_per_account.amount <= maximum_amount_limit->amount,
              "account limit must be smaller than total limit");
        check(
            amount_per_account.symbol == maximum_amount_limit->symbol,
            "must be same amount_per_account and maximum_amount_limit symbol");
        limit = maximum_amount_limit.value();
    }

    if (duration.has_value()) {
        dur = duration.value();
    }

    products_table.emplace(get_self(), [&](product &a) {
        a.id = products_table.available_primary_key();
        a.name = product_name;
        a.duration = dur;
        a.current_amount = zero;
        a.amount_per_account = amount_per_account;
        a.maximum_amount_limit = limit;
        a.minimum_yield = minimum_yield;
        a.maximum_yield = maximum_yield;
        a.has_lem_rewards = has_lem_rewards;
        a.has_prediction = has_prediction;
    });
}

void lemonade::rmproduct(const name &product_name) {
    require_auth(get_self());

    products products_table(get_self(), get_self().value);
    auto productIdx = products_table.get_index<eosio::name("byname")>();
    auto existing_product =
        productIdx.require_find(product_name.value, "product does not exists");

    check(existing_product->current_amount.amount == 0,
          "this product already sold for other users");

    productIdx.erase(existing_product);
}

void lemonade::stake(const name &owner, const asset &quantity,
                     const name &product_name,
                     const optional<name> &price_prediction) {
    require_auth(owner);

    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must transfer positive quantity");

    products products_table(get_self(), get_self().value);
    auto productIdx = products_table.get_index<eosio::name("byname")>();
    auto existing_product = productIdx.find(product_name.value);
    check(existing_product != productIdx.end(), "product does not exist");

    configs config_table(get_self(), get_self().value);
    auto existing_config = config_table.find(0);
    check(existing_config != config_table.end(), "contract not initialized");

    configs2 config2_table(get_self(), get_self().value);
    auto config2Idx = config2_table.get_index<eosio::name("bysymbol")>();
    auto existing_config2 = config2Idx.find("led"_n.value);
    check(existing_config2 != config2Idx.end(), "led price not exist");

    stakings stakings_table(get_self(), owner.value);
    auto stakingIdx = stakings_table.get_index<eosio::name("byproductid")>();
    auto existing_staking = stakingIdx.find(existing_product->id);

    uint32_t started_at = now();
    uint32_t ended_at = 0;
    if (existing_product->duration != 0) {
        ended_at = now() + existing_product->duration;
    }
    uint32_t last_lem_reward = existing_product->has_lem_rewards ? now() : 0;
    double base = 0;

    // If staking doesn't exist, create new staking
    if (existing_staking == stakingIdx.end()) {
        if (existing_product->amount_per_account.amount != 0) {
            check(
                existing_product->amount_per_account.amount >= quantity.amount,
                "exceed amount per account limits");
        }

        if (existing_product->maximum_amount_limit.amount != 0) {
            check(
                existing_product->maximum_amount_limit.amount >=
                    (existing_product->current_amount.amount + quantity.amount),
                "exceed product total amount limits");
        }

        asset zero_led = asset(0, symbol("LED", 4));
        asset zero_lem = asset(0, symbol("LEM", 4));

        uint32_t last_lem_reward =
            existing_product->has_lem_rewards ? now() : 0;
        name prediction = "none"_n;
        if (price_prediction.has_value() &&
            price_prediction.value() != "none"_n) {
            check(existing_product->has_prediction,
                  "Product doesn't accept price prediction");
            prediction = price_prediction.value();
            base = existing_config2->price;
        }

        stakings_table.emplace(get_self(), [&](staking &a) {
            a.id = stakings_table.available_primary_key();
            a.balance = quantity;
            a.product_id = existing_product->id;
            a.current_yield = existing_product->minimum_yield;
            a.price_prediction = prediction;
            a.started_at = started_at;
            a.base_price = base;
            a.ended_at = ended_at;
            a.lem_rewards = zero_lem;
            a.led_rewards = zero_led;
            a.last_claim_led_reward = started_at;
            a.last_claim_lem_reward = last_lem_reward;
        });

        productIdx.modify(existing_product, same_payer, [&](product &a) {
            a.current_amount += quantity;
            a.buyers.push_back(owner);
        });
    }
    // If staking exist, update exist staking
    else if (existing_staking != stakingIdx.end()) {
        check(existing_staking->ended_at > now(), "staking already ended");
        if (existing_product->amount_per_account.amount != 0) {
            check(existing_product->amount_per_account.amount >=
                      existing_staking->balance.amount + quantity.amount,
                  "exceed amount per account limits");
        }

        if (existing_product->maximum_amount_limit.amount != 0) {
            check(existing_product->maximum_amount_limit.amount >=
                      (existing_product->current_amount.amount +
                       quantity.amount + existing_staking->balance.amount),
                  "exceed product total amount limits");
        }

        asset zero_led = asset(0, symbol("LED", 4));
        asset zero_lem = asset(0, symbol("LEM", 4));
        if (existing_product->has_prediction) {
            base = existing_config2->price;
        }

        // Give unreceived reward
        issue_lem();
        auto current = now();

        auto yield = existing_staking->current_yield;

        // calculate total led rewards
        const auto yield_per_sec = (yield - 1) / secondsPerYear;
        asset total_led_reward =
            asset(existing_staking->balance.amount * yield_per_sec *
                      (current - existing_staking->started_at),
                  existing_staking->balance.symbol);
        asset to_owner_led = total_led_reward - existing_staking->led_rewards;

        // calculate total lem rewards
        asset to_owner_lem = asset(0, symbol("LEM", 4));
        uint32_t total_lem_reward_amount = 0;
        uint32_t last_reward = existing_staking->started_at;
        if (existing_product->has_lem_rewards == true) {
            for (int i = 0; i < 3; i++) {
                if (existing_config->last_half_life_updated[i] <= current &&
                    current < existing_config->last_half_life_updated[i + 1]) {
                    total_lem_reward_amount +=
                        ((current - last_reward) / (uint32_t)pow(2, i));
                } else if (existing_config->last_half_life_updated[i + 1] <=
                           current) {
                    total_lem_reward_amount +=
                        ((existing_config->last_half_life_updated[i + 1] -
                          last_reward) /
                         (uint32_t)pow(2, i));
                    last_reward =
                        existing_config->last_half_life_updated[i + 1];
                }
            }

            asset total_lem_reward = asset(existing_staking->balance.amount *
                                               total_lem_reward_amount *
                                               lem_reward_rate / secondsPerHour,
                                           symbol("LEM", 4));
            to_owner_lem = total_lem_reward - existing_staking->lem_rewards;
        }

        auto sender_id = now();
        auto delay = product_name == "normal"_n ? 1 : delay_transfer_sec;

        eosio::transaction txn;
        if (to_owner_led.amount > 0) {
            txn.actions.emplace_back(permission_level{get_self(), "active"_n},
                                     "led.token"_n, "transfer"_n,
                                     make_tuple(get_self(), owner, to_owner_led,
                                                string("stake reward")));
        }
        if (existing_product->has_lem_rewards == true &&
            to_owner_lem.amount > 0) {
            txn.actions.emplace_back(permission_level{get_self(), "active"_n},
                                     "led.token"_n, "transfer"_n,
                                     make_tuple(get_self(), owner, to_owner_lem,
                                                string("stake reward")));
        }
        txn.delay_sec = delay;
        txn.send(sender_id, get_self());

        stakingIdx.modify(existing_staking, same_payer, [&](staking &a) {
            a.balance += quantity;
            a.started_at = started_at;
            a.ended_at = ended_at;
            a.base_price = base;
            a.lem_rewards = zero_lem;
            a.led_rewards = zero_led;
            a.last_claim_led_reward = started_at;
            a.last_claim_lem_reward = last_lem_reward;
        });

        productIdx.modify(existing_product, same_payer,
                          [&](product &a) { a.current_amount += quantity; });
    }
}

void lemonade::unstake(const name &owner, const name &product_name) {
    require_auth(owner);

    configs config_table(get_self(), get_self().value);
    auto existing_config = config_table.find(0);
    check(existing_config != config_table.end(), "contract not initialized");

    configs2 config2_table(get_self(), get_self().value);
    auto config2Idx = config2_table.get_index<eosio::name("bysymbol")>();
    auto existing_config2 = config2Idx.find("led"_n.value);
    check(existing_config2 != config2Idx.end(), "led price not exist");

    products products_table(get_self(), get_self().value);
    auto productIdx = products_table.get_index<eosio::name("byname")>();
    auto existing_product = productIdx.find(product_name.value);
    check(existing_product != productIdx.end(), "product does not exist");

    stakings stakings_table(get_self(), owner.value);
    auto stakingIdx = stakings_table.get_index<eosio::name("byproductid")>();
    auto existing_staking = stakingIdx.find(existing_product->id);
    check(existing_staking != stakingIdx.end(), "owner does not has product");

    if (existing_staking->ended_at != 0) {
        check(existing_staking->ended_at <= now(),
              "account end time is not over");
    }
    if (existing_staking->ended_at == 0) {
        check(existing_staking->started_at + secondsPerDay <= now(),
              "account end time is not over");
    }

    // issue new LEM
    issue_lem();
    auto current = now();

    if (existing_product->duration != 0) {
        current = existing_staking->ended_at;
    }

    auto yield = existing_staking->current_yield;
    if (existing_product->has_prediction == true) {
        if (existing_staking->price_prediction == "long"_n) {
            yield = existing_config2->price > existing_staking->base_price
                        ? existing_product->maximum_yield
                        : existing_product->minimum_yield;
        } else if (existing_staking->price_prediction == "short"_n) {
            yield = existing_config2->price > existing_staking->base_price
                        ? existing_product->minimum_yield
                        : existing_product->maximum_yield;
        }
    }

    // calculate total led rewards
    const auto yield_per_sec = (yield - 1) / secondsPerYear;
    asset total_led_reward =
        asset(existing_staking->balance.amount * yield_per_sec *
                  (current - existing_staking->started_at),
              existing_staking->balance.symbol);
    asset to_owner_led = existing_staking->balance + total_led_reward -
                         existing_staking->led_rewards;

    // calculate total lem rewards
    asset to_owner_lem = asset(0, symbol("LEM", 4));
    uint32_t total_lem_reward_amount = 0;
    uint32_t last_reward = existing_staking->started_at;
    if (existing_product->has_lem_rewards == true) {
        for (int i = 0; i < 3; i++) {
            if (existing_config->last_half_life_updated[i] <= current &&
                current < existing_config->last_half_life_updated[i + 1]) {
                total_lem_reward_amount +=
                    ((current - last_reward) / (uint32_t)pow(2, i));
            } else if (existing_config->last_half_life_updated[i + 1] <=
                       current) {
                total_lem_reward_amount +=
                    ((existing_config->last_half_life_updated[i + 1] -
                      last_reward) /
                     (uint32_t)pow(2, i));
                last_reward = existing_config->last_half_life_updated[i + 1];
            }
        }

        asset total_lem_reward =
            asset(existing_staking->balance.amount * total_lem_reward_amount *
                      lem_reward_rate / secondsPerHour,
                  symbol("LEM", 4));
        to_owner_lem = total_lem_reward - existing_staking->lem_rewards;
    }

    auto sender_id = now();
    auto delay = product_name == "normal"_n ? 1 : delay_transfer_sec;
    check(to_owner_led.amount > 0, "unstake amount must not be zero");

    eosio::transaction txn;
    txn.actions.emplace_back(
        permission_level{get_self(), "active"_n}, "led.token"_n, "transfer"_n,
        make_tuple(get_self(), owner, to_owner_led, string("unstake")));
    if (existing_product->has_lem_rewards == true && to_owner_lem.amount > 0) {
        txn.actions.emplace_back(
            permission_level{get_self(), "active"_n}, "led.token"_n,
            "transfer"_n,
            make_tuple(get_self(), owner, to_owner_lem, string("unstake")));
    }
    txn.delay_sec = delay;
    txn.send(sender_id, get_self());

    productIdx.modify(existing_product, same_payer, [&](product &a) {
        a.current_amount -= existing_staking->balance;
        a.buyers.erase(remove(a.buyers.begin(), a.buyers.end(), owner),
                       a.buyers.end());
    });

    stakingIdx.erase(existing_staking);
}

void lemonade::claimled(const name &owner, const name &product_name) {
    require_auth(owner);

    products products_table(get_self(), get_self().value);
    auto productIdx = products_table.get_index<eosio::name("byname")>();
    auto existing_product = productIdx.find(product_name.value);
    check(existing_product != productIdx.end(), "product does not exist");

    stakings stakings_table(get_self(), owner.value);
    auto stakingIdx = stakings_table.get_index<eosio::name("byproductid")>();
    auto existing_staking = stakingIdx.find(existing_product->id);
    check(existing_staking != stakingIdx.end(), "owner does not has product");

    auto current = now();
    if (existing_product->duration != 0) {
        current = now() >= existing_staking->ended_at
                      ? existing_staking->ended_at
                      : now();
    }

    const auto secs_since_last_reward =
        (current - existing_staking->last_claim_led_reward);

    const auto yield_per_sec =
        (existing_staking->current_yield - 1) / secondsPerYear;
    asset to_owner_led = asset(0, symbol("LED", 4));
    to_owner_led.amount = existing_staking->balance.amount *
                          secs_since_last_reward * yield_per_sec;

    if (to_owner_led.amount > 0) {
        action(permission_level{get_self(), "active"_n}, "led.token"_n,
               "transfer"_n,
               make_tuple(get_self(), owner, to_owner_led, string("claim led")))
            .send();
        stakingIdx.modify(existing_staking, same_payer, [&](staking &a) {
            a.last_claim_led_reward = current;
            a.led_rewards += to_owner_led;
        });
    }
}

void lemonade::claimlem(const name &owner, const name &product_name) {
    require_auth(owner);
    issue_lem();
    configs config_table(get_self(), get_self().value);
    auto existing_config = config_table.find(0);
    check(existing_config != config_table.end(), "contract not initialized");

    products products_table(get_self(), get_self().value);
    auto productIdx = products_table.get_index<eosio::name("byname")>();
    auto existing_product = productIdx.find(product_name.value);
    check(existing_product != productIdx.end(), "product does not exist");

    stakings stakings_table(get_self(), owner.value);
    auto stakingIdx = stakings_table.get_index<eosio::name("byproductid")>();
    auto existing_staking = stakingIdx.find(existing_product->id);
    check(existing_staking != stakingIdx.end(), "owner does not has product");

    check(existing_product->has_lem_rewards,
          "there are no LEM rewards for this product ");

    const auto current = now();
    auto rewards_end_time = current >= existing_staking->ended_at
                                ? existing_staking->ended_at
                                : current;
    auto last_reward = existing_staking->last_claim_lem_reward;
    auto amount = 0;

    for (int i = 0; i < 3; i++) {
        if (existing_config->last_half_life_updated[i] <= rewards_end_time &&
            rewards_end_time < existing_config->last_half_life_updated[i + 1]) {
            amount += ((rewards_end_time - last_reward) / (uint32_t)pow(2, i));
        } else if (existing_config->last_half_life_updated[i + 1] <=
                   rewards_end_time) {
            amount += ((existing_config->last_half_life_updated[i + 1] -
                        last_reward) /
                       (uint32_t)pow(2, i));
            last_reward = existing_config->last_half_life_updated[i + 1];
        }
    }

    asset to_owner_lem = asset(existing_staking->balance.amount * amount *
                                   lem_reward_rate / secondsPerHour,
                               symbol("LEM", 4));

    if (to_owner_lem.amount > 0) {
        action(permission_level{get_self(), "active"_n}, "led.token"_n,
               "transfer"_n,
               make_tuple(get_self(), owner, to_owner_lem, string("claim lem")))
            .send();
        stakingIdx.modify(existing_staking, same_payer, [&](staking &a) {
            a.last_claim_lem_reward = rewards_end_time;
            a.lem_rewards += to_owner_lem;
        });
    }
}

void lemonade::changeyield(const name &owner, const name &product_name,
                           const double &yield, const string &memo) {
    require_auth(get_self());

    products products_table(get_self(), get_self().value);
    auto productIdx = products_table.get_index<eosio::name("byname")>();
    auto existing_product = productIdx.find(product_name.value);
    check(existing_product != productIdx.end(), "product does not exist");

    stakings stakings_table(get_self(), owner.value);
    auto stakingIdx = stakings_table.get_index<eosio::name("byproductid")>();
    auto existing_staking = stakingIdx.find(existing_product->id);
    check(existing_staking != stakingIdx.end(), "owner does not has product");

    check(yield >= existing_product->minimum_yield &&
              yield <= existing_product->maximum_yield,
          "exceed product yield range");

    stakingIdx.modify(existing_staking, same_payer,
                      [&](staking &a) { a.current_yield = yield; });
}

void lemonade::createbet(const uint32_t &started_at,
                         const uint32_t &betting_ended_at,
                         const uint32_t &ended_at) {
    require_auth(get_self());
    bettings bettings_table(get_self(), get_self().value);

    asset zero = asset(0, symbol("LED", 4));

    auto current = now();
    check(started_at >= 0 && betting_ended_at >= 0 && ended_at >= 0,
          "timestamp must be a positive integer");
    check(started_at <= betting_ended_at && betting_ended_at <= ended_at,
          "timestamp is not correct");
    check(current <= started_at, "start time must greater than now");

    bettings_table.emplace(get_self(), [&](betting &a) {
        a.id = bettings_table.available_primary_key();
        a.short_betting_amount = zero;
        a.long_betting_amount = zero;
        a.short_dividend = 0;
        a.long_dividend = 0;
        a.started_at = started_at;
        a.betting_ended_at = betting_ended_at;
        a.ended_at = ended_at;
        a.base_price = 0;
        a.final_price = 0;
        a.status = Status::NOT_STARTED;
    });
}

void lemonade::rmbet(const uint64_t bet_id) {
    require_auth(get_self());

    bettings bettings_table(get_self(), get_self().value);
    auto existing_betting = bettings_table.find(bet_id);
    check(existing_betting != bettings_table.end(), "game does not exist");

    check(existing_betting->status == Status::FINISHED, "game not finished");

    bettings_table.erase(existing_betting);
}

void lemonade::setbet(const uint64_t bet_id, const uint8_t &status) {
    require_auth(get_self());

    configs config_table(get_self(), get_self().value);
    auto existing_config = config_table.find(0);
    check(existing_config != config_table.end(), "contract not initialized");

    bettings bettings_table(get_self(), get_self().value);
    auto existing_betting = bettings_table.find(bet_id);
    check(existing_betting != bettings_table.end(), "game does not exist");
    check(status != Status::NOT_STARTED && status != Status::FINISHED,
          "you give wrong status");

    if (status == Status::IS_LIVE) {
        check(existing_betting->get_status() == Status::NOT_STARTED,
              "game status has wrong value");
        check(existing_betting->started_at <= now(),
              "the start time has not passed.");

        double base = existing_config->btc_price;

        bettings_table.modify(existing_betting, same_payer, [&](betting &a) {
            a.status = status;
            a.base_price = base;
        });
    }
    if (status == Status::BETTING_FINISH) {
        check(existing_betting->get_status() == Status::IS_LIVE,
              "game status has wrong value");
        check(existing_betting->betting_ended_at <= now(),
              "the betting end time has not passed.");
        // Betting one side only -> refund them all
        if (existing_betting->long_betters.size() == 0 ||
            existing_betting->short_betters.size() == 0) {
            if (existing_betting->long_betters.size() == 0) {
                for (auto k : existing_betting->short_betters) {
                    action(permission_level{get_self(), "active"_n},
                           "led.token"_n, "transfer"_n,
                           make_tuple(get_self(), k.first, k.second,
                                      string("refund ") + to_string(bet_id) +
                                          string("game, game not started")))
                        .send();
                }
            }
            if (existing_betting->short_betters.size() == 0) {
                for (auto k : existing_betting->long_betters) {
                    action(permission_level{get_self(), "active"_n},
                           "led.token"_n, "transfer"_n,
                           make_tuple(get_self(), k.first, k.second,
                                      string("refund ") + to_string(bet_id) +
                                          string("game, game not started")))
                        .send();
                }
            }
            bettings_table.modify(
                existing_betting, same_payer, [&](betting &a) {
                    a.status = Status::NO_GAME;
                    a.final_price = existing_config->btc_price;
                });
        } else {
            bettings_table.modify(existing_betting, same_payer,
                                  [&](betting &a) { a.status = status; });
        }
    }
    if (status == Status::NOT_CLAIMED) {
        check(existing_betting->get_status() == Status::BETTING_FINISH,
              "game status has wrong value");
        check(existing_betting->ended_at <= now(),
              "the end time has not passed.");

        double finalPrice = existing_config->btc_price;

        bettings_table.modify(existing_betting, same_payer, [&](betting &a) {
            a.status = status;
            a.final_price = finalPrice;
        });
    }
}

void lemonade::bet(const name &owner, const asset &quantity,
                   const uint64_t &bet_id, const string &position) {
    require_auth(owner);

    bettings bettings_table(get_self(), get_self().value);
    auto existing_betting = bettings_table.find(bet_id);
    check(existing_betting != bettings_table.end(), "game does not exist");

    check(position == "long" || position == "short",
          "you must bet long or short position");

    asset new_short_amount = existing_betting->short_betting_amount;
    asset new_long_amount = existing_betting->long_betting_amount;

    asset zero = asset(0, symbol("LED", 4));

    double short_dividend = 0;
    double long_dividend = 0;

    check(!existing_betting->long_better_exists(owner) &&
              !existing_betting->short_better_exists(owner),
          "you already betted");

    check(existing_betting->get_status() == Status::IS_LIVE,
          "game is not lived");
    check(existing_betting->betting_ended_at >= now(), "betting time is over");

    const double betting_ratio = 0.95;

    if (position == "long") {
        new_long_amount += quantity;
        if (existing_betting->short_betting_amount.amount != 0) {
            short_dividend =
                ((double)(new_short_amount.amount + new_long_amount.amount) /
                 (double)new_short_amount.amount * betting_ratio);
        }
        long_dividend =
            ((double)(new_short_amount.amount + new_long_amount.amount) /
             (double)new_long_amount.amount * betting_ratio);

        bettings_table.modify(existing_betting, same_payer, [&](betting &a) {
            a.short_betting_amount = new_short_amount;
            a.long_betting_amount = new_long_amount;
            a.long_betters.push_back({owner, quantity});
            a.short_dividend = short_dividend;
            a.long_dividend = long_dividend;
        });
    }
    if (position == "short") {
        new_short_amount += quantity;
        short_dividend =
            ((double)(new_short_amount.amount + new_long_amount.amount) /
             (double)new_short_amount.amount * betting_ratio);
        if (existing_betting->long_betting_amount.amount != 0) {
            long_dividend =
                ((double)(new_short_amount.amount + new_long_amount.amount) /
                 (double)new_long_amount.amount * betting_ratio);
        }
        bettings_table.modify(existing_betting, same_payer, [&](betting &a) {
            a.short_betting_amount = new_short_amount;
            a.long_betting_amount = new_long_amount;
            a.short_betters.push_back({owner, quantity});
            a.short_dividend = short_dividend;
            a.long_dividend = long_dividend;
        });
    }
}

void lemonade::claimbet(const uint64_t &bet_id) {
    require_auth(get_self());
    const double betting_ratio = 0.95;

    bettings bettings_table(get_self(), get_self().value);
    auto existing_betting = bettings_table.find(bet_id);
    check(existing_betting != bettings_table.end(), "game does not exist");

    check(existing_betting->get_status() == Status::NOT_CLAIMED,
          "game is not finished");
    check(existing_betting->ended_at <= now(), "betting is not over");
    check(existing_betting->long_betters.size() != 0 &&
              existing_betting->short_betters.size() != 0,
          "no betters");

    // Betting both side -> claim for winner
    {
        uint64_t winners_total;
        uint64_t losers_total;
        vector<pair<name, asset>> winners;
        double dividend;
        string win_position =
            existing_betting->base_price >= existing_betting->final_price
                ? "short"
                : "long";

        if (win_position == "long") {
            winners = existing_betting->long_betters;
            dividend = existing_betting->long_dividend;
            winners_total = existing_betting->long_betting_amount.amount;
            losers_total = existing_betting->short_betting_amount.amount;
        }
        if (win_position == "short") {
            winners = existing_betting->short_betters;
            dividend = existing_betting->short_dividend;
            losers_total = existing_betting->long_betting_amount.amount;
            winners_total = existing_betting->short_betting_amount.amount;
        }

        for (auto k : winners) {
            const asset price =
                asset(k.second.amount + k.second.amount * losers_total /
                                            winners_total * betting_ratio,
                      k.second.symbol);
            if (price.amount == 0) {
                continue;
            }
            action(permission_level{get_self(), "active"_n}, "led.token"_n,
                   "transfer"_n,
                   make_tuple(get_self(), k.first, price,
                              string("winner of ") + to_string(bet_id) +
                                  string("game!")))
                .send();
        }
        bettings_table.modify(existing_betting, same_payer,
                              [&](betting &a) { a.status = Status::FINISHED; });
    }
}

void lemonade::issue_lem() {
    configs config_table(get_self(), get_self().value);
    auto existing_config = config_table.find(0);
    check(existing_config != config_table.end(), "contract not initialized");

    if (existing_config->half_life_count == 3) {
        return;
    }

    const auto current = now();
    auto half_life = existing_config->half_life_count;
    auto last_issued = existing_config->last_lem_bucket_fill;
    uint32_t secs_since_last_fill = 0;

    for (int i = half_life; i < 3; i++) {
        if (existing_config->last_half_life_updated[i] <= current &&
            current < existing_config->last_half_life_updated[i + 1]) {
            secs_since_last_fill +=
                ((current - last_issued) / (uint32_t)pow(2, i));
            half_life = i;
        } else if (existing_config->last_half_life_updated[i + 1] <= current) {
            secs_since_last_fill +=
                ((existing_config->last_half_life_updated[i + 1] -
                  last_issued) /
                 (uint32_t)pow(2, i));
            last_issued = existing_config->last_half_life_updated[i + 1];
            half_life = i + 1;
        }
    }

    asset new_token = asset(secs_since_last_fill * 2'0000, symbol("LEM", 4));
    led_token_stats stats_table(led_token_contract,
                                symbol("LEM", 4).code().raw());
    auto statsIdx = stats_table.get_index<eosio::name("byissuer")>();
    auto existing_stats = statsIdx.find(get_self().value);
    check(existing_stats != statsIdx.end(), "lem token not created");

    if (new_token.amount + existing_stats->supply.amount > LEM_MAX) {
        new_token =
            asset(LEM_MAX - existing_stats->supply.amount, symbol("LEM", 4));
    }
    print("existing_stats: ", existing_stats->supply.amount, "\n");

    if (new_token.amount > 0) {
        action(permission_level{get_self(), "active"_n}, "led.token"_n,
               "issue"_n, make_tuple(get_self(), new_token, string("issue")))
            .send();

        config_table.modify(existing_config, same_payer, [&](config &a) {
            a.half_life_count = half_life;
            a.last_lem_bucket_fill = current;
        });

        asset to_rc_reward = asset(new_token.amount * 0.3, symbol("LEM", 4));
        asset to_marketing = asset(new_token.amount * 0.1, symbol("LEM", 4));
        asset to_team = asset(new_token.amount * 0.05, symbol("LEM", 4));
        asset to_reserve = asset(new_token.amount * 0.05, symbol("LEM", 4));

        if (to_rc_reward.amount > 0) {
            action(permission_level{get_self(), "active"_n}, "led.token"_n,
                   "transfer"_n,
                   make_tuple(get_self(), rc_reward_account, to_rc_reward,
                              string("fill bucket to rc reward")))
                .send();
        }

        if (to_marketing.amount > 0) {
            action(permission_level{get_self(), "active"_n}, "led.token"_n,
                   "transfer"_n,
                   make_tuple(get_self(), marketing_account, to_marketing,
                              string("fill bucket to marketing")))
                .send();
        }
        if (to_team.amount > 0) {
            action(permission_level{get_self(), "active"_n}, "led.token"_n,
                   "transfer"_n,
                   make_tuple(get_self(), team_account, to_team,
                              string("fill bucket to team")))
                .send();
        }
        if (to_reserve.amount > 0) {
            action(permission_level{get_self(), "active"_n}, "led.token"_n,
                   "transfer"_n,
                   make_tuple(get_self(), reserved_account, to_reserve,
                              string("fill bucket to reserve")))
                .send();
        }
    }
}

uint32_t lemonade::now() {
    return (uint32_t)(eosio::current_time_point().sec_since_epoch());
}

void lemonade::transfer_event(const name &from, const name &to,
                              const asset &quantity, const string &memo) {
    if (to != get_self()) return;
    if (from == get_self()) return;
    auto incoming = extended_asset{quantity, get_first_receiver()};

    vector<string> event = memoParser(memo);
    name position = name("none");
    if (event[0] == "stake") {
        if (event.size() >= 3) {
            position = name(event[2]);
        }
        stake(from, quantity, name(event[1]), position);
    }
    if (event[0] == "bet") {
        uint64_t bet_id = stoull(event[1]);
        bet(from, quantity, bet_id, event[2]);
    }
    if (event[0] == "deposit") {
        check(is_dex_open, "DEX is not open");
        add_signed_ext_balance(from, incoming);
    }
    if (event[0] == "exchange") {
        check(is_dex_open, "DEX is not open");
        if (event.size() >= 3) {
            symbol_code pair_token = symbol_code(event[1]);
            asset min_expected = asset_from_string(event[2]);
            memoexchange(from, incoming, pair_token, min_expected);
        }
    }
}

vector<string> lemonade::memoParser(const string &memo) {
    vector<string> result;

    size_t prev = memo.find('/');
    result.push_back(memo.substr(0, prev));
    size_t pos = prev;
    while (true) {
        pos = memo.find('/', pos + 1);
        if (pos == std::string::npos) {
            result.push_back(memo.substr(prev + 1));
            break;
        } else {
            result.push_back(memo.substr(prev + 1, (pos - (prev + 1))));
        }
        prev = pos;
    }

    return result;
}