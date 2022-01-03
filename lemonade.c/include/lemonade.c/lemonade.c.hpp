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

using std::string;
using std::vector;
typedef uint64_t id_type;

class [[eosio::contract("lemonade.c")]] lemonade : public contract {
private:
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

    uint64_t primary_key() const { return id; }
    uint64_t get_name() const { return name.value; }
  };

  struct [[eosio::table]] account {
    id_type id;
    asset balance;
    id_type product_id;
    double current_yield;
    uint32_t started_at; // (uint32_t)
                         // (eosio::current_time_point().sec_since_epoch());
    uint32_t
        ended_at; // (uint32_t) (eosio::current_time_point().sec_since_epoch());

    uint64_t primary_key() const { return id; }
    uint64_t get_product_id() const { return product_id; }
  };

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

  void stake(const name &owner, const asset &quantity,
             const name &product_name);
  uint32_t now();
  vector<string> memoParser(const string &memo);

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

  [[eosio::action]] void addproduct(
      const name &product_name, const double &minimum_yield,
      const double &maximum_yield, const asset &amount_per_account,
      const optional<asset> &maximum_amount_limit,
      const optional<uint32_t> &duration);

  [[eosio::action]] void rmproduct(const name &product_name);

  [[eosio::action]] void unstake(const name &owner, const name &product_name);

  [[eosio::action]] void changeyield(
      const name &owner, const name &product_name, const double &yield);

  [[eosio::on_notify("*::transfer")]] void transfer_event(
      const name &from, const name &to, const asset &quantity,
      const string &memo);

  using addproduct_action =
      eosio::action_wrapper<"addproduct"_n, &lemonade::addproduct>;
  using rmproduct_action =
      eosio::action_wrapper<"rmproduct"_n, &lemonade::rmproduct>;
  using unstake_action = eosio::action_wrapper<"unstake"_n, &lemonade::unstake>;
  using changeyield_action =
      eosio::action_wrapper<"changeyield"_n, &lemonade::changeyield>;
};
