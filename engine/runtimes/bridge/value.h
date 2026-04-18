#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace urpg {

struct Value;
using Array = std::vector<Value>;
using Object = std::map<std::string, Value>;

struct Value {
    using V = std::variant<std::monostate, bool, int64_t, double, std::string, Array, Object>;

    V v;

    static Value Nil() {
        Value out;
        out.v = std::monostate{};
        return out;
    }

    static Value Int(int64_t x) {
        Value out;
        out.v = x;
        return out;
    }

    static Value Obj(Object o) {
        Value out;
        out.v = std::move(o);
        return out;
    }

    static Value Arr(Array a) {
        Value out;
        out.v = std::move(a);
        return out;
    }

    static Value Str(std::string s) {
        Value out;
        out.v = std::move(s);
        return out;
    }
};

} // namespace urpg
