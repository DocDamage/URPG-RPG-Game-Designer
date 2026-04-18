#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace urpg::templates {

    /**
     * @brief Node types for Visual Novel story branching.
     * Part of Wave 3.13 Template Expansion.
     */
    enum class NodeType { Dialogue, Choice, Jump, Condition };

    /**
     * @brief Result of a choice node.
     */
    struct StoryBranch {
        std::string choiceText;
        std::string targetNodeId;
        std::map<std::string, bool> flagRequirements;
    };

    /**
     * @brief Represents a single beat in a Visual Novel scene.
     */
    struct VNNode {
        std::string id;
        NodeType type;
        std::string characterName;
        std::string lineContent;
        std::vector<StoryBranch> branches;
        bool isTerminal = false;
    };

    /**
     * @brief Visual Novel playback controller.
     * Manages state, persistent flags, and scene flow.
     */
    class VNInterpreter {
    public:
        void advance(int choiceIndex = -1) {
            if (m_currentNode.isTerminal) return;

            if (m_currentNode.type == NodeType::Choice && choiceIndex >= 0) {
                // Jump to the choice's target
                m_currentNodeId = m_currentNode.branches[choiceIndex].targetNodeId;
            } else {
                m_currentNodeId = m_nextNodeId;
            }
        }

        void setFlag(const std::string& key, bool value) {
            m_persistentFlags[key] = value;
        }

        bool getFlag(const std::string& key) const {
            if (m_persistentFlags.find(key) != m_persistentFlags.end()) {
                return m_persistentFlags.at(key);
            }
            return false;
        }

        const VNNode& getCurrentNode() const { return m_currentNode; }

    private:
        std::string m_currentNodeId;
        std::string m_nextNodeId;
        VNNode m_currentNode;
        std::map<std::string, bool> m_persistentFlags;
    };

} // namespace urpg::templates
