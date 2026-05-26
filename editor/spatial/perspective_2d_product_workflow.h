#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace urpg::editor {

struct Perspective2DProductWorkflowReport {
    bool ready = false;
    std::string map_id;
    size_t draft_layer_count = 0;
    size_t draft_tile_count = 0;
    size_t draft_event_count = 0;
    size_t runtime_layer_count = 0;
    size_t runtime_tile_count = 0;
    size_t runtime_event_count = 0;
    size_t export_package_file_count = 0;
    bool package_signature_present = false;
    size_t transfer_edge_count = 0;
    size_t supported_command_count = 0;
    size_t unsupported_command_count = 0;
    std::vector<std::string> blockers;
};

class Perspective2DProductWorkflow {
  public:
    static Perspective2DProductWorkflowReport Analyze(const std::string& draft_document_json,
                                                      const std::string& runtime_manifest_json,
                                                      const std::string& export_package_manifest_json);
};

} // namespace urpg::editor
