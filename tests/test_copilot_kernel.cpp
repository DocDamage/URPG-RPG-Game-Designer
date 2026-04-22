#include <catch2/catch_test_macros.hpp>
#include "engine/core/copilot/copilot_kernel.h"

using namespace urpg::copilot;

TEST_CASE("CopilotKernel validates edit proposals against canon", "[copilot]") {
    CopilotKernel kernel;
    
    CanonConstraint noGaps = {
        "NO_GAPS",
        "Map cannot have empty cells",
        "ERROR",
        true,
        [](const EditProposal& proposal) {
            return proposal.description.find("empty cells") == std::string::npos &&
                   proposal.diffHint.find("gap") == std::string::npos;
        }
    };
    kernel.addConstraint(noGaps);

    SECTION("Validates a simple compliant proposal") {
        EditProposal proposal = {"map_01.json", "Fixed tile alignment", "diff..."};
        REQUIRE(kernel.validateProposal(proposal));
        REQUIRE(proposal.isCanonCompliant);
    }

    SECTION("Rejects empty descriptions") {
        EditProposal empty = {"map_01.json", "", ""};
        REQUIRE_FALSE(kernel.validateProposal(empty));
        REQUIRE_FALSE(empty.isCanonCompliant);
    }

    SECTION("Rejects proposals that violate registered predicates") {
        EditProposal bad = {"map_01.json", "Introduced empty cells near the map border", "diff adds gap tiles"};
        REQUIRE_FALSE(kernel.validateProposal(bad));
        REQUIRE_FALSE(bad.isCanonCompliant);
    }

    SECTION("Allows proposals that do not trigger registered predicates") {
        EditProposal safe = {"map_01.json", "Adjusted cliff edge transitions", "diff updates autotile neighbors"};
        REQUIRE(kernel.validateProposal(safe));
        REQUIRE(safe.isCanonCompliant);
    }
}
