#pragma once

#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/gameplay_effect.h"
#include "engine/core/ability/pattern_field.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace urpg::ability {

struct AuthoredAbilityAsset {
    std::string ability_id = "skill.draft";
    float cooldown_seconds = 3.0f;
    float mp_cost = 5.0f;
    std::string effect_id = "draft.effect";
    std::string effect_attribute = "Attack";
    urpg::ModifierOp effect_operation = urpg::ModifierOp::Add;
    float effect_value = 10.0f;
    float effect_duration = 5.0f;
    urpg::PatternField pattern = urpg::PatternField("Draft Pattern");
};

struct AuthoredAbilityAssetRecord {
    std::filesystem::path absolute_path;
    std::string relative_path;
    std::string ability_id;
};

inline const char* modifierOpName(urpg::ModifierOp operation) {
    switch (operation) {
    case urpg::ModifierOp::Add:
        return "Add";
    case urpg::ModifierOp::Multiply:
        return "Multiply";
    case urpg::ModifierOp::Override:
        return "Override";
    }
    return "Add";
}

inline urpg::ModifierOp modifierOpFromString(const std::string& value) {
    if (value == "Multiply") {
        return urpg::ModifierOp::Multiply;
    }
    if (value == "Override") {
        return urpg::ModifierOp::Override;
    }
    return urpg::ModifierOp::Add;
}

inline void to_json(nlohmann::json& j, const AuthoredAbilityAsset& asset) {
    j = nlohmann::json{
        {"ability_id", asset.ability_id},
        {"cooldown_seconds", asset.cooldown_seconds},
        {"mp_cost", asset.mp_cost},
        {"effect_id", asset.effect_id},
        {"effect_attribute", asset.effect_attribute},
        {"effect_operation", modifierOpName(asset.effect_operation)},
        {"effect_value", asset.effect_value},
        {"effect_duration", asset.effect_duration},
        {"pattern", asset.pattern},
    };
}

inline void from_json(const nlohmann::json& j, AuthoredAbilityAsset& asset) {
    asset.ability_id = j.value("ability_id", asset.ability_id);
    asset.cooldown_seconds = j.value("cooldown_seconds", asset.cooldown_seconds);
    asset.mp_cost = j.value("mp_cost", asset.mp_cost);
    asset.effect_id = j.value("effect_id", asset.effect_id);
    asset.effect_attribute = j.value("effect_attribute", asset.effect_attribute);
    asset.effect_operation = modifierOpFromString(j.value("effect_operation", std::string("Add")));
    asset.effect_value = j.value("effect_value", asset.effect_value);
    asset.effect_duration = j.value("effect_duration", asset.effect_duration);
    if (j.contains("pattern") && j["pattern"].is_object()) {
        asset.pattern = j["pattern"].get<urpg::PatternField>();
    }
}

inline std::shared_ptr<GameplayAbility> makeGameplayAbilityFromAsset(const AuthoredAbilityAsset& asset) {
    class AuthoredAssetAbility final : public GameplayAbility {
    public:
        explicit AuthoredAssetAbility(AuthoredAbilityAsset value)
            : asset_(std::move(value)),
              pattern_(std::make_shared<urpg::PatternField>(asset_.pattern)) {}

        const std::string& getId() const override {
            return asset_.ability_id;
        }

        const ActivationInfo& getActivationInfo() const override {
            activation_info_.cooldownSeconds = asset_.cooldown_seconds;
            activation_info_.mpCost = static_cast<int32_t>(asset_.mp_cost);
            activation_info_.pattern = pattern_;
            return activation_info_;
        }

        void activate(AbilitySystemComponent& source) override {
            commitAbility(source);

            urpg::GameplayEffect effect;
            effect.id = asset_.effect_id;
            effect.name = asset_.effect_id;
            effect.duration = asset_.effect_duration;

            urpg::GameplayEffectModifier modifier;
            modifier.attributeName = asset_.effect_attribute;
            modifier.operation = asset_.effect_operation;
            modifier.value = asset_.effect_value;
            effect.modifiers.push_back(modifier);

            source.applyEffect(effect);
        }

    private:
        AuthoredAbilityAsset asset_;
        std::shared_ptr<urpg::PatternField> pattern_;
        mutable ActivationInfo activation_info_;
    };

    return std::make_shared<AuthoredAssetAbility>(asset);
}

inline std::filesystem::path canonicalAbilityContentDirectory(const std::filesystem::path& project_root) {
    return project_root / "content" / "abilities";
}

inline bool saveAuthoredAbilityAssetToFile(const AuthoredAbilityAsset& asset, const std::filesystem::path& path,
                                           std::string* error = nullptr) {
    std::error_code ec;
    const auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            if (error != nullptr) {
                *error = "Unable to create ability asset directory: " + parent.generic_string() + " (" + ec.message() +
                         ")";
            }
            return false;
        }
    }

    std::ofstream ofs(path);
    if (!ofs) {
        if (error != nullptr) {
            *error = "Unable to open ability asset file for write: " + path.generic_string();
        }
        return false;
    }

    const nlohmann::json json = asset;
    ofs << json.dump(2);
    if (!ofs.good()) {
        if (error != nullptr) {
            *error = "Unable to finish writing ability asset file: " + path.generic_string();
        }
        return false;
    }
    if (error != nullptr) {
        error->clear();
    }
    return true;
}

inline std::optional<AuthoredAbilityAsset> loadAuthoredAbilityAssetFromFile(const std::filesystem::path& path,
                                                                           std::string* error = nullptr) {
    std::ifstream ifs(path);
    if (!ifs) {
        if (error != nullptr) {
            *error = "Unable to open ability asset file for read: " + path.generic_string();
        }
        return std::nullopt;
    }

    nlohmann::json json;
    try {
        ifs >> json;
    } catch (const nlohmann::json::exception& ex) {
        if (error != nullptr) {
            *error = "Unable to parse ability asset file: " + path.generic_string() + " (" + ex.what() + ")";
        }
        return std::nullopt;
    }

    if (!json.is_object()) {
        if (error != nullptr) {
            *error = "Ability asset file is not a JSON object: " + path.generic_string();
        }
        return std::nullopt;
    }

    try {
        auto asset = json.get<AuthoredAbilityAsset>();
        if (error != nullptr) {
            error->clear();
        }
        return asset;
    } catch (const nlohmann::json::exception& ex) {
        if (error != nullptr) {
            *error = "Ability asset file has invalid authored asset fields: " + path.generic_string() + " (" +
                     ex.what() + ")";
        }
        return std::nullopt;
    }
}

inline std::vector<AuthoredAbilityAssetRecord> discoverAuthoredAbilityAssets(const std::filesystem::path& project_root) {
    std::vector<AuthoredAbilityAssetRecord> records;
    const auto content_dir = canonicalAbilityContentDirectory(project_root);
    std::error_code ec;
    if (!std::filesystem::exists(content_dir, ec) || ec) {
        return records;
    }

    for (std::filesystem::recursive_directory_iterator it(content_dir, ec), end; !ec && it != end; it.increment(ec)) {
        if (!it->is_regular_file()) {
            continue;
        }
        if (it->path().extension() != ".json") {
            continue;
        }

        const auto asset = loadAuthoredAbilityAssetFromFile(it->path());
        if (!asset.has_value()) {
            continue;
        }

        AuthoredAbilityAssetRecord record;
        record.absolute_path = it->path();
        record.relative_path = std::filesystem::relative(it->path(), project_root, ec).generic_string();
        if (ec) {
            ec.clear();
            record.relative_path = it->path().generic_string();
        }
        record.ability_id = asset->ability_id;
        records.push_back(std::move(record));
    }

    std::sort(records.begin(),
              records.end(),
              [](const AuthoredAbilityAssetRecord& lhs, const AuthoredAbilityAssetRecord& rhs) {
                  return lhs.relative_path < rhs.relative_path;
              });
    return records;
}

} // namespace urpg::ability
