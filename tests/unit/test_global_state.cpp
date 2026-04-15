#include <catch2/catch_test_macros.hpp>
#include "engine/core/global_state_hub.h"

using namespace urpg;

TEST_CASE("GlobalStateHub Persistence and Access", "[core][global_state]") {
    auto& hub = GlobalStateHub::getInstance();
    hub.clearSessionState();

    hub.setSwitch("1", true);
    hub.setSwitch("2", false);
    hub.setVariable("gold", 100);
    hub.setVariable("playerName", std::string("Hero"));
    hub.setConfig("ui.scale", "1.5");

    SECTION("Switches return correct values") {
        REQUIRE(hub.getSwitch("1") == true);
        REQUIRE(hub.getSwitch("2") == false);
        REQUIRE(hub.getSwitch("99") == false); // Non-existent
    }

    SECTION("Variables store variants") {
        auto gold = hub.getVariable("gold");
        REQUIRE(std::get<int32_t>(gold) == 100);
        
        auto name = hub.getVariable("playerName");
        REQUIRE(std::get<std::string>(name) == "Hero");
    }

    SECTION("Config persists across session clears") {
        hub.clearSessionState();
        REQUIRE(hub.getSwitch("1") == false);
        REQUIRE(hub.getConfig("ui.scale") == "1.5");
    }
}
