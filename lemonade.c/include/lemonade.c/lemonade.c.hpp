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
    const name feeAccount = "fee.rlc"_n;
    const name ledgisAccount = "income.rlc"_n;
    const uint32_t minAuctionPendingTime = 2 * config::secondsPerWeek;

    enum class SettleType { AUCTION, SALE };
    const vector<pair<string, double>> settleData = {
        {"auction", 0.05},
        {"sale", 0.05},
    };
    const vector<name> valid_code = {"realcollectn", "led.token"};

    struct [[eosio::table]] account {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;
    
    template <typename T>
    void cleanTable(name self, uint64_t scope = 0) {
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

    [[eosio::action]] void auction(
        const name &owner, const NftGuid &nft, const extended_asset &price, 
        const double &service_fee, const double &ledgis_fee,  
        const uint32_t &exStartDate, const uint32_t &auStartDate,
        const uint32_t &auEndDate);

    using auction_action =
        eosio::action_wrapper<"auction"_n, &lemonade::auction>;
};
