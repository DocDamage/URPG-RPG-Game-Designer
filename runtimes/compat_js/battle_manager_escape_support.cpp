#include "runtimes/compat_js/battle_manager_support.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace urpg {
namespace compat {
namespace {
double normalizeAgility(int32_t actionSpeed) {
    return static_cast<double>(std::max(1, actionSpeed));
}

double averageAgility(const std::vector<BattleSubject>& subjects) {
    if (subjects.empty()) {
        return 100.0;
    }

    double total = 0.0;
    int32_t count = 0;
    for (const auto& subject : subjects) {
        if (subject.hidden || subject.hp <= 0) {
            continue;
        }
        total += normalizeAgility(subject.actionSpeed);
        ++count;
    }

    if (count == 0) {
        return 100.0;
    }
    return total / static_cast<double>(count);
}
} // namespace

double computeBaseEscapeRatio(const std::vector<BattleSubject>& actors,
                              const std::vector<BattleSubject>& enemies) {
    const double partyAgi = averageAgility(actors);
    const double troopAgi = averageAgility(enemies);
    if (troopAgi <= 0.0) {
        return 1.0;
    }
    return std::clamp(0.5 * (partyAgi / troopAgi), 0.0, 1.0);
}

uint32_t mixEscapeSeed(int32_t troopId) {
    uint32_t seed = 0x9E3779B9u;
    seed ^= static_cast<uint32_t>(troopId) + 0x85EBCA6Bu + (seed << 6) + (seed >> 2);
    return seed == 0 ? 0xA341316Cu : seed;
}

double nextEscapeRoll(uint32_t& state) {
    if (state == 0) {
        state = 0xA341316Cu;
    }
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);
    return static_cast<double>(state) / static_cast<double>(UINT32_MAX);
}

} // namespace compat
} // namespace urpg
