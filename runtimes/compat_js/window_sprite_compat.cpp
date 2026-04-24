// WindowCompat - Sprite Surface Implementation
// Phase 2 - Compat Layer

#include "window_compat.h"
#include "window_render_helpers.h"

#include <algorithm>
#include <unordered_map>

namespace urpg {
namespace compat {

namespace {

std::unordered_map<BitmapHandle, SpriteBitmapInfo> g_spriteBitmaps;
BitmapHandle g_nextSpriteBitmapHandle = 0x40000000u;

BitmapHandle allocateTrackedSpriteBitmap(const std::string& assetId) {
    if (assetId.empty()) {
        return INVALID_BITMAP;
    }

    BitmapHandle handle = g_nextSpriteBitmapHandle++;
    if (handle == INVALID_BITMAP) {
        handle = g_nextSpriteBitmapHandle++;
    }
    g_spriteBitmaps[handle] = SpriteBitmapInfo{handle, assetId};
    return handle;
}

void releaseTrackedSpriteBitmap(BitmapHandle handle) {
    if (handle == INVALID_BITMAP) {
        return;
    }
    g_spriteBitmaps.erase(handle);
}

std::optional<SpriteBitmapInfo> lookupTrackedSpriteBitmap(BitmapHandle handle) {
    if (handle == INVALID_BITMAP) {
        return std::nullopt;
    }

    const auto it = g_spriteBitmaps.find(handle);
    if (it == g_spriteBitmaps.end()) {
        return std::nullopt;
    }
    return it->second;
}

} // namespace

// ============================================================================
// Sprite_Character Implementation
// ============================================================================

Sprite_Character::Sprite_Character(const CreateParams& params)
    : x_(params.x)
    , y_(params.y)
    , characterName_(params.characterName)
    , characterIndex_(normalizeCharacterIndex(params.characterIndex))
{
    refreshSourceRect();
    reloadBitmapForCurrentAsset();
}

Sprite_Character::~Sprite_Character() {
    releaseBitmap();
}

void Sprite_Character::setCharacterName(const std::string& name) {
    if (characterName_ != name) {
        characterName_ = name;
        reloadBitmapForCurrentAsset();
    }
}

void Sprite_Character::setCharacterIndex(int32_t index) {
    const int32_t normalizedIndex = normalizeCharacterIndex(index);
    if (characterIndex_ != normalizedIndex) {
        characterIndex_ = normalizedIndex;
        refreshSourceRect();
    }
}

void Sprite_Character::setDirection(int32_t dir) {
    const int32_t normalizedDirection = normalizeCharacterDirection(dir);
    if (direction_ != normalizedDirection) {
        direction_ = normalizedDirection;
        refreshSourceRect();
    }
}

void Sprite_Character::setPattern(int32_t pattern) {
    const int32_t normalizedPattern = normalizeCharacterPattern(pattern);
    if (pattern_ != normalizedPattern) {
        pattern_ = normalizedPattern;
        animationFrameCounter_ = 0;
        refreshSourceRect();
    }
}

void Sprite_Character::update() {
    constexpr int32_t kAnimationStepFrames = 12;
    ++animationFrameCounter_;
    if (animationFrameCounter_ < kAnimationStepFrames) {
        return;
    }

    animationFrameCounter_ = 0;
    pattern_ = normalizeCharacterPattern(pattern_ + 1);
    refreshSourceRect();
}

std::optional<SpriteBitmapInfo> Sprite_Character::getBitmapInfo() const {
    return lookupTrackedSpriteBitmap(bitmap_);
}

std::optional<SpriteBitmapInfo> Sprite_Character::lookupBitmapInfo(BitmapHandle handle) {
    return lookupTrackedSpriteBitmap(handle);
}

void Sprite_Character::reloadBitmapForCurrentAsset() {
    if (bitmap_ != INVALID_BITMAP && bitmapAssetId_ == characterName_) {
        return;
    }

    releaseBitmap();
    bitmapAssetId_.clear();

    if (characterName_.empty()) {
        return;
    }

    bitmap_ = allocateTrackedSpriteBitmap(characterName_);
    bitmapAssetId_ = characterName_;
}

void Sprite_Character::releaseBitmap() {
    releaseTrackedSpriteBitmap(bitmap_);
    bitmap_ = INVALID_BITMAP;
    bitmapAssetId_.clear();
}

void Sprite_Character::refreshSourceRect() {
    sourceRect_ = resolveCharacterSourceRect(characterIndex_, direction_, pattern_);
}

void Sprite_Character::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    const std::string spriteCharacterStubNote =
        "Fixture-backed object registration cannot mutate per-instance Sprite_Character state yet.";

    methods.push_back({"setX", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteCharacterStubNote});

    methods.push_back({"setY", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteCharacterStubNote});

    methods.push_back({"setDirection", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteCharacterStubNote});

    methods.push_back({"setPattern", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteCharacterStubNote});

    methods.push_back({"setVisible", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteCharacterStubNote});

    methods.push_back({"setBlendMode", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteCharacterStubNote});

    methods.push_back({"setOpacity", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteCharacterStubNote});

    methods.push_back({"setScale", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteCharacterStubNote});

    ctx.registerObject("Sprite_Character", methods);
}

// ============================================================================
// Sprite_Actor Implementation
// ============================================================================

Sprite_Actor::Sprite_Actor(const CreateParams& params)
    : actorId_(params.actorId)
    , battlerName_(params.battlerName.empty() ? resolveActorBattlerAssetId(params.actorId) : params.battlerName)
    , x_(params.x)
    , y_(params.y)
{
    reloadBitmapForCurrentAsset();
}

Sprite_Actor::~Sprite_Actor() {
    releaseBitmap();
}

void Sprite_Actor::setBattlerName(const std::string& name) {
    if (battlerName_ == name) {
        return;
    }

    battlerName_ = name;
    reloadBitmapForCurrentAsset();
}

void Sprite_Actor::setMotion(int32_t motion) {
    motion_ = std::max(0, motion);
    motionFramesRemaining_ = 0;
    motionLoops_ = false;
}

void Sprite_Actor::startMotion(int32_t motion) {
    const int32_t resolvedMotion = std::max(0, motion);
    startResolvedMotion(resolvedMotion, isLoopingActorMotion(resolvedMotion));
}

void Sprite_Actor::startAnimation(int32_t animationId) {
    animationId_ = animationId;
    animationFramesRemaining_ = resolveAnimationDurationFrames(animationId);
    animationPlaying_ = animationFramesRemaining_ > 0;
    if (!animationPlaying_) {
        animationId_ = 0;
    }
}

void Sprite_Actor::startEffect(const std::string& effect) {
    currentEffect_ = effect;
    effectDurationFrames_ = resolveEffectDurationFrames(effect);
    effecting_ = effectDurationFrames_ > 0;

    if (effect == "instantCollapse" && effecting_) {
        opacity_ = 0;
    }
    if (!effecting_) {
        currentEffect_.clear();
    }
}

void Sprite_Actor::update() {
    if (motionFramesRemaining_ > 0) {
        --motionFramesRemaining_;
        if (motionFramesRemaining_ <= 0) {
            if (motionLoops_) {
                motionFramesRemaining_ = resolveActorMotionDurationFrames(motion_);
            } else {
                motion_ = 0;
                motionFramesRemaining_ = 0;
                motionLoops_ = false;
            }
        }
    }

    if (animationPlaying_) {
        if (animationFramesRemaining_ > 0) {
            --animationFramesRemaining_;
        }
        if (animationFramesRemaining_ <= 0) {
            animationPlaying_ = false;
            animationFramesRemaining_ = 0;
            animationId_ = 0;
        }
    }

    if (!effecting_) {
        return;
    }

    if (currentEffect_ == "collapse") {
        opacity_ = std::max(0, opacity_ - 8);
    } else if (currentEffect_ == "bossCollapse") {
        opacity_ = std::max(0, opacity_ - 4);
    } else if (currentEffect_ == "instantCollapse") {
        opacity_ = 0;
    }

    if (effectDurationFrames_ > 0) {
        --effectDurationFrames_;
    }
    if (effectDurationFrames_ <= 0) {
        effecting_ = false;
        effectDurationFrames_ = 0;
        currentEffect_.clear();
    }
}

std::optional<SpriteBitmapInfo> Sprite_Actor::getBitmapInfo() const {
    return lookupTrackedSpriteBitmap(bitmap_);
}

std::optional<SpriteBitmapInfo> Sprite_Actor::lookupBitmapInfo(BitmapHandle handle) {
    return lookupTrackedSpriteBitmap(handle);
}

void Sprite_Actor::reloadBitmapForCurrentAsset() {
    if (bitmap_ != INVALID_BITMAP && bitmapAssetId_ == battlerName_) {
        return;
    }

    releaseBitmap();
    bitmapAssetId_.clear();

    if (battlerName_.empty()) {
        return;
    }

    bitmap_ = allocateTrackedSpriteBitmap(battlerName_);
    bitmapAssetId_ = battlerName_;
}

void Sprite_Actor::releaseBitmap() {
    releaseTrackedSpriteBitmap(bitmap_);
    bitmap_ = INVALID_BITMAP;
    bitmapAssetId_.clear();
}

void Sprite_Actor::startResolvedMotion(int32_t motion, bool looping) {
    motion_ = motion;
    motionLoops_ = looping;
    motionFramesRemaining_ = resolveActorMotionDurationFrames(motion);
}

void Sprite_Actor::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    const std::string spriteActorStubNote =
        "Fixture-backed object registration cannot mutate or observe per-instance Sprite_Actor state yet.";

    methods.push_back({"setMotion", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteActorStubNote});

    methods.push_back({"startMotion", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteActorStubNote});

    methods.push_back({"startAnimation", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteActorStubNote});

    methods.push_back({"isAnimationPlaying", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteActorStubNote});

    methods.push_back({"startEffect", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteActorStubNote});

    methods.push_back({"setVisible", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteActorStubNote});

    methods.push_back({"setBlendMode", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteActorStubNote});

    methods.push_back({"setOpacity", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, spriteActorStubNote});

    ctx.registerObject("Sprite_Actor", methods);
}

} // namespace compat
} // namespace urpg
