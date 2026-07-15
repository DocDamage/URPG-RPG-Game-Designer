#include "editor/assets/asset_relink_panel.h"
#include <imgui.h>

namespace urpg::editor {

void AssetRelinkPanel::setProjectRoot(const std::filesystem::path& root) {
    project_root_ = root;
    scanned_ = false;
    selected_candidates_.clear();
    last_relinked_asset_id_.clear();
    status_message_.clear();
}

void AssetRelinkPanel::Render(const urpg::FrameContext&) {
    if (!m_visible) return;

    ImGui::Begin("Asset Relink Manager", &m_visible);

    if (project_root_.empty()) {
        ImGui::TextDisabled("Open a project to use the Asset Relink Manager.");
        ImGui::End();
        return;
    }

    if (!scanned_) {
        missing_assets_ = relink_service_.scanMissingAssets(project_root_);
        scanned_ = true;
    }

    if (ImGui::Button("Scan / Refresh")) {
        scanned_ = false;
    }

    ImGui::SameLine();
    ImGui::Text("Missing Assets: %zu", missing_assets_.size());
    if (!last_relinked_asset_id_.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Undo Last Relink")) {
            if (relink_service_.undoLastRelink(project_root_, last_relinked_asset_id_)) {
                status_message_ = "Restored the previous attachment manifest for " + last_relinked_asset_id_ + ".";
                last_relinked_asset_id_.clear();
                scanned_ = false;
            } else {
                status_message_ = "Could not undo the last relink.";
            }
        }
    }
    if (!status_message_.empty()) ImGui::TextWrapped("%s", status_message_.c_str());

    ImGui::Separator();

    if (missing_assets_.empty()) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "All attached project assets are fully resolved.");
    } else {
        ImGui::BeginChild("MissingAssetsList");
        for (size_t i = 0; i < missing_assets_.size(); ++i) {
            auto& missing = missing_assets_[i];
            ImGui::PushID(missing.asset_id.c_str());

            ImGui::Text("Asset ID: %s", missing.asset_id.c_str());
            ImGui::TextDisabled("Recorded Path: %s", missing.recorded_promoted_path.generic_string().c_str());
            for (const auto& reference : missing.affected_reference_paths) {
                ImGui::BulletText("Will update: %s", reference.generic_string().c_str());
            }

            if (missing.candidates.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No candidates found. Please copy the asset to content/assets/imported first.");
            } else {
                size_t& selectedCandidate = selected_candidates_[missing.asset_id];
                std::vector<std::string> combo_items;
                for (const auto& c : missing.candidates) {
                    std::string label = c.path.filename().string() + " (";
                    if (c.confidence == urpg::assets::RelinkConfidence::High) label += "High Confidence / Hash Match";
                    else if (c.confidence == urpg::assets::RelinkConfidence::Medium) label += "Medium Confidence / Size + Name Match";
                    else label += "Low Confidence / Name-Only Match";
                    label += ")";
                    combo_items.push_back(label);
                }

                if (selectedCandidate >= combo_items.size()) {
                    selectedCandidate = 0;
                }

                std::string current_item = combo_items[selectedCandidate];
                if (ImGui::BeginCombo("Match Candidates", current_item.c_str())) {
                    for (size_t n = 0; n < combo_items.size(); ++n) {
                        const bool is_selected = (selectedCandidate == n);
                        if (ImGui::Selectable(combo_items[n].c_str(), is_selected)) {
                            selectedCandidate = n;
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();
                if (ImGui::Button("Relink")) {
                    const auto& chosen = missing.candidates[selectedCandidate];
                    if (relink_service_.applyRelink(project_root_, missing.asset_id, chosen.path)) {
                        last_relinked_asset_id_ = missing.asset_id;
                        status_message_ = "Relink applied atomically; Undo Last Relink is available.";
                        scanned_ = false;
                    }
                }
            }

            ImGui::Separator();
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

} // namespace urpg::editor
