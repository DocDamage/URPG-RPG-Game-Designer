#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::level {

struct PathGridPoint {
    int32_t x = 0;
    int32_t y = 0;

    friend bool operator==(const PathGridPoint&, const PathGridPoint&) = default;
};

struct PathfindingDiagnostic {
    std::string code;
    PathGridPoint point;
    std::string reason;

    friend bool operator==(const PathfindingDiagnostic&, const PathfindingDiagnostic&) = default;
};

struct PathfindingResult {
    bool found = false;
    std::string reason;
    int32_t total_cost = 0;
    std::vector<PathGridPoint> nodes;
    std::vector<PathfindingDiagnostic> diagnostics;
};

class PathfindingGraph {
  public:
    PathfindingGraph(int32_t width, int32_t height);

    [[nodiscard]] int32_t width() const { return width_; }
    [[nodiscard]] int32_t height() const { return height_; }
    [[nodiscard]] bool contains(PathGridPoint point) const;
    [[nodiscard]] bool blocked(PathGridPoint point) const;
    [[nodiscard]] int32_t cellCost(PathGridPoint point) const;
    [[nodiscard]] std::string blockReason(PathGridPoint point) const;
    [[nodiscard]] bool traversalBlocked(PathGridPoint from, PathGridPoint to) const;
    [[nodiscard]] std::string traversalBlockReason(PathGridPoint from, PathGridPoint to) const;

    bool setBlocked(int32_t x, int32_t y, bool blocked, std::string reason = {});
    bool setCellCost(int32_t x, int32_t y, int32_t cost);
    bool setTraversalBlocked(PathGridPoint from, PathGridPoint to, bool blocked, std::string reason = {});

    [[nodiscard]] PathfindingResult findPath(PathGridPoint start, PathGridPoint goal) const;

  private:
    struct Cell {
        bool blocked = false;
        int32_t cost = 1;
        std::string reason;
    };

    struct Traversal {
        bool blocked = false;
        std::string reason;
    };

    [[nodiscard]] size_t index(PathGridPoint point) const;
    [[nodiscard]] std::optional<size_t> traversalIndex(PathGridPoint from, PathGridPoint to) const;

    int32_t width_ = 0;
    int32_t height_ = 0;
    std::vector<Cell> cells_;
    std::vector<Traversal> traversals_;
};

} // namespace urpg::level
