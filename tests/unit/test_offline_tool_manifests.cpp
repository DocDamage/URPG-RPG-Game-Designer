#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::filesystem::path repoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path{URPG_SOURCE_DIR};
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json loadJson(const std::filesystem::path& path) {
    std::ifstream file(path);
    REQUIRE(file.good());
    nlohmann::json parsed;
    file >> parsed;
    return parsed;
}

std::string readText(const std::filesystem::path& path) {
    std::ifstream file(path);
    REQUIRE(file.good());
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void requireStringProperty(const nlohmann::json& properties, const std::string& name) {
    REQUIRE(properties.contains(name));
    REQUIRE(properties.at(name).at("type") == "string");
}

void requireArrayProperty(const nlohmann::json& properties, const std::string& name) {
    REQUIRE(properties.contains(name));
    REQUIRE(properties.at(name).at("type") == "array");
}

std::vector<std::string> requiredFields(const nlohmann::json& schema) {
    REQUIRE(schema.contains("required"));
    REQUIRE(schema.at("required").is_array());
    return schema.at("required").get<std::vector<std::string>>();
}

void requireRequiredField(const nlohmann::json& schema, const std::string& field) {
    const auto fields = requiredFields(schema);
    REQUIRE(std::find(fields.begin(), fields.end(), field) != fields.end());
}

} // namespace

TEST_CASE("offline retrieval manifests stay behind tooling boundary", "[offline][manifest][retrieval]") {
    const auto root = repoRoot();

    REQUIRE(std::filesystem::exists(root / "tools" / "retrieval" / "build_index.py"));
    REQUIRE(std::filesystem::exists(root / "tools" / "retrieval" / "query_index.py"));
    REQUIRE(std::filesystem::exists(root / "tools" / "retrieval" / "requirements.txt"));
    REQUIRE_FALSE(std::filesystem::exists(root / "engine" / "core" / "retrieval"));

    const auto manifestSchema =
        loadJson(root / "content" / "schemas" / "retrieval_index_manifest.schema.json");
    REQUIRE(manifestSchema.at("$id") ==
            "https://urpg.dev/schemas/retrieval_index_manifest.schema.json");
    requireRequiredField(manifestSchema, "schema");
    requireRequiredField(manifestSchema, "tool_id");
    requireRequiredField(manifestSchema, "source_paths");
    requireRequiredField(manifestSchema, "chunks");
    requireRequiredField(manifestSchema, "index");

    const auto properties = manifestSchema.at("properties");
    requireStringProperty(properties, "schema");
    requireStringProperty(properties, "tool_id");
    requireArrayProperty(properties, "source_paths");
    requireArrayProperty(properties, "chunks");
    REQUIRE(properties.at("index").at("type") == "object");

    const nlohmann::json example = {
        {"schema", "content/schemas/retrieval_index_manifest.schema.json"},
        {"tool_id", "faiss_compatible_retrieval"},
        {"source_paths", nlohmann::json::array({"docs/agent/INDEX.md"})},
        {"chunks",
         nlohmann::json::array({{{"chunk_id", "docs-agent-index-0000"},
                                 {"source_path", "docs/agent/INDEX.md"},
                                 {"chunk_index", 0},
                                 {"excerpt", "Agent knowledge index"}}})},
        {"index",
         {{"engine", "faiss_compatible_json"},
          {"metadata_path", "build/retrieval/index.json"},
          {"dimension", 64}}},
    };
    REQUIRE(example.at("chunks").at(0).at("chunk_id").get<std::string>().find("docs-agent") ==
            0);
}

TEST_CASE("offline segmentation manifests capture reviewable asset outputs", "[offline][manifest][segmentation]") {
    const auto root = repoRoot();

    REQUIRE(std::filesystem::exists(root / "tools" / "vision" / "segment_assets.py"));
    REQUIRE(std::filesystem::exists(root / "tools" / "vision" / "requirements.txt"));
    REQUIRE_FALSE(std::filesystem::exists(root / "engine" / "core" / "vision"));

    const auto schema =
        loadJson(root / "content" / "schemas" / "segmentation_manifest.schema.json");
    REQUIRE(schema.at("$id") == "https://urpg.dev/schemas/segmentation_manifest.schema.json");
    for (const std::string field : {"schema", "tool_id", "source_image", "outputs"}) {
        requireRequiredField(schema, field);
    }

    const auto outputProperties =
        schema.at("properties").at("outputs").at("items").at("properties");
    for (const std::string field : {"mask_path", "cutout_path", "state"}) {
        REQUIRE(outputProperties.contains(field));
    }
    REQUIRE(outputProperties.at("manual_override").at("type") == "boolean");
    REQUIRE(outputProperties.at("reviewed").at("type") == "boolean");
    REQUIRE(outputProperties.at("rerun_protection").at("type") == "string");

    const nlohmann::json positive = {
        {"schema", "content/schemas/segmentation_manifest.schema.json"},
        {"tool_id", "sam2_compatible_segmenter"},
        {"source_image", "imports/sprites/hero.png"},
        {"outputs",
         nlohmann::json::array({{{"asset_id", "hero_body"},
                                 {"mask_path", "build/vision/hero_body_mask.png"},
                                 {"cutout_path", "build/vision/hero_body_cutout.png"},
                                 {"manual_override", true},
                                 {"reviewed", true},
                                 {"rerun_protection", "preserve_manual_override"},
                                 {"state", "reused"}}})},
    };
    REQUIRE(positive.at("outputs").at(0).at("manual_override") == true);
    REQUIRE(positive.at("outputs").at(0).at("state") == "reused");
}

TEST_CASE("offline audio tool manifests keep generated prototypes non-release", "[offline][manifest][audio]") {
    const auto root = repoRoot();

    REQUIRE(std::filesystem::exists(root / "tools" / "audio" / "process_audio_assets.py"));
    REQUIRE(std::filesystem::exists(root / "tools" / "audio" / "requirements.txt"));
    REQUIRE_FALSE(std::filesystem::exists(root / "engine" / "core" / "audio_tools"));

    const auto schema =
        loadJson(root / "content" / "schemas" / "audio_tool_manifest.schema.json");
    REQUIRE(schema.at("$id") == "https://urpg.dev/schemas/audio_tool_manifest.schema.json");
    for (const std::string field : {"schema", "tool_id", "source_audio", "outputs"}) {
        requireRequiredField(schema, field);
    }

    const auto outputProperties =
        schema.at("properties").at("outputs").at("items").at("properties");
    for (const std::string field :
         {"output_id", "kind", "path", "source_attribution", "review_status", "release_eligible"}) {
        REQUIRE(outputProperties.contains(field));
    }
    REQUIRE(outputProperties.at("generated_prototype").at("type") == "boolean");
    REQUIRE(outputProperties.at("release_eligible").at("type") == "boolean");

    const nlohmann::json example = {
        {"schema", "content/schemas/audio_tool_manifest.schema.json"},
        {"tool_id", "demucs_encodec_offline_processor"},
        {"source_audio", "imports/audio/theme.wav"},
        {"outputs",
         nlohmann::json::array({{{"output_id", "theme_drums"},
                                 {"kind", "demucs_stem"},
                                 {"path", "build/audio/theme_drums.wav"},
                                 {"source_attribution", "creator_supplied"},
                                 {"review_status", "pending_review"},
                                 {"generated_prototype", true},
                                 {"release_eligible", false}}})},
    };
    REQUIRE(example.at("outputs").at(0).at("generated_prototype") == true);
    REQUIRE(example.at("outputs").at(0).at("release_eligible") == false);
}

TEST_CASE("Phase 11 offline tooling closure is ready in canonical release docs",
          "[offline][tools][phase11]") {
    const auto root = repoRoot();
    const auto inventory = readText(root / "docs" / "release" / "100_PERCENT_COMPLETION_INVENTORY.md");
    const auto programStatus = readText(root / "docs" / "PROGRAM_COMPLETION_STATUS.md");
    const auto statusMirror = readText(root / "docs" / "status" / "PROGRAM_COMPLETION_STATUS.md");

    const auto phase11Pos = inventory.find("`phase_11_offline_tooling_pipelines`");
    REQUIRE(phase11Pos != std::string::npos);
    const auto phase11LineEnd = inventory.find('\n', phase11Pos);
    const auto phase11Line = inventory.substr(phase11Pos, phase11LineEnd - phase11Pos);

    REQUIRE(phase11Line.find("`READY`") != std::string::npos);
    REQUIRE(phase11Line.find("`MANDATORY_OPEN`") == std::string::npos);
    REQUIRE(phase11Line.find("FAISS retrieval") != std::string::npos);
    REQUIRE(phase11Line.find("SAM/SAM2-compatible segmentation manifests") != std::string::npos);
    REQUIRE(phase11Line.find("Demucs/Encodec-compatible audio manifests") != std::string::npos);
    REQUIRE(phase11Line.find("job-runner-addressable") != std::string::npos);

    REQUIRE(inventory.find("\nun_local_gates.ps1`") == std::string::npos);
    REQUIRE(programStatus.find("Phase 11 offline tooling closure is complete") != std::string::npos);
    REQUIRE(statusMirror.find("Phase 11 offline tooling closure is complete") != std::string::npos);
}
