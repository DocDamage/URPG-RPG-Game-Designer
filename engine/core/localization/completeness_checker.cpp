#include "engine/core/localization/completeness_checker.h"

#include <algorithm>

namespace urpg::localization {

void CompletenessChecker::setMasterCatalog(const LocaleCatalog& master) {
    m_master = master;
}

std::vector<std::string> CompletenessChecker::checkAgainst(const LocaleCatalog& target) const {
    std::vector<std::string> missing;
    std::vector<std::string> extra;
    checkAgainst(target, missing, extra);
    return missing;
}

void CompletenessChecker::checkAgainst(const LocaleCatalog& target,
                                       std::vector<std::string>& outMissing,
                                       std::vector<std::string>& outExtra) const {
    outMissing.clear();
    outExtra.clear();

    const auto masterKeys = m_master.getAllKeys();
    const auto targetKeys = target.getAllKeys();

    for (const auto& key : masterKeys) {
        if (!target.hasKey(key)) {
            outMissing.push_back(key);
        }
    }

    for (const auto& key : targetKeys) {
        if (!m_master.hasKey(key)) {
            outExtra.push_back(key);
        }
    }
}

float CompletenessChecker::getCoveragePercent(const LocaleCatalog& target) const {
    const size_t masterCount = m_master.keyCount();
    if (masterCount == 0) {
        return 0.0f;
    }

    const auto masterKeys = m_master.getAllKeys();
    size_t sharedCount = 0;
    for (const auto& key : masterKeys) {
        if (target.hasKey(key)) {
            ++sharedCount;
        }
    }

    return (static_cast<float>(sharedCount) / static_cast<float>(masterCount)) * 100.0f;
}

} // namespace urpg::localization
