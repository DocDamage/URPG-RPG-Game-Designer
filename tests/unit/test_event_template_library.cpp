#include "engine/core/events/event_template_library.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::events;

namespace {

EventTemplateDefinition rewardTemplate(const uint32_t version, const int base_gold = 100) {
    return {"narrative.reward", version, EventTemplateKind::Narrative,
            {{"speaker", EventTemplateParameterType::Reference, true, {}, "actor"},
             {"message", EventTemplateParameterType::String, true, {}, ""},
             {"gold", EventTemplateParameterType::Integer, false, base_gold, ""}},
            {EventCommand{"message", EventCommandKind::Message, "${speaker}", "${message}"},
             EventCommand{"reward", EventCommandKind::Gold, "", "", base_gold,
                          {}, {}, {{"bound_gold", "${gold}"}}}}};
}

} // namespace

TEST_CASE("event templates validate parameters instantiate and preserve overrides on update",
          "[events][templates][pcq405]") {
    EventTemplateLibrary library;
    REQUIRE(library.registerDefinition(rewardTemplate(1)));
    REQUIRE(library.validateBindings("narrative.reward", {{"speaker", "actor.guide"}}) ==
            std::vector<std::string>{"missing_parameter:message"});
    REQUIRE(library.validateBindings("narrative.reward", {{"speaker", 3}, {"message", "Thanks"}}) ==
            std::vector<std::string>{"invalid_parameter_type:speaker"});

    EventDocument document;
    EventTemplateInstance instance{"reward.instance", "narrative.reward", 0, "ce.reward",
                                   {{"speaker", "actor.guide"}, {"message", "Thank you"}, {"gold", 150}},
                                   {{"reward", "amount", 275}}};
    REQUIRE(library.instantiate(instance, document));
    REQUIRE(document.commonEvents().at("ce.reward").commands[0].target == "actor.guide");
    REQUIRE(document.commonEvents().at("ce.reward").commands[0].value == "Thank you");
    REQUIRE(document.commonEvents().at("ce.reward").commands[1].amount == 275);
    REQUIRE(document.commonEvents().at("ce.reward").commands[1].payload["bound_gold"] == 150);

    document.addEvent({"guide", "town", 1, 1, {{"main", 0, EventTrigger::ActionButton, {},
                                                {{"call", EventCommandKind::CommonEvent, "ce.reward"}}}}});
    const auto uses = library.findUses("narrative.reward", document);
    REQUIRE(uses.size() == 1);
    REQUIRE(uses[0].caller_source_ids == std::vector<std::string>{"guide/main"});

    auto replacement = rewardTemplate(2);
    replacement.commands.push_back({"sound", EventCommandKind::Sound, "audio.reward"});
    const auto preview = library.previewUpdate(replacement, document);
    REQUIRE(preview.valid);
    REQUIRE(preview.affected_instance_count == 1);
    REQUIRE(preview.preserved_override_count == 1);
    REQUIRE(preview.caller_count == 1);
    REQUIRE(library.applyUpdate(replacement, document));
    REQUIRE(library.instances().at("reward.instance").template_version == 2);
    REQUIRE(document.commonEvents().at("ce.reward").commands.size() == 3);
    REQUIRE(document.commonEvents().at("ce.reward").commands[1].amount == 275);

    const auto saved = library.toJson();
    const auto restored = EventTemplateLibrary::fromJson(saved);
    REQUIRE(restored);
    REQUIRE(restored->instances().at("reward.instance").overrides.size() == 1);
}

TEST_CASE("template deletion previews callers refuses unsafe deletion and supports replacement",
          "[events][templates][impact][pcq405]") {
    EventTemplateLibrary library;
    REQUIRE(library.registerDefinition(rewardTemplate(1)));
    auto replacement = rewardTemplate(1);
    replacement.id = "narrative.reward.v2";
    REQUIRE(library.registerDefinition(replacement));
    EventDocument document;
    document.addMap({"town", 20, 20});
    REQUIRE(library.instantiate({"reward.instance", "narrative.reward", 0, "ce.reward",
                                 {{"speaker", "actor.guide"}, {"message", "Thanks"}},
                                 {{"reward", "amount", 325}}}, document));
    document.addEvent({"guide", "town", 0, 0, {{"main", 0, EventTrigger::ActionButton, {},
                                                {{"call", EventCommandKind::CommonEvent, "ce.reward"}}}}});

    const auto refused = library.previewDelete("narrative.reward", "", document);
    REQUIRE_FALSE(refused.valid);
    REQUIRE(refused.code == "event_template_delete_has_uses");
    REQUIRE(refused.caller_source_ids == std::vector<std::string>{"guide/main"});
    REQUIRE_FALSE(library.deleteTemplate("narrative.reward", "", document));

    EventTemplateDefinition incompatible{"narrative.incompatible", 1, EventTemplateKind::Narrative, replacement.parameters,
                                         {{"message", EventCommandKind::Message, "${speaker}", "${message}"}}};
    REQUIRE(library.registerDefinition(incompatible));
    const auto incompatible_preview = library.previewDelete("narrative.reward", "narrative.incompatible", document);
    REQUIRE_FALSE(incompatible_preview.valid);
    REQUIRE(incompatible_preview.code == "event_template_override_command_missing");
    REQUIRE(library.instances().at("reward.instance").template_id == "narrative.reward");

    EventTemplateImpact applied;
    REQUIRE(library.deleteTemplate("narrative.reward", "narrative.reward.v2", document, &applied));
    REQUIRE(applied.affected_instance_count == 1);
    REQUIRE_FALSE(library.definitions().contains("narrative.reward"));
    REQUIRE(library.instances().at("reward.instance").template_id == "narrative.reward.v2");
    REQUIRE(document.commonEvents().at("ce.reward").commands[1].amount == 325);
    REQUIRE(document.commonEvents().contains("ce.reward"));
    REQUIRE(document.validate().empty());
}
