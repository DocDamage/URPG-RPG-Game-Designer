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
