#pragma once

#include <functional>
#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace urpg::copilot {

struct EditProposal;

/**
 * @brief Represents a rule or constraint that the Producer Copilot must uphold.
 */
struct CanonConstraint {
    std::string id;
    std::string description;
    std::string severity; // "ERROR", "WARNING", "INFO"
    
    // Hard constraints fail validation when their predicate returns false.
    bool isHardConstraint = true;

    // When provided, returns true for compliant proposals and false for violations.
    std::function<bool(const EditProposal&)> predicate;
};

/**
 * @brief A proposal from the Copilot for a project change.
 */
struct EditProposal {
    std::string targetFile;
    std::string description;
    std::string diffHint;
    bool isCanonCompliant = false;
};

/**
 * @brief Base kernel for the Producer Copilot system.
 * Responsible for canon validation and atomic edit orchestration.
 */
class CopilotKernel {
public:
    void addConstraint(const CanonConstraint& constraint) {
        m_constraints.push_back(constraint);
    }

    /**
     * @brief Validates a proposal against known canon constraints.
     */
    bool validateProposal(EditProposal& proposal) {
        // Basic baseline: check for "TODO" or empty descriptions as a dummy failure
        if (proposal.description.empty() || proposal.description.find("TODO") != std::string::npos) {
            proposal.isCanonCompliant = false;
            return false;
        }

        for (const auto& constraint : m_constraints) {
            if (constraint.isHardConstraint && constraint.predicate && !constraint.predicate(proposal)) {
                proposal.isCanonCompliant = false;
                return false;
            }
        }

        proposal.isCanonCompliant = true;
        return true;
    }

private:
    std::vector<CanonConstraint> m_constraints;
};

} // namespace urpg::copilot
