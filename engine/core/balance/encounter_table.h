#pragma once

#include <cstdint>
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

class EncounterTable {
public:
    void addEncounter(EncounterEntry entry);
    std::vector<EncounterDiagnostic> validate() const;
    std::vector<std::string> preview(const std::string& region_id, uint64_t seed, std::size_t count) const;

private:
    std::vector<EncounterEntry> entries_;
};

} // namespace urpg::balance
