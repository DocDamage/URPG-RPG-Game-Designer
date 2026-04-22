#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::filesystem::path repoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json loadJson(const std::filesystem::path& path) {
    std::ifstream f(path);
    REQUIRE(f.is_open());
    nlohmann::json j;
    f >> j;
    return j;
}

} // namespace

TEST_CASE("readiness_status.schema.json validates a known-good subset", "[governance][truth]") {
    const auto schemaPath = repoRoot() / "content" / "schemas" / "readiness_status.schema.json";
    const auto schema = loadJson(schemaPath);

    REQUIRE(schema.contains("definitions"));
    REQUIRE(schema["definitions"].contains("readinessStatus"));
    const auto& statuses = schema["definitions"]["readinessStatus"]["enum"];
    REQUIRE(statuses.size() == 5);

    const auto evidenceFields = schema["definitions"]["evidence"]["required"];
    REQUIRE(evidenceFields.size() == 6);

    // Known-good minimal subset that conforms to the schema structure
    const nlohmann::json goodSubset = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-20",
        "subsystems": [
            {
                "id": "ui_menu_core",
                "status": "READY",
                "summary": "Wave 1 native menu ownership is closed.",
                "mainGaps": [],
                "evidence": {
                    "runtimeOwner": true,
                    "editorSurface": true,
                    "schemaMigration": true,
                    "diagnostics": true,
                    "testsValidation": true,
                    "docsAligned": true
                }
            }
        ],
        "templates": [
            {
                "id": "jrpg",
                "status": "PARTIAL",
                "requiredSubsystems": ["ui_menu_core"],
                "bars": {
                    "accessibility": "PARTIAL",
                    "audio": "PARTIAL",
                    "input": "PARTIAL",
                    "localization": "PARTIAL",
                    "performance": "PARTIAL"
                },
                "safeScope": "Native-first baseline.",
                "mainBlockers": ["Cross-cutting bars are not complete."]
            }
        ]
    })");

    REQUIRE(goodSubset.contains("schemaVersion"));
    REQUIRE(goodSubset["subsystems"].is_array());
    REQUIRE(goodSubset["templates"].is_array());

    const auto& sub = goodSubset["subsystems"][0];
    REQUIRE(sub["id"] == "ui_menu_core");
    REQUIRE(sub["status"] == "READY");
    REQUIRE(sub["evidence"]["runtimeOwner"] == true);

    const auto& tmpl = goodSubset["templates"][0];
    REQUIRE(tmpl["id"] == "jrpg");
    REQUIRE(tmpl["status"] == "PARTIAL");
    REQUIRE(tmpl["requiredSubsystems"][0] == "ui_menu_core");

    // Verify enum membership against schema
    const std::string status = sub["status"];
    bool statusValid = false;
    for (const auto& s : statuses) {
        if (s == status) {
            statusValid = true;
            break;
        }
    }
    REQUIRE(statusValid);
}

TEST_CASE("mocked overclaim document is detectable by regex scan", "[governance][truth]") {
    const std::string mockDoc = R"(
# Subsystem Status

The `battle_core` is READY for production use.
The `message_text_core` is also READY.
)";

    const std::vector<std::string> nonReadySubsystems = {
        "battle_core",
        "save_data_core",
        "presentation_runtime"
    };

    std::vector<std::string> detected;
    std::istringstream stream(mockDoc);
    std::string line;
    while (std::getline(stream, line)) {
        if (!std::regex_search(line, std::regex("is READY|are READY"))) {
            continue;
        }
        for (const auto& id : nonReadySubsystems) {
            const std::string backtickPattern = "`" + id + "`";
            const std::string wordPattern = "\\b" + id + "\\b";
            if (line.find(backtickPattern) != std::string::npos ||
                std::regex_search(line, std::regex(wordPattern))) {
                detected.push_back(id);
            }
        }
    }

    REQUIRE(detected.size() == 1);
    REQUIRE(detected[0] == "battle_core");
}

TEST_CASE("schema changelog coverage can be verified by filename enumeration", "[governance][truth]") {
    const std::string mockChangelog = R"(
# Schema Changelog

### `readiness_status` (`readiness_status.schema.json`)
- Version: 1.0.0

### `input_bindings` (`input_bindings.schema.json`)
- Version: 1.0.0
)";

    const std::vector<std::string> schemaFiles = {
        "readiness_status.schema.json",
        "input_bindings.schema.json",
        "missing_schema.schema.json"
    };

    std::vector<std::string> uncovered;
    for (const auto& name : schemaFiles) {
        if (mockChangelog.find(name) == std::string::npos) {
            uncovered.push_back(name);
        }
    }

    REQUIRE(uncovered.size() == 1);
    REQUIRE(uncovered[0] == "missing_schema.schema.json");
}
