#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string source_root = URPG_SOURCE_DIR;
    if (source_root.size() >= 2 && source_root.front() == '"' && source_root.back() == '"') {
        source_root = source_root.substr(1, source_root.size() - 2);
    }
    return std::filesystem::path(source_root);
#else
    return {};
#endif
}

nlohmann::json LoadJson(const std::filesystem::path& path) {
    std::ifstream in(path);
    REQUIRE(in.is_open());
    nlohmann::json parsed;
    in >> parsed;
    return parsed;
}

} // namespace

TEST_CASE("Message schema contract files exist and expose required roots", "[message][schema]") {
    const auto root = sourceRootFromMacro();
    REQUIRE_FALSE(root.empty());
    const auto schema_dir = root / "content" / "schemas";

    const auto dialogue_schema_path = schema_dir / "dialogue_sequences.schema.json";
    const auto style_schema_path = schema_dir / "message_styles.schema.json";
    const auto token_schema_path = schema_dir / "rich_text_tokens.schema.json";
    const auto choice_schema_path = schema_dir / "choice_prompts.schema.json";

    REQUIRE(std::filesystem::exists(dialogue_schema_path));
    REQUIRE(std::filesystem::exists(style_schema_path));
    REQUIRE(std::filesystem::exists(token_schema_path));
    REQUIRE(std::filesystem::exists(choice_schema_path));

    const auto dialogue_schema = LoadJson(dialogue_schema_path);
    const auto style_schema = LoadJson(style_schema_path);
    const auto token_schema = LoadJson(token_schema_path);
    const auto choice_schema = LoadJson(choice_schema_path);

    REQUIRE(dialogue_schema["$id"] == "https://urpg.dev/schemas/dialogue_sequences.schema.json");
    REQUIRE(style_schema["$id"] == "https://urpg.dev/schemas/message_styles.schema.json");
    REQUIRE(token_schema["$id"] == "https://urpg.dev/schemas/rich_text_tokens.schema.json");
    REQUIRE(choice_schema["$id"] == "https://urpg.dev/schemas/choice_prompts.schema.json");

    REQUIRE(dialogue_schema["required"].is_array());
    REQUIRE(style_schema["required"].is_array());
    REQUIRE(token_schema["required"].is_array());
    REQUIRE(choice_schema["required"].is_array());

    REQUIRE(dialogue_schema["required"][0] == "_urpg_format_version");
    REQUIRE(style_schema["required"][0] == "_urpg_format_version");
    REQUIRE(token_schema["required"][0] == "_urpg_format_version");
    REQUIRE(choice_schema["required"][0] == "_urpg_format_version");
}

TEST_CASE("Dialogue sequence schema includes native presentation enums", "[message][schema]") {
    const auto root = sourceRootFromMacro();
    REQUIRE_FALSE(root.empty());
    const auto dialogue_schema_path = root / "content" / "schemas" / "dialogue_sequences.schema.json";
    const auto dialogue_schema = LoadJson(dialogue_schema_path);

    const auto enums =
        dialogue_schema["properties"]["sequences"]["items"]["properties"]["pages"]["items"]["properties"]["presentation_mode"]["enum"];
    REQUIRE(enums.is_array());
    REQUIRE(enums.size() == 3);
    REQUIRE(enums[0] == "speaker");
    REQUIRE(enums[1] == "narration");
    REQUIRE(enums[2] == "system");
}
