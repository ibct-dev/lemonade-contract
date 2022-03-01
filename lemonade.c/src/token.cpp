#include "lemonade.c/lemonade.c.hpp"

void lemonade::transfer(const name& from, const name& to, const asset& quantity,
                        const string& memo) {
    check(from != to, "cannot transfer to self");
    require_auth(from);
    check_frozen(from);
    check(is_account(to), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable(get_self(), sym.raw());
    const auto& st = statstable.get(sym.raw());

    require_recipient(from);
    require_recipient(to);

    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must transfer positive quantity");
    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    auto payer = has_auth(to) ? to : from;

    sub_balance(from, quantity);
    add_balance(to, quantity, payer);
    if (to == get_self())
        transfer_event(from, to, quantity,
                       memo);  // line added to code from eosio.token
}

void lemonade::sub_balance(const name& owner, const asset& value) {
    accounts from_acnts(get_self(), owner.value);

    const auto& from =
        from_acnts.get(value.symbol.code().raw(), "no balance object found");
    check(from.balance.amount >= value.amount, "overdrawn balance");

    from_acnts.modify(from, owner, [&](auto& a) { a.balance -= value; });
}

void lemonade::add_balance(const name& owner, const asset& value,
                           const name& ram_payer) {
    accounts to_acnts(get_self(), owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());
    if (to == to_acnts.end()) {
        to_acnts.emplace(ram_payer, [&](auto& a) { a.balance = value; });
    } else {
        to_acnts.modify(to, same_payer, [&](auto& a) { a.balance += value; });
    }
}

void lemonade::open(const name& owner, const symbol& symbol,
                    const name& ram_payer) {
    require_auth(ram_payer);

    check(is_account(owner), "owner account does not exist");

    auto sym_code_raw = symbol.code().raw();
    stats statstable(get_self(), sym_code_raw);
    const auto& st = statstable.get(sym_code_raw, "symbol does not exist");
    check(st.supply.symbol == symbol, "symbol precision mismatch");

    accounts acnts(get_self(), owner.value);
    auto it = acnts.find(sym_code_raw);
    if (it == acnts.end()) {
        acnts.emplace(ram_payer, [&](auto& a) {
            a.balance = asset{0, symbol};
        });
    }
}

void lemonade::close(const name& owner, const symbol& symbol) {
    require_auth(owner);
    accounts acnts(get_self(), owner.value);
    auto it = acnts.find(symbol.code().raw());
    check(it != acnts.end(),
          "Balance row already deleted or never existed. Action won't have any "
          "effect.");
    check(it->balance.amount == 0,
          "Cannot close because the balance is not zero.");
    acnts.erase(it);
}

void lemonade::freeze(const name& account) {
    require_auth(get_self());

    require_recipient(account);

    frozens frozentable(get_self(), get_self().value);
    auto fitr = frozentable.find(account.value);
    check(fitr == frozentable.end(), "account is already frozen");

    frozentable.emplace(get_self(), [&](auto& f) { f.freezer = account; });
}

void lemonade::unfreeze(const name& account) {
    require_auth(get_self());

    require_recipient(account);

    frozens frozentable(get_self(), get_self().value);
    auto fitr = frozentable.find(account.value);
    check(fitr != frozentable.end(), "account is not frozen");

    frozentable.erase(fitr);
}

void lemonade::check_frozen(const name& account) {
    frozens frozenstable(get_self(), get_self().value);
    auto fitr = frozenstable.find(account.value);
    check(fitr == frozenstable.end(), "account is frozen");
}
