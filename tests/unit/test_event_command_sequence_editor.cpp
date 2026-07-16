#include "editor/events/event_command_sequence_editor.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Structured event editor keeps a long nested event navigable and transactional", "[events][sequence_editor]") {
    using namespace urpg;using namespace urpg::events;using namespace urpg::editor;EventCommandSequenceEditor editor;
    std::vector<EventCommandSequenceRow> rows={{EventCommand{"condition",EventCommandKind::Condition,"door"},0,false}};
    for(int i=0;i<1000;++i)rows.push_back({EventCommand{"message."+std::to_string(i),EventCommandKind::Message,"","Line"},1,false});
    rows.push_back({EventCommand{"else",EventCommandKind::ElseBranch},0,false});rows.push_back({EventCommand{"wait",EventCommandKind::Wait,"","",1},1,false});rows.push_back({EventCommand{"end",EventCommandKind::EndBranch},0,false});editor.load(rows);
    REQUIRE(editor.validateNesting().empty());REQUIRE(editor.rows().size()==1004);REQUIRE(editor.keyAction(EventSequenceKeyAction::ToggleCollapse,0).success);REQUIRE(editor.visibleRows().size()==4);
    REQUIRE(editor.copy(0,1004).success);REQUIRE(editor.paste(1004).success);REQUIRE(editor.rows().size()==2008);REQUIRE(editor.validateNesting().empty());
    const auto before=editor.rows();REQUIRE_FALSE(editor.reorder(1003,1,1).success);REQUIRE(editor.rows().size()==before.size());REQUIRE(editor.validateNesting().empty());
    REQUIRE(editor.multiEdit({1,2,3},"speaker.guide","Updated",0).success);REQUIRE(editor.rows()[2].command.value=="Updated");
}
TEST_CASE("Event command search favorites recent templates and keyboard operations share one model", "[events][sequence_editor]") {
    using namespace urpg::events;using namespace urpg::editor;EventCommandSequenceEditor editor;editor.load({});REQUIRE(editor.setFavorite("transfer",true));auto search=editor.search("world");REQUIRE_FALSE(search.empty());REQUIRE(search[0].favorite);
    REQUIRE(editor.registerTemplate("gate",{{EventCommand{"if",EventCommandKind::Condition,"gate"},0,false},{EventCommand{"msg",EventCommandKind::Message,"","Open"},1,false},{EventCommand{"end",EventCommandKind::EndBranch},0,false}}));
    REQUIRE(editor.insertTemplate(0,"gate").success);REQUIRE(editor.rows().size()==3);REQUIRE(editor.keyAction(EventSequenceKeyAction::Copy,0,3).success);REQUIRE(editor.keyAction(EventSequenceKeyAction::Paste,3).success);REQUIRE(editor.rows().size()==6);
    REQUIRE_FALSE(editor.keyAction(EventSequenceKeyAction::Delete,0,1).success);REQUIRE(editor.rows().size()==6);REQUIRE(editor.validateNesting().empty());
}
