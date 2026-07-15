#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::shop {

struct VendorDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct VendorStockItem {
    std::string item_id;
    int32_t quantity = 0;
    int32_t buy_price = 0;
    int32_t sell_price = 0;
    std::set<std::string> required_flags;
};

struct VendorDefinition {
    std::string id;
    std::vector<VendorStockItem> stock;
};

class VendorCatalog {
public:
    void setKnownItems(std::set<std::string> item_ids);
    void addVendor(VendorDefinition vendor);
    bool upsertStockItem(const std::string& vendor_id, VendorStockItem item);
    const VendorDefinition* findVendor(const std::string& vendor_id) const;
    std::vector<VendorStockItem> refreshStock(const std::string& vendor_id, const std::set<std::string>& active_flags) const;
    std::vector<VendorDiagnostic> validate() const;
    nlohmann::json toJson() const;
    static VendorCatalog fromJson(const nlohmann::json& json);

private:
    std::set<std::string> known_items_;
    std::map<std::string, VendorDefinition> vendors_;
};

} // namespace urpg::shop
