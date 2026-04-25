#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::codex {

struct BestiaryDiagnostic {
    std::string code;
    std::string message;
};

struct BestiaryEntry {
    std::string enemy_id;
    std::string name;
    std::vector<std::string> weaknesses;
    std::vector<std::string> drops;
    std::string lore_key;
};

struct BestiaryState {
    bool seen{false};
    bool scanned{false};
    bool defeated{false};
};

class BestiaryRegistry {
public:
    void addEntry(BestiaryEntry entry);
    void markSeen(const std::string& enemy_id);
    void markScanned(const std::string& enemy_id);
    void markDefeated(const std::string& enemy_id);
    [[nodiscard]] BestiaryState stateFor(const std::string& enemy_id) const;
    [[nodiscard]] double completionRatio() const;
    [[nodiscard]] std::vector<BestiaryDiagnostic> validate(const std::vector<std::string>& known_enemy_ids) const;
    [[nodiscard]] nlohmann::json toJson() const;
    [[nodiscard]] static BestiaryRegistry fromJson(const nlohmann::json& json);

private:
    std::vector<BestiaryEntry> entries_;
    std::vector<std::pair<std::string, BestiaryState>> states_;
};

} // namespace urpg::codex
