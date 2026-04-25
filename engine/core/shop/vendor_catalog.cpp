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

} // namespace urpg::shop
