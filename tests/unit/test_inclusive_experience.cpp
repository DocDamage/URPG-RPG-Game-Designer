#include "engine/core/accessibility/inclusive_experience.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/map/project_world_graph.h"
#include "engine/core/map/tile_layer_document.h"
#include "engine/core/localization/locale_catalog.h"
#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/ui/menu_authoring_document.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Unified input routes hot-plug ownership contexts chords axes repeat conflicts calibration and glyphs",
          "[input][inclusive][pcq650]") {
    using namespace urpg::accessibility;
    UnifiedInputRouter router;
    REQUIRE(router.connectDevice({"keyboard", InclusiveDeviceKind::KeyboardMouse, InputOwnership::Shared,
                                  true, 0.0F, 1.0F, "keyboard"}));
    REQUIRE(router.connectDevice({"pad1", InclusiveDeviceKind::Controller, InputOwnership::PlayerOne,
                                  true, 0.2F, 1.25F, "xbox"}));
    REQUIRE(router.bind({InclusiveInputContext::Menu, urpg::input::InputAction::Confirm,
                         {"a"}, 300, 80}, false));
    REQUIRE(router.bind({InclusiveInputContext::Gameplay, urpg::input::InputAction::Menu,
                         {"left_bumper", "start"}, 350, 100}, false));
    REQUIRE(router.conflictFor({InclusiveInputContext::Menu, urpg::input::InputAction::Cancel,
                                {"a"}, 300, 80}).has_value());
    REQUIRE_FALSE(router.bind({InclusiveInputContext::Menu, urpg::input::InputAction::Cancel,
                               {"a"}, 300, 80}, false));
    REQUIRE(router.bind({InclusiveInputContext::Menu, urpg::input::InputAction::Cancel,
                         {"a"}, 300, 80}, true));
    REQUIRE(router.calibrate("pad1", 0.25F, 1.1F));
    const auto routed = router.route("pad1", InclusiveInputContext::Menu, {"a"}, 0.75F, 0);
    REQUIRE(routed.accepted);
    REQUIRE(routed.action == urpg::input::InputAction::Cancel);
    REQUIRE(routed.active_device_changed);
    REQUIRE(routed.normalized_value > 0.0F);
    REQUIRE(routed.glyph == "xbox:a");
    REQUIRE_FALSE(router.route("pad1", InclusiveInputContext::Menu, {"a"}, 0.75F, 100,
                               InputOwnership::PlayerOne).accepted);
    REQUIRE(router.route("pad1", InclusiveInputContext::Menu, {"a"}, 0.75F, 300,
                         InputOwnership::PlayerOne).accepted);
    REQUIRE_FALSE(router.route("pad1", InclusiveInputContext::Menu, {"a"}, 0.75F, 301,
                               InputOwnership::PlayerOne).accepted);
    REQUIRE(router.route("pad1", InclusiveInputContext::Menu, {"a"}, 0.75F, 380,
                         InputOwnership::PlayerOne).accepted);
    REQUIRE_FALSE(router.route("pad1", InclusiveInputContext::Menu, {"a"}, 0.75F, 0,
                               InputOwnership::PlayerTwo).accepted);
    urpg::input::InputCore core;
    REQUIRE(router.dispatch(core, "pad1", InclusiveInputContext::Menu, {"a"},
                            urpg::input::ActionState::Pressed, 0.0F, 0,
                            InputOwnership::PlayerOne).accepted);
    REQUIRE(core.isActionJustPressed(urpg::input::InputAction::Cancel));
    REQUIRE(router.dispatch(core, "pad1", InclusiveInputContext::Menu, {"a"},
                            urpg::input::ActionState::Released, 0.0F, 0,
                            InputOwnership::PlayerOne).accepted);
    REQUIRE(core.isActionJustReleased(urpg::input::InputAction::Cancel));
    REQUIRE(router.disconnectDevice("pad1"));
    REQUIRE_FALSE(router.route("pad1", InclusiveInputContext::Menu, {"a"}, 1.0F, 0).accepted);
    REQUIRE(router.reconnectDevice("pad1"));
    REQUIRE(router.recoveryActions().size() == 4);
}

TEST_CASE("Inclusive settings persist scale contrast filters motion captions audio and mono alternatives",
          "[accessibility][settings][pcq651]") {
    using namespace urpg::accessibility;
    auto settings = InclusiveSettings::safeDefaults();
    settings.text_scale = TextScaleProfile::ExtraLarge;
    settings.high_contrast = true;
    settings.color_filter = ColorFilter::Deuteranopia;
    settings.reduced_motion = true;
    settings.screen_shake = 0.0F;
    settings.flash_intensity = 0.2F;
    settings.caption_scale = 1.75F;
    settings.music_volume = 0.4F;
    settings.voice_volume = 0.8F;
    settings.mono_audio = true;
    REQUIRE(settings.isValid());
    const auto restored = InclusiveSettings::fromJson(settings.toJson());
    REQUIRE(restored.has_value());
    REQUIRE(restored->text_scale == TextScaleProfile::ExtraLarge);
    REQUIRE(restored->high_contrast);
    REQUIRE(restored->reduced_motion);
    REQUIRE(restored->screen_shake == 0.0F);
    REQUIRE(restored->captions);
    REQUIRE(restored->mono_audio);
    auto invalid = settings.toJson();
    invalid["non_color_cues"] = false;
    REQUIRE_FALSE(InclusiveSettings::fromJson(invalid).has_value());
}

TEST_CASE("Inclusive localization resolves fallback plural grammar RTL IME glyphs formats and stale keys",
          "[localization][inclusive][pcq652]") {
    using namespace urpg::accessibility;
    InclusiveLocalizationModel model;
    std::set<char32_t> latin;
    for (char32_t glyph = 32; glyph < 127; ++glyph) latin.insert(glyph);
    REQUIRE(model.defineLocale({"en-US", {}, TextDirection::LeftToRight, "latin", latin, true}));
    REQUIRE(model.defineLocale({"fr-FR", "en-US", TextDirection::LeftToRight, "latin", latin, true}));
    REQUIRE(model.defineLocale({"ar", "en-US", TextDirection::RightToLeft, "arabic", {U' '}, true}));
    REQUIRE(model.addVariant({"quest.items", "en-US", "One item", "one", GrammarVariant::Neutral, 2}));
    REQUIRE(model.addVariant({"quest.items", "en-US", "{count} items", "other", GrammarVariant::Neutral, 1}));
    const auto one = model.resolve("quest.items", "en-US", 1, GrammarVariant::Feminine);
    REQUIRE(one.valid);
    REQUIRE(one.text == "One item");
    const auto fallback = model.resolve("quest.items", "fr-FR", 5, GrammarVariant::Neutral);
    REQUIRE(fallback.valid);
    REQUIRE(fallback.used_fallback);
    REQUIRE(fallback.text == "5 items");
    REQUIRE(model.staleKeys(2) == std::vector<std::string>{"quest.items@en-US"});
    REQUIRE(model.missingGlyphs("en-US", U"OK").empty());
    REQUIRE(model.missingGlyphs("en-US", U"✓").size() == 1);
    REQUIRE(model.formatNumber("fr-FR", 12.5) == "12,50");
    REQUIRE(model.formatDate("en-US", 2026, 7, 16) == "07/16/2026");
    REQUIRE(model.supportsIme("ar"));
}

TEST_CASE("Inclusive localization imports the production locale catalog with font glyph and IME policy",
          "[localization][inclusive][catalog_adapter][pcq652]") {
    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson({{"locale", "en-US"}, {"font_profile_id", "font.ui.latin"},
                          {"keys", {{"menu.start", "Start"}, {"menu.quit", "Quit"}}}});
    std::set<char32_t> glyphs;
    for (char32_t glyph = 32; glyph < 127; ++glyph) glyphs.insert(glyph);
    urpg::accessibility::InclusiveLocalizationModel model;
    REQUIRE(model.importCatalog(catalog, {}, urpg::accessibility::TextDirection::LeftToRight,
                                glyphs, true, 3));
    const auto resolved = model.resolve("menu.start", "en-US", 1,
                                        urpg::accessibility::GrammarVariant::Neutral);
    REQUIRE(resolved.valid);
    REQUIRE(resolved.text == "Start");
    REQUIRE(model.supportsIme("en-US"));
    REQUIRE(model.missingGlyphs("en-US", U"Start").empty());
    REQUIRE(model.staleKeys(4).size() == 2);
}

TEST_CASE("Caption audit enforces alignment speaker identity non-speech cues locales takes and alternatives",
          "[accessibility][captions][pcq653]") {
    using namespace urpg::accessibility;
    const std::vector<CaptionCue> complete = {
        {"guide.en", "guide", "en-US", "take-1", "Welcome", {"door opens"}, 0, 1000, 1000, true},
        {"guide.fr", "guide", "fr-FR", "take-2", "Bienvenue", {"porte"}, 0, 1050, 1000, true}};
    const auto passing = auditCaptionTrack(complete, {"en-US", "fr-FR"});
    REQUIRE(passing.complete);
    REQUIRE(passing.diagnostics.empty());
    auto broken = complete;
    broken[0].speaker_id.clear();
    broken[0].muted_alternative = false;
    broken[0].end_ms = 2000;
    const auto failing = auditCaptionTrack(broken, {"en-US", "fr-FR", "ja-JP"});
    REQUIRE_FALSE(failing.complete);
    REQUIRE(failing.diagnostics.size() == 4);
}

TEST_CASE("Dialogue voice and localization references build an auditable timed caption track",
          "[accessibility][captions][dialogue_adapter][pcq653]") {
    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson({{"locale", "en-US"}, {"font_profile_id", "font.ui.latin"},
                          {"keys", {{"dialogue.guide", "Welcome, traveler."}}}});
    urpg::dialogue::DialogueGraph graph;
    urpg::dialogue::DialogueNode line;
    line.id = "welcome";
    line.speaker_id = "guide";
    line.speaker_name = "Guide";
    line.localization_key = "dialogue.guide";
    line.text_preview = "Welcome.";
    line.ending = true;
    line.voice_asset_id = "voice.guide.en";
    line.caption_localization_key = "dialogue.guide";
    REQUIRE(graph.addNode(line));
    urpg::accessibility::DialogueCaptionTrackInput input;
    input.locale = "en-US";
    input.default_take_id = "take-1";
    input.catalog = &catalog;
    input.start_ms_by_node["welcome"] = 250;
    input.voice_duration_ms_by_asset["voice.guide.en"] = 1200;
    input.non_speech_cues_by_node["welcome"] = {"door opens"};
    input.muted_alternative_voice_assets.insert("voice.guide.en");
    const auto built = urpg::accessibility::buildDialogueCaptionTrack(graph, input);
    REQUIRE(built.diagnostics.empty());
    REQUIRE(built.cues.size() == 1);
    REQUIRE(built.cues[0].text == "Welcome, traveler.");
    REQUIRE(built.cues[0].start_ms == 250);
    REQUIRE(built.cues[0].end_ms == 1450);
    REQUIRE(built.cues[0].non_speech_cues == std::vector<std::string>{"door opens"});
    REQUIRE(urpg::accessibility::auditCaptionTrack(built.cues, {"en-US"}).complete);
}

TEST_CASE("Semantic editor alternative supports ordered tree properties and connection creation",
          "[accessibility][semantic_editor][pcq654]") {
    using namespace urpg::accessibility;
    SemanticEditorAlternative editor;
    REQUIRE(editor.addNode({"start", "Start", "dialogue", 10, {}, {}}));
    REQUIRE(editor.addNode({"choice", "Choice", "branch", 20, {}, {}}));
    REQUIRE(editor.updateProperty("choice", "text", "Continue?"));
    REQUIRE(editor.connect("start", "choice"));
    REQUIRE(editor.reorder("choice", 5));
    const auto ordered = editor.orderedNodes();
    REQUIRE(ordered.front().id == "choice");
    REQUIRE(ordered.back().connections == std::vector<std::string>{"choice"});
    REQUIRE(editor.diagnostics().empty());
    REQUIRE_FALSE(editor.connect("choice", "missing"));
}

TEST_CASE("Semantic alternatives adapt menu dialogue quest tile and world graph owners",
          "[accessibility][semantic_editor][adapters][pcq654]") {
    using namespace urpg::accessibility;
    urpg::ui::MenuAuthoringDocument menu;
    urpg::ui::MenuCanvasNode panel;
    panel.id = "panel";
    panel.label = "Panel";
    REQUIRE(menu.addNode(panel));
    urpg::ui::MenuCanvasNode button;
    button.id = "button";
    button.parent_id = "panel";
    button.kind = urpg::ui::MenuElementKind::Button;
    button.label = "Continue";
    REQUIRE(menu.addNode(button));
    const auto menu_alternative = semanticAlternativeForMenu(menu).orderedNodes();
    REQUIRE(menu_alternative.size() == 2);
    REQUIRE(menu_alternative[0].connections == std::vector<std::string>{"button"});

    urpg::dialogue::DialogueGraph dialogue;
    urpg::dialogue::DialogueNode dialogue_start;
    dialogue_start.id = "start";
    dialogue_start.speaker_id = "guide";
    dialogue_start.speaker_name = "Guide";
    dialogue_start.localization_key = "dialogue.start";
    dialogue_start.text_preview = "Hello";
    dialogue_start.choices = {{"next", "Continue", "end", {}, {}, {}}};
    REQUIRE(dialogue.addNode(dialogue_start));
    urpg::dialogue::DialogueNode dialogue_end;
    dialogue_end.id = "end";
    dialogue_end.speaker_id = "guide";
    dialogue_end.speaker_name = "Guide";
    dialogue_end.localization_key = "dialogue.end";
    dialogue_end.text_preview = "Done";
    dialogue_end.ending = true;
    REQUIRE(dialogue.addNode(dialogue_end));
    const auto dialogue_alternative = semanticAlternativeForDialogue(dialogue).orderedNodes();
    REQUIRE(dialogue_alternative.size() == 2);
    REQUIRE(dialogue_alternative[1].connections == std::vector<std::string>{"end"});

    urpg::quest::QuestObjectiveGraphDocument quest;
    urpg::quest::QuestGraphNode quest_begin;
    quest_begin.id = "begin";
    quest_begin.type = "start";
    quest_begin.title = "Begin";
    quest_begin.localization_key = "quest.begin";
    urpg::quest::QuestGraphNode quest_finish;
    quest_finish.id = "finish";
    quest_finish.type = "complete";
    quest_finish.title = "Finish";
    quest_finish.localization_key = "quest.finish";
    quest.nodes = {quest_begin, quest_finish};
    quest.links = {{"begin", "finish"}};
    const auto quest_alternative = semanticAlternativeForQuest(quest).orderedNodes();
    REQUIRE(quest_alternative[0].connections == std::vector<std::string>{"finish"});

    urpg::map::TileLayerDocument tile_map(2, 2);
    tile_map.addLayer({"ground", true, false, false, true, 0, {0, 0, 0, 0}});
    REQUIRE(semanticAlternativeForTileMap(tile_map).orderedNodes()[0].properties.at("visible") == "true");

    urpg::map::ProjectWorldGraph world;
    REQUIRE(world.addMap({"map-a", "Map A", {}, {{{"exit-a", "Exit", 0, 0}}}, {}, {}}));
    REQUIRE(world.addMap({"map-b", "Map B", {{{"entry-b", "Entry", 0, 0}}}, {}, {}, {}}));
    REQUIRE(world.addRoute({"route", "Route", "map-a", "exit-a", "map-b", "entry-b", ""}));
    const auto world_alternative = semanticAlternativeForWorldMap(world).orderedNodes();
    REQUIRE(world_alternative[0].connections == std::vector<std::string>{"map-b"});
}

TEST_CASE("Inclusive UI audit links focus contrast hit target clipping overflow and motion failures to objects",
          "[accessibility][audit][pcq655]") {
    using namespace urpg::accessibility;
    const auto issues = auditInclusiveUi({
        {"bad.primary", {}, true, 0, 3.0F, 30, 30, true, true, 300, false},
        {"bad.secondary", "Secondary", true, 0, 7.0F, 44, 44, false, false, 0, false}}, true, true);
    REQUIRE(issues.size() == 7);
    const std::set<std::string> codes = {"missing_label", "focus_order", "contrast", "hit_target",
                                          "clipping", "localization_overflow", "unsafe_motion"};
    for (const auto& issue : issues) {
        REQUIRE(codes.contains(issue.code));
        REQUIRE_FALSE(issue.object_id.empty());
        REQUIRE_FALSE(issue.message.empty());
    }
}
