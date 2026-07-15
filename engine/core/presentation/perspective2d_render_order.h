#pragma once

#include <string>
#include <vector>

namespace urpg::presentation {

enum class Perspective2DRenderLayer { Ground = 0, Occluder = 1, Prop = 2 };

struct Perspective2DGroundAnchor {
    float spriteLeft = 0.0f;
    float spriteTop = 0.0f;
    float spriteWidth = 0.0f;
    float spriteHeight = 0.0f;
    float feetOffsetY = 0.0f;

    float feetX() const { return spriteLeft + spriteWidth * 0.5f; }
    float feetY() const { return spriteTop + spriteHeight + feetOffsetY; }
};

struct Perspective2DRenderEntry {
    std::string objectId;
    Perspective2DRenderLayer layer = Perspective2DRenderLayer::Prop;
    Perspective2DGroundAnchor anchor;
};

void sortPerspective2DRenderOrder(std::vector<Perspective2DRenderEntry>& entries);

} // namespace urpg::presentation
