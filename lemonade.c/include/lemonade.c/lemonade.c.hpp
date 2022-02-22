#pragma once

#include <eosio/asset.hpp>
#include <eosio/binary_extension.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/time.hpp>
#include <string>
#include <vector>

#include "config.hpp"

using namespace eosio;
using namespace std;
using namespace config;

using std::string;
using std::vector;
typedef uint64_t id_type;

class [[eosio::contract("lemonade.c")]] lemonade : public contract {
private:
  const uint64_t delay_transfer_sec = 1;
  const double lem_reward_rate = 0.001 * 24; // Reward per day

  const enum Status {
    NOT_STARTED,
    IS_LIVE,
    BETTING_FINISH,
    NOT_CLAIMED,
    FINISHED
  };

  struct [[eosio::table]] config {
    id_type id;
    bool is_active;
    uint32_t last_lem_bucket_fill;
    vector<uint32_t> last_half_life_updated;
    uint32_t half_life_count;
    double btc_price;

    uint64_t primary_key() const { return id; }
  };

  struct [[eosio::table]] product {
    id_type id;
    name name;
    uint32_t
        duration; // (uint32_t) (eosio::current_time_point().sec_since_epoch());
    asset current_amount;
    asset amount_per_account;
    asset maximum_amount_limit;
    double minimum_yield;
    double maximum_yield;
    bool has_lem_rewards;
    bool has_prediction;
    vector<eosio::name> buyers;

    uint64_t primary_key() const { return id; }
    uint64_t get_name() const { return name.value; }
    bool buyer_exists(const eosio::name owner) const {
      return find_if(buyers.begin(), buyers.end(),
                     [&owner](const eosio::name &p) { return p == owner; }) !=
             buyers.end();
    }
    bool buyer_count() const { return buyers.size(); }
  };

  struct [[eosio::table]] account {
    id_type id;
    asset balance;
    id_type product_id;
    double current_yield;
    name price_prediction;
    double base_price;
    uint32_t started_at; // (uint32_t)
                         // (eosio::current_time_point().sec_since_epoch());
    uint32_t
        ended_at; // (uint32_t) (eosio::current_time_point().sec_since_epoch());
    uint32_t last_claim_led_reward;
    uint32_t last_claim_lem_reward;
    asset lem_rewards;
    asset led_rewards;

    uint64_t primary_key() const { return id; }
    uint64_t get_product_id() const { return product_id; }
  };

  struct [[eosio::table]] betting {
    id_type id;
    double base_price;
    double final_price;
    vector<pair<name, asset>> short_betters;
    vector<pair<name, asset>> long_betters;
    asset short_betting_amount;
    asset long_betting_amount;
    double short_dividend;
    double long_dividend;
    uint32_t started_at;
    uint32_t betting_ended_at;
    uint32_t ended_at;
    uint8_t status;

    uint64_t primary_key() const { return id; }
    double get_base_price() const { return base_price; }
    double get_short_dividend() const { return short_dividend; }
    double get_long_dividend() const { return long_dividend; }
    bool short_better_exists(const name owner) const {
      return find_if(short_betters.begin(), short_betters.end(),
                     [&owner](const pair<name, asset> &p) {
                       return p.first == owner;
                     }) != short_betters.end();
    }
    bool long_better_exists(const name owner) const {
      return find_if(long_betters.begin(), long_betters.end(),
                     [&owner](const pair<name, asset> &p) {
                       return p.first == owner;
                     }) != long_betters.end();
    }
    uint8_t get_status() const { return status; }
  };

  typedef eosio::multi_index<"configs"_n, config> configs;

  typedef eosio::multi_index<
      "products"_n, product,
      indexed_by<"byname"_n,
                 const_mem_fun<product, uint64_t, &product::get_name>>>
      products;

  typedef eosio::multi_index<
      "accounts"_n, account,
      indexed_by<"byproductid"_n,
                 const_mem_fun<account, uint64_t, &account::get_product_id>>>
      accounts;

  typedef eosio::multi_index<"bettings"_n, betting> bettings;

  uint32_t now();
  vector<string> memoParser(const string &memo);
  void issue_lem();
  void stake(const name &owner, const asset &quantity, const name &product_name,
             const optional<name> &price_prediction);
  void bet(const name &owner, const asset &quantity, const uint64_t &bet_id,
           const string &position);

  template <typename T> void cleanTable(name self, uint64_t scope = 0) {
    uint64_t s = scope ? scope : self.value;
    T db(self, s);
    while (db.begin() != db.end()) {
      auto itr = --db.end();
      db.erase(itr);
    }
  };

public:
  using contract::contract;
  lemonade(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds) {}

  // [[eosio::action]] void clearproduct() {
  //   require_auth(get_self());
  //   printl("cleaning", 8);

  //   cleanTable<products>(get_self(), get_self().value);
  // }

  // [[eosio::action]] void clearaccount(name account) {
  //   require_auth(get_self());
  //   printl("cleaning", 8);

  //   cleanTable<accounts>(get_self(), account.value);
  // }

  // [[eosio::action]] void clearbetting() {
  //   require_auth(get_self());
  //   printl("cleaning", 8);

  //   cleanTable<bettings>(get_self(), get_self().value);
  // }

  [[eosio::action]] void clearconfig() {
    require_auth(get_self());
    printl("cleaning", 8);

    cleanTable<configs>(get_self(), get_self().value);
  }

  [[eosio::action]] void init();

  [[eosio::action]] void setbtcprice(const double &price);

  [[eosio::action]] void addproduct(
      const name &product_name, const double &minimum_yield,
      const double &maximum_yield, const bool &has_lem_rewards,
      const bool &has_prediction, const asset &amount_per_account,
      const optional<asset> &maximum_amount_limit,
      const optional<uint32_t> &duration);

  [[eosio::action]] void rmproduct(const name &product_name);

  [[eosio::action]] void unstake(const name &owner, const name &product_name);

  [[eosio::action]] void claimled(const name &owner, const name &product_name);

  [[eosio::action]] void claimlem(const name &owner, const name &product_name);

  [[eosio::action]] void changeyield(const name &owner,
                                     const name &product_name,
                                     const double &yield, const string &memo);

  [[eosio::action]] void createbet(const uint32_t &started_at,
                                   const uint32_t &betting_ended_at,
                                   const uint32_t &ended_at);

  [[eosio::action]] void setbet(const uint64_t bet_id, const uint8_t &status,
                                const optional<double> &base_price,
                                const optional<double> &final_price);

  [[eosio::action]] void rmbet(const uint64_t bet_id);

  [[eosio::action]] void claimbet(const uint64_t &bet_id);

  [[eosio::on_notify("*::transfer")]] void transfer_event(
      const name &from, const name &to, const asset &quantity,
      const string &memo);

  using init_action = eosio::action_wrapper<"init"_n, &lemonade::init>;
  using setbtcprice_action = eosio::action_wrapper<"setbtcprice"_n, &lemonade::setbtcprice>;
  using addproduct_action =
      eosio::action_wrapper<"addproduct"_n, &lemonade::addproduct>;
  using rmproduct_action =
      eosio::action_wrapper<"rmproduct"_n, &lemonade::rmproduct>;
  using unstake_action = eosio::action_wrapper<"unstake"_n, &lemonade::unstake>;
  using claimled_action =
      eosio::action_wrapper<"claimled"_n, &lemonade::claimled>;
  using claimlem_action =
      eosio::action_wrapper<"claimlem"_n, &lemonade::claimlem>;
  using changeyield_action =
      eosio::action_wrapper<"changeyield"_n, &lemonade::changeyield>;
  using createbet_action =
      eosio::action_wrapper<"createbet"_n, &lemonade::createbet>;
  using rmbet_action = eosio::action_wrapper<"rmbet"_n, &lemonade::rmbet>;
  using setbet_action = eosio::action_wrapper<"setbet"_n, &lemonade::setbet>;
  using claimbet_action =
      eosio::action_wrapper<"claimbet"_n, &lemonade::claimbet>;
};
