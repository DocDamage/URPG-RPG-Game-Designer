#include "engine/runtimes/bridge/value.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Bridge Value round-trip is stable", "[bridge]") {
    using urpg::Value;

    urpg::Object nested;
    nested["y"] = Value::Int(2);

    urpg::Object root;
    root["x"] = Value::Int(1);
    root["nested"] = Value::Obj(nested);
    root["arr"] = Value::Arr({Value::Int(10), Value::Int(20), Value::Int(30)});

    Value value = Value::Obj(root);
    auto* object = std::get_if<urpg::Object>(&value.v);

    REQUIRE(object != nullptr);
    REQUIRE(object->at("x").v.index() == 2);
}
