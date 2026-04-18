#include <catch2/catch_test_macros.hpp>
#include "engine/core/save/save_serialization_hub.h"
#include "engine/core/global_state_hub.h"
#include <nlohmann/json.hpp>

using namespace urpg::save;
using namespace urpg;

TEST_CASE("SaveSerializationHub: GlobalState Serialization", "[save][serialization][state]") {
    auto& hub = GlobalStateHub::getInstance();
    hub.resetAll();

    SECTION("Snapshot empty state") {
        std::string json = SaveSerializationHub::snapshotGlobalState(hub);
        auto root = nlohmann::json::parse(json);
        REQUIRE(root.contains("switches"));
        REQUIRE(root.contains("variables"));
        REQUIRE(root["switches"].empty());
    }

    SECTION("Snapshot with data") {
        hub.setSwitch("S001_DoorOpen", true);
        hub.setVariable("V001_Gold", 150);
        hub.setVariable("V002_Name", std::string("Hero"));

        std::string json = SaveSerializationHub::snapshotGlobalState(hub);
        auto root = nlohmann::json::parse(json);
        
        REQUIRE(root["switches"]["S001_DoorOpen"] == true);
        REQUIRE(root["variables"]["V001_Gold"] == 150);
        REQUIRE(root["variables"]["V002_Name"] == "Hero");
    }

    SECTION("Round-trip restoration") {
        hub.setSwitch("S001", true);
        hub.setVariable("V001", 42);
        
        std::string json = SaveSerializationHub::snapshotGlobalState(hub);
        hub.clearSessionState();
        REQUIRE(hub.getSwitch("S001") == false);

        SaveSerializationHub::restoreGlobalState(hub, json);
        REQUIRE(hub.getSwitch("S001") == true);
        REQUIRE(std::get<int32_t>(hub.getVariable("V001")) == 42);
    }

    SECTION("Differential saving") {
        hub.setSwitch("S001_Baseline", true);
        hub.setVariable("V001_Baseline", 100);
        hub.snapshotBaseline();

        // Change one thing, add another
        hub.setVariable("V001_Baseline", 200);
        hub.setSwitch("S002_New", true);

        std::string json = SaveSerializationHub::snapshotGlobalState(hub, true);
        auto root = nlohmann::json::parse(json);

        // Should NOT contain S001_Baseline (identical to baseline)
        REQUIRE_FALSE(root["switches"].contains("S001_Baseline"));
        REQUIRE(root["switches"]["S002_New"] == true);
        REQUIRE(root["variables"]["V001_Baseline"] == 200);
        REQUIRE(root["differential"] == true);

        // Restoration should merge
        hub.clearSessionState();
        hub.setSwitch("S001_Baseline", true); // restore baseline manually
        SaveSerializationHub::restoreGlobalState(hub, json);
        
        REQUIRE(hub.getSwitch("S001_Baseline") == true);
        REQUIRE(hub.getSwitch("S002_New") == true);
        REQUIRE(std::get<int32_t>(hub.getVariable("V001_Baseline")) == 200);
    }

    SECTION("Binary format integrity") {
        std::string json = "{\"test\":true}";
        auto binary = SaveSerializationHub::jsonToBinary(json);
        
        // Header URSV
        REQUIRE(binary[0] == 'U');
        REQUIRE(binary[3] == 'V');
        
        std::string restored = SaveSerializationHub::binaryToJson(binary);
        REQUIRE(restored == json);
    }
}
