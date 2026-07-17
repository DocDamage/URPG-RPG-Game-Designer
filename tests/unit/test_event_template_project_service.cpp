#include "editor/events/event_authoring_panel.h"
#include "engine/core/events/event_template_project_service.h"
#include "engine/core/project/project_operation_journal.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

urpg::events::EventTemplateDefinition rewardTemplate(const uint32_t version,
                                                      const std::string& id = "narrative.reward") {
    return {id,
            version,
            urpg::events::EventTemplateKind::Narrative,
            {{"speaker", urpg::events::EventTemplateParameterType::Reference, true, {}, "actor"},
             {"message", urpg::events::EventTemplateParameterType::String, true, {}, {}}},
            {{"message", urpg::events::EventCommandKind::Message, "${speaker}", "${message}"},
             {"reward", urpg::events::EventCommandKind::Gold, {}, {}, 100}}};
}

std::filesystem::path uniqueRoot() {
    return std::filesystem::temp_directory_path() /
           ("urpg_event_template_project_" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
}

} // namespace

TEST_CASE("event template project owner saves reopens and participates in project history",
          "[events][templates][project][history][pcq405]") {
    const auto root = uniqueRoot();
    std::filesystem::remove_all(root);

    urpg::project::ProjectOperationJournal journal(root / ".urpg" / "operations.json");
    urpg::project::ProjectOperationCoordinator coordinator(&journal);
    urpg::events::EventTemplateProjectService service(coordinator);

    urpg::events::EventTemplateLibrary library;
    REQUIRE(library.registerDefinition(rewardTemplate(1)));
    urpg::events::EventDocument document;
    document.addMap({"town", 20, 20});
    REQUIRE(library.instantiate(
        {"reward.instance", "narrative.reward", 0, "ce.reward",
         {{"speaker", "actor.guide"}, {"message", "Thanks"}}, {{"reward", "amount", 325}}},
        document));
    document.addEvent({"guide", "town", 1, 1,
                       {{"main", 0, urpg::events::EventTrigger::ActionButton, {},
                         {{"call", urpg::events::EventCommandKind::CommonEvent, "ce.reward"}}}}});

    REQUIRE(service.initialize(root, library, document).success);
    REQUIRE(service.revision() == 1);
    REQUIRE(std::filesystem::is_regular_file(service.documentPath()));

    auto update = rewardTemplate(2);
    update.commands.push_back({"sound", urpg::events::EventCommandKind::Sound, "audio.reward"});
    const auto preview = service.previewUpdate(update);
    REQUIRE(preview.valid);
    REQUIRE(preview.affected_instance_count == 1);
    REQUIRE(preview.caller_source_ids == std::vector<std::string>{"guide/main"});

    const auto applied = service.applyReviewedUpdate("template-update-001", update, service.revision());
    REQUIRE(applied.success);
    REQUIRE(service.revision() == 2);
    REQUIRE(service.document().commonEvents().at("ce.reward").commands.size() == 3);
    REQUIRE(coordinator.undoLabel() == "Update event template");

    urpg::project::ProjectOperationCoordinator reopenCoordinator;
    urpg::events::EventTemplateProjectService reopened(reopenCoordinator);
    REQUIRE(reopened.open(root).success);
    REQUIRE(reopened.revision() == 2);
    REQUIRE(reopened.library().definitions().at("narrative.reward").version == 2);
    REQUIRE(reopened.document().commonEvents().at("ce.reward").commands[1].amount == 325);

    REQUIRE(service.undoLast().success);
    REQUIRE(service.library().definitions().at("narrative.reward").version == 1);
    REQUIRE(service.document().commonEvents().at("ce.reward").commands.size() == 2);
    REQUIRE(service.redoLast().success);
    REQUIRE(service.library().definitions().at("narrative.reward").version == 2);

    std::filesystem::remove_all(root);
}

TEST_CASE("event authoring route exposes template picker and reviewed replacement impact",
          "[events][templates][editor][pcq405]") {
    const auto root = uniqueRoot();
    std::filesystem::remove_all(root);
    urpg::project::ProjectOperationCoordinator coordinator;
    urpg::events::EventTemplateProjectService service(coordinator);

    urpg::events::EventTemplateLibrary library;
    REQUIRE(library.registerDefinition(rewardTemplate(1)));
    REQUIRE(library.registerDefinition(rewardTemplate(1, "narrative.reward.v2")));
    urpg::events::EventDocument document;
    document.addMap({"town", 10, 10});
    REQUIRE(library.instantiate(
        {"reward.instance", "narrative.reward", 0, "ce.reward",
         {{"speaker", "actor.guide"}, {"message", "Thanks"}}, {{"reward", "amount", 450}}},
        document));
    REQUIRE(service.initialize(root, library, document).success);

    urpg::editor::EventAuthoringPanel panel;
    panel.model().bindTemplateProject(&service);
    REQUIRE(panel.model().selectTemplate("narrative.reward"));
    const auto preview = panel.model().previewTemplateDelete("narrative.reward", "narrative.reward.v2");
    REQUIRE(preview.valid);
    panel.render();
    const auto& review = panel.lastRenderSnapshot();
    REQUIRE(review.template_project_open);
    REQUIRE(review.template_picker_ids ==
            std::vector<std::string>{"narrative.reward", "narrative.reward.v2"});
    REQUIRE(review.selected_template_id == "narrative.reward");
    REQUIRE(review.template_review_pending);
    REQUIRE(review.template_review_can_apply);
    REQUIRE(review.template_review_affected_instance_count == 1);

    REQUIRE(panel.model().applyReviewedTemplateChange("template-replace-001").success);
    panel.render();
    REQUIRE_FALSE(panel.lastRenderSnapshot().template_review_pending);
    REQUIRE_FALSE(service.library().definitions().contains("narrative.reward"));
    REQUIRE(service.library().instances().at("reward.instance").template_id == "narrative.reward.v2");
    REQUIRE(service.document().commonEvents().at("ce.reward").commands[1].amount == 450);

    std::filesystem::remove_all(root);
}
