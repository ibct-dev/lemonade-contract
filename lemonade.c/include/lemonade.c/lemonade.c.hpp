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
    const double lem_reward_rate = 0.001;  // Reward per sec
    const double swap_fee = 0.003;         // 0.3%
    const int64_t MAX = eosio::asset::max_amount;
    const int64_t INIT_MAX = 1000000000000000;  // 10^15
    const int DEX_LEM_REWARD = 51840'0000;
    const name led_token_contract = "led.token"_n;
    const int64_t LEM_MAX = 220752000'0000;
    const name rc_reward_account = "rclemonade.p"_n;
    const name marketing_account = "malemonade.p"_n;
    const name team_account = "tmlemonade.p"_n;
    const name reserved_account = "rvlemonade.p"_n;
    const bool is_dex_open = false;

    const enum Status {
        NOT_STARTED,
        IS_LIVE,
        BETTING_FINISH,
        NOT_CLAIMED,
        FINISHED,
        NO_GAME
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

    struct [[eosio::table]] config2 {
        id_type id;
        name symbol;
        double price;
        uint64_t primary_key() const { return id; }
        uint64_t get_symbol() const { return symbol.value; }
    };

    struct [[eosio::table]] pool_reward {
        symbol_code pair_symbol_code;
        uint32_t last_reward;
        uint64_t primary_key() const { return pair_symbol_code.raw(); }
    };

    struct [[eosio::table]] product {
        id_type id;
        name name;
        uint32_t duration;  // (uint32_t)
                            // (eosio::current_time_point().sec_since_epoch());
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
                           [&owner](const eosio::name &p) {
                               return p == owner;
                           }) != buyers.end();
        }
        bool buyer_count() const { return buyers.size(); }
    };

    struct [[eosio::table]] staking {
        id_type id;
        asset balance;
        id_type product_id;
        double current_yield;
        name price_prediction;
        double base_price;
        uint32_t
            started_at;     // (uint32_t)
                            // (eosio::current_time_point().sec_since_epoch());
        uint32_t ended_at;  // (uint32_t)
                            // (eosio::current_time_point().sec_since_epoch());
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
        uint64_t get_status() const { return (uint64_t)status; }
    };

    struct [[eosio::table]] account {
        asset balance;
        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };

    struct [[eosio::table]] currency_stats {
        asset supply;
        asset max_supply;
        name issuer;
        extended_asset pool1;
        extended_asset pool2;
        extended_asset fee1;
        extended_asset fee2;
        extended_asset fee3;
        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };

    struct [[eosio::table]] user_info {
        name account;
        asset balance;
        uint64_t primary_key() const { return account.value; }
    };

    struct [[eosio::table]] swap_list {
        symbol pair_symbol;
        uint64_t primary_key() const { return pair_symbol.code().raw(); }
    };

    struct [[eosio::table]] pool_list {
        symbol lp_symbol;
        extended_symbol symbol1;
        extended_symbol symbol2;
        uint64_t primary_key() const { return lp_symbol.code().raw(); }
    };

    struct [[eosio::table]] frozen {
        name freezer;
        uint64_t primary_key() const { return freezer.value; }
    };

    struct [[eosio::table]] dexaccount {
        extended_asset balance;
        uint64_t id;
        uint64_t primary_key() const { return id; }
        uint128_t secondary_key() const {
            return make128key(balance.contract.value,
                              balance.quantity.symbol.raw());
        }
    };

    struct [[eosio::table]] index_struct {
        symbol lp_symbol;
        checksum256 id_256;
        uint64_t primary_key() const { return lp_symbol.code().raw(); }
        checksum256 secondary_key() const { return id_256; }
    };

    // Table from led.token
    struct [[eosio::table]] led_token_stat {
        asset supply;
        asset max_supply;
        name issuer;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
        uint64_t get_issuer() const { return issuer.value; }
    };

    typedef eosio::multi_index<"configs"_n, config> configs;
    typedef eosio::multi_index<
        "configs2"_n, config2,
        indexed_by<"bysymbol"_n,
                   const_mem_fun<config2, uint64_t, &config2::get_symbol>>>
        configs2;
    typedef eosio::multi_index<"poolrewards"_n, pool_reward> pool_rewards;

    typedef eosio::multi_index<
        "products"_n, product,
        indexed_by<"byname"_n,
                   const_mem_fun<product, uint64_t, &product::get_name>>>
        products;

    typedef eosio::multi_index<
        "stakings"_n, staking,
        indexed_by<"byproductid"_n,
                   const_mem_fun<staking, uint64_t, &staking::get_product_id>>>
        stakings;

    typedef eosio::multi_index<
        "bettings"_n, betting,
        indexed_by<"bystatus"_n,
                   const_mem_fun<betting, uint64_t, &betting::get_status>>>
        bettings;

    typedef eosio::multi_index<"stats"_n, currency_stats> stats;
    typedef eosio::multi_index<"userinfo"_n, user_info> user_infos;

    typedef eosio::multi_index<"swaplists"_n, swap_list> swap_lists;
    typedef eosio::multi_index<"poollists"_n, pool_list> pool_lists;

    typedef eosio::multi_index<"accounts"_n, account> accounts;
    typedef eosio::multi_index<"frozens"_n, frozen> frozens;
    typedef eosio::multi_index<
        "dexacnts"_n, dexaccount,
        indexed_by<"extended"_n, const_mem_fun<dexaccount, uint128_t,
                                               &dexaccount::secondary_key>>>
        dexacnts;
    typedef eosio::multi_index<
        "lemindexes"_n, index_struct,
        indexed_by<"extended"_n, const_mem_fun<index_struct, checksum256,
                                               &index_struct::secondary_key>>>
        lemindexes;

    // Table from led.token
    typedef eosio::multi_index<
        "stat"_n, led_token_stat,
        indexed_by<"byissuer"_n, const_mem_fun<led_token_stat, uint64_t,
                                               &led_token_stat::get_issuer>>>
        led_token_stats;

    uint32_t now();
    vector<string> memoParser(const string &memo);
    void issue_lem();
    void stake(const name &owner, const asset &quantity,
               const name &product_name,
               const optional<name> &price_prediction);
    void bet(const name &owner, const asset &quantity, const uint64_t &bet_id,
             const string &position);
    static uint128_t make128key(uint64_t a, uint64_t b);
    static checksum256 make256key(uint64_t a, uint64_t b, uint64_t c,
                                  uint64_t d);
    void add_balance(const name &owner, const asset &value,
                     const name &ram_payer);
    void sub_balance(const name &owner, const asset &value);
    void check_frozen(const name &account);
    void add_signed_ext_balance(const name &owner, const extended_asset &value);
    void add_signed_liq(name user, asset to_buy, bool is_buying,
                        asset max_asset1, asset max_asset2);
    int64_t compute(int64_t x, int64_t y, int64_t z);
    extended_asset process_exch(symbol_code evo_token, extended_asset paying,
                                asset min_expected);
    void memoexchange(name user, extended_asset ext_asset_in,
                      symbol_code pair_token, asset min_expected);
    void placeindex(name user, symbol lp_symbol, extended_asset pool1,
                    extended_asset pool2);

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

    // [[eosio::action]] void clearconfig() {
    //     require_auth(get_self());
    //     printl("cleaning", 8);

    //     cleanTable<configs>(get_self(), get_self().value);
    // }

    // [[eosio::action]] void clearconfig2() {
    //     require_auth(get_self());
    //     printl("cleaning", 8);

    //     cleanTable<configs2>(get_self(), get_self().value);
    // }
    // [[eosio::action]] void clear(string pair_token_symbol) {
    //     require_auth(get_self());
    //     printl("cleaning", 8);
    //     symbol_code pair_token = symbol_code(pair_token_symbol);

    //     cleanTable<stats>(get_self(), pair_token.raw());
    // }

    // [[eosio::action]] void test(name owner, string pair_token_symbol);
    [[eosio::action]] void rmpool(const name& user, const symbol& lp_symbol);

    // Initialize Actions
    [[eosio::action]] void init();

    [[eosio::action]] void issuelem();

    [[eosio::action]] void setbtcprice(const double &price);

    [[eosio::action]] void setledprice(const double &price);

    // Token Actions
    [[eosio::action]] void transfer(const name &from, const name &to,
                                    const asset &quantity, const string &memo);
    [[eosio::action]] void open(const name &owner, const symbol &symbol,
                                const name &ram_payer);
    [[eosio::action]] void close(const name &owner, const symbol &symbol);

    [[eosio::action]] void freeze(const name &account);

    [[eosio::action]] void unfreeze(const name &account);

    // Stake Actions
    [[eosio::action]] void addproduct(
        const name &product_name, const double &minimum_yield,
        const double &maximum_yield, const bool &has_lem_rewards,
        const bool &has_prediction, const asset &amount_per_account,
        const optional<asset> &maximum_amount_limit,
        const optional<uint32_t> &duration);

    [[eosio::action]] void rmproduct(const name &product_name);

    [[eosio::action]] void unstake(const name &owner, const name &product_name);

    [[eosio::action]] void claimled(const name &owner,
                                    const name &product_name);

    [[eosio::action]] void claimlem(const name &owner,
                                    const name &product_name);

    [[eosio::action]] void changeyield(const name &owner,
                                       const name &product_name,
                                       const double &yield, const string &memo);

    // Betting Actions
    [[eosio::action]] void createbet(const uint32_t &started_at,
                                     const uint32_t &betting_ended_at,
                                     const uint32_t &ended_at);

    [[eosio::action]] void setbet(const uint64_t bet_id, const uint8_t &status);

    [[eosio::action]] void rmbet(const uint64_t bet_id);

    [[eosio::action]] void claimbet(const uint64_t &bet_id);

    // DEX Actions
    [[eosio::action]] void inittoken(name user, symbol new_symbol,
                                     extended_asset initial_pool1,
                                     extended_asset initial_pool2);

    [[eosio::action]] void openext(const name &user,
                                   const extended_symbol &ext_symbol);
    [[eosio::action]] void closeext(const name &user, const name &to,
                                    const extended_symbol &ext_symbol,
                                    string memo);
    [[eosio::action]] void withdraw(name user, name to,
                                    extended_asset to_withdraw, string memo);
    [[eosio::action]] void addliquidity(name user, asset to_buy,
                                        asset max_asset1, asset max_asset2);
    [[eosio::action]] void rmliquidity(name user, asset to_sell,
                                       asset min_asset1, asset min_asset2);

    [[eosio::action]] void exchange(name user, symbol_code pair_token,
                                    extended_asset ext_asset_in,
                                    asset min_expected);

    [[eosio::action]] void exchangeall(name user, symbol_code pair_token,
                                       extended_symbol asset_in);

    [[eosio::action]] void indexpair(
        name user,
        symbol lp_symbol);  // This action is only temporarily useful

    [[eosio::action]] void clmpoolreward(string pair_token_symbol, double total,
                                         double pool);

    // On Notify
    [[eosio::on_notify("*::transfer")]] void transfer_event(
        const name &from, const name &to, const asset &quantity,
        const string &memo);

    using init_action = eosio::action_wrapper<"init"_n, &lemonade::init>;
    using setbtcprice_action =
        eosio::action_wrapper<"setbtcprice"_n, &lemonade::setbtcprice>;
    using addproduct_action =
        eosio::action_wrapper<"addproduct"_n, &lemonade::addproduct>;
    using rmproduct_action =
        eosio::action_wrapper<"rmproduct"_n, &lemonade::rmproduct>;
    using unstake_action =
        eosio::action_wrapper<"unstake"_n, &lemonade::unstake>;
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

    using transfer_action =
        eosio::action_wrapper<"transfer"_n, &lemonade::transfer>;
    using open_action = eosio::action_wrapper<"open"_n, &lemonade::open>;
    using close_action = eosio::action_wrapper<"close"_n, &lemonade::close>;
    using freeze_action = eosio::action_wrapper<"freeze"_n, &lemonade::freeze>;
    using unfreeze_action =
        eosio::action_wrapper<"unfreeze"_n, &lemonade::unfreeze>;
};
