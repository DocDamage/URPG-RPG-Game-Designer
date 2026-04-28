#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
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

struct LootBaseItem {
    std::string id;
    std::string slot;
    std::string rarity;
    int32_t base_value = 0;
    std::vector<std::string> tags;
};

struct GeneratedLootItem {
    std::string item_id;
    std::string slot;
    std::string rarity;
    int32_t value = 0;
    std::vector<LootAffix> affixes;
};

struct LootGeneratorPreview {
    std::string table_id;
    uint64_t seed = 0;
    std::vector<GeneratedLootItem> items;
    std::vector<LootAffixDiagnostic> diagnostics;
};

class LootAffixGenerator {
public:
    void addAffix(LootAffix affix);
    std::optional<LootAffix> roll(const std::string& rarity, uint64_t seed) const;
    std::vector<LootAffixDiagnostic> validate() const;

private:
    std::vector<LootAffix> affixes_;
};

class LootGeneratorDocument {
public:
    std::string id;
    int32_t affixes_per_item = 1;
    std::vector<LootBaseItem> base_items;
    std::vector<LootAffix> affixes;

    [[nodiscard]] std::vector<LootAffixDiagnostic> validate(const std::set<std::string>& known_slots = {}) const;
    [[nodiscard]] LootAffixGenerator toRuntimeGenerator() const;
    [[nodiscard]] LootGeneratorPreview preview(uint64_t seed, std::size_t count) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static LootGeneratorDocument fromJson(const nlohmann::json& json);
};

nlohmann::json generatedLootItemToJson(const GeneratedLootItem& item);
nlohmann::json lootGeneratorPreviewToJson(const LootGeneratorPreview& preview);

} // namespace urpg::items
