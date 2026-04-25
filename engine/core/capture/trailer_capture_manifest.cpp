#include "engine/core/capture/trailer_capture_manifest.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace urpg::capture {

TrailerCaptureManifest::TrailerCaptureManifest(std::string id, int frame_count, std::string preset)
    : id_(std::move(id))
    , frame_count_(frame_count)
    , preset_(std::move(preset)) {}

std::string TrailerCaptureManifest::frameName(int frame) const {
    std::ostringstream out;
    out << id_ << "_" << preset_ << "_frame_" << std::setw(6) << std::setfill('0') << frame << ".png";
    return out.str();
}

std::string TrailerCaptureManifest::thumbnailName() const {
    return id_ + "_" + preset_ + "_thumbnail.png";
}

std::vector<TrailerDiagnostic> TrailerCaptureManifest::validate() const {
    std::vector<TrailerDiagnostic> diagnostics;
    if (frame_count_ <= 0) {
        diagnostics.push_back({"empty_frame_sequence", "trailer capture requires at least one frame"});
    }
    if (preset_.empty()) {
        diagnostics.push_back({"missing_media_preset", "trailer capture requires a media preset"});
    }
    return diagnostics;
}

} // namespace urpg::capture
