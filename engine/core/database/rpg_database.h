#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::database {

struct DatabaseDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct ActorRecord {
    std::string id;
    std::string name;
    std::string class_id;
    int32_t max_hp = 1;
    int32_t attack = 1;
};

struct ItemRecord {
    std::string id;
    std::string name;
    int32_t price = 0;
    std::set<std::string> tags;
};

struct AutofillProfile {
    int32_t level = 1;
    int32_t target_price = 10;
    int32_t target_attack = 10;
    int32_t stat_cap = 999;
};

class RpgDatabase {
public:
    void upsertActor(ActorRecord actor);
    void upsertItem(ItemRecord item);

    const std::map<std::string, ActorRecord>& actors() const { return actors_; }
    const std::map<std::string, ItemRecord>& items() const { return items_; }

    std::string exportItemsCsv() const;
    std::vector<DatabaseDiagnostic> validate() const;
    nlohmann::json toJson() const;

    static RpgDatabase fromItemsCsv(const std::string& csv);
    static RpgDatabase autofill(const AutofillProfile& profile, uint64_t seed);

private:
    std::map<std::string, ActorRecord> actors_;
    std::map<std::string, ItemRecord> items_;
    std::map<std::string, int32_t> item_insert_counts_;
    std::vector<DatabaseDiagnostic> parse_diagnostics_;
};

} // namespace urpg::database
