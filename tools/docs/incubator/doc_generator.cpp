#include "doc_generator.h"

#include <fstream>
#include <iostream>

namespace urpg::editor {

void DocGenerator::registerDoc(const std::string& category,
                               const std::string& name,
                               const std::string& sig,
                               const std::string& desc) {
    m_entries[category].push_back({name, sig, desc, category});
}

void DocGenerator::exportMarkdown(const std::string& filePath) {
    std::ofstream out(filePath);
    if (!out) {
        return;
    }

    out << "# URPG Plugin API Reference\n\n";
    out << "> Status note: this reference includes exported plugin and bridge symbols whose implementation is still partially stubbed or fixture-backed in the current engine.\n\n";
    out << "This document outlines public C++ and JS bridge exports currently exposed by the URPG engine v3.1.\n\n";

    for (const auto& [category, entries] : m_entries) {
        out << "## " << category << "\n\n";
        for (const auto& entry : entries) {
            out << "### `" << entry.name << "`\n\n";
            out << "**Signature:** `" << entry.signature << "`\n\n";
            out << entry.description << "\n\n";
            out << "---\n\n";
        }
    }
}

void DocGenerator::exportHtml(const std::string& filePath) {
    std::ofstream out(filePath);
    if (!out) {
        return;
    }

    out << "<!DOCTYPE html><html><head><title>URPG API Docs</title>";
    out << "<style>body{font-family:sans-serif;background:#202020;color:#ddd;padding:25px;}";
    out << ".category{color:#f58220;border-bottom:1px solid #444;margin-top:40px;}";
    out << ".entry{background:#2a2a2a;padding:15px;margin:10px 0;border-left:4px solid #f58220;}";
    out << "code{color:#88ccff;}</style></head><body>";
    out << "<h1>URPG Plugin API Reference</h1>";
    out << "<p><strong>Status note:</strong> this reference includes exported plugin and bridge symbols whose implementation is still partially stubbed or fixture-backed in the current engine.</p>";

    for (const auto& [category, entries] : m_entries) {
        out << "<h2 class='category'>" << category << "</h2>";
        for (const auto& entry : entries) {
            out << "<div class='entry'>";
            out << "<h3>" << entry.name << "</h3>";
            out << "<p><code>" << entry.signature << "</code></p>";
            out << "<p>" << entry.description << "</p>";
            out << "</div>";
        }
    }

    out << "</body></html>";
}

} // namespace urpg::editor
