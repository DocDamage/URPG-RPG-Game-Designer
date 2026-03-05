#include "engine/core/format/canonical_json.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Canonical JSON emits deterministic compact output", "[format]") {
    nlohmann::json value;
    value["z"] = 3;
    value["a"] = 1;
    value["nested"] = {
        {"b", 2},
        {"a", 1}
    };

    const std::string output = urpg::DumpCanonicalJson(value);
    REQUIRE(output == "{\"a\":1,\"nested\":{\"a\":1,\"b\":2},\"z\":3}");
}
