#include "engine/core/battle/battle_core.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stdexcept>
#include <utility>

namespace urpg::battle {

namespace {

bool IsBattleTerminal(BattleFlowPhase phase) {
    return phase == BattleFlowPhase::Victory || phase == BattleFlowPhase::Defeat || phase == BattleFlowPhase::Abort ||
           phase == BattleFlowPhase::None;
}

bool ActionSortLess(const BattleQueuedAction& lhs, const BattleQueuedAction& rhs) {
    if (lhs.speed != rhs.speed) {
        return lhs.speed > rhs.speed;
    }
    if (lhs.priority != rhs.priority) {
        return lhs.priority < rhs.priority;
    }
    if (lhs.subject_id != rhs.subject_id) {
        return lhs.subject_id < rhs.subject_id;
    }
    if (lhs.target_id != rhs.target_id) {
        return lhs.target_id < rhs.target_id;
    }
    return lhs.command < rhs.command;
}

int32_t ClampPositive(int32_t value) {
    return std::max(1, value);
}

int32_t ClampRatioPercent(int32_t value) {
    return std::clamp(value, 0, 100);
}

std::string NormalizeImportKey(const std::string& value) {
    std::string normalized;
    normalized.reserve(value.size());
    for (const unsigned char ch : value) {
        if (std::isalnum(ch) != 0) {
            normalized.push_back(static_cast<char>(std::tolower(ch)));
        }
    }
    return normalized;
}

std::string TrimAscii(std::string value) {
    const auto first =
        std::find_if_not(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
    const auto last =
        std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) { return std::isspace(ch) != 0; }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

std::string LowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

bool IsSupportedZeroDamagePolicyValue(const std::string& value) {
    const std::string normalized = NormalizeImportKey(value);
    return normalized == "miss" || normalized == "evasion" || normalized == "evade" || normalized == "zeroasevasion" ||
           normalized == "immune" || normalized == "zeroasimmune" || normalized == "noeffect" || normalized == "none" ||
           normalized == "zeroasnoeffect";
}

const nlohmann::json& FeedbackPolicySourceObject(const nlohmann::json& fixture_json) {
    static const nlohmann::json empty_object = nlohmann::json::object();
    if (!fixture_json.is_object()) {
        return empty_object;
    }

    for (const auto* key : {"parameters", "feedback_policy", "feedbackPolicy", "battleFeedback", "BattleFeedback"}) {
        const auto it = fixture_json.find(key);
        if (it != fixture_json.end() && it->is_object()) {
            return *it;
        }
    }
    return fixture_json;
}

bool ReadImportValue(const nlohmann::json& source, const std::vector<std::string>& aliases, nlohmann::json& value,
                     std::string& source_key) {
    if (!source.is_object()) {
        return false;
    }

    std::vector<std::string> normalized_aliases;
    normalized_aliases.reserve(aliases.size());
    for (const auto& alias : aliases) {
        normalized_aliases.push_back(NormalizeImportKey(alias));
    }

    for (auto it = source.begin(); it != source.end(); ++it) {
        const std::string normalized_key = NormalizeImportKey(it.key());
        if (std::find(normalized_aliases.begin(), normalized_aliases.end(), normalized_key) !=
            normalized_aliases.end()) {
            value = it.value();
            source_key = it.key();
            return true;
        }
    }
    return false;
}

void AddImportDiagnostic(std::vector<BattleFeedbackImportDiagnostic>& diagnostics, std::string code,
                         std::string message, std::string target) {
    diagnostics.push_back({std::move(code), std::move(message), std::move(target)});
}

std::optional<int32_t> ReadImportInt(const nlohmann::json& source, const std::vector<std::string>& aliases,
                                     std::string& source_key,
                                     std::vector<BattleFeedbackImportDiagnostic>& diagnostics) {
    nlohmann::json value;
    if (!ReadImportValue(source, aliases, value, source_key)) {
        return std::nullopt;
    }
    if (value.is_number_integer()) {
        return value.get<int32_t>();
    }
    if (value.is_string()) {
        const std::string trimmed = TrimAscii(value.get<std::string>());
        try {
            size_t parsed_count = 0;
            const int parsed = std::stoi(trimmed, &parsed_count, 10);
            if (parsed_count == trimmed.size()) {
                return parsed;
            }
        } catch (const std::invalid_argument&) {
        } catch (const std::out_of_range&) {
        }
    }
    AddImportDiagnostic(diagnostics, "feedback_policy_value_invalid",
                        "Battle feedback fixture value must be an integer.", source_key);
    return std::nullopt;
}

std::optional<bool> ReadImportBool(const nlohmann::json& source, const std::vector<std::string>& aliases,
                                   std::string& source_key, std::vector<BattleFeedbackImportDiagnostic>& diagnostics) {
    nlohmann::json value;
    if (!ReadImportValue(source, aliases, value, source_key)) {
        return std::nullopt;
    }
    if (value.is_boolean()) {
        return value.get<bool>();
    }
    if (value.is_number_integer()) {
        return value.get<int32_t>() != 0;
    }
    if (value.is_string()) {
        const std::string lowered = LowerAscii(TrimAscii(value.get<std::string>()));
        if (lowered == "true" || lowered == "yes" || lowered == "on" || lowered == "1") {
            return true;
        }
        if (lowered == "false" || lowered == "no" || lowered == "off" || lowered == "0") {
            return false;
        }
    }
    AddImportDiagnostic(diagnostics, "feedback_policy_value_invalid",
                        "Battle feedback fixture value must be a boolean.", source_key);
    return std::nullopt;
}

std::optional<std::string> ReadImportString(const nlohmann::json& source, const std::vector<std::string>& aliases,
                                            std::string& source_key,
                                            std::vector<BattleFeedbackImportDiagnostic>& diagnostics) {
    nlohmann::json value;
    if (!ReadImportValue(source, aliases, value, source_key)) {
        return std::nullopt;
    }
    if (value.is_string()) {
        return TrimAscii(value.get<std::string>());
    }
    AddImportDiagnostic(diagnostics, "feedback_policy_value_invalid", "Battle feedback fixture value must be a string.",
                        source_key);
    return std::nullopt;
}

int32_t ClampImportedValue(int32_t value, int32_t min_value, int32_t max_value, const std::string& source_key,
                           std::vector<BattleFeedbackImportDiagnostic>& diagnostics) {
    const int32_t clamped = std::clamp(value, min_value, max_value);
    if (clamped != value) {
        AddImportDiagnostic(diagnostics, "feedback_policy_value_clamped",
                            "Battle feedback fixture value was clamped into the supported range.", source_key);
    }
    return clamped;
}

BattleFeedbackFixtureCoverageRow MakeCoverageRow(std::string id, std::string source_key) {
    BattleFeedbackFixtureCoverageRow row;
    row.id = std::move(id);
    row.covered = !source_key.empty();
    row.source_key = std::move(source_key);
    row.summary = row.covered ? "covered by imported/plugin-style fixture" : "missing from imported fixture";
    return row;
}

} // namespace

void BattleFlowController::beginBattle(bool can_escape) {
    phase_ = BattleFlowPhase::Start;
    allow_escape_ = can_escape;
    turn_count_ = 1;
    escape_failures_ = 0;
}

void BattleFlowController::enterInput() {
    if (IsBattleTerminal(phase_)) {
        return;
    }
    phase_ = BattleFlowPhase::Input;
}

void BattleFlowController::enterAction() {
    if (IsBattleTerminal(phase_)) {
        return;
    }
    phase_ = BattleFlowPhase::Action;
}

void BattleFlowController::endTurn() {
    if (IsBattleTerminal(phase_)) {
        return;
    }
    phase_ = BattleFlowPhase::TurnEnd;
    ++turn_count_;
}

void BattleFlowController::markVictory() {
    phase_ = BattleFlowPhase::Victory;
}

void BattleFlowController::markDefeat() {
    phase_ = BattleFlowPhase::Defeat;
}

void BattleFlowController::abort() {
    phase_ = BattleFlowPhase::Abort;
}

bool BattleFlowController::isActive() const {
    return !IsBattleTerminal(phase_);
}

bool BattleFlowController::canEscape() const {
    return isActive() && allow_escape_;
}

void BattleFlowController::noteEscapeFailure() {
    if (canEscape()) {
        ++escape_failures_;
    }
}

void BattleActionQueue::enqueue(BattleQueuedAction action) {
    queue_.push_back(std::move(action));
}

std::optional<BattleQueuedAction> BattleActionQueue::popNext() {
    if (queue_.empty()) {
        return std::nullopt;
    }

    auto it = std::min_element(
        queue_.begin(), queue_.end(),
        [](const BattleQueuedAction& lhs, const BattleQueuedAction& rhs) { return ActionSortLess(lhs, rhs); });
    BattleQueuedAction action = *it;
    queue_.erase(it);
    return action;
}

void BattleActionQueue::clear() {
    queue_.clear();
}

std::vector<BattleQueuedAction> BattleActionQueue::snapshotOrdered() const {
    std::vector<BattleQueuedAction> snapshot = queue_;
    std::sort(snapshot.begin(), snapshot.end(), ActionSortLess);
    return snapshot;
}

int32_t BattleRuleResolver::resolveDamage(const BattleDamageContext& context) {
    const int32_t atk_like = context.magical ? context.subject.mat : context.subject.atk;
    const int32_t def_like = context.magical ? context.target.mdf : context.target.def;

    int32_t damage = std::max(0, context.power + (atk_like * 2) - def_like);

    if (context.critical) {
        damage = static_cast<int32_t>(damage * 1.5);
    }

    if (context.variance_percent > 0) {
        const int32_t variance = (damage * std::min(context.variance_percent, 100)) / 100;
        // Deterministic midpoint variance adjustment so replay stays stable.
        damage += variance / 2;
    }

    if (context.target.guarding && damage > 0) {
        damage /= 2;
    }

    return std::clamp(damage, 0, ClampPositive(context.target.hp));
}

BattleFeedbackPreview BattleRuleResolver::resolveFeedbackPreview(int32_t damage, int32_t healing,
                                                                 int32_t current_buff_level, int32_t buff_delta,
                                                                 const BattleFeedbackPolicy& policy) {
    BattleFeedbackPreview preview;
    const int32_t safe_damage_percent = std::clamp(policy.chip_damage_percent, 0, 100);
    const int32_t safe_healing_percent = std::clamp(policy.chip_healing_percent, 0, 100);
    if (damage > 0) {
        preview.chip_damage = std::max(policy.min_chip_damage, (damage * safe_damage_percent) / 100);
        preview.chip_damage = std::min(preview.chip_damage, damage);
    }
    if (healing > 0) {
        preview.chip_healing = std::max(policy.min_chip_healing, (healing * safe_healing_percent) / 100);
        preview.chip_healing = std::min(preview.chip_healing, healing);
    }
    preview.buff_level = std::clamp(current_buff_level + buff_delta, -std::max(0, policy.max_buff_level),
                                    std::max(0, policy.max_buff_level));
    preview.zero_damage_label = toString(policy.zero_damage_policy);
    return preview;
}

TroopPositionReuseResult
BattleRuleResolver::resolveTroopPositions(const std::vector<TroopMemberPosition>& authored_positions,
                                          const std::vector<TroopMemberPosition>& reusable_positions,
                                          const BattleFeedbackPolicy& policy) {
    TroopPositionReuseResult result;
    result.positions = authored_positions;
    if (!policy.reuse_troop_positions) {
        return result;
    }

    for (auto& authored : result.positions) {
        const auto reusable =
            std::find_if(reusable_positions.begin(), reusable_positions.end(),
                         [&](const TroopMemberPosition& candidate) { return candidate.enemy_id == authored.enemy_id; });
        if (reusable != reusable_positions.end()) {
            authored.x = reusable->x;
            authored.y = reusable->y;
            ++result.reused_count;
        }
    }
    return result;
}

std::string BattleRuleResolver::toString(ZeroDamagePresentationPolicy policy) {
    switch (policy) {
    case ZeroDamagePresentationPolicy::Evasion:
        return "evasion";
    case ZeroDamagePresentationPolicy::Immune:
        return "immune";
    case ZeroDamagePresentationPolicy::NoEffect:
        return "no_effect";
    case ZeroDamagePresentationPolicy::Miss:
        return "miss";
    }
    return "miss";
}

ZeroDamagePresentationPolicy BattleRuleResolver::zeroDamagePolicyFromString(const std::string& value) {
    if (value == "evasion" || value == "evade" || value == "zero_as_evasion") {
        return ZeroDamagePresentationPolicy::Evasion;
    }
    if (value == "immune" || value == "zero_as_immune") {
        return ZeroDamagePresentationPolicy::Immune;
    }
    if (value == "no_effect" || value == "none" || value == "zero_as_no_effect") {
        return ZeroDamagePresentationPolicy::NoEffect;
    }
    return ZeroDamagePresentationPolicy::Miss;
}

nlohmann::json BattleRuleResolver::feedbackPolicyToJson(const BattleFeedbackPolicy& policy) {
    return {
        {"schemaVersion", "1.0.0"},
        {"chipDamagePercent", std::clamp(policy.chip_damage_percent, 0, 100)},
        {"chipHealingPercent", std::clamp(policy.chip_healing_percent, 0, 100)},
        {"minChipDamage", std::max(0, policy.min_chip_damage)},
        {"minChipHealing", std::max(0, policy.min_chip_healing)},
        {"maxBuffLevel", std::max(0, policy.max_buff_level)},
        {"zeroDamagePolicy", toString(policy.zero_damage_policy)},
        {"reuseTroopPositions", policy.reuse_troop_positions},
    };
}

BattleFeedbackPolicy BattleRuleResolver::feedbackPolicyFromJson(const nlohmann::json& json) {
    BattleFeedbackPolicy policy;
    if (!json.is_object()) {
        return policy;
    }
    policy.chip_damage_percent = std::clamp(json.value("chipDamagePercent", policy.chip_damage_percent), 0, 100);
    policy.chip_healing_percent = std::clamp(json.value("chipHealingPercent", policy.chip_healing_percent), 0, 100);
    policy.min_chip_damage = std::max(0, json.value("minChipDamage", policy.min_chip_damage));
    policy.min_chip_healing = std::max(0, json.value("minChipHealing", policy.min_chip_healing));
    policy.max_buff_level = std::max(0, json.value("maxBuffLevel", policy.max_buff_level));
    policy.zero_damage_policy =
        zeroDamagePolicyFromString(json.value("zeroDamagePolicy", toString(policy.zero_damage_policy)));
    policy.reuse_troop_positions = json.value("reuseTroopPositions", policy.reuse_troop_positions);
    return policy;
}

BattleFeedbackPolicy BattleRuleResolver::migrateFeedbackPolicy(const nlohmann::json& legacy_json) {
    return importFeedbackPolicyFixture(legacy_json).policy;
}

BattleFeedbackPolicyImportResult BattleRuleResolver::importFeedbackPolicyFixture(const nlohmann::json& fixture_json) {
    BattleFeedbackPolicyImportResult result;
    const auto& source = FeedbackPolicySourceObject(fixture_json);
    const std::string fixture_name =
        fixture_json.is_object() ? fixture_json.value("name", "battle_feedback_fixture") : "battle_feedback_fixture";

    std::string chip_damage_key;
    if (const auto value = ReadImportInt(source,
                                         {"chipDamagePercent", "chip_damage_percent", "Chip Damage Percent",
                                          "Chip Damage Percentage", "Chip Damage"},
                                         chip_damage_key, result.diagnostics);
        value.has_value()) {
        result.policy.chip_damage_percent = ClampImportedValue(*value, 0, 100, chip_damage_key, result.diagnostics);
        result.imported = true;
    }

    std::string chip_healing_key;
    if (const auto value = ReadImportInt(source,
                                         {"chipHealingPercent", "chip_healing_percent", "Chip Healing Percent",
                                          "Chip Healing Percentage", "Chip Healing"},
                                         chip_healing_key, result.diagnostics);
        value.has_value()) {
        result.policy.chip_healing_percent = ClampImportedValue(*value, 0, 100, chip_healing_key, result.diagnostics);
        result.imported = true;
    }

    std::string min_chip_damage_key;
    if (const auto value =
            ReadImportInt(source, {"minChipDamage", "min_chip_damage", "Minimum Chip Damage", "Min Chip Damage"},
                          min_chip_damage_key, result.diagnostics);
        value.has_value()) {
        result.policy.min_chip_damage =
            ClampImportedValue(*value, 0, INT32_MAX, min_chip_damage_key, result.diagnostics);
        result.imported = true;
    }

    std::string min_chip_healing_key;
    if (const auto value =
            ReadImportInt(source, {"minChipHealing", "min_chip_healing", "Minimum Chip Healing", "Min Chip Healing"},
                          min_chip_healing_key, result.diagnostics);
        value.has_value()) {
        result.policy.min_chip_healing =
            ClampImportedValue(*value, 0, INT32_MAX, min_chip_healing_key, result.diagnostics);
        result.imported = true;
    }

    std::string max_buff_key;
    if (const auto value = ReadImportInt(source,
                                         {"maxBuffLevel", "max_buff_level", "custom_buff_cap", "Custom Buff Levels",
                                          "Custom Buff Cap", "Max Buff Level"},
                                         max_buff_key, result.diagnostics);
        value.has_value()) {
        result.policy.max_buff_level = ClampImportedValue(*value, 0, INT32_MAX, max_buff_key, result.diagnostics);
        result.imported = true;
    }

    std::string zero_damage_key;
    if (const auto value = ReadImportString(
            source,
            {"zeroDamagePolicy", "zero_damage_policy", "Zero Damage Presentation", "Zero Damage Policy", "Zero Damage"},
            zero_damage_key, result.diagnostics);
        value.has_value()) {
        if (!IsSupportedZeroDamagePolicyValue(*value)) {
            AddImportDiagnostic(result.diagnostics, "feedback_policy_value_unsupported",
                                "Battle feedback fixture zero-damage presentation is not supported; using miss.",
                                zero_damage_key);
        }
        result.policy.zero_damage_policy = zeroDamagePolicyFromString(*value);
        result.imported = true;
    }

    std::string reuse_troop_key;
    if (const auto value = ReadImportBool(source,
                                          {"reuseTroopPositions", "reuse_troop_positions", "Reuse Troop Positions",
                                           "Troop Position Reuse", "Use Troop Positions"},
                                          reuse_troop_key, result.diagnostics);
        value.has_value()) {
        result.policy.reuse_troop_positions = *value;
        result.imported = true;
    }

    result.coverage_rows.push_back(
        MakeCoverageRow("chip_damage", chip_damage_key.empty() ? min_chip_damage_key : chip_damage_key));
    result.coverage_rows.push_back(
        MakeCoverageRow("chip_healing", chip_healing_key.empty() ? min_chip_healing_key : chip_healing_key));
    result.coverage_rows.push_back(MakeCoverageRow("zero_damage_presentation", zero_damage_key));
    result.coverage_rows.push_back(MakeCoverageRow("custom_buff_caps", max_buff_key));
    result.coverage_rows.push_back(MakeCoverageRow("troop_position_reuse", reuse_troop_key));

    if (result.imported) {
        result.diagnostics.insert(
            result.diagnostics.begin(),
            {"feedback_fixture_imported", "Battle feedback fixture imported with coverage evidence.", fixture_name});
    } else {
        result.diagnostics.insert(result.diagnostics.begin(),
                                  {"feedback_fixture_no_feedback_policy",
                                   "Battle feedback fixture did not contain recognized policy keys.", fixture_name});
    }
    return result;
}

int32_t BattleRuleResolver::resolveEscapeRatio(int32_t party_agi, int32_t troop_agi, int32_t fail_count) {
    const int32_t safe_party_agi = ClampPositive(party_agi);
    const int32_t safe_troop_agi = ClampPositive(troop_agi);
    const int32_t base_ratio = (safe_party_agi * 100) / safe_troop_agi;
    const int32_t fail_bonus = std::max(0, fail_count) * 10;
    return ClampRatioPercent(base_ratio + fail_bonus);
}

} // namespace urpg::battle
