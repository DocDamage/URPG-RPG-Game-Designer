#include "editor/relationship/relationship_panel.h"
#include "engine/core/relationship/relationship_affinity.h"
#include "engine/core/relationship/relationship_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <vector>

namespace {

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 && sourceRoot.front() == '"' && sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

} // namespace

TEST_CASE("relationship registry persists affinity and lists reputation gated content", "[relationship][narrative][ffs10]") {
    urpg::relationship::RelationshipRegistry registry;
    registry.setAffinity("guide", 150);
    registry.addGate({"guide_sidequest", "guide", 20});
    registry.addGate({"guide_secret", "guide", 120});

    REQUIRE(registry.affinity("guide") == 100);
    REQUIRE(registry.availableContent() == std::vector<std::string>{"guide_sidequest"});

    const auto restored = urpg::relationship::RelationshipRegistry::deserialize(registry.serialize());
    REQUIRE(restored.affinity("guide") == 100);
    REQUIRE(restored.availableContent() == std::vector<std::string>{"guide_sidequest"});
}

TEST_CASE("relationship affinity document applies events and previews editor state", "[relationship][affinity]") {
    const auto fixturePath = sourceRootFromMacro() / "content" / "fixtures" / "relationship_affinity_fixture.json";
    std::ifstream input(fixturePath);
    REQUIRE(input.is_open());
    nlohmann::json fixture;
    input >> fixture;

    const auto document = urpg::relationship::RelationshipAffinityDocument::fromJson(fixture);
    REQUIRE(document.validate().empty());

    urpg::relationship::RelationshipRegistry registry;
    registry.setAffinity("guide", 5);

    const urpg::relationship::AffinityEvent event{"gift", "favorite_flower"};
    const auto preview = document.preview(registry, event);
    REQUIRE(preview.applied_rule_ids == std::vector<std::string>{"guide.favorite_flower"});
    REQUIRE(preview.projected_affinity["guide"] == 20);
    REQUIRE(preview.unlocked_content_ids == std::vector<std::string>{"guide_bond_scene"});
    REQUIRE(registry.affinity("guide") == 5);

    auto apply = document.apply(registry, event);
    REQUIRE(apply.applied_rule_ids == std::vector<std::string>{"guide.favorite_flower"});
    REQUIRE(registry.affinity("guide") == 20);

    urpg::editor::RelationshipPanel panel;
    panel.setRegistry(registry);
    panel.bindAffinityDocument(document);
    panel.setPreviewEvent({"dialogue_choice", "kind_answer"});
    REQUIRE(panel.applyPreviewEvent());
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "relationship");
    REQUIRE(snapshot["preview"]["applied_rule_ids"][0] == "guide.kind_answer");
    REQUIRE(snapshot["registry"]["affinity"]["guide"] == 25);
}

TEST_CASE("relationship affinity document reports invalid rules and gates", "[relationship][affinity]") {
    const auto document = urpg::relationship::RelationshipAffinityDocument::fromJson(nlohmann::json{
        {"document_id", "broken"},
        {"rules", {{{"id", ""}, {"subject_id", "guide"}, {"source", "gift"}, {"tag", ""}, {"delta", 5}}}},
        {"gates", {{{"content_id", ""}, {"subject_id", "guide"}, {"minimum", 10}}}},
    });

    const auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() == 2);
    REQUIRE(diagnostics[0].code == "invalid_affinity_rule");
    REQUIRE(diagnostics[1].code == "invalid_affinity_gate");
}
