#include "engine/core/assets/local_asset_catalog.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path uniqueCatalogRoot() {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / ("urpg_local_asset_catalog_" + std::to_string(tick));
}

void writeJson(const std::filesystem::path& path, const nlohmann::json& value) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::trunc);
    output << value.dump(2);
}

void writeShard(const std::filesystem::path& path, const std::vector<nlohmann::json>& records) {
    std::ofstream output(path, std::ios::trunc);
    for (const auto& record : records) {
        output << record.dump() << '\n';
    }
}

} // namespace

TEST_CASE("LocalAssetCatalog loads compatible metadata and streams paged matches", "[assets][local_catalog]") {
    const auto root = uniqueCatalogRoot();
    std::filesystem::create_directories(root);
    writeShard(root / "catalog-test-00001.jsonl",
               {{{"asset_id", "local:1"},
                 {"virtual_path", "external/assets/Hero.PNG"},
                 {"source_root", "external-assets"},
                 {"filename", "Hero.PNG"},
                 {"extension", "png"},
                 {"media_kind", "image"},
                 {"archive_kind", ""},
                 {"size_bytes", 12},
                 {"tags", {"name:hero", "kind:image"}}},
                {{"asset_id", "local:2"},
                 {"virtual_path", "external/assets/characters.7z"},
                 {"source_root", "external-assets"},
                 {"filename", "characters.7z"},
                 {"extension", "7z"},
                 {"media_kind", "archive"},
                 {"archive_kind", "7z"},
                 {"size_bytes", 20},
                 {"tags", {"kind:archive"}}}});
    writeJson(root / "catalog_meta.json",
              {{"schema_version", urpg::assets::kLocalAssetCatalogSchema},
               {"generated_at", "2026-07-13T00:00:00+00:00"},
               {"scan_complete", true},
               {"counts", {{"asset_count", 2}, {"hash_pending_count", 1}, {"archive_count", 1}}},
               {"roots", {{{"id", "external-assets"}, {"state", "complete"}, {"asset_count", 2}, {"hash_pending_count", 1}}}},
               {"shards", {{{"path", "catalog-test-00001.jsonl"}, {"record_count", 2}}}}});

    urpg::assets::LocalAssetCatalog catalog;
    const auto loaded = catalog.load(root);
    REQUIRE(loaded.success);
    REQUIRE(catalog.metadata().assetCount == 2);
    REQUIRE(catalog.metadata().roots.size() == 1);

    urpg::assets::LocalAssetCatalogQuery incrementalQuery;
    incrementalQuery.text = "hero";
    auto incremental = catalog.beginQuery(incrementalQuery);
    REQUIRE_FALSE(incremental.advance(1));
    REQUIRE(incremental.progress().processedRecords == 1);
    REQUIRE(incremental.result().records.size() == 1);
    while (!incremental.advance(1)) {
    }
    REQUIRE(incremental.progress().complete);
    REQUIRE(incremental.result().totalMatches == 1);

    urpg::assets::LocalAssetCatalogQuery heroQuery;
    heroQuery.text = "HERO";
    heroQuery.pageSize = 1;
    const auto hero = catalog.query(heroQuery);
    REQUIRE(hero.records.size() == 1);
    REQUIRE(hero.records.front().assetId == "local:1");
    REQUIRE(hero.records.front().normalizedFilename == "hero.png");

    urpg::assets::LocalAssetCatalogQuery archiveQuery;
    archiveQuery.archiveOnly = true;
    const auto archives = catalog.query(archiveQuery);
    REQUIRE(archives.totalMatches == 1);
    REQUIRE(archives.records.front().archiveKind == "7z");

    urpg::assets::LocalAssetCatalogQuery cappedQuery;
    cappedQuery.pageSize = 500;
    const auto capped = catalog.query(cappedQuery);
    REQUIRE(capped.records.size() == 2);
    REQUIRE_FALSE(capped.hasMore);

    std::filesystem::remove_all(root);
}

TEST_CASE("LocalAssetCatalog rejects incompatible schema without losing its loaded catalog", "[assets][local_catalog]") {
    const auto root = uniqueCatalogRoot();
    std::filesystem::create_directories(root);
    writeJson(root / "catalog_meta.json",
              {{"schema_version", "urpg.asset_catalog.v0"}, {"shards", nlohmann::json::array()}});

    urpg::assets::LocalAssetCatalog catalog;
    const auto result = catalog.load(root);
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(catalog.isLoaded());
    REQUIRE(result.diagnostics.front().find("catalog_schema_incompatible") != std::string::npos);
    REQUIRE(result.diagnostics.front().find("catalog_interchange.py") != std::string::npos);

    std::filesystem::remove_all(root);
}
