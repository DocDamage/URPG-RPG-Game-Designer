#pragma once

#include <string>
#include <vector>
#include <map>

namespace urpg::editor {

/**
 * @brief Represents a single documented API entry.
 */
struct DocEntry {
    std::string name;
    std::string signature;
    std::string description;
    std::string category;
};

/**
 * @brief Simple tool to generate markdown/HTML documentation for the Plugin API.
 */
class DocGenerator {
public:
    static DocGenerator& instance() {
        static DocGenerator inst;
        return inst;
    }

    /**
     * @brief Manually register an official API export for documentation.
     */
    void registerDoc(const std::string& category, const std::string& name, const std::string& sig, const std::string& desc);

    /**
     * @brief Exports all documented entries to a Markdown file.
     */
    void exportMarkdown(const std::string& filePath);

    /**
     * @brief Exports all documented entries to a simplified HTML page for the editor dashboard.
     */
    void exportHtml(const std::string& filePath);

private:
    DocGenerator() = default;
    std::map<std::string, std::vector<DocEntry>> m_entries; // Category -> Entries
};

} // namespace urpg::editor
