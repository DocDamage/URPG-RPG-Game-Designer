#pragma once

#include "engine/core/shop/vendor_catalog.h"

namespace urpg::editor {

struct VendorPanelSnapshot {
    std::size_t visible_stock_count = 0;
    std::size_t diagnostic_count = 0;
};

class VendorPanel {
public:
    shop::VendorCatalog& catalog() { return catalog_; }
    void setVendorId(std::string vendor_id);
    void setFlags(std::set<std::string> flags);
    VendorPanelSnapshot snapshot() const;
    void render();
    const VendorPanelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    shop::VendorCatalog catalog_;
    std::string vendor_id_;
    std::set<std::string> flags_;
    VendorPanelSnapshot last_render_snapshot_{};
};

} // namespace urpg::editor
