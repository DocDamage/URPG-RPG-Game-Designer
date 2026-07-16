#include "engine/core/events/native_event_command_matrix.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Native event command matrix maps every required authoring and runtime surface", "[events][native_command_matrix]") {
    using namespace urpg::events; REQUIRE(validateNativeEventCommandMatrix().empty()); const auto& matrix=nativeEventCommandMatrix(); REQUIRE(matrix.size()==26);
    for(const auto& row:matrix){REQUIRE(row.kind!=EventCommandKind::Unsupported);REQUIRE_FALSE(row.authoring_ui.empty());REQUIRE_FALSE(row.serialization_key.empty());REQUIRE_FALSE(row.runtime_effect.empty());REQUIRE(row.undo_supported);REQUIRE_FALSE(row.diagnostic_codes.empty());
        EventCommand command{"command."+row.id,row.kind,"target","value",1};const auto json=EventDocument::fromJson({{"maps",nlohmann::json::array()},{"events",nlohmann::json::array({{{"id","event"},{"map_id","map"},{"pages",nlohmann::json::array({{{"id","page"},{"commands",nlohmann::json::array({{{"id",command.id},{"kind",toString(command.kind)},{"target",command.target},{"value",command.value},{"amount",command.amount}}})}}})}}})}}).toJson();
        REQUIRE(json["events"][0]["pages"][0]["commands"][0]["kind"]==row.serialization_key);}
}
TEST_CASE("Native event executor applies state control flow timing and controlled calls", "[events][native_command_matrix]") {
    using namespace urpg::events;NativeEventRuntimeState state;state.allowed_calls={"safe.extension"};
    REQUIRE(executeNativeEventCommand({"switch",EventCommandKind::Switch,"door","true"},state).success);REQUIRE(state.switches["door"]);
    REQUIRE(executeNativeEventCommand({"variable",EventCommandKind::Variable,"score","",42},state).success);REQUIRE(state.variables["score"]==42);
    REQUIRE(executeNativeEventCommand({"condition",EventCommandKind::Condition,"door"},state).branch_taken);REQUIRE(state.branch_depth==1);
    REQUIRE(executeNativeEventCommand({"end",EventCommandKind::EndBranch},state).success);REQUIRE(state.branch_depth==0);
    REQUIRE_FALSE(executeNativeEventCommand({"break",EventCommandKind::BreakLoop},state).success);
    REQUIRE(executeNativeEventCommand({"loop",EventCommandKind::Loop},state).success);REQUIRE(executeNativeEventCommand({"break",EventCommandKind::BreakLoop},state).success);
    REQUIRE(executeNativeEventCommand({"wait",EventCommandKind::Wait,"","",30},state).waits);REQUIRE(state.wait_frames==30);
    REQUIRE_FALSE(executeNativeEventCommand({"script",EventCommandKind::Script,"unsafe"},state).success);REQUIRE(executeNativeEventCommand({"extension",EventCommandKind::Extension,"safe.extension"},state).success);
}
