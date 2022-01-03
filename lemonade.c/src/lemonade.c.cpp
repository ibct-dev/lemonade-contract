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

  accounts accounts_table(owner, owner.value);
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
  name status = ""_n;
  if (betting.has_value()) {
    status = betting.value();
  }

  action(permission_level{owner, "active"_n}, "led.token"_n, "transfer"_n,
         make_tuple(owner, get_self(), quantity, string("stake")))
      .send();

  accounts_table.emplace(owner, [&](account &a) {
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

  accounts accounts_table(owner, owner.value);
  auto accountIdx = accounts_table.get_index<eosio::name("byproductid")>();
  auto existing_account = accountIdx.find(existing_product->id);
  check(existing_account != accountIdx.end(), "owner does not has product");

  if (existing_account->ended_at != 0) {
    check(existing_account->ended_at <= now(), "account end time is not over");
  }

  asset toUnstake = asset(existing_account->balance.amount *
                              log(existing_account->current_yield),
                          existing_product->amount_per_account.symbol);

  action(permission_level{get_self(), "active"_n}, "led.token"_n, "transfer"_n,
         make_tuple(get_self(), owner, toUnstake, string("unstake")))
      .send();

  productIdx.modify(existing_product, same_payer, [&](product &a) {
    a.current_amount -= existing_account->balance;
  });

  accountIdx.erase(existing_account);
}

void lemonade::changeyield(const name &owner, const name &product_name,
                           const double &yield) {
  require_auth(get_self());

  products products_table(get_self(), get_self().value);
  auto productIdx = products_table.get_index<eosio::name("byname")>();
  auto existing_product = productIdx.find(product_name.value);
  check(existing_product != productIdx.end(), "product does not exist");

  accounts accounts_table(owner, owner.value);
  auto accountIdx = accounts_table.get_index<eosio::name("byproductid")>();
  auto existing_account = accountIdx.find(existing_product->id);
  check(existing_account != accountIdx.end(), "owner does not has product");

  accountIdx.modify(existing_account, same_payer,
                    [&](account &a) { a.current_yield = yield; });
}

uint32_t lemonade::now() {
  return (uint32_t)(eosio::current_time_point().sec_since_epoch());
}

// void lemonade::transfer_event(const name &from, const name &to,
//                               const asset &quantity, const string &memo) {
//   if (to != get_self())
//     return;

//   vector<string> event = memoParser(memo);

//   stake(from, quantity, name(event[0]), name(event[1]));
// }

// vector<string> lemonade::memoParser(const string &memo) {
//   vector<string> result;

//   size_t prev = memo.find(':');
//   result.push_back(memo.substr(0, prev));
//   size_t pos = prev;
//   while (true) {
//     pos = memo.find(':', pos + 1);
//     if (pos == std::string::npos) {
//       result.push_back(memo.substr(prev + 1));
//       break;
//     } else {
//       result.push_back(memo.substr(prev + 1, (pos - (prev + 1))));
//     }
//     prev = pos;
//   }

//   return result;
// }
