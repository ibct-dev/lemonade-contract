#include "lemonade.c/lemonade.c.hpp"

void lemonade::openext( const name& user, const extended_symbol& ext_symbol) {
    require_auth( user );
    dexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find( 
      make128key(ext_symbol.get_contract().value, ext_symbol.get_symbol().raw()) );
    if( acnt_balance == index.end() ) {
        acnts.emplace( user, [&]( auto& a ){
            a.balance = extended_asset{0, ext_symbol};
            a.id = acnts.available_primary_key();
        });
    }
}

void lemonade::closeext( const name& user, const name& to, const extended_symbol& ext_symbol, string memo) {
    require_auth( user );
    dexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find( make128key(ext_symbol.get_contract().value, ext_symbol.get_symbol().raw()) );
    check( acnt_balance != index.end(), "User does not have such token" );
    auto ext_balance = acnt_balance->balance;
    if (ext_balance.quantity.amount > 0) {
        action(permission_level{ get_self(), "active"_n }, ext_balance.contract, "transfer"_n,
          std::make_tuple( get_self(), to, ext_balance.quantity, memo) ).send();
    }
    index.erase( acnt_balance );
}

void lemonade::withdraw(name user, name to, extended_asset to_withdraw, string memo){
    require_auth( user );
    check(to_withdraw.quantity.amount > 0, "quantity must be positive");
    add_signed_ext_balance(user, -to_withdraw);
    action(permission_level{ get_self(), "active"_n }, to_withdraw.contract, "transfer"_n,
      std::make_tuple( get_self(), to, to_withdraw.quantity, memo) ).send();
}

void lemonade::addliquidity(name user, asset to_buy, 
  asset max_asset1, asset max_asset2) {
    require_auth(user);
    check( (to_buy.amount > 0), "to_buy amount must be positive");
    check( (max_asset1.amount >= 0) && (max_asset2.amount >= 0), "assets must be nonnegative");
    add_signed_liq(user, to_buy, true, max_asset1, max_asset2);
}

void lemonade::remliquidity(name user, asset to_sell,
  asset min_asset1, asset min_asset2) {
    require_auth(user);
    check(to_sell.amount > 0, "to_sell amount must be positive");
    check( (min_asset1.amount >= 0) && (min_asset2.amount >= 0), "assets must be nonnegative");
    add_signed_liq(user, -to_sell, false, -min_asset1, -min_asset2);
}

void lemonade::exchange( name user, symbol_code pair_token, 
  extended_asset ext_asset_in, asset min_expected) {
    require_auth(user);
    check( ((ext_asset_in.quantity.amount > 0) && (min_expected.amount >= 0)) ||
           ((ext_asset_in.quantity.amount < 0) && (min_expected.amount <= 0)), 
           "ext_asset_in must be nonzero and min_expected must have same sign or be zero");
    auto ext_asset_out = process_exch(pair_token, ext_asset_in, min_expected);
    add_signed_ext_balance(user, -ext_asset_in);
    add_signed_ext_balance(user, ext_asset_out);
}

extended_asset lemonade::process_exch(symbol_code pair_token,
  extended_asset ext_asset_in, asset min_expected){
    stats statstable( get_self(), pair_token.raw() );
    const auto token = statstable.find( pair_token.raw() );
    check ( token != statstable.end(), "pair token does not exist" );
    bool in_first;
    if ((token->pool1.get_extended_symbol() == ext_asset_in.get_extended_symbol()) && 
        (token->pool2.quantity.symbol == min_expected.symbol)) {
        in_first = true;
    } else if ((token->pool1.quantity.symbol == min_expected.symbol) &&
               (token->pool2.get_extended_symbol() == ext_asset_in.get_extended_symbol())) {
        in_first = false;
    }
    else check(false, "extended_symbol mismatch");
    int64_t P_in, P_out;
    if (in_first) { 
      P_in = token-> pool1.quantity.amount;
      P_out = token-> pool2.quantity.amount;
    } else {
      P_in = token-> pool2.quantity.amount;
      P_out = token-> pool1.quantity.amount;
    }
    auto A_in = ext_asset_in.quantity.amount;
    int64_t A_out = compute(-A_in, P_out, P_in + A_in, token->fee);
    check(min_expected.amount <= -A_out, "available is less than expected");
    extended_asset ext_asset1, ext_asset2, ext_asset_out;
    if (in_first) { 
      ext_asset1 = ext_asset_in;
      ext_asset2 = extended_asset{A_out, token-> pool2.get_extended_symbol()};
      ext_asset_out = -ext_asset2;
    } else {
      ext_asset1 = extended_asset{A_out, token-> pool1.get_extended_symbol()};
      ext_asset2 = ext_asset_in;
      ext_asset_out = -ext_asset1;
    }
    statstable.modify( token, same_payer, [&]( auto& a ) {
      a.pool1 += ext_asset1;
      a.pool2 += ext_asset2;
    });
    return ext_asset_out;
}

// computes x * y / z plus the fee
int64_t lemonade::compute(int64_t x, int64_t y, int64_t z, int fee) {
    check( (x != 0) && (y > 0) && (z > 0), "invalid parameters");
    int128_t prod = int128_t(x) * int128_t(y);
    int128_t tmp = 0;
    int128_t tmp_fee = 0;
    if (x > 0) {
        tmp = 1 + (prod - 1) / int128_t(z);
        check( (tmp <= MAX), "computation overflow" );
        tmp_fee = (tmp * fee + 9999) / 10000;
    } else {
        tmp = prod / int128_t(z);
        check( (tmp >= -MAX), "computation underflow" );
        tmp_fee =  (-tmp * fee + 9999) / 10000;
    }
    tmp += tmp_fee;
    return int64_t(tmp);
}

void lemonade::changefee(symbol_code pair_token, int newfee) {
    stats statstable( get_self(), pair_token.raw() );
    const auto& token = statstable.find( pair_token.raw() );
    check ( token != statstable.end(), "pair token does not exist" );
    require_auth(token->fee_contract);
    statstable.modify( token, same_payer, [&]( auto& a ) {
      a.fee = newfee;
    } );
}

void lemonade::add_signed_liq(name user, asset to_add, bool is_buying,
  asset max_asset1, asset max_asset2){
    check( to_add.is_valid(), "invalid asset");
    stats statstable( get_self(), to_add.symbol.code().raw() );
    const auto& token = statstable.find( to_add.symbol.code().raw() );
    check ( token != statstable.end(), "pair token does not exist" );
    auto A = token-> supply.amount;
    auto P1 = token-> pool1.quantity.amount;
    auto P2 = token-> pool2.quantity.amount;

    int fee = is_buying? ADD_LIQUIDITY_FEE : 0;
    auto to_pay1 = extended_asset{ asset{compute(to_add.amount, P1, A, fee),
      token->pool1.quantity.symbol}, token->pool1.contract};
    auto to_pay2 = extended_asset{ asset{compute(to_add.amount, P2, A, fee),
      token->pool2.quantity.symbol}, token->pool2.contract};
    check( (to_pay1.quantity.symbol == max_asset1.symbol) && 
           (to_pay2.quantity.symbol == max_asset2.symbol), "incorrect symbol");
    check( (to_pay1.quantity.amount <= max_asset1.amount) && 
           (to_pay2.quantity.amount <= max_asset2.amount), "available is less than expected");

    add_signed_ext_balance(user, -to_pay1);
    add_signed_ext_balance(user, -to_pay2);
    (to_add.amount > 0)? add_balance(user, to_add, user) : sub_balance(user, -to_add);
    if (token->fee_contract) require_recipient(token->fee_contract);
    statstable.modify( token, same_payer, [&]( auto& a ) {
      a.supply += to_add;
      a.pool1 += to_pay1;
      a.pool2 += to_pay2;
    });
    check(token->supply.amount != 0, "the pool cannot be left empty");
}

void lemonade::add_signed_ext_balance( const name& user, const extended_asset& to_add )
{
    check( to_add.quantity.is_valid(), "invalid asset" );
    dexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find( make128key(to_add.contract.value, to_add.quantity.symbol.raw() ) );
    check( acnt_balance != index.end(), "extended_symbol not registered for this user,\
 please run openext action or write exchange details in the memo of your transfer");
    index.modify( acnt_balance, same_payer, [&]( auto& a ) {
        a.balance += to_add;
        check( a.balance.quantity.amount >= 0, "insufficient funds");
    });
}

uint128_t lemonade::make128key(uint64_t a, uint64_t b) {
    uint128_t aa = a;
    uint128_t bb = b;
    return (aa << 64) + bb;
}

checksum256 lemonade::make256key(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    if (make128key(a,b) < make128key(c,d))
      return checksum256::make_from_word_sequence<uint64_t>(a,b,c,d);
    else
      return checksum256::make_from_word_sequence<uint64_t>(c,d,a,b);
}
