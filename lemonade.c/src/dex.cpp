#include "lemonade.c/lemonade.c.hpp"

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
