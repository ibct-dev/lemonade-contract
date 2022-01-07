#include "lemonade.c/lemonade.c.hpp"

void lemonade::addproduct(const name &product_name, const double &minimum_yield,
                          const double &maximum_yield,
                          const asset &amount_per_account,
                          const optional<asset> &maximum_amount_limit,
                          const optional<uint32_t> &duration) {
  require_auth(get_self());

  products products_table(get_self(), get_self().value);
  auto productIdx = products_table.get_index<eosio::name("byname")>();
  auto existing_product = productIdx.find(product_name.value);
  check(existing_product == productIdx.end(), "product name already exist");

  check(minimum_yield <= maximum_yield, "maximum yield must be same or bigger "
                                        "then minimum yield");

  asset zero;
  zero.amount = 0;
  zero.symbol = amount_per_account.symbol;

  asset limit = zero;
  uint32_t dur = 0;

  if (maximum_amount_limit.has_value()) {
    check(amount_per_account.amount <= maximum_amount_limit->amount,
          "account limit must be smaller than total limit");
    check(amount_per_account.symbol == maximum_amount_limit->symbol,
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
                     const name &product_name, const optional<name> &betting) {
  require_auth(owner);

  check(quantity.is_valid(), "invalid quantity");
  check(quantity.amount > 0, "must transfer positive quantity");

  products products_table(get_self(), get_self().value);
  auto productIdx = products_table.get_index<eosio::name("byname")>();
  auto existing_product = productIdx.find(product_name.value);
  check(existing_product != productIdx.end(), "product does not exist");

  accounts accounts_table(get_self(), owner.value);
  auto accountIdx = accounts_table.get_index<eosio::name("byproductid")>();
  auto existing_account = accountIdx.find(existing_product->id);
  check(existing_account == accountIdx.end(), "already has same product");

  if (existing_product->amount_per_account.amount != 0) {
    check(existing_product->amount_per_account.amount >= quantity.amount,
          "exceed amount per account limits");
  }

  if (existing_product->maximum_amount_limit.amount != 0) {
    check(existing_product->maximum_amount_limit.amount >=
              (existing_product->current_amount.amount + quantity.amount),
          "exceed product total amount limits");
  }

  uint32_t started_at = now();
  uint32_t ended_at = 0;
  if (existing_product->duration != 0) {
    ended_at = now() + existing_product->duration;
  }
  name status = "none"_n;
  if (betting.has_value()) {
    status = betting.value();
  }

  accounts_table.emplace(get_self(), [&](account &a) {
    a.id = accounts_table.available_primary_key();
    a.balance = quantity;
    a.product_id = existing_product->id;
    a.current_yield = existing_product->minimum_yield;
    a.betting = status;
    a.started_at = started_at;
    a.ended_at = ended_at;
  });

  productIdx.modify(existing_product, same_payer,
                    [&](product &a) { a.current_amount += quantity; });
}

void lemonade::unstake(const name &owner, const name &product_name) {
  require_auth(owner);

  products products_table(get_self(), get_self().value);
  auto productIdx = products_table.get_index<eosio::name("byname")>();
  auto existing_product = productIdx.find(product_name.value);
  check(existing_product != productIdx.end(), "product does not exist");

  accounts accounts_table(get_self(), owner.value);
  auto accountIdx = accounts_table.get_index<eosio::name("byproductid")>();
  auto existing_account = accountIdx.find(existing_product->id);
  check(existing_account != accountIdx.end(), "owner does not has product");

  if (existing_account->ended_at != 0) {
    check(existing_account->ended_at <= now(), "account end time is not over");
  }

  asset toUnstake = existing_account->balance;

  action(permission_level{get_self(), "active"_n}, "led.token"_n, "transfer"_n,
         make_tuple(get_self(), owner, toUnstake, string("unstake")))
      .send();

  productIdx.modify(existing_product, same_payer, [&](product &a) {
    a.current_amount -= existing_account->balance;
  });

  accountIdx.erase(existing_account);
}

void lemonade::changeyield(const name &owner, const name &product_name,
                           const double &yield, const string &memo) {
  require_auth(get_self());

  products products_table(get_self(), get_self().value);
  auto productIdx = products_table.get_index<eosio::name("byname")>();
  auto existing_product = productIdx.find(product_name.value);
  check(existing_product != productIdx.end(), "product does not exist");

  accounts accounts_table(get_self(), owner.value);
  auto accountIdx = accounts_table.get_index<eosio::name("byproductid")>();
  auto existing_account = accountIdx.find(existing_product->id);
  check(existing_account != accountIdx.end(), "owner does not has product");

  check(yield >= existing_product->minimum_yield &&
            yield <= existing_product->maximum_yield,
        "exceed product yield range");

  accountIdx.modify(existing_account, same_payer,
                    [&](account &a) { a.current_yield = yield; });
}

void lemonade::createbet(const uint32_t &started_at,
                         const uint32_t &betting_ended_at,
                         const uint32_t &ended_at) {
  require_auth(get_self());
  bettings bettings_table(get_self(), get_self().value);

  asset zero;
  zero.amount = 0;
  zero.symbol = symbol("LED", 4);

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
    a.status = Status::NOT_STARTED;
  });
}

void lemonade::rmbet(const uint64_t bet_id) {
  require_auth(get_self());

  bettings bettings_table(get_self(), get_self().value);
  auto existing_betting = bettings_table.find(bet_id);
  check(existing_betting != bettings_table.end(), "game does not exist");

  check(existing_betting->short_betters.size() == 0 &&
            existing_betting->long_betters.size() == 0,
        "betters are exists");

  bettings_table.erase(existing_betting);
}

void lemonade::setbet(const uint64_t bet_id, const uint8_t &status,
                      const optional<double> &base_price) {
  require_auth(get_self());

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
    check(base_price.has_value(),
          "when you change status to live, you must give base_price");
  }

  double base = existing_betting->base_price;
  if (status == Status::IS_LIVE && base_price.has_value()) {
    base = base_price.value();
  }

  bettings_table.modify(existing_betting, same_payer, [&](betting &a) {
    a.status = status;
    a.base_price = base;
  });
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

  asset zero;
  zero.amount = 0;
  zero.symbol = symbol("LED", 4);

  double short_dividend = 0;
  double long_dividend = 0;

  check(!existing_betting->long_better_exists(owner) &&
            !existing_betting->short_better_exists(owner),
        "you already betted");

  check(existing_betting->get_status() == Status::IS_LIVE, "game is not lived");
  check(existing_betting->betting_ended_at >= now(), "betting time is over");

  const double betting_ratio = 0.95;

  if (position == "long") {
    new_long_amount += quantity;
    if (existing_betting->short_betting_amount.amount != 0) {
      short_dividend = ((new_short_amount.amount + new_long_amount.amount) /
                        new_short_amount.amount * betting_ratio);
    }
    long_dividend = ((new_short_amount.amount + new_long_amount.amount) /
                     new_long_amount.amount * betting_ratio);

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
    short_dividend = ((new_short_amount.amount + new_long_amount.amount) /
                      new_short_amount.amount * betting_ratio);
    if (existing_betting->long_betting_amount.amount != 0) {
      long_dividend = ((new_short_amount.amount + new_long_amount.amount) /
                       new_long_amount.amount * betting_ratio);
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

void lemonade::claimbet(const uint64_t &bet_id, const string &win_position) {
  require_auth(get_self());

  bettings bettings_table(get_self(), get_self().value);
  auto existing_betting = bettings_table.find(bet_id);
  check(existing_betting != bettings_table.end(), "game does not exist");

  check(win_position == "long" || win_position == "short",
        "must choose long or short position");

  check(existing_betting->get_status() == Status::IS_LIVE, "game is not lived");
  check(existing_betting->ended_at <= now(), "betting is not over");

  vector<pair<name, asset>> winners;
  double dividend;

  if (win_position == "long") {
    winners = existing_betting->long_betters;
    dividend = existing_betting->long_dividend;
  }
  if (win_position == "short") {
    winners = existing_betting->short_betters;
    dividend = existing_betting->short_dividend;
  }

  for (auto k : winners) {
    const asset price = asset(k.second.amount * dividend, k.second.symbol);
    if (price.amount == 0) {
      continue;
    }
    action(
        permission_level{get_self(), "active"_n}, "led.token"_n, "transfer"_n,
        make_tuple(get_self(), k.first, price,
                   string("winner of ") + to_string(bet_id) + string("game!")))
        .send();
  }

  bettings_table.modify(existing_betting, same_payer,
                        [&](betting &a) { a.status = Status::FINISHED; });
}

uint32_t lemonade::now() {
  return (uint32_t)(eosio::current_time_point().sec_since_epoch());
}

void lemonade::transfer_event(const name &from, const name &to,
                              const asset &quantity, const string &memo) {
  if (to != get_self())
    return;

  vector<string> event = memoParser(memo);
  if (event[0] == "staking") {
    stake(from, quantity, name(event[1]), name(event[2]));
  }
  if (event[0] == "bet") {
    uint64_t bet_id = stoull(event[1]);
    bet(from, quantity, bet_id, event[2]);
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
