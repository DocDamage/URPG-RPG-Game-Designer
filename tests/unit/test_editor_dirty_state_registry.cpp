#include "editor/project/editor_dirty_state_registry.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("EditorDirtyStateRegistry saves all dirty documents before navigation", "[project][dirty state]") {
    urpg::editor::EditorDirtyStateRegistry registry;
    int saves = 0;
    REQUIRE(registry.registerSurface({"map:opening", "map/canvas", true,
                                      [&saves] { ++saves; return urpg::editor::EditorDirtySaveResult{true, "saved", ""}; },
                                      {}, {}}));
    REQUIRE(registry.registerSurface({"ability:fire", "map/abilities", true,
                                      [&saves] { ++saves; return urpg::editor::EditorDirtySaveResult{true, "saved", ""}; },
                                      {}, {}}));

    const auto result = registry.resolveNavigation(urpg::editor::EditorNavigationDecision::Save);
    REQUIRE(result.allowed);
    REQUIRE(saves == 2);
    REQUIRE(registry.dirtyDocumentIds().empty());
}

TEST_CASE("EditorDirtyStateRegistry preserves dirty state and focuses a failed save", "[project][dirty state]") {
    urpg::editor::EditorDirtyStateRegistry registry;
    bool focused = false;
    REQUIRE(registry.registerSurface({"map:opening", "map/canvas", true,
                                      [] { return urpg::editor::EditorDirtySaveResult{false, "atomic_write_failed", "Disk full"}; },
                                      [&focused] { focused = true; }, {}}));

    const auto result = registry.resolveNavigation(urpg::editor::EditorNavigationDecision::Save);
    REQUIRE_FALSE(result.allowed);
    REQUIRE(result.failed_document_id == "map:opening");
    REQUIRE(result.diagnostic.code == "atomic_write_failed");
    REQUIRE(registry.isDirty("map:opening"));
    REQUIRE(focused);
}

TEST_CASE("EditorDirtyStateRegistry makes discard and cancel explicit", "[project][dirty state]") {
    urpg::editor::EditorDirtyStateRegistry registry;
    REQUIRE(registry.registerSurface({"map:opening", "map/canvas", true,
                                      [] { return urpg::editor::EditorDirtySaveResult{true, "saved", ""}; }, {}, {}}));

    const auto cancelled = registry.resolveNavigation(urpg::editor::EditorNavigationDecision::Cancel);
    REQUIRE_FALSE(cancelled.allowed);
    REQUIRE(cancelled.diagnostic.code == "navigation_cancelled");
    REQUIRE(registry.isDirty("map:opening"));

    const auto discarded = registry.resolveNavigation(urpg::editor::EditorNavigationDecision::Discard);
    REQUIRE(discarded.allowed);
    REQUIRE(registry.isDirty("map:opening"));
}
