#pragma once

#include "editor/diagnostics/diagnostics_facade.h"
#include "editor/diagnostics/diagnostics_workspace.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/input/input_core.h"
#include "engine/core/message/message_core.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"

#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class WorkspaceAbility final : public urpg::ability::GameplayAbility {
  public:
    explicit WorkspaceAbility(std::string ability_id) : ability_id_(std::move(ability_id)) {}

    const std::string& getId() const override { return ability_id_; }
    const ActivationInfo& getActivationInfo() const override { return info_; }
    void activate([[maybe_unused]] urpg::ability::AbilitySystemComponent& source) override {}

    ActivationInfo& editInfo() { return info_; }

  private:
    std::string ability_id_;
    ActivationInfo info_;
};

class SceneBoundWorkspaceAbility final : public urpg::ability::GameplayAbility {
  public:
    SceneBoundWorkspaceAbility() {
        id = "skill.live_scene";
        mpCost = 5.0f;
        cooldownTime = 3.0f;
    }

    const std::string& getId() const override { return id; }
    const ActivationInfo& getActivationInfo() const override { return info_; }

    void activate(urpg::ability::AbilitySystemComponent& source) override { commitAbility(source); }

  private:
    ActivationInfo info_;
};

[[maybe_unused]] urpg::scene::BattleParticipant* findParticipant(
    std::vector<urpg::scene::BattleParticipant>& participants,
    bool is_enemy) {
    for (auto& participant : participants) {
        if (participant.isEnemy == is_enemy) {
            return &participant;
        }
    }
    return nullptr;
}

[[maybe_unused]] urpg::message::DialoguePage makeDialoguePage(
    std::string id,
    std::string body,
    urpg::message::MessagePresentationVariant variant,
    bool wait_for_advance = true,
    std::vector<urpg::message::ChoiceOption> choices = {},
    int32_t default_choice_index = 0) {
    urpg::message::DialoguePage page;
    page.id = std::move(id);
    page.body = std::move(body);
    page.variant = std::move(variant);
    page.wait_for_advance = wait_for_advance;
    page.choices = std::move(choices);
    page.default_choice_index = default_choice_index;
    return page;
}

[[maybe_unused]] void writeWorkspaceTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}


class WorkspacePreviewAbility final : public urpg::ability::GameplayAbility {
  public:
    WorkspacePreviewAbility(std::string ability_id, float cooldown_seconds, float mp_cost)
        : id_(std::move(ability_id)) {
        info_.cooldownSeconds = cooldown_seconds;
        info_.mpCost = static_cast<int32_t>(mp_cost);
    }

    const std::string& getId() const override { return id_; }
    const ActivationInfo& getActivationInfo() const override { return info_; }

    void activate(urpg::ability::AbilitySystemComponent& source) override { commitAbility(source); }

  private:
    std::string id_;
    ActivationInfo info_;
};

} // namespace
