#pragma once

#include "engine/core/localization/locale_catalog.h"

#include <string>
#include <vector>

namespace urpg::localization {

class CompletenessChecker {
public:
    void setMasterCatalog(const LocaleCatalog& master);

    std::vector<std::string> checkAgainst(const LocaleCatalog& target) const;
    void checkAgainst(const LocaleCatalog& target,
                      std::vector<std::string>& outMissing,
                      std::vector<std::string>& outExtra) const;

    float getCoveragePercent(const LocaleCatalog& target) const;

private:
    LocaleCatalog m_master;
};

} // namespace urpg::localization
