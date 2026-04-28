#pragma once

#include "renderer_backend.h"
#include <cstddef>
#include <iostream>

namespace urpg {

struct HeadlessRenderFrameSummary {
    size_t frameIndex = 0;
    size_t batchCount = 0;
    size_t batchVertexCount = 0;
    size_t commandCount = 0;
    size_t clearCommandCount = 0;
    size_t spriteCommandCount = 0;
    size_t tileCommandCount = 0;
    size_t textCommandCount = 0;
    size_t rectCommandCount = 0;
};

/**
 * @brief Soft renderer mock for CI/Headless environments.
 * Records shell-owned render batches and frame commands so non-OpenGL paths can
 * prove they consumed the same runtime scene output as hardware backends.
 */
class HeadlessRenderer : public RendererBackend {
public:
    RenderTier getTier() const override {
        return RenderTier::Basic;
    }

    bool initialize(IPlatformSurface* /*surface*/) override {
        m_initialized = true;
        m_lastFrame = {};
        m_frameHistory.clear();
        return true;
    }

    void beginFrame() override {
        m_frameOpen = true;
        m_lastFrame = {};
        m_lastFrame.frameIndex = ++m_frameIndex;
    }

    void endFrame() override {
        if (m_frameOpen) {
            m_frameHistory.push_back(m_lastFrame);
            m_frameOpen = false;
        }
    }

    void renderBatches(const std::vector<SpriteDrawData>& batches) override {
        m_lastFrame.batchCount = batches.size();
        m_lastFrame.batchVertexCount = 0;
        for (const auto& batch : batches) {
            m_lastFrame.batchVertexCount += batch.vertices.size();
        }
    }

    void processFrameCommands(const std::vector<FrameRenderCommand>& commands) override {
        m_lastFrame.commandCount = commands.size();
        m_lastFrame.clearCommandCount = 0;
        m_lastFrame.spriteCommandCount = 0;
        m_lastFrame.tileCommandCount = 0;
        m_lastFrame.textCommandCount = 0;
        m_lastFrame.rectCommandCount = 0;
        m_lastFrameCommands = commands;
        for (const auto& command : commands) {
            switch (command.type) {
            case RenderCmdType::Clear:
                ++m_lastFrame.clearCommandCount;
                break;
            case RenderCmdType::Sprite:
                ++m_lastFrame.spriteCommandCount;
                break;
            case RenderCmdType::Tile:
                ++m_lastFrame.tileCommandCount;
                break;
            case RenderCmdType::Text:
                ++m_lastFrame.textCommandCount;
                break;
            case RenderCmdType::Rect:
                ++m_lastFrame.rectCommandCount;
                break;
            }
        }
    }

    void processCommands(const std::vector<std::shared_ptr<RenderCommand>>& commands) override {
        std::vector<FrameRenderCommand> frameCommands;
        frameCommands.reserve(commands.size());
        for (const auto& command : commands) {
            if (command) {
                frameCommands.push_back(toFrameRenderCommand(*command));
            }
        }
        processFrameCommands(frameCommands);
    }

    void shutdown() override {
        m_initialized = false;
        m_frameOpen = false;
    }

    void onResize(int /*width*/, int /*height*/) override {
        // Viewport resize is irrelevant here.
    }

    bool initialized() const { return m_initialized; }
    const HeadlessRenderFrameSummary& lastFrameSummary() const { return m_lastFrame; }
    const std::vector<HeadlessRenderFrameSummary>& frameHistory() const { return m_frameHistory; }
    const std::vector<FrameRenderCommand>& lastFrameCommands() const { return m_lastFrameCommands; }

private:
    bool m_initialized = false;
    bool m_frameOpen = false;
    size_t m_frameIndex = 0;
    HeadlessRenderFrameSummary m_lastFrame;
    std::vector<HeadlessRenderFrameSummary> m_frameHistory;
    std::vector<FrameRenderCommand> m_lastFrameCommands;
};

} // namespace urpg
