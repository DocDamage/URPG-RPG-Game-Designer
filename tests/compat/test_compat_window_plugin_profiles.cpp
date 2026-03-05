#include "runtimes/compat_js/quickjs_runtime.h"
#include "runtimes/compat_js/window_compat.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

using urpg::compat::CompatStatus;

struct ApiRequirement {
    std::string apiName;
    CompatStatus maxAllowedStatus = CompatStatus::PARTIAL;
};

struct PluginProfile {
    std::string pluginName;
    std::vector<ApiRequirement> requiredApis;
};

bool isStatusAccepted(CompatStatus actual, CompatStatus maxAllowed) {
    return static_cast<int>(actual) <= static_cast<int>(maxAllowed);
}

std::vector<PluginProfile> buildCuratedProfiles() {
    return {
        {
            "VisuStella_CoreEngine_MZ",
            {
                {"Window_Base.drawText", CompatStatus::FULL},
                {"Window_Base.drawIcon", CompatStatus::FULL},
                {"Window_Base.drawTextEx", CompatStatus::PARTIAL},
                {"Window_Base.lineHeight", CompatStatus::FULL},
            }
        },
        {
            "VisuStella_MainMenuCore_MZ",
            {
                {"Window_Selectable.setIndex", CompatStatus::FULL},
                {"Window_Selectable.cursorDown", CompatStatus::FULL},
                {"Window_Selectable.cursorUp", CompatStatus::FULL},
                {"Window_Command.addCommand", CompatStatus::FULL},
                {"Window_Command.selectSymbol", CompatStatus::FULL},
            }
        },
        {
            "VisuStella_OptionsCore_MZ",
            {
                {"Window_Command.clearCommands", CompatStatus::FULL},
                {"Window_Command.getCurrentSymbol", CompatStatus::FULL},
                {"Window_Command.selectExt", CompatStatus::FULL},
            }
        },
        {
            "CGMZ_MenuCommandWindow",
            {
                {"Window_Selectable.getItemWidth", CompatStatus::FULL},
                {"Window_Selectable.setTopRow", CompatStatus::FULL},
                {"Window_Selectable.cursorPagedown", CompatStatus::FULL},
                {"Window_Selectable.cursorPageup", CompatStatus::FULL},
            }
        },
        {
            "CGMZ_Encyclopedia",
            {
                {"Window_Command.drawAllItems", CompatStatus::FULL},
                {"Window_Command.drawItem", CompatStatus::FULL},
                {"Window_Command.findSymbol", CompatStatus::FULL},
            }
        },
        {
            "EliMZ_Book",
            {
                {"Window_Base.drawActorName", CompatStatus::FULL},
                {"Window_Base.drawActorLevel", CompatStatus::FULL},
                {"Window_Base.drawActorHp", CompatStatus::FULL},
                {"Window_Base.drawActorMp", CompatStatus::FULL},
            }
        },
        {
            "MOG_CharacterMotion_MZ",
            {
                {"Sprite_Character.setDirection", CompatStatus::FULL},
                {"Sprite_Character.setPattern", CompatStatus::FULL},
                {"Sprite_Character.setScale", CompatStatus::FULL},
            }
        },
        {
            "MOG_BattleHud_MZ",
            {
                {"Sprite_Actor.startMotion", CompatStatus::FULL},
                {"Sprite_Actor.startEffect", CompatStatus::PARTIAL},
                {"Sprite_Actor.setOpacity", CompatStatus::FULL},
            }
        },
        {
            "Galv_QuestLog_MZ",
            {
                {"Window_Command.isCommandEnabled", CompatStatus::FULL},
                {"Window_Command.isCurrentItemEnabled", CompatStatus::FULL},
                {"Window_Command.callOkHandler", CompatStatus::FULL},
            }
        },
        {
            "AltMenuScreen_MZ",
            {
                {"Window_Base.open", CompatStatus::FULL},
                {"Window_Base.close", CompatStatus::FULL},
                {"Window_Base.show", CompatStatus::FULL},
                {"Window_Base.hide", CompatStatus::FULL},
            }
        },
    };
}

} // namespace

TEST_CASE("Compat profiles: curated MZ plugin surface requirements are available", "[compat][profiles]") {
    urpg::compat::QuickJSContext ctx;
    REQUIRE(ctx.initialize(urpg::compat::QuickJSConfig{}));

    urpg::compat::WindowCompatManager manager;
    manager.registerAllAPIs(ctx);

    const auto profiles = buildCuratedProfiles();
    REQUIRE(profiles.size() == 10);

    for (const auto& profile : profiles) {
        INFO("Profile: " << profile.pluginName);
        REQUIRE_FALSE(profile.requiredApis.empty());

        for (const auto& requirement : profile.requiredApis) {
            INFO("Requirement: " << requirement.apiName);
            const auto status = ctx.getAPIStatus(requirement.apiName);
            REQUIRE(status.status != CompatStatus::UNSUPPORTED);
            REQUIRE(status.status != CompatStatus::STUB);
            REQUIRE(isStatusAccepted(status.status, requirement.maxAllowedStatus));
        }
    }
}
