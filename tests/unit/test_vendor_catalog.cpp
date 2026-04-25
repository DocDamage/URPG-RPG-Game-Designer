#include "engine/core/shop/vendor_catalog.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Vendor catalog refresh respects progression gates and database references", "[shop][balance][ffs12]") {
    urpg::shop::VendorCatalog catalog;
    catalog.setKnownItems({"item.potion", "item.elixir"});
    catalog.addVendor({"vendor.town", {{"item.potion", 3, 100, 50, {}}, {"item.elixir", 1, 300, 150, {"flag.elixir"}}}});

    const auto locked = catalog.refreshStock("vendor.town", {});
    const auto unlocked = catalog.refreshStock("vendor.town", {"flag.elixir"});

    REQUIRE(locked.size() == 1);
    REQUIRE(unlocked.size() == 2);
    REQUIRE(catalog.validate().empty());

    catalog.addVendor({"vendor.bad", {{"item.missing", 1, 1, 1, {}}}});
    REQUIRE(catalog.validate().back().code == "missing_vendor_item");
}
