#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::items {

struct LootAffixDiagnostic {
    std::string code;
    std::string message;
};

struct LootAffix {
    std::string id;
    std::string rarity;
    int32_t min_value = 0;
    int32_t max_value = 0;
    int32_t economy_multiplier = 100;
    int32_t value = 0;
};

class LootAffixGenerator {
public:
    void addAffix(LootAffix affix);
    std::optional<LootAffix> roll(const std::string& rarity, uint64_t seed) const;
    std::vector<LootAffixDiagnostic> validate() const;

private:
    std::vector<LootAffix> affixes_;
};

} // namespace urpg::items
