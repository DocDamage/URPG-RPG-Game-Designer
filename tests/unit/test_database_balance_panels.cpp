#include "editor/balance/balance_panel.h"
#include "editor/database/database_panel.h"
#include "editor/shop/vendor_panel.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Database balance and vendor panels expose headless snapshots", "[editor][database][balance][ffs12]") {
    urpg::database::RpgDatabase database;
    database.upsertActor({"actor.hero", "Hero", "class.hero", 100, 20});
    database.upsertItem({"item.potion", "Potion", 50, {}});

    urpg::editor::DatabasePanel database_panel;
    database_panel.setDatabase(database);
    database_panel.render();
    REQUIRE(database_panel.lastRenderSnapshot().actor_count == 1);
    REQUIRE(database_panel.lastRenderSnapshot().item_count == 1);

    urpg::editor::BalancePanel balance_panel;
    balance_panel.setRoute({10, {{"reward", 15, 4, {}, false, 0}}});
    balance_panel.render();
    REQUIRE(balance_panel.lastRenderSnapshot().final_gold == 25);
    REQUIRE(balance_panel.lastRenderSnapshot().total_xp == 4);

    urpg::editor::VendorPanel vendor_panel;
    vendor_panel.catalog().setKnownItems({"item.potion"});
    vendor_panel.catalog().addVendor({"vendor.town", {{"item.potion", 2, 50, 25, {}}}});
    vendor_panel.setVendorId("vendor.town");
    vendor_panel.render();
    REQUIRE(vendor_panel.lastRenderSnapshot().visible_stock_count == 1);
}
