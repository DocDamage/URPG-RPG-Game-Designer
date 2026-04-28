#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

namespace urpg::balance {

struct EncounterDiagnostic {
    std::string code;
    std::string message;
};

struct EncounterEntry {
    std::string enemy_id;
    std::string region_id;
    int32_t weight = 0;
    int32_t difficulty = 1;
};

struct EncounterRegion {
    std::string id;
    std::string map_id;
    std::string biome;
    int32_t min_party_level = 1;
    int32_t max_party_level = 99;
    std::vector<std::string> required_flags;
};

struct EncounterDesignerEntry {
    std::string id;
    std::string enemy_id;
    std::string region_id;
    int32_t weight = 0;
    int32_t difficulty = 1;
    int32_t min_party_level = 1;
    int32_t max_party_level = 99;
    std::vector<std::string> tags;
    std::vector<std::string> rewards;
};

struct EncounterPreviewEntry {
    std::string encounter_id;
    std::string enemy_id;
    int32_t difficulty = 1;
    std::vector<std::string> rewards;
};

struct EncounterDesignerPreview {
    std::string region_id;
    int32_t party_level = 1;
    uint64_t seed = 0;
    std::vector<EncounterPreviewEntry> encounters;
    std::vector<EncounterDiagnostic> diagnostics;
};

class EncounterTable {
public:
    void addEncounter(EncounterEntry entry);
    std::vector<EncounterDiagnostic> validate() const;
    std::vector<std::string> preview(const std::string& region_id, uint64_t seed, std::size_t count) const;

private:
    std::vector<EncounterEntry> entries_;
};

class EncounterDesignerDocument {
public:
    std::string id;
    std::vector<EncounterRegion> regions;
    std::vector<EncounterDesignerEntry> encounters;

    [[nodiscard]] std::vector<EncounterDiagnostic> validate(const std::set<std::string>& known_enemies = {},
                                                            const std::set<std::string>& known_flags = {}) const;
    [[nodiscard]] EncounterTable toRuntimeTable() const;
    [[nodiscard]] EncounterDesignerPreview preview(const std::string& region_id,
                                                   int32_t party_level,
                                                   const std::set<std::string>& active_flags,
                                                   uint64_t seed,
                                                   std::size_t count) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static EncounterDesignerDocument fromJson(const nlohmann::json& json);
};

nlohmann::json encounterDesignerPreviewToJson(const EncounterDesignerPreview& preview);

} // namespace urpg::balance
