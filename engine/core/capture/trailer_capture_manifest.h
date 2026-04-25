#pragma once

#include <string>
#include <vector>

namespace urpg::capture {

struct TrailerDiagnostic {
    std::string code;
    std::string message;
};

class TrailerCaptureManifest {
public:
    TrailerCaptureManifest(std::string id, int frame_count, std::string preset);

    [[nodiscard]] std::string frameName(int frame) const;
    [[nodiscard]] std::string thumbnailName() const;
    [[nodiscard]] std::vector<TrailerDiagnostic> validate() const;

private:
    std::string id_;
    int frame_count_{0};
    std::string preset_;
};

} // namespace urpg::capture
