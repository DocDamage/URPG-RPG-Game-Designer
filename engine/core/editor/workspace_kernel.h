#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace urpg::editor {

    /**
     * @brief A single node in the Editor Scene Hierarchy.
     * Part of Wave 5: Workspace Foundations.
     */
    struct SceneNode {
        std::string id;
        std::string name;
        std::string type; // e.g., "Entity", "Light", "Camera", "Trigger"
        std::string parentId;
        std::vector<std::string> childrenIds;
        bool isVisible = true;
        bool isLocked = false;
        bool isSelected = false;
    };

    /**
     * @brief High-level Asset Browser metadata.
     */
    struct AssetEntry {
        std::string path;
        std::string type; // "Texture", "Audio", "Script", "Prefab"
        size_t sizeBytes;
        uint64_t lastModified;
        std::vector<std::string> tags;
    };

    /**
     * @brief Kernel for managing the Editor's workspace state.
     * Bridges the native Scene Authority with the ImGui UI layer.
     */
    class WorkspaceKernel {
    public:
        void addNode(const std::string& name, const std::string& type, const std::string& parentId = "") {
            std::string newId = "node_" + std::to_string(m_nodes.size());
            SceneNode node{newId, name, type, parentId};
            m_nodes[newId] = node;
            if (!parentId.empty() && m_nodes.count(parentId)) {
                m_nodes[parentId].childrenIds.push_back(newId);
            } else {
                m_rootNodes.push_back(newId);
            }
        }

        void deleteNode(const std::string& id) {
            if (!m_nodes.count(id)) return;
            // Recursive deletion logic would go here
            m_nodes.erase(id);
        }

        void selectNode(const std::string& id) {
            for (auto& [nodeId, node] : m_nodes) {
                node.isSelected = (nodeId == id);
            }
        }

        const std::map<std::string, SceneNode>& getNodes() const { return m_nodes; }
        const std::vector<std::string>& getRootNodes() const { return m_rootNodes; }

        // Asset Browser Sync
        void refreshAssets(const std::string& rootPath) {
            m_assets.clear();
            // File system crawling logic (std::filesystem)
        }

    private:
        std::map<std::string, SceneNode> m_nodes;
        std::vector<std::string> m_rootNodes;
        std::vector<AssetEntry> m_assets;
    };

} // namespace urpg::editor
