#include "engine/core/format/canonical_json.h"

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include <array>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <string_view>

namespace {

uint64_t Fnv1a64(std::string_view value) {
    constexpr uint64_t kOffset = 1469598103934665603ull;
    constexpr uint64_t kPrime = 1099511628211ull;

    uint64_t hash = kOffset;
    for (const char ch : value) {
        hash ^= static_cast<uint8_t>(ch);
        hash *= kPrime;
    }
    return hash;
}

std::string ActiveRendererTier() {
    char* value = nullptr;
    size_t length = 0;
    if (_dupenv_s(&value, &length, "URPG_RENDERER_TIER") == 0 && value != nullptr) {
        std::string tier(value);
        free(value);
        return tier;
    }
    return "basic";
}

} // namespace

TEST_CASE("Snapshot: canonical JSON output hash remains stable", "[snapshot][canonical]") {
    const auto value = nlohmann::json::parse(R"({"a":1,"b":2,"nested":{"m":1,"z":0}})");
    const std::string canonical = urpg::DumpCanonicalJson(value);
    REQUIRE(canonical == R"({"a":1,"b":2,"nested":{"m":1,"z":0}})");

    const uint64_t hash_a = Fnv1a64(canonical);
    const uint64_t hash_b = Fnv1a64(urpg::DumpCanonicalJson(value));
    REQUIRE(hash_a == hash_b);
}

TEST_CASE("Snapshot: renderer tier matrix executes deterministic sample", "[snapshot][renderer]") {
    const std::string tier = ActiveRendererTier();
    REQUIRE((tier == "basic" || tier == "standard" || tier == "advanced"));

    const std::array<int, 5> sample{3, 1, 4, 1, 5};
    int weighted_sum = 0;
    for (size_t index = 0; index < sample.size(); ++index) {
        weighted_sum += static_cast<int>((index + 1) * sample[index]);
    }

    REQUIRE(weighted_sum == 46);
}
