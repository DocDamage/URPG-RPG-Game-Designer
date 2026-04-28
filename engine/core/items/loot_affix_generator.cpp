#include "engine/core/items/loot_affix_generator.h"

#include <algorithm>
#include <random>
#include <set>
#include <utility>

namespace urpg::items {

namespace {

LootAffix affixFromJson(const nlohmann::json& json) {
    LootAffix affix;
    affix.id = json.value("id", "");
    affix.rarity = json.value("rarity", "");
    affix.min_value = json.value("min_value", 0);
    affix.max_value = json.value("max_value", 0);
    affix.economy_multiplier = json.value("economy_multiplier", 100);
    return affix;
}

nlohmann::json affixToJson(const LootAffix& affix) {
    nlohmann::json json{{"id", affix.id},
                        {"rarity", affix.rarity},
                        {"min_value", affix.min_value},
                        {"max_value", affix.max_value},
                        {"economy_multiplier", affix.economy_multiplier}};
    if (affix.value != 0) {
        json["value"] = affix.value;
    }
    return json;
}

LootBaseItem baseItemFromJson(const nlohmann::json& json) {
    LootBaseItem item;
    item.id = json.value("id", "");
    item.slot = json.value("slot", "");
    item.rarity = json.value("rarity", "");
    item.base_value = json.value("base_value", 0);
    item.tags = json.value("tags", std::vector<std::string>{});
    return item;
}

nlohmann::json baseItemToJson(const LootBaseItem& item) {
    return {{"id", item.id}, {"slot", item.slot}, {"rarity", item.rarity}, {"base_value", item.base_value}, {"tags", item.tags}};
}

} // namespace

void LootAffixGenerator::addAffix(LootAffix affix) {
    affixes_.push_back(std::move(affix));
}

std::optional<LootAffix> LootAffixGenerator::roll(const std::string& rarity, uint64_t seed) const {
    std::vector<LootAffix> pool;
    for (const auto& affix : affixes_) {
        if (affix.rarity == rarity) {
            pool.push_back(affix);
        }
    }
    std::stable_sort(pool.begin(), pool.end(), [](const auto& lhs, const auto& rhs) { return lhs.id < rhs.id; });
    if (pool.empty()) {
        return std::nullopt;
    }
    std::mt19937_64 rng(seed);
    auto selected = pool[static_cast<std::size_t>(rng() % pool.size())];
    const auto span = std::max(1, selected.max_value - selected.min_value + 1);
    selected.value = selected.min_value + static_cast<int32_t>(rng() % span);
    return selected;
}

std::vector<LootAffixDiagnostic> LootAffixGenerator::validate() const {
    std::vector<LootAffixDiagnostic> diagnostics;
    for (const auto& affix : affixes_) {
        if (affix.min_value > affix.max_value) {
            diagnostics.push_back({"invalid_affix_bounds", "Affix minimum exceeds maximum."});
        }
        if (affix.economy_multiplier <= 0) {
            diagnostics.push_back({"invalid_affix_economy", "Affix economy multiplier must be positive."});
        }
    }
    return diagnostics;
}

std::vector<LootAffixDiagnostic> LootGeneratorDocument::validate(const std::set<std::string>& known_slots) const {
    std::vector<LootAffixDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_loot_table_id", "Loot generator document is missing an id."});
    }
    if (affixes_per_item < 0) {
        diagnostics.push_back({"invalid_affix_count", "Affixes per item cannot be negative."});
    }
    std::set<std::string> item_ids;
    for (const auto& item : base_items) {
        if (item.id.empty()) {
            diagnostics.push_back({"missing_base_item_id", "Loot base item is missing an id."});
        } else if (!item_ids.insert(item.id).second) {
            diagnostics.push_back({"duplicate_base_item_id", "Loot base item id is duplicated: " + item.id});
        }
        if (!known_slots.empty() && !known_slots.contains(item.slot)) {
            diagnostics.push_back({"unknown_item_slot", "Loot base item references an unknown slot: " + item.slot});
        }
        if (item.base_value < 0) {
            diagnostics.push_back({"invalid_base_value", "Loot base item value cannot be negative: " + item.id});
        }
        if (item.rarity.empty()) {
            diagnostics.push_back({"missing_base_rarity", "Loot base item is missing a rarity: " + item.id});
        }
    }
    std::set<std::string> affix_ids;
    LootAffixGenerator generator;
    for (const auto& affix : affixes) {
        generator.addAffix(affix);
        if (affix.id.empty()) {
            diagnostics.push_back({"missing_affix_id", "Loot affix is missing an id."});
        } else if (!affix_ids.insert(affix.id).second) {
            diagnostics.push_back({"duplicate_affix_id", "Loot affix id is duplicated: " + affix.id});
        }
    }
    const auto affix_diagnostics = generator.validate();
    diagnostics.insert(diagnostics.end(), affix_diagnostics.begin(), affix_diagnostics.end());
    return diagnostics;
}

LootAffixGenerator LootGeneratorDocument::toRuntimeGenerator() const {
    LootAffixGenerator generator;
    for (const auto& affix : affixes) {
        generator.addAffix(affix);
    }
    return generator;
}

LootGeneratorPreview LootGeneratorDocument::preview(uint64_t seed, std::size_t count) const {
    LootGeneratorPreview result;
    result.table_id = id;
    result.seed = seed;
    result.diagnostics = validate();
    if (base_items.empty()) {
        result.diagnostics.push_back({"empty_base_item_pool", "Loot generator has no base items."});
        return result;
    }

    auto sorted_items = base_items;
    std::stable_sort(sorted_items.begin(), sorted_items.end(), [](const auto& lhs, const auto& rhs) { return lhs.id < rhs.id; });
    const auto generator = toRuntimeGenerator();
    std::mt19937_64 rng(seed);
    for (std::size_t index = 0; index < count; ++index) {
        const auto& base = sorted_items[static_cast<std::size_t>(rng() % sorted_items.size())];
        GeneratedLootItem item;
        item.item_id = base.id;
        item.slot = base.slot;
        item.rarity = base.rarity;
        item.value = base.base_value;
        int32_t multiplier_total = 100;
        for (int32_t affix_index = 0; affix_index < affixes_per_item; ++affix_index) {
            const auto affix = generator.roll(base.rarity, rng() + static_cast<uint64_t>(affix_index));
            if (!affix.has_value()) {
                result.diagnostics.push_back({"missing_affix_pool", "No affixes exist for rarity: " + base.rarity});
                break;
            }
            item.affixes.push_back(*affix);
            item.value += affix->value;
            multiplier_total += affix->economy_multiplier - 100;
        }
        item.value = std::max(0, (item.value * multiplier_total) / 100);
        result.items.push_back(std::move(item));
    }
    return result;
}

nlohmann::json LootGeneratorDocument::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["affixes_per_item"] = affixes_per_item;
    json["base_items"] = nlohmann::json::array();
    for (const auto& item : base_items) {
        json["base_items"].push_back(baseItemToJson(item));
    }
    json["affixes"] = nlohmann::json::array();
    for (const auto& affix : affixes) {
        json["affixes"].push_back(affixToJson(affix));
    }
    return json;
}

LootGeneratorDocument LootGeneratorDocument::fromJson(const nlohmann::json& json) {
    LootGeneratorDocument document;
    document.id = json.value("id", "");
    document.affixes_per_item = json.value("affixes_per_item", 1);
    for (const auto& item_json : json.value("base_items", nlohmann::json::array())) {
        document.base_items.push_back(baseItemFromJson(item_json));
    }
    for (const auto& affix_json : json.value("affixes", nlohmann::json::array())) {
        document.affixes.push_back(affixFromJson(affix_json));
    }
    return document;
}

nlohmann::json generatedLootItemToJson(const GeneratedLootItem& item) {
    nlohmann::json json{{"item_id", item.item_id}, {"slot", item.slot}, {"rarity", item.rarity}, {"value", item.value}};
    json["affixes"] = nlohmann::json::array();
    for (const auto& affix : item.affixes) {
        json["affixes"].push_back(affixToJson(affix));
    }
    return json;
}

nlohmann::json lootGeneratorPreviewToJson(const LootGeneratorPreview& preview) {
    nlohmann::json json{{"table_id", preview.table_id}, {"seed", preview.seed}};
    json["items"] = nlohmann::json::array();
    for (const auto& item : preview.items) {
        json["items"].push_back(generatedLootItemToJson(item));
    }
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}});
    }
    return json;
}

} // namespace urpg::items
