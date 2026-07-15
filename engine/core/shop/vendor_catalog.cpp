#include "engine/core/shop/vendor_catalog.h"

#include <stdexcept>

#include <algorithm>
#include <utility>

namespace urpg::shop {

void VendorCatalog::setKnownItems(std::set<std::string> item_ids) {
    known_items_ = std::move(item_ids);
}

void VendorCatalog::addVendor(VendorDefinition vendor) {
    vendors_[vendor.id] = std::move(vendor);
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

nlohmann::json VendorCatalog::serialize() const {
    nlohmann::json vendors = nlohmann::json::array();
    for (const auto& [id, vendor] : vendors_) {
        nlohmann::json stock = nlohmann::json::array();
        for (const auto& item : vendor.stock) {
            stock.push_back({{"item_id", item.item_id}, {"quantity", item.quantity}, {"buy_price", item.buy_price},
                             {"sell_price", item.sell_price}, {"required_flags", item.required_flags}});
        }
        vendors.push_back({{"id", id}, {"stock", std::move(stock)}});
    }
    return {{"schema", "urpg.vendor_catalog.v1"}, {"known_items", known_items_}, {"vendors", std::move(vendors)}};
}

VendorCatalog VendorCatalog::deserialize(const nlohmann::json& json) {
    if (!json.is_object() || json.value("schema", "") != "urpg.vendor_catalog.v1") {
        throw std::invalid_argument("Vendor catalog JSON has an unsupported schema.");
    }
    VendorCatalog catalog;
    catalog.setKnownItems(json.value("known_items", std::set<std::string>{}));
    for (const auto& source_vendor : json.value("vendors", nlohmann::json::array())) {
        VendorDefinition vendor;
        vendor.id = source_vendor.value("id", "");
        for (const auto& source_item : source_vendor.value("stock", nlohmann::json::array())) {
            vendor.stock.push_back({source_item.value("item_id", ""), source_item.value("quantity", 0),
                                    source_item.value("buy_price", 0), source_item.value("sell_price", 0),
                                    source_item.value("required_flags", std::set<std::string>{})});
        }
        catalog.addVendor(std::move(vendor));
    }
    return catalog;
}

} // namespace urpg::shop
