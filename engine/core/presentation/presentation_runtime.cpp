#include "presentation_runtime.h"
#include "map_scene_translator.h"
#include "battle_scene_translator.h"

namespace urpg::presentation {

PresentationFrameIntent PresentationRuntime::BuildPresentationFrame(
    const PresentationContext& context,
    const PresentationAuthoringData& data) 
{
    PresentationFrameIntent intent;
    // Use context values if they differ from project defaults (runtime overrides)
    intent.activeMode = context.activeMode;
    intent.activeTier = context.activeTier;

    // 1. Background Pass
    RenderPass bgPass{"Background", RenderPassType::Background};
    bgPass.commandStartIndex = intent.commands.size();
    // (Static skybox/backdrop commands would go here)
    bgPass.commandCount = intent.commands.size() - bgPass.commandStartIndex;
    intent.AddPass(bgPass);

    // 2. World Spatial Pass (The core MapScene/BattleScene emission)
    RenderPass worldPass{"WorldSpatial", RenderPassType::WorldSpatial};
    worldPass.commandStartIndex = intent.commands.size();
    
    // Dispatch to correct scene translator
    if (!context.battleState.battleArenaId.empty()) {
        BattleSceneTranslatorImpl battleTranslator;
        battleTranslator.Translate(context, data, context.battleState, intent);
    } else {
        MapSceneTranslatorImpl mapTranslator;
        mapTranslator.Translate(context, data, context.mapState, intent);
    }

    PresentationRuntime::ResolveEnvironmentCommands(intent);

    worldPass.commandCount = intent.commands.size() - worldPass.commandStartIndex;
    intent.AddPass(worldPass);

    // 3. UI Pass
    RenderPass uiPass{"UserInterface", RenderPassType::UserInterface};
    uiPass.useDepthTesting = false;
    uiPass.commandStartIndex = intent.commands.size();
    // (UI icons, dialogue boxes, HUD commands would go here)
    uiPass.commandCount = intent.commands.size() - uiPass.commandStartIndex;
    intent.AddPass(uiPass);

    return intent;
}

} // namespace urpg::presentation
