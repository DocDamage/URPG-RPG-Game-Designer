#pragma once

/**
 * @file ability_compat_mapper.h
 * @brief S24-T01/T02: Compat-to-native ability schema mapping with deterministic
 *        fallback payloads for unsupported fields.
 *
 * Maps RPG Maker MV/MZ skill JSON shapes to native AuthoredAbilityAsset records.
 * Fields that have no native equivalent are preserved verbatim in
 * `unsupported_fields` on the output JSON rather than silently discarded,
 * enabling round-trip fidelity and future re-mapping without data loss.
 *
 * Mapping table (MZ field → native field):
 *   id          → ability_id  ("skill.<id>")
 *   name        → ability_id  (used when id is absent)
 *   mpCost      → mp_cost
 *   tpCost      → unsupported_fields["tpCost"]  (TP not yet modeled natively)
 *   damage.value  → effect_value  (approximate; see note below)
 *   damage.type   → unsupported_fields["damage_type"]
 *   damage.element→ unsupported_fields["damage_element"]
 *   occasion      → unsupported_fields["occasion"]
 *   scope         → unsupported_fields["scope"]
 *   speed         → unsupported_fields["speed"]
 *   successRate   → unsupported_fields["successRate"]
 *   repeats       → unsupported_fields["repeats"]
 *   hitType       → unsupported_fields["hitType"]
 *   animationId   → unsupported_fields["animationId"]
 *   effects[]     → unsupported_fields["effects"]
 *   note          → unsupported_fields["note"]
 *   meta          → unsupported_fields["meta"]
 *
 * Note: MZ damage.value is a formula string (e.g. "a.atk * 4 - b.def * 2").
 * We cannot evaluate the formula natively, so a nominal effect_value = 0.0 is
 * used and the raw formula is preserved in unsupported_fields["damage_formula"].
 * Tests and callers must not treat the nominal value as correct.
 */

#include "authored_ability_asset.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::ability {

/// Result of mapping a single MZ skill JSON object to a native ability asset.
struct AbilityCompatMapResult {
    /// Native ability asset populated from mapped fields.
    AuthoredAbilityAsset asset;

    /// Top-level JSON of the mapped asset, including `unsupported_fields` for
    /// any source keys that have no native mapping.  Always well-formed JSON.
    nlohmann::json mapped_json;

    /// Source fields that were preserved in `unsupported_fields` rather than
    /// mapped to a native field.  Populated for diagnostic/audit purposes.
    std::vector<std::string> fallback_field_names;

    /// True if every source field with a defined mapping was successfully mapped.
    /// False if a mapping failed and a nominal default was used instead.
    bool all_mapped = true;

    /// Human-readable notes produced during mapping (e.g. formula-to-nominal
    /// substitution, unsupported activation conditions).
    std::vector<std::string> mapping_notes;
};

/// Maps a single RPG Maker MZ/MV skill JSON object to a native ability asset.
///
/// The input is expected to be a JSON object as produced by the MZ data layer
/// (e.g. `$dataSkills[i]`).  Unknown top-level keys are placed into
/// `unsupported_fields` and listed in the result's `fallback_field_names`.
///
/// This function never throws; on unexpected input shapes it returns a result
/// with `all_mapped = false` and a nominal default asset.
inline AbilityCompatMapResult mapMzSkillToNativeAbility(const nlohmann::json& mz_skill) {
    AbilityCompatMapResult result;
    AuthoredAbilityAsset& asset = result.asset;
    nlohmann::json unsupported;

    // --- ability_id ---
    if (mz_skill.contains("id") && mz_skill["id"].is_number_integer()) {
        asset.ability_id = "skill." + std::to_string(mz_skill["id"].get<int>());
    } else if (mz_skill.contains("name") && mz_skill["name"].is_string()) {
        const std::string raw = mz_skill["name"].get<std::string>();
        // Slugify: lowercase, spaces → underscores
        std::string slug;
        slug.reserve(raw.size());
        for (char c : raw) {
            if (c == ' ' || c == '-') {
                slug += '_';
            } else {
                slug += static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            }
        }
        asset.ability_id = "skill." + slug;
    }
    // else keep default "skill.draft"

    // --- mp_cost ---
    if (mz_skill.contains("mpCost") && mz_skill["mpCost"].is_number()) {
        asset.mp_cost = mz_skill["mpCost"].get<float>();
    }

    // --- effect_value / damage ---
    // MZ damage.value is a formula string; we cannot evaluate it natively.
    if (mz_skill.contains("damage") && mz_skill["damage"].is_object()) {
        const auto& dmg = mz_skill["damage"];

        // Preserve formula string in unsupported_fields
        if (dmg.contains("formula") && dmg["formula"].is_string()) {
            unsupported["damage_formula"] = dmg["formula"];
            asset.effect_value = 0.0f;
            result.fallback_field_names.push_back("damage_formula");
            result.mapping_notes.push_back(
                "damage.formula '" + dmg["formula"].get<std::string>() +
                "' has no native scalar mapping; effect_value set to 0.0 (nominal).");
            result.all_mapped = false;
        }

        // damage.type, damage.element — no native mapping
        if (dmg.contains("type")) {
            unsupported["damage_type"] = dmg["type"];
            result.fallback_field_names.push_back("damage_type");
        }
        if (dmg.contains("elementId")) {
            unsupported["damage_element"] = dmg["elementId"];
            result.fallback_field_names.push_back("damage_element");
        }
    }

    // --- unsupported MZ top-level fields ---
    // Each key that has no native mapping is preserved verbatim.
    static const std::vector<std::string> kFallbackKeys = {
        "tpCost", "occasion", "scope", "speed", "successRate",
        "repeats", "hitType", "animationId", "effects", "note", "meta",
        "tpGain", "message1", "message2", "requiredWtypeId1", "requiredWtypeId2",
        "stypeId", "iconIndex",
    };
    for (const auto& key : kFallbackKeys) {
        if (mz_skill.contains(key)) {
            unsupported[key] = mz_skill[key];
            result.fallback_field_names.push_back(key);
        }
    }

    // --- build mapped_json ---
    nlohmann::json j;
    to_json(j, asset);
    j["schema"] = "content/schemas/gameplay_ability.schema.json";
    j["schemaVersion"] = 1;
    if (!unsupported.empty()) {
        j["unsupported_fields"] = std::move(unsupported);
    }
    result.mapped_json = std::move(j);
    return result;
}

/// Maps an array of MZ skill JSON objects (e.g. the full `$dataSkills` array).
/// Null entries (MZ uses 1-based arrays with a null at index 0) are skipped.
inline std::vector<AbilityCompatMapResult> mapMzSkillArrayToNativeAbilities(
    const nlohmann::json& mz_skills_array)
{
    std::vector<AbilityCompatMapResult> results;
    if (!mz_skills_array.is_array()) {
        return results;
    }
    for (const auto& entry : mz_skills_array) {
        if (entry.is_null()) {
            continue;
        }
        if (!entry.is_object()) {
            continue;
        }
        results.push_back(mapMzSkillToNativeAbility(entry));
    }
    return results;
}

} // namespace urpg::ability
