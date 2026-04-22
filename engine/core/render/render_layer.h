#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace urpg {

/**
 * @brief Command types for the native renderer.
 */
enum class RenderCmdType : uint8_t {
    Sprite = 0,
    Tile = 1,
    Text = 2,
    Rect = 3,
    Clear = 4
};

/**
 * @brief Base structure for a generic render command.
 */
struct RenderCommand {
    virtual ~RenderCommand() = default;
    RenderCmdType type;
    int32_t zOrder = 0;
    float x = 0.0f;
    float y = 0.0f;
};

/**
 * @brief Command to draw a specific sprite from a texture.
 */
struct SpriteCommand : public RenderCommand {
    SpriteCommand() { type = RenderCmdType::Sprite; }
    std::string textureId;
    int32_t srcX = 0;
    int32_t srcY = 0;
    int32_t width = 0;
    int32_t height = 0;
    float opacity = 1.0f;
};

/**
 * @brief Command to draw a specific tile.
 */
struct TileCommand : public RenderCommand {
    TileCommand() { type = RenderCmdType::Tile; }
    std::string tilesetId;
    int32_t tileIndex = 0;
};

/**
 * @brief Command to draw text via the renderer backend.
 */
struct TextCommand : public RenderCommand {
    TextCommand() { type = RenderCmdType::Text; }
    std::string text;
    std::string fontFace;
    int32_t fontSize = 22;
    int32_t maxWidth = 0;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
};

/**
 * @brief Command to draw a solid colored rectangle.
 */
struct RectCommand : public RenderCommand {
    RectCommand() { type = RenderCmdType::Rect; }
    float w = 0.0f;
    float h = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

/**
 * @brief Value-owned payload for sprite draw commands.
 */
struct SpriteRenderData {
    std::string textureId;
    int32_t srcX = 0;
    int32_t srcY = 0;
    int32_t width = 0;
    int32_t height = 0;
    float opacity = 1.0f;
};

/**
 * @brief Value-owned payload for tile draw commands.
 */
struct TileRenderData {
    std::string tilesetId;
    int32_t tileIndex = 0;
};

/**
 * @brief Value-owned payload for text draw commands.
 */
struct TextRenderData {
    std::string text;
    std::string fontFace;
    int32_t fontSize = 22;
    int32_t maxWidth = 0;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
};

/**
 * @brief Value-owned payload for filled rectangle commands.
 */
struct RectRenderData {
    float w = 0.0f;
    float h = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

using FrameRenderPayload = std::variant<std::monostate, SpriteRenderData, TileRenderData, TextRenderData, RectRenderData>;

/**
 * @brief Frame-owned render command record used by the shell/backend boundary.
 */
struct FrameRenderCommand {
    RenderCmdType type = RenderCmdType::Clear;
    int32_t zOrder = 0;
    float x = 0.0f;
    float y = 0.0f;
    FrameRenderPayload payload;

    template <typename T>
    const T* tryGet() const {
        return std::get_if<T>(&payload);
    }
};

inline FrameRenderCommand toFrameRenderCommand(const RenderCommand& cmd) {
    FrameRenderCommand frameCmd;
    frameCmd.type = cmd.type;
    frameCmd.zOrder = cmd.zOrder;
    frameCmd.x = cmd.x;
    frameCmd.y = cmd.y;

    switch (cmd.type) {
        case RenderCmdType::Sprite: {
            if (const auto* spriteCmd = dynamic_cast<const SpriteCommand*>(&cmd)) {
                frameCmd.payload = SpriteRenderData{
                    spriteCmd->textureId,
                    spriteCmd->srcX,
                    spriteCmd->srcY,
                    spriteCmd->width,
                    spriteCmd->height,
                    spriteCmd->opacity,
                };
            }
            break;
        }
        case RenderCmdType::Tile: {
            if (const auto* tileCmd = dynamic_cast<const TileCommand*>(&cmd)) {
                frameCmd.payload = TileRenderData{tileCmd->tilesetId, tileCmd->tileIndex};
            }
            break;
        }
        case RenderCmdType::Text: {
            if (const auto* textCmd = dynamic_cast<const TextCommand*>(&cmd)) {
                frameCmd.payload = TextRenderData{
                    textCmd->text,
                    textCmd->fontFace,
                    textCmd->fontSize,
                    textCmd->maxWidth,
                    textCmd->r,
                    textCmd->g,
                    textCmd->b,
                    textCmd->a,
                };
            }
            break;
        }
        case RenderCmdType::Rect: {
            if (const auto* rectCmd = dynamic_cast<const RectCommand*>(&cmd)) {
                frameCmd.payload = RectRenderData{
                    rectCmd->w,
                    rectCmd->h,
                    rectCmd->r,
                    rectCmd->g,
                    rectCmd->b,
                    rectCmd->a,
                };
            }
            break;
        }
        case RenderCmdType::Clear:
        default:
            frameCmd.payload = std::monostate{};
            break;
    }

    return frameCmd;
}

inline std::shared_ptr<RenderCommand> toLegacyRenderCommand(const FrameRenderCommand& cmd) {
    switch (cmd.type) {
        case RenderCmdType::Sprite: {
            auto legacyCmd = std::make_shared<SpriteCommand>();
            legacyCmd->zOrder = cmd.zOrder;
            legacyCmd->x = cmd.x;
            legacyCmd->y = cmd.y;
            if (const auto* spriteData = cmd.tryGet<SpriteRenderData>()) {
                legacyCmd->textureId = spriteData->textureId;
                legacyCmd->srcX = spriteData->srcX;
                legacyCmd->srcY = spriteData->srcY;
                legacyCmd->width = spriteData->width;
                legacyCmd->height = spriteData->height;
                legacyCmd->opacity = spriteData->opacity;
            }
            return legacyCmd;
        }
        case RenderCmdType::Tile: {
            auto legacyCmd = std::make_shared<TileCommand>();
            legacyCmd->zOrder = cmd.zOrder;
            legacyCmd->x = cmd.x;
            legacyCmd->y = cmd.y;
            if (const auto* tileData = cmd.tryGet<TileRenderData>()) {
                legacyCmd->tilesetId = tileData->tilesetId;
                legacyCmd->tileIndex = tileData->tileIndex;
            }
            return legacyCmd;
        }
        case RenderCmdType::Text: {
            auto legacyCmd = std::make_shared<TextCommand>();
            legacyCmd->zOrder = cmd.zOrder;
            legacyCmd->x = cmd.x;
            legacyCmd->y = cmd.y;
            if (const auto* textData = cmd.tryGet<TextRenderData>()) {
                legacyCmd->text = textData->text;
                legacyCmd->fontFace = textData->fontFace;
                legacyCmd->fontSize = textData->fontSize;
                legacyCmd->maxWidth = textData->maxWidth;
                legacyCmd->r = textData->r;
                legacyCmd->g = textData->g;
                legacyCmd->b = textData->b;
                legacyCmd->a = textData->a;
            }
            return legacyCmd;
        }
        case RenderCmdType::Rect: {
            auto legacyCmd = std::make_shared<RectCommand>();
            legacyCmd->zOrder = cmd.zOrder;
            legacyCmd->x = cmd.x;
            legacyCmd->y = cmd.y;
            if (const auto* rectData = cmd.tryGet<RectRenderData>()) {
                legacyCmd->w = rectData->w;
                legacyCmd->h = rectData->h;
                legacyCmd->r = rectData->r;
                legacyCmd->g = rectData->g;
                legacyCmd->b = rectData->b;
                legacyCmd->a = rectData->a;
            }
            return legacyCmd;
        }
        case RenderCmdType::Clear:
        default: {
            auto legacyCmd = std::make_shared<RenderCommand>();
            legacyCmd->type = RenderCmdType::Clear;
            legacyCmd->zOrder = cmd.zOrder;
            legacyCmd->x = cmd.x;
            legacyCmd->y = cmd.y;
            return legacyCmd;
        }
    }
}

inline std::vector<std::shared_ptr<RenderCommand>> toLegacyRenderCommands(const std::vector<FrameRenderCommand>& commands) {
    std::vector<std::shared_ptr<RenderCommand>> legacyCommands;
    legacyCommands.reserve(commands.size());

    for (const auto& command : commands) {
        legacyCommands.push_back(toLegacyRenderCommand(command));
    }

    return legacyCommands;
}

/**
 * @brief The bridge between the Map/Menu scenes and the low-level rendering backend.
 */
class RenderLayer {
public:
    static RenderLayer& getInstance() {
        static RenderLayer instance;
        return instance;
    }

    /**
     * @brief Adds a command to the current frame's batch.
     */
    void submit(const std::shared_ptr<RenderCommand>& cmd) {
        if (!cmd) {
            return;
        }
        submit(toFrameRenderCommand(*cmd));
    }

    /**
     * @brief Adds a concrete legacy command to the current frame without heap reboxing.
     */
    template <typename CommandT>
    void submit(CommandT&& cmd)
        requires std::is_base_of_v<RenderCommand, std::remove_cvref_t<CommandT>>
    {
        submit(toFrameRenderCommand(cmd));
    }

    /**
     * @brief Adds a value-owned command to the current frame's batch.
     */
    void submit(FrameRenderCommand cmd) {
        m_frameCommands.push_back(std::move(cmd));
        m_legacyCacheDirty = true;
    }

    /**
     * @brief Clears the current frame's commands.
     */
    void flush() {
        m_frameCommands.clear();
        m_legacyCommands.clear();
        m_legacyCacheDirty = false;
    }

    /**
     * @brief Returns the current frame's value-owned command queue.
     */
    const std::vector<FrameRenderCommand>& getFrameCommands() const {
        return m_frameCommands;
    }

    /**
     * @brief Returns a legacy pointer-backed view for compatibility callers.
     */
    const std::vector<std::shared_ptr<RenderCommand>>& getCommands() const {
        if (m_legacyCacheDirty) {
            m_legacyCommands = toLegacyRenderCommands(m_frameCommands);
            m_legacyCacheDirty = false;
        }
        return m_legacyCommands;
    }

private:
    RenderLayer() = default;
    std::vector<FrameRenderCommand> m_frameCommands;
    mutable std::vector<std::shared_ptr<RenderCommand>> m_legacyCommands;
    mutable bool m_legacyCacheDirty = false;
};

} // namespace urpg
