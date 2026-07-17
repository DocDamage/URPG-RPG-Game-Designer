#include "editor/playtest/playtest_hot_reload.h"
#include "editor/project/project_recovery_coordinator.h"
#include "engine/core/diagnostics/redacted_support_bundle.h"
#include "engine/core/presentation/runtime_presentation_bible.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

namespace {

nlohmann::json loadWaveDJson(const std::filesystem::path& relative_path) {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) / relative_path, std::ios::binary);
    REQUIRE(input.is_open());
    return nlohmann::json::parse(input);
}

std::set<std::string> stringSet(const nlohmann::json& values) {
    return values.get<std::set<std::string>>();
}

} // namespace

TEST_CASE("Wave D hot reload evidence stays synchronized with the product capability authority",
          "[wave_d][qualification][playtest][hot_reload]") {
    const auto matrix = loadWaveDJson("content/readiness/playtest_hot_reload_capability_matrix.json");
    const auto product = urpg::editor::PlaytestHotReloadCoordinator::productCapabilityMatrix();
    REQUIRE(matrix.at("capabilities").size() == product.size());
    for (const auto& capability : product) {
        const auto row = std::ranges::find_if(matrix.at("capabilities"), [&](const auto& candidate) {
            return candidate.at("resource_class") == urpg::editor::playtestResourceClassName(capability.resource_class);
        });
        REQUIRE(row != matrix.at("capabilities").end());
        REQUIRE(row->at("behavior") == urpg::editor::playtestReloadBehaviorName(capability.behavior));
        REQUIRE(row->at("supports_state_preservation") == capability.supports_state_preservation);
        REQUIRE_FALSE(row->at("failureRecovery").get<std::string>().empty());
    }
}

TEST_CASE("Wave D recovery corpus covers every canonical owner and recovery class",
          "[wave_d][qualification][recovery]") {
    const auto corpus = loadWaveDJson("content/fixtures/playtest_recovery_fault_corpus.json");
    REQUIRE(corpus.at("primaryDocuments") == urpg::editor::primaryRecoveryDocumentTypes());
    REQUIRE(corpus.at("adapters").size() == 6);
    std::set<std::string> adapter_classes;
    for (const auto& adapter : corpus.at("adapters")) {
        adapter_classes.insert(adapter.at("class").get<std::string>());
        REQUIRE(std::filesystem::is_regular_file(std::filesystem::path(URPG_SOURCE_DIR) /
                                                 adapter.at("source").get<std::string>()));
        REQUIRE_FALSE(adapter.at("owner").get<std::string>().empty());
    }
    REQUIRE(adapter_classes == std::set<std::string>{"primary_document", "asset_job",
        "external_change_conflict", "layout_or_settings", "migration", "package"});
    std::vector<urpg::editor::ProjectRecoveryFault> faults;
    for (const auto& owner : corpus.at("primaryDocuments")) {
        faults.push_back({"document." + owner.get<std::string>(), urpg::editor::ProjectRecoveryClass::PrimaryDocument,
                          owner, true, true, true, "injected"});
    }
    faults.push_back({"asset", urpg::editor::ProjectRecoveryClass::AssetJob, "assets", true, true, false, "injected"});
    faults.push_back({"conflict", urpg::editor::ProjectRecoveryClass::ExternalChangeConflict, "map", true, false, true, "injected"});
    faults.push_back({"settings", urpg::editor::ProjectRecoveryClass::LayoutOrSettings, "settings", true, false, false, "injected"});
    faults.push_back({"migration", urpg::editor::ProjectRecoveryClass::Migration, "migration", true, true, false, "injected"});
    faults.push_back({"package", urpg::editor::ProjectRecoveryClass::Package, "package", true, false, false, "injected"});
    const auto coverage = urpg::editor::ProjectRecoveryCoordinator{}.evaluate(faults);
    REQUIRE(coverage.complete);
    REQUIRE(coverage.actions.size() == faults.size());
    REQUIRE(std::ranges::all_of(coverage.actions, [](const auto& action) { return action.project_data_preserved; }));
}

TEST_CASE("Wave D support redaction corpus never survives preview serialization",
          "[wave_d][qualification][support][redaction]") {
    const auto corpus = loadWaveDJson("content/fixtures/support_redaction_corpus.json");
    urpg::diagnostics::RedactedSupportBundleInput input;
    for (const auto& row : corpus.at("values")) input.logs.push_back(row.at("value"));
    input.diagnostics = {{"message", corpus.at("values")[0].at("value")},
                         {"source_path", corpus.at("values")[2].at("value")}};
    const auto preview = urpg::diagnostics::RedactedSupportBundleBuilder{}.preview(input);
    REQUIRE(preview.valid);
    REQUIRE_FALSE(preview.redactions.empty());
    const auto serialized = preview.bundle.dump();
    for (const auto& row : corpus.at("values")) {
        REQUIRE(serialized.find(row.at("value").get<std::string>()) == std::string::npos);
    }
}

TEST_CASE("Wave D presentation input locale and accessibility matrices cover required variants",
          "[wave_d][qualification][presentation][input][accessibility][localization]") {
    const auto presentation = loadWaveDJson("content/readiness/runtime_presentation_showcase_matrix.json");
    const auto bible = urpg::presentation::makeRuntimePresentationBible();
    REQUIRE(urpg::presentation::validateRuntimePresentationBible(bible).empty());
    REQUIRE(presentation.at("variants").size() == bible.accessibility_variants.size());
    for (const auto& variant : bible.accessibility_variants) {
        const auto showcase = urpg::presentation::runtimePresentationComponentShowcase(bible, variant);
        REQUIRE(showcase.at("components").size() == presentation.at("components").size());
    }

    const auto input = loadWaveDJson("content/readiness/input_device_hotplug_matrix.json");
    REQUIRE(input.at("devices").size() == 5);
    REQUIRE(input.at("scenarios").size() == 12);
    REQUIRE(input.at("requiredContexts").size() == 6);

    const auto locale = loadWaveDJson("content/readiness/wave_d_locale_accessibility_corpus.json");
    REQUIRE(locale.at("locales").size() == 4);
    REQUIRE(stringSet(locale.at("accessibilityFailures")) ==
            std::set<std::string>{"clipping", "contrast", "hit_target", "invalid_focus_order",
                                  "localization_overflow", "missing_label", "unsafe_motion"});
    REQUIRE(stringSet(locale.at("localizationCases")).contains("ime_entry"));
    REQUIRE(stringSet(locale.at("localizationCases")).contains("mixed_rtl_number"));
}
