#include "engine/core/shop/vendor_catalog.h"

#include <algorithm>
#include <utility>

namespace urpg::shop {

void VendorCatalog::setKnownItems(std::set<std::string> item_ids) {
    known_items_ = std::move(item_ids);
}

void VendorCatalog::addVendor(VendorDefinition vendor) {
    vendors_[vendor.id] = std::move(vendor);
}

bool VendorCatalog::upsertStockItem(const std::string& vendor_id, VendorStockItem item) {
    if (vendor_id.empty() || item.item_id.empty() || item.quantity < 0 || item.buy_price < 0 || item.sell_price < 0) {
        return false;
    }
    auto& stock = vendors_[vendor_id].stock;
    vendors_[vendor_id].id = vendor_id;
    const auto existing = std::find_if(stock.begin(), stock.end(), [&item](const auto& candidate) {
        return candidate.item_id == item.item_id;
    });
    if (existing == stock.end()) stock.push_back(std::move(item));
    else *existing = std::move(item);
    return true;
}

const VendorDefinition* VendorCatalog::findVendor(const std::string& vendor_id) const {
    const auto it = vendors_.find(vendor_id);
    return it == vendors_.end() ? nullptr : &it->second;
}

std::vector<VendorStockItem> VendorCatalog::refreshStock(const std::string& vendor_id, const std::set<std::string>& active_flags) const {
    std::vector<VendorStockItem> stock;
    const auto it = vendors_.find(vendor_id);
    if (it == vendors_.end()) {
        return stock;
    }
    for (const auto& item : it->second.stock) {
        if (std::includes(active_flags.begin(), active_flags.end(), item.required_flags.begin(), item.required_flags.end())) {
            stock.push_back(item);
        }
    }
    std::stable_sort(stock.begin(), stock.end(), [](const auto& lhs, const auto& rhs) { return lhs.item_id < rhs.item_id; });
    return stock;
}

std::vector<VendorDiagnostic> VendorCatalog::validate() const {
    std::vector<VendorDiagnostic> diagnostics;
    for (const auto& [vendor_id, vendor] : vendors_) {
        for (const auto& item : vendor.stock) {
            if (!known_items_.empty() && known_items_.count(item.item_id) == 0) {
                diagnostics.push_back({"missing_vendor_item", "Vendor references an unknown item.", vendor_id + ":" + item.item_id});
            }
        }
    }
    return diagnostics;
}

nlohmann::json VendorCatalog::toJson() const {
    nlohmann::json vendors = nlohmann::json::array();
    for (const auto& [id, vendor] : vendors_) {
        nlohmann::json stock = nlohmann::json::array();
        for (const auto& item : vendor.stock) {
            stock.push_back({{"item_id", item.item_id}, {"quantity", item.quantity}, {"buy_price", item.buy_price},
                             {"sell_price", item.sell_price}, {"required_flags", item.required_flags}});
        }
        vendors.push_back({{"id", id}, {"stock", std::move(stock)}});
    }
    return {{"schema", "urpg.vendor_catalog.v1"}, {"vendors", std::move(vendors)}};
}

VendorCatalog VendorCatalog::fromJson(const nlohmann::json& json) {
    VendorCatalog catalog;
    if (!json.is_object()) return catalog;
    for (const auto& vendorJson : json.value("vendors", nlohmann::json::array())) {
        if (!vendorJson.is_object()) continue;
        VendorDefinition vendor;
        vendor.id = vendorJson.value("id", "");
        if (vendor.id.empty()) continue;
        for (const auto& stockJson : vendorJson.value("stock", nlohmann::json::array())) {
            if (!stockJson.is_object()) continue;
            VendorStockItem item;
            item.item_id = stockJson.value("item_id", "");
            item.quantity = stockJson.value("quantity", 0);
            item.buy_price = stockJson.value("buy_price", 0);
            item.sell_price = stockJson.value("sell_price", 0);
            for (const auto& flag : stockJson.value("required_flags", nlohmann::json::array())) {
                if (flag.is_string()) item.required_flags.insert(flag.get<std::string>());
            }
            if (!item.item_id.empty()) vendor.stock.push_back(std::move(item));
        }
        catalog.addVendor(std::move(vendor));
    }
    return catalog;
}

} // namespace urpg::shop
