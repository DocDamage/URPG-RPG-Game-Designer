#include "engine/core/events/event_static_analyzer.h"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>

TEST_CASE("Event static analyzer produces actionable object-linked bad-event diagnostics", "[events][static_analysis]") {
    using namespace urpg::events;EventDocument document;document.addMap({"town",10,10});document.addMap({"dead_end",10,10});document.setKnownSwitches({"known"});
    EventPage page;page.id="bad";page.trigger=EventTrigger::Autorun;page.commands={
        {"parallel_a",EventCommandKind::Switch,"known","true",0,{}, {},{{"parallel_lane",true}}},
        {"parallel_b",EventCommandKind::Switch,"known","false",0,{}, {},{{"parallel_lane",true}}},
        {"loop",EventCommandKind::Loop},{"loop_end",EventCommandKind::EndLoop},
        {"voice",EventCommandKind::Message,"","",0,{}, {},{{"voice_id","voice.guide"}}},
        {"transfer",EventCommandKind::Transfer,"dead_end","",0,{}, {},{{"terminal",true}}},
        {"unknown",EventCommandKind::Switch,"missing","true"},
        {"unreachable",EventCommandKind::Gold,"","",10}};
    document.addEvent({"event.bad","town",1,1,{page}});
    EventPage softlock;softlock.id="autorun";softlock.trigger=EventTrigger::Autorun;softlock.commands={{"text",EventCommandKind::Message,"","",0,{}, {},{{"localization_id","softlock.text"}}}};
    document.addEvent({"event.softlock","town",2,1,{softlock}});const auto findings=analyzeEventDocument(document);
    const auto has=[&](const std::string& code){return std::find_if(findings.begin(),findings.end(),[&](const auto& finding){return finding.code==code;});};
    for(const auto& code:{"unsafe_parallel_mutation","tight_loop","infinite_loop","localization_id_missing","voice_caption_missing","transfer_dead_end","missing_switch_reference","event_command_unreachable"}){const auto found=has(code);REQUIRE(found!=findings.end());REQUIRE(found->event_id=="event.bad");REQUIRE_FALSE(found->suggestion.empty());}
    const auto likely=has("likely_autorun_softlock");REQUIRE(likely!=findings.end());REQUIRE(likely->event_id=="event.softlock");
}
TEST_CASE("Event static analyzer keeps a bounded localized transfer event clean", "[events][static_analysis]") {
    using namespace urpg::events;EventDocument document;document.addMap({"town",10,10});document.addMap({"field",10,10});
    EventPage town;town.id="main";town.commands={{"message",EventCommandKind::Message,"","",0,{}, {},{{"localization_id","dialogue.hello"}}},{"transfer",EventCommandKind::Transfer,"field"}};
    EventPage field;field.id="main";field.commands={{"return",EventCommandKind::Transfer,"town"}};document.addEvent({"town.event","town",1,1,{town}});document.addEvent({"field.event","field",1,1,{field}});REQUIRE(analyzeEventDocument(document).empty());
}
