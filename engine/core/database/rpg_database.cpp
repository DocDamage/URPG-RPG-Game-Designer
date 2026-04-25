#include "engine/core/database/rpg_database.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace urpg::database {

namespace {

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> parts;
    std::stringstream stream(line);
    std::string part;
    while (std::getline(stream, part, ',')) {
        parts.push_back(part);
    }
    return parts;
}

std::string joinTags(const std::set<std::string>& tags) {
    std::string joined;
    for (const auto& tag : tags) {
        if (!joined.empty()) {
            joined += "|";
        }
        joined += tag;
    }
    return joined;
}

std::set<std::string> splitTags(const std::string& value) {
    std::set<std::string> tags;
    std::stringstream stream(value);
    std::string tag;
    while (std::getline(stream, tag, '|')) {
        if (!tag.empty()) {
            tags.insert(tag);
        }
    }
    return tags;
}

} // namespace

void RpgDatabase::upsertActor(ActorRecord actor) {
    actors_[actor.id] = std::move(actor);
}

void RpgDatabase::upsertItem(ItemRecord item) {
    ++item_insert_counts_[item.id];
    items_[item.id] = std::move(item);
}

std::string RpgDatabase::exportItemsCsv() const {
    std::ostringstream out;
    out << "id,name,price,tags\n";
    for (const auto& [_, item] : items_) {
        out << item.id << "," << item.name << "," << item.price << "," << joinTags(item.tags) << "\n";
    }
    return out.str();
}

std::vector<DatabaseDiagnostic> RpgDatabase::validate() const {
    auto diagnostics = parse_diagnostics_;
    for (const auto& [id, count] : item_insert_counts_) {
        if (count > 1) {
            diagnostics.push_back({"duplicate_item_id", "Item id was inserted more than once.", id});
        }
    }
    for (const auto& [id, actor] : actors_) {
        if (actor.max_hp <= 0 || actor.attack <= 0) {
            diagnostics.push_back({"invalid_actor_stat", "Actor generated values must be positive.", id});
        }
        if (actor.max_hp > 999 || actor.attack > 999) {
            diagnostics.push_back({"generated_value_exceeds_cap", "Generated actor stat exceeds configured cap.", id});
        }
    }
    for (const auto& [id, item] : items_) {
        if (item.price < 0) {
            diagnostics.push_back({"invalid_item_price", "Item price cannot be negative.", id});
        }
    }
    return diagnostics;
}

nlohmann::json RpgDatabase::toJson() const {
    nlohmann::json json;
    json["actors"] = nlohmann::json::array();
    for (const auto& [_, actor] : actors_) {
        json["actors"].push_back({
            {"id", actor.id},
            {"name", actor.name},
            {"class_id", actor.class_id},
            {"max_hp", actor.max_hp},
            {"attack", actor.attack},
        });
    }
    json["items"] = nlohmann::json::array();
    for (const auto& [_, item] : items_) {
        json["items"].push_back({{"id", item.id}, {"name", item.name}, {"price", item.price}, {"tags", item.tags}});
    }
    return json;
}

RpgDatabase RpgDatabase::fromItemsCsv(const std::string& csv) {
    RpgDatabase database;
    std::stringstream stream(csv);
    std::string line;
    if (!std::getline(stream, line)) {
        database.parse_diagnostics_.push_back({"csv_missing_required_column", "CSV is empty.", {}});
        return database;
    }

    const auto headers = splitCsvLine(line);
    if (headers.size() < 4 || headers[0] != "id" || headers[1] != "name" || headers[2] != "price" || headers[3] != "tags") {
        database.parse_diagnostics_.push_back({"csv_missing_required_column", "CSV must contain id,name,price,tags columns.", {}});
        return database;
    }

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        const auto parts = splitCsvLine(line);
        if (parts.size() < 4) {
            database.parse_diagnostics_.push_back({"csv_malformed_row", "CSV row has too few columns.", {}});
            continue;
        }
        database.upsertItem(ItemRecord{parts[0], parts[1], std::stoi(parts[2]), splitTags(parts[3])});
    }
    return database;
}

RpgDatabase RpgDatabase::autofill(const AutofillProfile& profile, uint64_t seed) {
    RpgDatabase database;
    const auto hp = std::min(profile.stat_cap, 80 + profile.level * 8 + static_cast<int32_t>(seed % 7));
    const auto attack = std::min(profile.stat_cap, profile.target_attack + static_cast<int32_t>(seed % 5));
    database.upsertActor({"actor.generated", "Generated Actor", "class.generated", hp, attack});
    database.upsertItem({"item.generated", "Generated Item", profile.target_price, {"generated"}});
    return database;
}

} // namespace urpg::database
