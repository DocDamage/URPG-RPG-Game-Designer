#include "engine/core/assets/local_asset_catalog.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

std::filesystem::path uniqueCatalogPerfRoot() {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / ("urpg_local_catalog_perf_" + std::to_string(tick));
}

std::string paddedNumber(size_t value) {
    std::ostringstream output;
    output << std::setw(6) << std::setfill('0') << value;
    return output.str();
}

} // namespace

TEST_CASE("LocalAssetCatalog pages a deterministic 100000-row metadata fixture", "[assets][local_catalog][perf]") {
    constexpr size_t recordCount = 100'000;
    constexpr size_t shardSize = 1'000;
    const auto root = uniqueCatalogPerfRoot();
    std::filesystem::create_directories(root);
    const auto fixtureStart = std::chrono::steady_clock::now();

    nlohmann::json shards = nlohmann::json::array();
    for (size_t shardIndex = 0; shardIndex < recordCount / shardSize; ++shardIndex) {
        const auto shardName = "catalog-perf-" + paddedNumber(shardIndex) + ".jsonl";
        std::ofstream shard(root / shardName, std::ios::trunc);
        for (size_t recordIndex = 0; recordIndex < shardSize; ++recordIndex) {
            const size_t id = shardIndex * shardSize + recordIndex;
            const auto filename = "sprite_" + paddedNumber(id) + ".png";
            shard << nlohmann::json({
                         {"asset_id", "local:" + std::to_string(id)},
                         {"virtual_path", "external/library/sprites/" + filename},
                         {"source_root", "external/library"},
                         {"filename", filename},
                         {"extension", "png"},
                         {"media_kind", "image"},
                         {"archive_kind", ""},
                         {"size_bytes", 256},
                         {"pack", id % 2 == 0 ? "heroes" : "environment"},
                         {"category", "sprites"},
                         {"tags", nlohmann::json::array({"kind:image", id % 2 == 0 ? "name:hero" : "name:tree"})},
                     })
                         .dump()
                      << '\n';
        }
        shards.push_back({{"path", shardName}, {"record_count", shardSize}});
    }
    {
        std::ofstream manifest(root / "catalog_meta.json", std::ios::trunc);
        manifest << nlohmann::json({
                        {"schema_version", urpg::assets::kLocalAssetCatalogSchema},
                        {"generated_at", "2026-07-13T00:00:00+00:00"},
                        {"scan_complete", true},
                        {"counts", {{"asset_count", recordCount}, {"hash_pending_count", recordCount}, {"archive_count", 0}}},
                        {"roots", {{{"id", "external/library"}, {"state", "complete"}, {"asset_count", recordCount}}}},
                        {"shards", std::move(shards)},
                    })
                        .dump(2);
    }

    urpg::assets::LocalAssetCatalog catalog;
    const auto loadStart = std::chrono::steady_clock::now();
    const auto load = catalog.load(root);
    const auto loadElapsed = std::chrono::steady_clock::now() - loadStart;
    REQUIRE(load.success);
    REQUIRE(catalog.metadata().assetCount == recordCount);

    urpg::assets::LocalAssetCatalogQuery query;
    query.text = "hero";
    query.pageSize = 75;
    query.offset = 75;
    const auto queryStart = std::chrono::steady_clock::now();
    const auto page = catalog.query(query);
    const auto queryElapsed = std::chrono::steady_clock::now() - queryStart;
    INFO("fixture_ms=" << std::chrono::duration_cast<std::chrono::milliseconds>(
                                 std::chrono::steady_clock::now() - fixtureStart)
                                 .count()
                       << " load_ms=" << std::chrono::duration_cast<std::chrono::milliseconds>(loadElapsed).count()
                       << " query_ms=" << std::chrono::duration_cast<std::chrono::milliseconds>(queryElapsed).count());
    REQUIRE(page.totalMatches == 50'000);
    REQUIRE(page.records.size() == 75);
    REQUIRE(page.hasMore);
    REQUIRE(page.records.front().assetId == "local:150");
    REQUIRE(loadElapsed < std::chrono::seconds(2));
    REQUIRE(queryElapsed < std::chrono::seconds(30));

    std::filesystem::remove_all(root);
}
