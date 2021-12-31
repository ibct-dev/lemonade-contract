#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/transaction.hpp>

#include <math.h>
#include <string>

using namespace eosio;
using namespace std;

namespace config {
    static constexpr uint32_t secondsPerYear   = 52 * 7 * 24 * 3600;
    static constexpr uint32_t secondsPerMonth  = 31449600;
    static constexpr uint32_t secondsPerWeek   = 24 * 3600 * 7;
    static constexpr uint32_t secondsPerDay    = 24 * 3600;
    static constexpr uint32_t secondsPerHour   = 3600;
    static constexpr uint32_t secondsPerMinute = 60;
    static constexpr uint64_t usecondsPerYear = uint64_t(secondsPerYear) * 1000'000ll;
    static constexpr uint64_t usecondsPerMonth = uint64_t(secondsPerMonth) * 1000'000ll;
    static constexpr uint64_t usecondsPerWeek = uint64_t(secondsPerWeek) * 1000'000ll;
    static constexpr uint64_t usecondsPerDay = uint64_t(secondsPerDay) * 1000'000ll;
    static constexpr uint64_t usecondsPerHour = uint64_t(secondsPerHour) * 1000'000ll;
    static constexpr uint64_t usecondsPerMinute = uint64_t(secondsPerMinute) * 1000'000ll;
    static constexpr uint32_t blocksPerYear  = 2 * secondsPerYear;
    static constexpr uint32_t blocksPerMonth = 2 * secondsPerMonth;
    static constexpr uint32_t blocksPerWeek  = 2 * secondsPerWeek;
    static constexpr uint32_t blocksPerDay   = 2 * secondsPerDay;
}   // namespace config