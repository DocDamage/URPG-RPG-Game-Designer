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

TEST_CASE("Save schema contract files exist and expose required roots", "[save][schema]") {
    const auto root = sourceRootFromMacro();
    REQUIRE_FALSE(root.empty());
    const auto schema_dir = root / "content" / "schemas";

    const auto policies_schema_path = schema_dir / "save_policies.schema.json";
    const auto slots_schema_path = schema_dir / "save_slots.schema.json";
    const auto metadata_schema_path = schema_dir / "save_metadata.schema.json";
    const auto migrations_schema_path = schema_dir / "save_migrations.schema.json";

    REQUIRE(std::filesystem::exists(policies_schema_path));
    REQUIRE(std::filesystem::exists(slots_schema_path));
    REQUIRE(std::filesystem::exists(metadata_schema_path));
    REQUIRE(std::filesystem::exists(migrations_schema_path));

    const auto policies_schema = LoadJson(policies_schema_path);
    const auto slots_schema = LoadJson(slots_schema_path);
    const auto metadata_schema = LoadJson(metadata_schema_path);
    const auto migrations_schema = LoadJson(migrations_schema_path);

    REQUIRE(policies_schema["$id"] == "https://urpg.dev/schemas/save_policies.schema.json");
    REQUIRE(slots_schema["$id"] == "https://urpg.dev/schemas/save_slots.schema.json");
    REQUIRE(metadata_schema["$id"] == "https://urpg.dev/schemas/save_metadata.schema.json");
    REQUIRE(migrations_schema["$id"] == "https://urpg.dev/schemas/save_migrations.schema.json");

    REQUIRE(policies_schema["required"].is_array());
    REQUIRE(slots_schema["required"].is_array());
    REQUIRE(metadata_schema["required"].is_array());
    REQUIRE(migrations_schema["required"].is_array());

    REQUIRE(policies_schema["required"][0] == "_urpg_format_version");
    REQUIRE(slots_schema["required"][0] == "_urpg_format_version");
    REQUIRE(metadata_schema["required"][0] == "_urpg_format_version");
    REQUIRE(migrations_schema["required"][0] == "_urpg_format_version");
}

TEST_CASE("Save policies schema exposes autosave retention and metadata field contracts", "[save][schema]") {
    const auto root = sourceRootFromMacro();
    REQUIRE_FALSE(root.empty());
    const auto schema = LoadJson(root / "content" / "schemas" / "save_policies.schema.json");

    const auto properties = schema["properties"];
    REQUIRE(properties.contains("metadata_fields"));
    REQUIRE(properties.contains("retention"));
    REQUIRE(properties.contains("autosave"));

    const auto metadata_field_properties = properties["metadata_fields"]["items"]["properties"];
    REQUIRE(metadata_field_properties.contains("key"));
    REQUIRE(metadata_field_properties.contains("display_label"));
    REQUIRE(metadata_field_properties.contains("required"));
    REQUIRE(metadata_field_properties.contains("default_value"));

    const auto retention_properties = properties["retention"]["properties"];
    REQUIRE(retention_properties.contains("max_autosave_slots"));
    REQUIRE(retention_properties.contains("max_quicksave_slots"));
    REQUIRE(retention_properties.contains("max_manual_slots"));
    REQUIRE(retention_properties.contains("prune_excess_on_save"));

    const auto autosave_properties = properties["autosave"]["properties"];
    REQUIRE(autosave_properties.contains("enabled"));
    REQUIRE(autosave_properties.contains("slot_id"));
}

TEST_CASE("Save slots schema exposes slot descriptor fields", "[save][schema]") {
    const auto root = sourceRootFromMacro();
    REQUIRE_FALSE(root.empty());
    const auto schema = LoadJson(root / "content" / "schemas" / "save_slots.schema.json");

    const auto slot_properties = schema["properties"]["slots"]["items"]["properties"];
    REQUIRE(slot_properties.contains("slot_id"));
    REQUIRE(slot_properties.contains("category"));
    REQUIRE(slot_properties.contains("label"));
    REQUIRE(slot_properties.contains("reserved"));

    const auto category_enum = slot_properties["category"]["enum"];
    REQUIRE(category_enum.is_array());
    REQUIRE(category_enum.size() == 3);
    REQUIRE(category_enum[0] == "autosave");
    REQUIRE(category_enum[1] == "quicksave");
    REQUIRE(category_enum[2] == "manual");
}

TEST_CASE("Save metadata and migration schemas expose typed native save contracts", "[save][schema]") {
    const auto root = sourceRootFromMacro();
    REQUIRE_FALSE(root.empty());

    const auto metadata_schema = LoadJson(root / "content" / "schemas" / "save_metadata.schema.json");
    const auto migrations_schema = LoadJson(root / "content" / "schemas" / "save_migrations.schema.json");

    const auto metadata_properties = metadata_schema["properties"];
    REQUIRE(metadata_properties.contains("slot_id"));
    REQUIRE(metadata_properties.contains("slot_category"));
    REQUIRE(metadata_properties.contains("retention_class"));
    REQUIRE(metadata_properties.contains("save_version"));
    REQUIRE(metadata_properties.contains("timestamp"));
    REQUIRE(metadata_properties.contains("map_display_name"));
    REQUIRE(metadata_properties.contains("flags"));
    REQUIRE(metadata_properties.contains("custom_metadata"));

    const auto migration_properties = migrations_schema["properties"];
    REQUIRE(migration_properties.contains("migrations"));
    const auto migration_item_properties = migration_properties["migrations"]["items"]["properties"];
    REQUIRE(migration_item_properties.contains("from"));
    REQUIRE(migration_item_properties.contains("to"));
    REQUIRE(migration_item_properties.contains("ops"));
}
