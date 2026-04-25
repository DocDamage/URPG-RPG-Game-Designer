#include "engine/core/save/save_compatibility_preview.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("save compatibility preview preserves unrelated blobs", "[save][compatibility_preview][ffs09]") {
    const nlohmann::json old_save = {
        {"meta", {{"slotId", 2}, {"mapName", "Old Town"}}},
        {"gold", 100},
        {"pluginData", {{"customSystem", {{"rank", 7}}}}},
    };

    const urpg::save::SaveCompatibilityPreviewer previewer;
    const auto preview = previewer.preview(old_save);

    REQUIRE_FALSE(preview.ok);
    REQUIRE(preview.native_payload["player"]["gold"] == 100);
    REQUIRE(preview.migrated_metadata["_slot_id"] == 2);
    REQUIRE(preview.retained_payload["/pluginData"]["customSystem"]["rank"] == 7);
}

TEST_CASE("save compatibility preview diagnoses unknown save fields", "[save][compatibility_preview][ffs09]") {
    const nlohmann::json old_save = {
        {"meta", {{"slotId", 3}}},
        {"mysteryBlob", {{"value", 42}}},
    };

    const urpg::save::SaveCompatibilityPreviewer previewer;
    const auto preview = previewer.preview(old_save);

    REQUIRE(preview.retained_payload["/mysteryBlob"]["value"] == 42);
    REQUIRE(std::find(preview.diagnostics.begin(),
                      preview.diagnostics.end(),
                      "warning:retained_compat_payload_field:/mysteryBlob") != preview.diagnostics.end());
}
