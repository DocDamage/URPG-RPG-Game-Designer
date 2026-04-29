#include "engine/core/message/message_migration.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <nlohmann/json.hpp>

using nlohmann::json;

TEST_CASE("Message migration maps compat pages into native dialogue schemas", "[message][migration]") {
    const json compat = {
        {"id", "imported_sequence"},
        {"pages",
         json::array({
             {
                 {"id", "speaker_page"},
                 {"route", "speaker"},
                 {"speaker", "Alicia"},
                 {"faceActorId", 3},
                 {"body", "\\C[2]HP\\I[5]\\V[2]\\G"},
                 {"choices", json::array({{{"id", "yes"}, {"label", "Yes"}, {"enabled", true}}})},
             },
             {
                 {"id", "custom_route"},
                 {"route", "cinematic"},
                 {"faceActorId", 8},
                 {"body", "\\Q[9] Unknown escape"},
                 {"choices", json::array({{{"id", "locked"}, {"label", "Locked"}, {"enabled", false}}})},
             },
             {
                 {"id", "object_body"},
                 {"route", "narration"},
                 {"body", {{"line", "Serialized"}}},
             },
         })},
    };

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);

    REQUIRE(migrated.dialogue_sequences["_urpg_format_version"] == "1.0");
    REQUIRE(migrated.message_styles["_urpg_format_version"] == "1.0");
    REQUIRE(migrated.dialogue_sequences["sequences"].size() == 1);
    REQUIRE(migrated.dialogue_sequences["sequences"][0]["id"] == "imported_sequence");
    REQUIRE(migrated.dialogue_sequences["sequences"][0]["pages"].size() == 3);

    const auto& page0 = migrated.dialogue_sequences["sequences"][0]["pages"][0];
    REQUIRE(page0["id"] == "speaker_page");
    REQUIRE(page0["presentation_mode"] == "speaker");
    REQUIRE(page0["tone"] == "portrait");
    REQUIRE(page0["speaker"] == "Alicia");
    REQUIRE(page0["face_actor_id"] == 3);
    REQUIRE(page0["style_id"] == "default");
    REQUIRE(page0["choices"].size() == 1);

    const auto& page1 = migrated.dialogue_sequences["sequences"][0]["pages"][1];
    REQUIRE(page1["id"] == "custom_route");
    REQUIRE(page1["presentation_mode"] == "narration");
    REQUIRE(page1["style_id"] == "safe_text_only");
    REQUIRE(page1["face_actor_id"] == 0);

    const auto& page2 = migrated.dialogue_sequences["sequences"][0]["pages"][2];
    REQUIRE(page2["id"] == "object_body");
    REQUIRE(page2["style_id"] == "safe_text_only");
    REQUIRE(page2["body"].get<std::string>().find("Serialized") != std::string::npos);

    REQUIRE(migrated.used_safe_fallback);
    REQUIRE(migrated.message_styles["styles"].size() == 2);
    REQUIRE(migrated.message_styles["styles"][1]["id"] == "safe_text_only");

    const auto has_code = [&](std::string_view code) {
        return std::any_of(
            migrated.diagnostics.begin(),
            migrated.diagnostics.end(),
            [&](const urpg::message::MessageMigrationDiagnostic& d) { return d.code == code; });
    };

    REQUIRE(has_code("unsupported_route"));
    REQUIRE(has_code("unsupported_escape"));
    REQUIRE(has_code("choice_unreachable"));
    REQUIRE(has_code("body_shape_fallback"));
}

TEST_CASE("Message migration provides safe fallback for invalid compat payload", "[message][migration]") {
    const json compat = json::array({1, 2, 3});

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);
    REQUIRE(migrated.used_safe_fallback);
    REQUIRE_FALSE(migrated.diagnostics.empty());
    REQUIRE(migrated.dialogue_sequences["sequences"][0]["pages"].size() == 1);

    const auto& page = migrated.dialogue_sequences["sequences"][0]["pages"][0];
    REQUIRE(page["id"] == "compat_page_0");
    REQUIRE(page["presentation_mode"] == "speaker");
}

TEST_CASE("Message migration diagnostics export emits JSONL stream", "[message][migration]") {
    const json compat = {
        {"pages", json::array({{{"id", "p0"}, {"route", "unknown"}, {"body", "\\X[1]"}}})},
    };

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);
    const std::string jsonl = urpg::message::ExportMessageMigrationDiagnosticsJsonl(migrated);
    REQUIRE_FALSE(jsonl.empty());

    size_t line_count = 1;
    for (char c : jsonl) {
        if (c == '\n') {
            ++line_count;
        }
    }
    REQUIRE(line_count == migrated.diagnostics.size());

    const auto first = json::parse(jsonl.substr(0, jsonl.find('\n')));
    REQUIRE(first.contains("subsystem"));
    REQUIRE(first["subsystem"] == "message_migration");
    REQUIRE(first.contains("level"));
    REQUIRE(first.contains("code"));
    REQUIRE(first.contains("page_id"));
}

TEST_CASE("MessageMigration maps defaultChoiceIndex to native default_choice_index", "[message][migration]") {
    const json compat = {
        {"id", "seq_with_default_choice"},
        {"pages",
         json::array({{
             {"id", "page_with_default"},
             {"route", "speaker"},
             {"speaker", "Guide"},
             {"body", "Choose wisely."},
             {"choices", json::array({{{"id", "opt_a"}, {"label", "A"}}, {{"id", "opt_b"}, {"label", "B"}}})},
             {"defaultChoiceIndex", 1},
         }})},
    };

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);
    const auto& page = migrated.dialogue_sequences["sequences"][0]["pages"][0];
    REQUIRE(page.contains("default_choice_index"));
    REQUIRE(page["default_choice_index"] == 1);
}

TEST_CASE("MessageMigration maps command tool hook", "[message][migration]") {
    const json compat = {
        {"id", "seq_with_command"},
        {"pages",
         json::array({{
             {"id", "page_with_command"},
             {"route", "narration"},
             {"body", "A command fires."},
             {"command", "fire_event"},
         }})},
    };

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);
    const auto& page = migrated.dialogue_sequences["sequences"][0]["pages"][0];
    REQUIRE(page.contains("command"));
    REQUIRE(page["command"] == "fire_event");
}

TEST_CASE("MessageMigration maps window style fields", "[message][migration]") {
    const json compat = {
        {"id", "seq_with_window_style"},
        {"pages",
         json::array({{
             {"id", "page_window_style"},
             {"route", "speaker"},
             {"speaker", "NPC"},
             {"body", "Styled window."},
             {"windowSkin", "img/system/CustomWindow.png"},
             {"windowOpacity", 200},
             {"padding", 12},
             {"fontSize", 18},
             {"lineHeight", 28},
         }})},
    };

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);
    const auto& styles = migrated.message_styles["styles"];
    const auto it = std::find_if(styles.begin(), styles.end(), [](const json& s) { return s["id"] == "default"; });
    REQUIRE(it != styles.end());
    REQUIRE((*it).contains("window"));
    const auto& window = (*it)["window"];
    REQUIRE(window["skin"] == "img/system/CustomWindow.png");
    REQUIRE(window["opacity"] == 200);
    REQUIRE(window["padding"] == 12);
    REQUIRE(window["font_size"] == 18);
    REQUIRE(window["line_height"] == 28);
}

TEST_CASE("MessageMigration maps audio style fields", "[message][migration]") {
    const json compat = {
        {"id", "seq_with_audio_style"},
        {"pages",
         json::array({{
             {"id", "page_audio_style"},
             {"route", "narration"},
             {"body", "Audio styled."},
             {"typing_se", "se/type.wav"},
             {"open_se", "se/open.wav"},
             {"close_se", "se/close.wav"},
         }})},
    };

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);
    const auto& styles = migrated.message_styles["styles"];
    const auto it = std::find_if(styles.begin(), styles.end(), [](const json& s) { return s["id"] == "default"; });
    REQUIRE(it != styles.end());
    REQUIRE((*it).contains("audio"));
    const auto& audio = (*it)["audio"];
    REQUIRE(audio["typing_se"] == "se/type.wav");
    REQUIRE(audio["open_se"] == "se/open.wav");
    REQUIRE(audio["close_se"] == "se/close.wav");
}

TEST_CASE("MessageMigration output conforms to dialogue_sequences schema", "[message][migration]") {
    const json compat = {
        {"id", "conformance_seq"},
        {"pages",
         json::array({{
             {"id", "conformance_page"},
             {"route", "speaker"},
             {"speaker", "Tester"},
             {"body", "Conformance check."},
             {"waitForAdvance", true},
             {"defaultChoiceIndex", 0},
         }})},
    };

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);
    const auto& page = migrated.dialogue_sequences["sequences"][0]["pages"][0];

    // Structural check for required / expected page fields
    REQUIRE(page.contains("id"));
    REQUIRE(page.contains("style_id"));
    REQUIRE(page.contains("presentation_mode"));
    REQUIRE(page.contains("tone"));
    REQUIRE(page.contains("body"));
    REQUIRE(page.contains("wait_for_advance"));
    REQUIRE(page.contains("default_choice_index"));
    REQUIRE(page["id"] == "conformance_page");
    REQUIRE(page["presentation_mode"] == "speaker");
    REQUIRE(page["body"] == "Conformance check.");
    REQUIRE(page["wait_for_advance"] == true);
    REQUIRE(page["default_choice_index"] == 0);
}

TEST_CASE("Message migration maps scoped state banks and picture task adapters",
          "[message][migration][state][picture]") {
    const json compat = {
        {"id", "state_picture_import"},
        {"pages", json::array({{{"id", "p0"}, {"route", "narration"}, {"body", "Adapters."}}})},
        {"stateBanks",
         {{"switches",
           json::array({
               {{"scope", "self"}, {"mapId", "map_1"}, {"eventId", "event_2"}, {"id", "A"}, {"value", true}},
               {{"scope", "bad_scope"}, {"id", "drop_me"}, {"value", true}},
           })},
          {"variables",
           json::array({
               {{"scope", "map"}, {"map_id", "map_1"}, {"id", "weather_seed"}, {"value", 42}},
               {{"scope", "js"}, {"scopeId", "plugin_cache"}, {"id", "cached_result"}, {"value", "ok"}},
           })}}},
        {"scopedVariables", json::array({{{"scope", "scoped"}, {"scope_id", "quest"}, {"id", "progress"}, {"value", 3}}})},
        {"pictureTasks",
         {{"maxPictures", 1000},
          {"bindings",
           json::array({
               {{"pictureId", 350},
                {"taskId", "open_codex_hotspot"},
                {"commonEventId", "common_event.open_codex"},
                {"trigger", "confirm"},
                {"enabled", true}},
               {{"picture_id", 351},
                {"task_id", "hover_codex_hotspot"},
                {"common_event_id", "common_event.preview_codex"},
                {"trigger", "press"},
                {"enabled", false}},
           })}}},
    };

    const auto migrated = urpg::message::UpgradeCompatMessageDocument(compat);

    REQUIRE(migrated.scoped_state_banks["version"] == "1.0.0");
    REQUIRE(migrated.scoped_state_banks["switches"].size() == 1);
    REQUIRE(migrated.scoped_state_banks["switches"][0]["scope"] == "self");
    REQUIRE(migrated.scoped_state_banks["switches"][0]["map_id"] == "map_1");
    REQUIRE(migrated.scoped_state_banks["switches"][0]["event_id"] == "event_2");
    REQUIRE(migrated.scoped_state_banks["switches"][0]["id"] == "A");
    REQUIRE(migrated.scoped_state_banks["switches"][0]["value"] == true);
    REQUIRE(migrated.scoped_state_banks["variables"].size() == 3);
    REQUIRE(migrated.scoped_state_banks["variables"][0]["scope"] == "map");
    REQUIRE(migrated.scoped_state_banks["variables"][0]["value"] == 42);
    REQUIRE(migrated.scoped_state_banks["variables"][1]["scope"] == "js");
    REQUIRE(migrated.scoped_state_banks["variables"][1]["scope_id"] == "plugin_cache");
    REQUIRE(migrated.scoped_state_banks["variables"][2]["scope"] == "scoped");

    REQUIRE(migrated.picture_tasks["version"] == "1.0.0");
    REQUIRE(migrated.picture_tasks["max_pictures"] == 1000);
    REQUIRE(migrated.picture_tasks["bindings"].size() == 2);
    REQUIRE(migrated.picture_tasks["bindings"][0]["picture_id"] == 350);
    REQUIRE(migrated.picture_tasks["bindings"][0]["task_id"] == "open_codex_hotspot");
    REQUIRE(migrated.picture_tasks["bindings"][0]["common_event_id"] == "common_event.open_codex");
    REQUIRE(migrated.picture_tasks["bindings"][0]["trigger"] == "confirm");
    REQUIRE(migrated.picture_tasks["bindings"][1]["trigger"] == "click");
    REQUIRE(migrated.picture_tasks["bindings"][1]["enabled"] == false);

    const auto has_code = [&](std::string_view code) {
        return std::any_of(
            migrated.diagnostics.begin(),
            migrated.diagnostics.end(),
            [&](const urpg::message::MessageMigrationDiagnostic& d) { return d.code == code; });
    };
    REQUIRE(has_code("unsupported_state_scope"));
    REQUIRE(has_code("normalized_picture_task_trigger"));
}
