#include "engine/core/events/event_reference_picker.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Event reference picker covers every golden stable reference domain", "[events][reference_picker]") {
    using namespace urpg::events;EventReferencePickerCatalog catalog;const auto kinds=catalog.supportedKinds();REQUIRE(kinds.size()==13);
    for(const auto kind:kinds){const std::string id=std::string(eventReferenceKindId(kind))+".stable";REQUIRE(catalog.add({kind,id,"Friendly Label","Search Detail","project"}));const auto found=catalog.search(kind,"friendly");REQUIRE(found.size()==1);REQUIRE(found[0].stable_id==id);const auto bound=catalog.bind({"command",EventCommandKind::Message},kind,id);REQUIRE(bound.success);REQUIRE(bound.command.target==id);REQUIRE(bound.command.payload["reference"]["label"]=="Friendly Label");}
    REQUIRE_FALSE(catalog.add({EventReferenceKind::Map,"map.stable","Duplicate","","",true}));
}
TEST_CASE("Event command reference binding rejects stale choices and uses typed picker mappings", "[events][reference_picker]") {
    using namespace urpg::events;EventReferencePickerCatalog catalog;REQUIRE(catalog.add({EventReferenceKind::MapEntry,"entry.forest.west","Forest West","","forest",true}));REQUIRE(catalog.add({EventReferenceKind::Audio,"audio.gate","Gate Chime","","",true}));REQUIRE(catalog.add({EventReferenceKind::Animation,"animation.spark","Spark","","",true}));REQUIRE(catalog.add({EventReferenceKind::Item,"item.key","Old Key","","",false}));
    const auto transferKind=pickerKindForEventCommand(EventCommandKind::Transfer);REQUIRE(transferKind==EventReferenceKind::MapEntry);const auto transfer=catalog.bind({"transfer",EventCommandKind::Transfer},*transferKind,"entry.forest.west");REQUIRE(transfer.success);REQUIRE(transfer.command.payload["reference"]["owner_id"]=="forest");
    REQUIRE(pickerKindForEventCommand(EventCommandKind::Sound)==EventReferenceKind::Audio);REQUIRE(pickerKindForEventCommand(EventCommandKind::Animation)==EventReferenceKind::Animation);REQUIRE_FALSE(catalog.bind({"item",EventCommandKind::Item},EventReferenceKind::Item,"item.key").success);REQUIRE_FALSE(catalog.bind({"missing",EventCommandKind::Transfer},EventReferenceKind::MapEntry,"entry.deleted").success);
}
