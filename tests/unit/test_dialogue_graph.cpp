#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/assets/asset_promotion_manifest.h"
#include "engine/core/localization/project_localization_audit.h"
#include "editor/dialogue/dialogue_graph_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

TEST_CASE("dialogue graph supports speaker metadata localization choices effects and preview", "[dialogue][narrative][ffs10]") {
    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({
        "start",
        "guide",
        "Guide",
        "dialogue.start",
        "Welcome.",
        false,
        {{"choice_help", "Help", "end", {{"guide_affinity", ">=", 0}}, {{"guide_affinity", 5}}, {}}},
        {}, {}, 0, 0, false, {}, 0, 0, {},
    }));
    REQUIRE(graph.addNode({"end", "guide", "Guide", "dialogue.end", "Thanks.", true, {}, {}, {}, 0, 0, false, {}, 0, 0, {}}));

    const auto route = graph.previewRoute();
    const auto json = graph.serialize();

    REQUIRE(route == std::vector<std::string>{"start", "end"});
    REQUIRE(json["nodes"].size() == 2);
    const auto* start = graph.findNode("start");
    REQUIRE(start != nullptr);
    REQUIRE(start->choices[0].effects[0].key == "guide_affinity");

    urpg::editor::DialogueGraphPanel panel;
    panel.setGraph(graph);
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["node_count"] == 2);
    REQUIRE(panel.lastRenderSnapshot()["choice_count"] == 1);
    REQUIRE(panel.lastRenderSnapshot()["ending_count"] == 1);
    REQUIRE(panel.lastRenderSnapshot()["has_start_node"] == true);
    REQUIRE(panel.lastRenderSnapshot()["route_coverage"] == 1.0f);
    REQUIRE(panel.lastRenderSnapshot()["ux_focus_lane"] == "route_preview");
}

TEST_CASE("dialogue choices preserve optional localization references", "[dialogue][localization]") {
    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({"start", "guide", "Guide", "dialogue.start", "Welcome.", false, {}, {}, {}, 0, 0, false, {}, 0, 0, {}}));
    REQUIRE(graph.addNode({"end", "guide", "Guide", "dialogue.end", "Thanks.", true, {}, {}, {}, 0, 0, false, {}, 0, 0, {}}));
    REQUIRE(graph.addChoice("start", {"choice_help", "Help", "end", {}, {}, "dialogue.choice.help"}));

    const auto serialized = graph.serialize();
    const auto restored = urpg::dialogue::DialogueGraph::fromJson(serialized);
    REQUIRE(restored.has_value());
    const auto* start = restored->findNode("start");
    REQUIRE(start != nullptr);
    REQUIRE(start->choices[0].localization_key == "dialogue.choice.help");

    urpg::editor::DialogueGraphPanel panel;
    panel.setGraph(*restored);
    REQUIRE(panel.beginInteractivePreview());
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["interactive_preview"]["choices"][0]["localization_key"] ==
            "dialogue.choice.help");

    const auto missing = restored->validateLocalizationKeys({"dialogue.start", "dialogue.end"});
    REQUIRE(missing.size() == 1);
    CHECK(missing[0].code == "missing_choice_localization_key");
    CHECK(missing[0].node_id == "start");
    CHECK(missing[0].choice_id == "choice_help");
    REQUIRE(restored->validateLocalizationKeys(
                {"dialogue.start", "dialogue.end", "dialogue.choice.help"})
                .empty());

    auto legacy = serialized;
    for (auto& node : legacy["nodes"]) {
        if (node["id"] == "start") {
            node["choices"][0].erase("localization_key");
        }
    }
    const auto legacy_restored = urpg::dialogue::DialogueGraph::fromJson(legacy);
    REQUIRE(legacy_restored.has_value());
    const auto* legacy_start = legacy_restored->findNode("start");
    REQUIRE(legacy_start != nullptr);
    REQUIRE(legacy_start->choices[0].localization_key.empty());
}

TEST_CASE("dialogue media tracks persist locale takes timing cues and deterministic selection",
          "[dialogue][captions][voice][pcq653]") {
    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({"line", "guide", "Guide", "dialogue.line", "Hello", true, {}, {}, {}, 0, 0, false, {}, 0, 0, {}}));
    REQUIRE(graph.upsertNodeVoiceTake("line", {"en-US", "take.en.1", "voice.en", 1200, "muted.en"}));
    REQUIRE(graph.upsertNodeVoiceTake("line", {"fr-FR", "take.fr.1", "voice.fr", 1250, "muted.fr"}));
    REQUIRE(graph.updateNodeCaptionCue("line", "caption.line", 250, 1500, {"door opens"}));

    const auto* exact = urpg::dialogue::selectDialogueVoiceTake(*graph.findNode("line"), "fr-FR", "en-US");
    REQUIRE(exact != nullptr);
    CHECK(exact->voice_asset_id == "voice.fr");
    const auto* language = urpg::dialogue::selectDialogueVoiceTake(*graph.findNode("line"), "fr-CA", "en-US");
    REQUIRE(language != nullptr);
    CHECK(language->take_id == "take.fr.1");
    const auto* fallback = urpg::dialogue::selectDialogueVoiceTake(*graph.findNode("line"), "ja-JP", "en-US");
    REQUIRE(fallback != nullptr);
    CHECK(fallback->voice_asset_id == "voice.en");
    CHECK(graph.validate().empty());

    const auto restored = urpg::dialogue::DialogueGraph::fromJson(graph.serialize());
    REQUIRE(restored.has_value());
    const auto* line = restored->findNode("line");
    REQUIRE(line != nullptr);
    CHECK(line->voice_takes.size() == 2);
    CHECK(line->caption_start_ms == 250);
    CHECK(line->caption_end_ms == 1500);
    CHECK(line->non_speech_cues == std::vector<std::string>{"door opens"});

    REQUIRE(graph.upsertNodeVoiceTake("line", {"fr-FR", "take.fr.1", "voice.fr", 2000, "muted.fr"}));
    const auto diagnostics = graph.validate();
    REQUIRE(std::ranges::any_of(diagnostics, [](const auto& diagnostic) {
        return diagnostic.code == "voice_caption_alignment_exceeded";
    }));
    REQUIRE(graph.removeNodeVoiceTake("line", "fr-FR", "take.fr.1"));
    CHECK(graph.findNode("line")->voice_takes.size() == 1);
}

TEST_CASE("project localization audit reports dialogue voice, caption, and governed take custody",
          "[dialogue][localization][assets]") {
    const auto root = std::filesystem::temp_directory_path() /
                      ("urpg_dialogue_media_audit_" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "content" / "dialogues");
    std::filesystem::create_directories(root / "content" / "localization");
    const auto payload = root / "content" / "assets" / "imported" / "voice.present" / "line.wav";
    std::filesystem::create_directories(payload.parent_path());
    std::ofstream(payload, std::ios::binary | std::ios::trunc) << "audio";

    urpg::assets::AssetPromotionManifest voice;
    voice.assetId = "voice.present";
    voice.promotedPath = payload.generic_string();
    voice.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    voice.preview.kind = "audio";
    voice.package.includeInRuntime = true;
    voice.licenseId = "reviewed-voice-license";
    voice.authoredMetadata = {
        {"voice_take",
         {{"schema", "urpg.promoted_audio_voice_take.v1"},
          {"locale", "en-US"},
          {"take_id", "guide-take-001"},
          {"muted_alternative_asset_id", "voice.muted"}}},
    };
    const auto manifestPath = root / "content" / "assets" / "manifests" / "voice.present.json";
    std::filesystem::create_directories(manifestPath.parent_path());
    std::ofstream(manifestPath, std::ios::binary | std::ios::trunc)
        << urpg::assets::serializeAssetPromotionManifest(voice).dump(2) << '\n';
    const auto mutedPayload = root / "content" / "assets" / "imported" / "voice.muted" / "line.wav";
    std::filesystem::create_directories(mutedPayload.parent_path());
    std::ofstream(mutedPayload, std::ios::binary | std::ios::trunc) << "muted audio";
    auto muted = voice;
    muted.assetId = "voice.muted";
    muted.promotedPath = mutedPayload.generic_string();
    muted.authoredMetadata = nlohmann::json::object();
    std::ofstream(root / "content" / "assets" / "manifests" / "voice.muted.json", std::ios::binary | std::ios::trunc)
        << urpg::assets::serializeAssetPromotionManifest(muted).dump(2) << '\n';

    auto malformed = voice;
    malformed.assetId = "voice.malformed";
    malformed.authoredMetadata = {
        {"voice_take",
         {{"schema", "urpg.promoted_audio_voice_take.v0"},
          {"locale", "en_US"},
          {"take_id", "bad/take"},
          {"muted_alternative_asset_id", ""}}},
    };
    std::ofstream(root / "content" / "assets" / "manifests" / "voice.malformed.json", std::ios::binary | std::ios::trunc)
        << urpg::assets::serializeAssetPromotionManifest(malformed).dump(2) << '\n';

    auto missingAlternative = voice;
    missingAlternative.assetId = "voice.missing-alternative";
    missingAlternative.authoredMetadata["voice_take"]["muted_alternative_asset_id"] = "voice.not-attached";
    std::ofstream(root / "content" / "assets" / "manifests" / "voice.missing-alternative.json",
                  std::ios::binary | std::ios::trunc)
        << urpg::assets::serializeAssetPromotionManifest(missingAlternative).dump(2) << '\n';
    std::ofstream(root / "content" / "localization" / "en.json", std::ios::binary | std::ios::trunc)
        << nlohmann::json{{"locale", "en"}, {"font_profile_id", "latin"},
                          {"keys", {{"dialogue.caption", "Caption"}}}}
               .dump(2)
        << '\n';

    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({"complete", "guide", "Guide", "", "Spoken", true, {}, "voice.present", "dialogue.caption", 0, 0, false, {}, 0, 0, {}}));
    REQUIRE(graph.addNode({"voice_only", "guide", "Guide", "", "Uncaptioned", true, {}, "voice.missing", "", 0, 0, false, {}, 0, 0, {}}));
    REQUIRE(graph.addNode({"caption_only", "guide", "Guide", "", "Caption only", true, {}, "", "dialogue.caption", 0, 0, false, {}, 0, 0, {}}));
    REQUIRE(graph.addNode({"metadata_missing", "guide", "Guide", "", "Missing take", true, {}, "voice.muted", "dialogue.caption", 0, 0, false, {}, 0, 0, {}}));
    REQUIRE(graph.addNode({"malformed", "guide", "Guide", "", "Malformed take", true, {}, "voice.malformed", "dialogue.caption", 0, 0, false, {}, 0, 0, {}}));
    REQUIRE(graph.addNode(
        {"missing_alternative", "guide", "Guide", "", "No muted attachment", true, {}, "voice.missing-alternative", "dialogue.caption", 0, 0, false, {}, 0, 0, {}}));
    std::ofstream(root / "content" / "dialogues" / "media.json", std::ios::binary | std::ios::trunc)
        << graph.serialize().dump(2) << '\n';

    const auto audit = urpg::localization::buildProjectLocalizationAudit(root);
    REQUIRE(audit.dialogue_media_references.size() == 6);
    const auto complete = std::find_if(audit.dialogue_media_references.begin(), audit.dialogue_media_references.end(),
                                       [](const auto& reference) { return reference.node_id == "complete"; });
    REQUIRE(complete != audit.dialogue_media_references.end());
    REQUIRE(complete->voice_asset_attached);
    REQUIRE(complete->caption_key_available);
    REQUIRE(complete->voice_take_metadata_present);
    REQUIRE(complete->voice_take_metadata_valid);
    REQUIRE(complete->voice_take_locale == "en-US");
    REQUIRE(complete->voice_take_id == "guide-take-001");
    REQUIRE(complete->muted_alternative_asset_id == "voice.muted");
    REQUIRE(complete->muted_alternative_asset_attached);
    REQUIRE(std::any_of(audit.dialogue_media_issues.begin(), audit.dialogue_media_issues.end(),
                        [](const auto& issue) { return issue.code == "dialogue_voice_asset_missing" && issue.node_id == "voice_only"; }));
    REQUIRE(std::any_of(audit.dialogue_media_issues.begin(), audit.dialogue_media_issues.end(),
                        [](const auto& issue) { return issue.code == "dialogue_voice_caption_missing" && issue.node_id == "voice_only"; }));
    REQUIRE(std::any_of(audit.dialogue_media_issues.begin(), audit.dialogue_media_issues.end(),
                        [](const auto& issue) { return issue.code == "dialogue_caption_without_voice" && issue.node_id == "caption_only"; }));
    REQUIRE(std::any_of(audit.dialogue_media_issues.begin(), audit.dialogue_media_issues.end(),
                        [](const auto& issue) {
                            return issue.code == "dialogue_voice_take_metadata_missing" && issue.node_id == "metadata_missing";
                        }));
    REQUIRE(std::any_of(audit.dialogue_media_issues.begin(), audit.dialogue_media_issues.end(),
                        [](const auto& issue) {
                            return issue.code == "dialogue_voice_take_metadata_invalid" && issue.node_id == "malformed";
                        }));
    REQUIRE(std::any_of(audit.dialogue_media_issues.begin(), audit.dialogue_media_issues.end(),
                        [](const auto& issue) {
                            return issue.code == "dialogue_voice_take_muted_alternative_attachment_invalid" &&
                                   issue.node_id == "missing_alternative";
                        }));
    std::filesystem::remove_all(root);
}
