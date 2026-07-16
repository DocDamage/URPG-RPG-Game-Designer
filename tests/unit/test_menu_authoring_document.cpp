#include "engine/core/ui/menu_authoring_document.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::ui::MenuCanvasNode node(std::string id, int x, int y, int width = 160, int height = 64) {
    urpg::ui::MenuCanvasNode result;
    result.id = std::move(id);
    result.layout = {x, y, width, height};
    return result;
}

} // namespace

TEST_CASE("Menu authoring canvas persists hierarchy geometry snapping alignment and undo",
          "[ui][menu][authoring][pcq480]") {
    using namespace urpg::ui;
    MenuAuthoringDocument document;
    REQUIRE(document.setCanvas({1920, 1080}));
    REQUIRE(document.setSafeAreaMargin(48));
    document.setSnapGrid(16);

    auto root = node("root", 48, 48, 1824, 984);
    REQUIRE(document.addNode(root));
    auto first = node("first", 61, 80); first.parent_id = "root";
    auto second = node("second", 301, 160); second.parent_id = "root";
    auto third = node("third", 541, 240); third.parent_id = "root";
    REQUIRE(document.addNode(first));
    REQUIRE(document.addNode(second));
    REQUIRE(document.addNode(third));
    REQUIRE(document.childrenOf("root").size() == 3);
    REQUIRE(document.select({"first", "second", "third"}));
    REQUIRE(document.moveSelection(4, 0, true).changed);
    REQUIRE(document.findNode("first")->layout.x == 64);
    REQUIRE(document.alignSelection(MenuCanvasAlignment::Top).changed);
    REQUIRE(document.findNode("third")->layout.y == document.findNode("first")->layout.y);
    REQUIRE(document.distributeSelection(MenuCanvasDistribution::Horizontal).changed);
    REQUIRE(document.resizeNode("second", 201, 81, true).changed);
    REQUIRE(document.findNode("second")->layout.width == 208);
    REQUIRE(document.undo());
    REQUIRE(document.findNode("second")->layout.width == 160);
    REQUIRE(document.redo());

    const auto serialized = document.toJson();
    const auto restored = MenuAuthoringDocument::fromJson(serialized);
    REQUIRE(restored.has_value());
    REQUIRE(restored->canvas().width == 1920);
    REQUIRE(restored->safeAreaMargin() == 48);
    REQUIRE(restored->nodes().size() == 4);
    REQUIRE(resolveMenuPaneLayoutForCanvas(restored->findNode("first")->layout,
                                           restored->canvas(), {1280, 720}).isValid());
}

TEST_CASE("Menu component updates preserve instance overrides and resolve style tokens",
          "[ui][menu][components][pcq481]") {
    using namespace urpg::ui;
    MenuComponentLibrary library;
    REQUIRE(library.defineStyleToken({"color.primary", "#4CC9F0"}));
    REQUIRE(library.resolveStyleToken("color.primary") == "#4CC9F0");
    REQUIRE(library.define({"action_card", 1, {"icon"}, {{"label", "Continue"}},
                            {{MenuVisualState::Default, {{"fill", "{color.primary}"}}}}}));
    auto instance = library.instantiate("action_card", "continue_card", {64, 64, 320, 96},
                                        {{"label", "Resume"}, {"accent", "gold"}});
    REQUIRE(instance.has_value());
    REQUIRE(instance->label == "Resume");
    MenuAuthoringDocument document;
    REQUIRE(document.addNode(*instance));

    MenuComponentDefinition replacement{"action_card", 2, {"icon", "badge"},
        {{"label", "Continue Game"}}, {{MenuVisualState::Focus, {{"outline", "2"}}}}};
    const auto preview = library.previewUpdate(document, replacement);
    REQUIRE(preview.valid);
    REQUIRE(preview.instance_ids == std::vector<std::string>{"continue_card"});
    REQUIRE(preview.preserved_override_keys.size() == 2);
    const auto applied = library.applyUpdate(document, replacement);
    REQUIRE(applied.valid);
    REQUIRE(document.findNode("continue_card")->component_version == 2);
    REQUIRE(document.findNode("continue_card")->label == "Resume");
    REQUIRE(document.findNode("continue_card")->instance_overrides.at("accent") == "gold");
}

TEST_CASE("Menu visual states resolve interruption reduced motion and audio policy",
          "[ui][menu][states][pcq482]") {
    using namespace urpg::ui;
    MenuCanvasNode button = node("button", 0, 0);
    button.state_styles = {
        {MenuVisualState::Default, {{"opacity", "1"}}}, {MenuVisualState::Focus, {{"outline", "2"}}},
        {MenuVisualState::Pressed, {{"scale", "0.98"}}}, {MenuVisualState::Disabled, {{"opacity", "0.5"}}},
        {MenuVisualState::Selected, {{"mark", "check"}}}, {MenuVisualState::Loading, {{"cue", "spinner"}}},
        {MenuVisualState::Error, {{"cue", "warning"}}}};
    button.transitions.push_back({MenuVisualState::Focus, MenuVisualState::Pressed, 180,
                                  MenuTransitionInterruption::Replace, "ui.confirm", false, false});
    REQUIRE(button.state_styles.size() == 7);
    const auto normal = resolveMenuTransition(button, MenuVisualState::Focus,
                                              MenuVisualState::Pressed, false, true);
    REQUIRE(normal.duration_ms == 180);
    REQUIRE(normal.audio_hook == "ui.confirm");
    REQUIRE_FALSE(normal.immediate);
    const auto reduced = resolveMenuTransition(button, MenuVisualState::Focus,
                                               MenuVisualState::Pressed, true, false);
    REQUIRE(reduced.duration_ms == 0);
    REQUIRE(reduced.audio_hook.empty());
    REQUIRE(reduced.immediate);
}

TEST_CASE("Menu typed bindings localize format and visibly reject required fallback-only data",
          "[ui][menu][bindings][pcq483]") {
    using namespace urpg::ui;
    MenuBindingContext context;
    context.runtime["party.gold"] = int64_t{1250};
    context.localization["menu.gold"] = "Gold";
    const auto gold = resolveMenuBinding({"text", MenuBindingSource::Runtime, "party.gold",
                                          MenuBindingValueType::Integer, int64_t{0}, "{} G", true}, context);
    REQUIRE(gold.valid);
    REQUIRE(gold.rendered == "1250 G");
    const auto localized = resolveMenuBinding({"text", MenuBindingSource::Localization, "menu.gold",
                                               MenuBindingValueType::String, std::string("Gold"), {}, true}, context);
    REQUIRE(localized.valid);
    REQUIRE(localized.rendered == "Gold");
    const auto missing = resolveMenuBinding({"text", MenuBindingSource::Localization, "menu.missing",
                                             MenuBindingValueType::String, std::string("[missing]"), {}, true}, context);
    REQUIRE_FALSE(missing.valid);
    REQUIRE(missing.used_fallback);
    REQUIRE(missing.rendered == "[missing]");
    REQUIRE(missing.diagnostics.size() == 2);
}

TEST_CASE("Menu authoring audit exposes focus safe area overflow text stress and glyph previews",
          "[ui][menu][audit][pcq484]") {
    using namespace urpg::ui;
    MenuAuthoringDocument document;
    auto first = node("first", 0, 0, 80, 40);
    first.kind = MenuElementKind::Button;
    first.label = "An intentionally overflowing localized label";
    first.focusable = true;
    first.required_action = true;
    first.focus_next_id = "missing";
    REQUIRE(document.addNode(first));
    auto orphan = node("orphan", 1100, 660, 240, 80);
    orphan.kind = MenuElementKind::Button;
    orphan.label = "Other";
    orphan.accessible_label = "Other";
    orphan.focusable = true;
    orphan.required_action = true;
    orphan.focus_next_id = "orphan";
    REQUIRE(document.addNode(orphan));
    const auto audit = auditMenuAuthoringDocument(document, {{1280, 720}, MenuInputPreview::Controller, 1.8F, true});
    REQUIRE_FALSE(audit.package_safe);
    REQUIRE(audit.controller_glyph_set == "generic-controller");
    REQUIRE(audit.issues.size() >= 6);
    REQUIRE(menuTargetResolutionPresets().size() == 5);
}

TEST_CASE("Original URPG starter menu templates cover every required editable surface",
          "[ui][menu][templates][pcq485]") {
    const auto library = urpg::ui::MenuStarterTemplateLibrary::originalUrpgTemplates();
    const std::vector<std::string> required = {"title", "save_load", "settings", "pause", "inventory",
        "equipment", "quest_log", "dialogue", "shop", "battle_hud", "results"};
    REQUIRE(library.templates().size() == required.size());
    for (const auto& id : required) {
        const auto* item = library.find(id);
        REQUIRE(item != nullptr);
        REQUIRE(item->editable);
        REQUIRE(item->package_safe);
        REQUIRE(item->accessible);
        REQUIRE(item->document.nodes().size() == 2);
        REQUIRE(urpg::ui::MenuAuthoringDocument::fromJson(item->document.toJson()).has_value());
    }
}
