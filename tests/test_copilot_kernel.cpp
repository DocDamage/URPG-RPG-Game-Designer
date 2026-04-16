#include <catch2/catch_test_macros.hpp>
#include "copilot/copilot_kernel.h"

using namespace urpg::copilot;

TEST_CASE("CopilotKernel validates edit proposals against canon", "[copilot]") {
    CopilotKernel kernel;
    
    CanonConstraint noGaps = {"NO_GAPS", "Map cannot have empty cells", "ERROR", true};
    kernel.addConstraint(noGaps);

    SECTION("Validates a simple compliant proposal") {
        EditProposal proposal = {"map_01.json", "Fixed tile alignment", "diff..."};
        REQUIRE(kernel.validateProposal(proposal));
        REQUIRE(proposal.isCanonCompliant);
    }

    SECTION("Rejects empty descriptions") {
        EditProposal empty = {"map_01.json", "", ""};
        REQUIRE_FALSE(kernel.validateProposal(empty));
    }

    SECTION("Flags deliberate violations") {
        EditProposal bad = {"map_01.json", "Will violate map constraints", "diff..."};
        REQUIRE_FALSE(kernel.validateProposal(bad));
        REQUIRE_FALSE(bad.isCanonCompliant);
    }
}
