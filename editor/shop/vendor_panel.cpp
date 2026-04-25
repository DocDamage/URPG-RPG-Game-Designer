#include "editor/shop/vendor_panel.h"

#include <utility>

namespace urpg::editor {

void VendorPanel::setVendorId(std::string vendor_id) {
    vendor_id_ = std::move(vendor_id);
}

void VendorPanel::setFlags(std::set<std::string> flags) {
    flags_ = std::move(flags);
}

VendorPanelSnapshot VendorPanel::snapshot() const {
    return VendorPanelSnapshot{catalog_.refreshStock(vendor_id_, flags_).size(), catalog_.validate().size()};
}

void VendorPanel::render() {
    last_render_snapshot_ = snapshot();
}

} // namespace urpg::editor
