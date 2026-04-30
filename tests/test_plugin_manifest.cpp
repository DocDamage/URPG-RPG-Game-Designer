#include "engine/core/plugin/plugin_manifest.h"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using namespace urpg::plugin;
using json = nlohmann::json;

TEST_CASE("PluginManifest parses JSON correcty", "[plugin]") {
    json j = R"({
      "id": "my_plugin",
      "name": "My Plugin",
      "version": "1.2.3",
      "author": "Alice",
      "description": "Example description",
      "dependencies": [
        "base_core",
        { "id": "optional_mod", "version": "^2.0.0", "optional": true }
      ],
      "permissions": ["fs.read", "net.http"],
      "parameters": {
        "maxLevel": 99,
        "mode": "advanced"
      }
    })"_json;

    PluginManifest m = PluginManifest::fromJson(j);

    REQUIRE(m.id == "my_plugin");
    REQUIRE(m.version == "1.2.3");
    REQUIRE(m.author == "Alice");

    REQUIRE(m.dependencies.size() == 2);
    REQUIRE(m.dependencies[0].pluginId == "base_core");
    REQUIRE_FALSE(m.dependencies[0].isOptional);

    REQUIRE(m.dependencies[1].pluginId == "optional_mod");
    REQUIRE(m.dependencies[1].versionRange == "^2.0.0");
    REQUIRE(m.dependencies[1].isOptional);

    REQUIRE(m.permissions.size() == 2);
    REQUIRE(m.permissions[0] == "fs.read");

    REQUIRE(m.parameters.size() == 2);
    REQUIRE(m.parameters.at("maxLevel") == "99");
}

TEST_CASE("PluginManifest normalizes dependency and permission metadata", "[plugin][robustness]") {
    json j = R"({
      "id": "robust_plugin",
      "dependencies": [
        "z_core",
        "  ",
        "z_core",
        { "id": "a_core", "version": "^1.0.0", "optional": true },
        { "id": "a_core", "version": "^2.0.0", "optional": false },
        { "id": "" }
      ],
      "permissions": ["net.http", " ", "fs.read", "net.http", 7]
    })"_json;

    PluginManifest m = PluginManifest::fromJson(j);

    REQUIRE(m.dependencies.size() == 2);
    REQUIRE(m.dependencies[0].pluginId == "a_core");
    REQUIRE(m.dependencies[0].versionRange == "^1.0.0");
    REQUIRE(m.dependencies[0].isOptional);
    REQUIRE(m.dependencies[1].pluginId == "z_core");
    REQUIRE(m.permissions == std::vector<std::string>{"fs.read", "net.http"});
}
