#include "opengl_renderer.h"

#define GL_GLEXT_PROTOTYPES
#define STB_EASY_FONT_IMPLEMENTATION
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <stb_easy_font.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/render/asset_loader.h"

namespace urpg {

namespace {

constexpr int kImmediateVertexStride = 6;
constexpr float kEasyFontBaseHeight = 12.0f;
constexpr size_t kEasyFontVertexBufferSize = 64 * 1024;

struct ImmediateColorVertex {
    float x = 0.0f;
    float y = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

struct EasyFontVertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    unsigned char color[4] = {255, 255, 255, 255};
};

struct GlImmediateApi {
    PFNGLCREATESHADERPROC createShader = nullptr;
    PFNGLSHADERSOURCEPROC shaderSource = nullptr;
    PFNGLCOMPILESHADERPROC compileShader = nullptr;
    PFNGLGETSHADERIVPROC getShaderiv = nullptr;
    PFNGLGETSHADERINFOLOGPROC getShaderInfoLog = nullptr;
    PFNGLDELETESHADERPROC deleteShader = nullptr;
    PFNGLCREATEPROGRAMPROC createProgram = nullptr;
    PFNGLATTACHSHADERPROC attachShader = nullptr;
    PFNGLLINKPROGRAMPROC linkProgram = nullptr;
    PFNGLGETPROGRAMIVPROC getProgramiv = nullptr;
    PFNGLGETPROGRAMINFOLOGPROC getProgramInfoLog = nullptr;
    PFNGLDELETEPROGRAMPROC deleteProgram = nullptr;
    PFNGLUSEPROGRAMPROC useProgram = nullptr;
    PFNGLGENVERTEXARRAYSPROC genVertexArrays = nullptr;
    PFNGLDELETEVERTEXARRAYSPROC deleteVertexArrays = nullptr;
    PFNGLBINDVERTEXARRAYPROC bindVertexArray = nullptr;
    PFNGLGENBUFFERSPROC genBuffers = nullptr;
    PFNGLDELETEBUFFERSPROC deleteBuffers = nullptr;
    PFNGLBINDBUFFERPROC bindBuffer = nullptr;
    PFNGLBUFFERDATAPROC bufferData = nullptr;
    PFNGLENABLEVERTEXATTRIBARRAYPROC enableVertexAttribArray = nullptr;
    PFNGLVERTEXATTRIBPOINTERPROC vertexAttribPointer = nullptr;
    PFNGLGETUNIFORMLOCATIONPROC getUniformLocation = nullptr;
    PFNGLUNIFORM1IPROC uniform1i = nullptr;
    PFNGLUNIFORM2FPROC uniform2f = nullptr;
    PFNGLACTIVETEXTUREPROC activeTexture = nullptr;
};

template<typename T> bool loadGlProc(T& target, const char* name) {
    target = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
    if (target == nullptr) {
        diagnostics::RuntimeDiagnostics::error("platform.opengl", "opengl.proc_load_failed",
                                               std::string("Failed to load GL proc: ") + name);
        return false;
    }
    return true;
}

bool loadImmediateApi(GlImmediateApi& api) {
    return loadGlProc(api.createShader, "glCreateShader") && loadGlProc(api.shaderSource, "glShaderSource") &&
           loadGlProc(api.compileShader, "glCompileShader") && loadGlProc(api.getShaderiv, "glGetShaderiv") &&
           loadGlProc(api.getShaderInfoLog, "glGetShaderInfoLog") && loadGlProc(api.deleteShader, "glDeleteShader") &&
           loadGlProc(api.createProgram, "glCreateProgram") && loadGlProc(api.attachShader, "glAttachShader") &&
           loadGlProc(api.linkProgram, "glLinkProgram") && loadGlProc(api.getProgramiv, "glGetProgramiv") &&
           loadGlProc(api.getProgramInfoLog, "glGetProgramInfoLog") &&
           loadGlProc(api.deleteProgram, "glDeleteProgram") && loadGlProc(api.useProgram, "glUseProgram") &&
           loadGlProc(api.genVertexArrays, "glGenVertexArrays") &&
           loadGlProc(api.deleteVertexArrays, "glDeleteVertexArrays") &&
           loadGlProc(api.bindVertexArray, "glBindVertexArray") && loadGlProc(api.genBuffers, "glGenBuffers") &&
           loadGlProc(api.deleteBuffers, "glDeleteBuffers") && loadGlProc(api.bindBuffer, "glBindBuffer") &&
           loadGlProc(api.bufferData, "glBufferData") &&
           loadGlProc(api.enableVertexAttribArray, "glEnableVertexAttribArray") &&
           loadGlProc(api.vertexAttribPointer, "glVertexAttribPointer") &&
           loadGlProc(api.getUniformLocation, "glGetUniformLocation") && loadGlProc(api.uniform1i, "glUniform1i") &&
           loadGlProc(api.uniform2f, "glUniform2f") && loadGlProc(api.activeTexture, "glActiveTexture");
}

GlImmediateApi g_gl;

float pixelToNdcX(float x, int viewportWidth) {
    return ((x / static_cast<float>(std::max(viewportWidth, 1))) * 2.0f) - 1.0f;
}

float pixelToNdcY(float y, int viewportHeight) {
    return 1.0f - ((y / static_cast<float>(std::max(viewportHeight, 1))) * 2.0f);
}

ImmediateColorVertex makeVertex(float x, float y, int viewportWidth, int viewportHeight, float r, float g, float b,
                                float a) {
    return {
        pixelToNdcX(x, viewportWidth), pixelToNdcY(y, viewportHeight), r, g, b, a,
    };
}

void appendQuad(std::vector<ImmediateColorVertex>& vertices, float x, float y, float w, float h, int viewportWidth,
                int viewportHeight, float r, float g, float b, float a) {
    const auto topLeft = makeVertex(x, y, viewportWidth, viewportHeight, r, g, b, a);
    const auto topRight = makeVertex(x + w, y, viewportWidth, viewportHeight, r, g, b, a);
    const auto bottomLeft = makeVertex(x, y + h, viewportWidth, viewportHeight, r, g, b, a);
    const auto bottomRight = makeVertex(x + w, y + h, viewportWidth, viewportHeight, r, g, b, a);

    vertices.insert(vertices.end(), {topLeft, bottomLeft, topRight, topRight, bottomLeft, bottomRight});
}

std::vector<float> flattenVertices(const std::vector<ImmediateColorVertex>& vertices) {
    std::vector<float> flattened;
    flattened.reserve(vertices.size() * kImmediateVertexStride);

    for (const auto& vertex : vertices) {
        flattened.push_back(vertex.x);
        flattened.push_back(vertex.y);
        flattened.push_back(vertex.r);
        flattened.push_back(vertex.g);
        flattened.push_back(vertex.b);
        flattened.push_back(vertex.a);
    }

    return flattened;
}

std::vector<float> buildRectBatch(const RectCommand& command, int viewportWidth, int viewportHeight) {
    std::vector<ImmediateColorVertex> vertices;
    vertices.reserve(6);
    appendQuad(vertices, command.x, command.y, command.w, command.h, viewportWidth, viewportHeight, command.r,
               command.g, command.b, command.a);
    return flattenVertices(vertices);
}

uint32_t stableHash(std::string_view value) {
    uint32_t hash = 2166136261u;
    for (const unsigned char ch : value) {
        hash ^= ch;
        hash *= 16777619u;
    }
    return hash;
}

std::array<float, 4> colorFromHash(uint32_t hash, float alpha = 1.0f) {
    const float r = 0.2f + (static_cast<float>((hash >> 16) & 0xFFu) / 255.0f) * 0.6f;
    const float g = 0.2f + (static_cast<float>((hash >> 8) & 0xFFu) / 255.0f) * 0.6f;
    const float b = 0.2f + (static_cast<float>(hash & 0xFFu) / 255.0f) * 0.6f;
    return {r, g, b, alpha};
}

std::vector<float> buildPlaceholderQuadBatch(float x, float y, float w, float h, int viewportWidth, int viewportHeight,
                                             const std::array<float, 4>& fillColor,
                                             const std::array<float, 4>& accentColor) {
    std::vector<ImmediateColorVertex> vertices;
    vertices.reserve(18);

    appendQuad(vertices, x, y, w, h, viewportWidth, viewportHeight, fillColor[0], fillColor[1], fillColor[2],
               fillColor[3]);

    const float accentInset = std::max(2.0f, std::min(w, h) * 0.18f);
    appendQuad(vertices, x + accentInset, y + accentInset, std::max(1.0f, w - accentInset * 2.0f),
               std::max(1.0f, h - accentInset * 2.0f), viewportWidth, viewportHeight, accentColor[0], accentColor[1],
               accentColor[2], accentColor[3]);

    const float stripeWidth = std::max(2.0f, std::min(w, h) * 0.12f);
    appendQuad(vertices, x, y, stripeWidth, h, viewportWidth, viewportHeight, accentColor[0], accentColor[1],
               accentColor[2], std::min(fillColor[3] + 0.15f, 1.0f));

    return flattenVertices(vertices);
}

float normalizeSpriteOpacity(float opacity) {
    if (opacity <= 0.0f) {
        return 0.0f;
    }

    if (opacity <= 1.0f) {
        return std::clamp(opacity, 0.0f, 1.0f);
    }

    return std::clamp(opacity / 255.0f, 0.0f, 1.0f);
}

SpriteDrawData buildSpriteCommandBatch(const SpriteCommand& command, const GLTexture& texture) {
    const auto makeSpriteVertex = [](float x, float y, float z, float u, float v, float alpha) {
        SpriteVertex vertex{};
        vertex.position[0] = x;
        vertex.position[1] = y;
        vertex.position[2] = z;
        vertex.uv[0] = u;
        vertex.uv[1] = v;
        vertex.color[0] = 1.0f;
        vertex.color[1] = 1.0f;
        vertex.color[2] = 1.0f;
        vertex.color[3] = alpha;
        return vertex;
    };

    const float textureWidth = static_cast<float>(std::max(texture.width, 1));
    const float textureHeight = static_cast<float>(std::max(texture.height, 1));
    const float width = static_cast<float>(std::max(command.width, 1));
    const float height = static_cast<float>(std::max(command.height, 1));
    const float srcX = static_cast<float>(std::clamp(command.srcX, 0, std::max(texture.width - 1, 0)));
    const float srcY = static_cast<float>(std::clamp(command.srcY, 0, std::max(texture.height - 1, 0)));
    const float srcWidth = std::clamp(width, 1.0f, textureWidth - srcX);
    const float srcHeight = std::clamp(height, 1.0f, textureHeight - srcY);
    const float u1 = srcX / textureWidth;
    const float v1 = srcY / textureHeight;
    const float u2 = (srcX + srcWidth) / textureWidth;
    const float v2 = (srcY + srcHeight) / textureHeight;
    const float alpha = normalizeSpriteOpacity(command.opacity);
    const float z = static_cast<float>(command.zOrder);

    SpriteDrawData batch;
    batch.textureId = texture.handle;
    batch.vertices = {
        makeSpriteVertex(command.x, command.y, z, u1, v1, alpha),
        makeSpriteVertex(command.x, command.y + height, z, u1, v2, alpha),
        makeSpriteVertex(command.x + width, command.y, z, u2, v1, alpha),
        makeSpriteVertex(command.x + width, command.y, z, u2, v1, alpha),
        makeSpriteVertex(command.x, command.y + height, z, u1, v2, alpha),
        makeSpriteVertex(command.x + width, command.y + height, z, u2, v2, alpha),
    };
    return batch;
}

SpriteDrawData buildTileCommandBatch(const TileCommand& command, const GLTexture& texture) {
    constexpr float kTileSize = 48.0f;
    const int32_t tilesetWidthInTiles = std::max(texture.width / static_cast<int32_t>(kTileSize), 1);
    const int32_t safeTileIndex = std::max(command.tileIndex, 0);
    const float srcX = static_cast<float>((safeTileIndex % tilesetWidthInTiles) * static_cast<int32_t>(kTileSize));
    const float srcY = static_cast<float>((safeTileIndex / tilesetWidthInTiles) * static_cast<int32_t>(kTileSize));
    const float textureWidth = static_cast<float>(std::max(texture.width, 1));
    const float textureHeight = static_cast<float>(std::max(texture.height, 1));
    const float clampedSrcX = std::clamp(srcX, 0.0f, std::max(textureWidth - kTileSize, 0.0f));
    const float clampedSrcY = std::clamp(srcY, 0.0f, std::max(textureHeight - kTileSize, 0.0f));
    const float u1 = clampedSrcX / textureWidth;
    const float v1 = clampedSrcY / textureHeight;
    const float u2 = (clampedSrcX + kTileSize) / textureWidth;
    const float v2 = (clampedSrcY + kTileSize) / textureHeight;
    const float z = static_cast<float>(command.zOrder);

    const auto makeSpriteVertex = [](float x, float y, float zValue, float u, float v) {
        SpriteVertex vertex{};
        vertex.position[0] = x;
        vertex.position[1] = y;
        vertex.position[2] = zValue;
        vertex.uv[0] = u;
        vertex.uv[1] = v;
        vertex.color[0] = 1.0f;
        vertex.color[1] = 1.0f;
        vertex.color[2] = 1.0f;
        vertex.color[3] = 1.0f;
        return vertex;
    };

    SpriteDrawData batch;
    batch.textureId = texture.handle;
    batch.vertices = {
        makeSpriteVertex(command.x, command.y, z, u1, v1),
        makeSpriteVertex(command.x, command.y + kTileSize, z, u1, v2),
        makeSpriteVertex(command.x + kTileSize, command.y, z, u2, v1),
        makeSpriteVertex(command.x + kTileSize, command.y, z, u2, v1),
        makeSpriteVertex(command.x, command.y + kTileSize, z, u1, v2),
        makeSpriteVertex(command.x + kTileSize, command.y + kTileSize, z, u2, v2),
    };
    return batch;
}

char32_t decodeUtf8Codepoint(const std::string& text, size_t& cursor) {
    if (cursor >= text.size()) {
        return U'\0';
    }

    const unsigned char lead = static_cast<unsigned char>(text[cursor]);
    if (lead < 0x80) {
        ++cursor;
        return static_cast<char32_t>(lead);
    }

    if ((lead >> 5) == 0x6 && cursor + 1 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        if ((b1 & 0xC0) == 0x80) {
            cursor += 2;
            return static_cast<char32_t>(((lead & 0x1F) << 6) | (b1 & 0x3F));
        }
    } else if ((lead >> 4) == 0xE && cursor + 2 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        const unsigned char b2 = static_cast<unsigned char>(text[cursor + 2]);
        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80) {
            cursor += 3;
            return static_cast<char32_t>(((lead & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F));
        }
    } else if ((lead >> 3) == 0x1E && cursor + 3 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        const unsigned char b2 = static_cast<unsigned char>(text[cursor + 2]);
        const unsigned char b3 = static_cast<unsigned char>(text[cursor + 3]);
        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80 && (b3 & 0xC0) == 0x80) {
            cursor += 4;
            return static_cast<char32_t>(((lead & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) |
                                         (b3 & 0x3F));
        }
    }

    ++cursor;
    return static_cast<char32_t>(lead);
}

int32_t rendererGlyphAdvance(char32_t cp, int32_t fontSize) {
    const double size = static_cast<double>(std::max(1, fontSize));
    if (cp == U'\t') {
        return static_cast<int32_t>(std::lround(size * 2.2));
    }
    if (cp == U' ') {
        return static_cast<int32_t>(std::lround(size * 0.35));
    }

    if (cp < 0x80) {
        const unsigned char ch = static_cast<unsigned char>(cp);
        if (std::isdigit(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.56));
        }
        if (std::ispunct(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.45));
        }
        if (std::isupper(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.62));
        }
        return static_cast<int32_t>(std::lround(size * 0.56));
    }

    if ((cp >= 0x2E80 && cp <= 0x9FFF) || (cp >= 0xF900 && cp <= 0xFAFF)) {
        return static_cast<int32_t>(std::lround(size));
    }
    if (cp >= 0x1F000) {
        return static_cast<int32_t>(std::lround(size * 1.1));
    }
    return static_cast<int32_t>(std::lround(size * 0.75));
}

std::string wrapTextForRenderer(const TextCommand& command) {
    if (command.maxWidth <= 0 || command.text.empty()) {
        return command.text;
    }

    std::string wrapped;
    wrapped.reserve(command.text.size() + 8);

    const int32_t maxWidth = std::max(command.maxWidth, 1);
    int32_t lineWidth = 0;
    size_t cursor = 0;

    while (cursor < command.text.size()) {
        const size_t cpStart = cursor;
        const char32_t cp = decodeUtf8Codepoint(command.text, cursor);

        if (cp == U'\0') {
            break;
        }

        if (cp == U'\r') {
            continue;
        }

        if (cp == U'\n') {
            wrapped.push_back('\n');
            lineWidth = 0;
            continue;
        }

        const int32_t advance = rendererGlyphAdvance(cp, command.fontSize);
        if (lineWidth > 0 && lineWidth + advance > maxWidth) {
            wrapped.push_back('\n');
            lineWidth = 0;
        }

        wrapped.append(command.text, cpStart, cursor - cpStart);
        lineWidth += advance;
    }

    return wrapped;
}

std::vector<float> buildTextBatch(const TextCommand& command, int viewportWidth, int viewportHeight) {
    const std::string wrappedText = wrapTextForRenderer(command);
    if (wrappedText.empty()) {
        return {};
    }

    std::array<char, kEasyFontVertexBufferSize> quadBuffer{};
    const int quadCount = stb_easy_font_print(0.0f, 0.0f, const_cast<char*>(wrappedText.c_str()), nullptr,
                                              quadBuffer.data(), static_cast<int>(quadBuffer.size()));

    if (quadCount <= 0) {
        return {};
    }

    const auto* easyVertices = reinterpret_cast<const EasyFontVertex*>(quadBuffer.data());
    const float scale = static_cast<float>(std::max(command.fontSize, 1)) / kEasyFontBaseHeight;
    const float r = static_cast<float>(command.r) / 255.0f;
    const float g = static_cast<float>(command.g) / 255.0f;
    const float b = static_cast<float>(command.b) / 255.0f;
    const float a = static_cast<float>(command.a) / 255.0f;

    std::vector<ImmediateColorVertex> vertices;
    vertices.reserve(static_cast<size_t>(quadCount) * 6);

    for (int quadIndex = 0; quadIndex < quadCount; ++quadIndex) {
        const auto* quad = easyVertices + (quadIndex * 4);
        const auto convert = [&](const EasyFontVertex& vertex) {
            return makeVertex(command.x + (vertex.x * scale), command.y + (vertex.y * scale), viewportWidth,
                              viewportHeight, r, g, b, a);
        };

        const auto topLeft = convert(quad[0]);
        const auto topRight = convert(quad[1]);
        const auto bottomRight = convert(quad[2]);
        const auto bottomLeft = convert(quad[3]);

        vertices.insert(vertices.end(), {topLeft, bottomLeft, topRight, topRight, bottomLeft, bottomRight});
    }

    return flattenVertices(vertices);
}

uint32_t compileShader(GLenum type, const char* source) {
    const uint32_t shader = g_gl.createShader(type);
    g_gl.shaderSource(shader, 1, &source, nullptr);
    g_gl.compileShader(shader);

    GLint success = 0;
    g_gl.getShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        return shader;
    }

    char infoLog[1024] = {};
    g_gl.getShaderInfoLog(shader, static_cast<GLsizei>(sizeof(infoLog)), nullptr, infoLog);
    diagnostics::RuntimeDiagnostics::error("platform.opengl", "opengl.shader_compile_failed",
                                           std::string("Shader compile failed: ") + infoLog);
    g_gl.deleteShader(shader);
    return 0;
}

} // namespace

const char* IMMEDIATE_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

out vec4 VertexColor;

void main() {
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
    VertexColor = aColor;
}
)";

const char* IMMEDIATE_FRAGMENT_SHADER = R"(
#version 330 core
in vec4 VertexColor;
out vec4 FragColor;

void main() {
    FragColor = VertexColor;
}
)";

const char* TEXTURED_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUv;
layout (location = 2) in vec4 aColor;

uniform vec2 uViewportSize;

out vec2 VertexUv;
out vec4 VertexColor;

void main() {
    vec2 ndc;
    ndc.x = (aPos.x / max(uViewportSize.x, 1.0)) * 2.0 - 1.0;
    ndc.y = 1.0 - ((aPos.y / max(uViewportSize.y, 1.0)) * 2.0);
    gl_Position = vec4(ndc.xy, 0.0, 1.0);
    VertexUv = aUv;
    VertexColor = aColor;
}
)";

const char* TEXTURED_FRAGMENT_SHADER = R"(
#version 330 core
in vec2 VertexUv;
in vec4 VertexColor;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = texture(uTexture, VertexUv) * VertexColor;
}
)";

bool OpenGLRenderer::initialize(IPlatformSurface* surface) {
    m_surface = surface;
    diagnostics::RuntimeDiagnostics::info("platform.opengl", "opengl.backend_initializing",
                                          "Initializing OpenGL 3.3 Backend (TIER_BASIC)");

    if (!loadImmediateApi(g_gl)) {
        return false;
    }

    setupDefaultShaders();
    setupDrawBuffers();

    if (m_immediatePipelineReady) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    return m_immediatePipelineReady || m_texturedPipelineReady;
}

void OpenGLRenderer::beginFrame() {
    if (!m_immediatePipelineReady) {
        return;
    }

    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::renderBatches(const std::vector<SpriteDrawData>& batches) {
    for (const auto& batch : batches) {
        submitTexturedBatch(batch);
    }
}

void OpenGLRenderer::endFrame() {
    if (m_surface != nullptr) {
        m_surface->present();
    }
}

void OpenGLRenderer::shutdown() {
    if (m_texturedVbo != 0) {
        g_gl.deleteBuffers(1, &m_texturedVbo);
        m_texturedVbo = 0;
    }
    if (m_texturedVao != 0) {
        g_gl.deleteVertexArrays(1, &m_texturedVao);
        m_texturedVao = 0;
    }
    if (m_texturedShaderProgram != 0) {
        g_gl.deleteProgram(m_texturedShaderProgram);
        m_texturedShaderProgram = 0;
    }
    if (m_vbo != 0) {
        g_gl.deleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        g_gl.deleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_shaderProgram != 0) {
        g_gl.deleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }

    m_immediatePipelineReady = false;
    m_texturedPipelineReady = false;
}

void OpenGLRenderer::onResize(int width, int height) {
    m_viewportWidth = std::max(width, 1);
    m_viewportHeight = std::max(height, 1);

    if (m_immediatePipelineReady) {
        glViewport(0, 0, m_viewportWidth, m_viewportHeight);
    }
}

void OpenGLRenderer::setupDefaultShaders() {
    const uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, IMMEDIATE_VERTEX_SHADER);
    if (vertexShader == 0) {
        return;
    }

    const uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, IMMEDIATE_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        g_gl.deleteShader(vertexShader);
        return;
    }

    m_shaderProgram = g_gl.createProgram();
    g_gl.attachShader(m_shaderProgram, vertexShader);
    g_gl.attachShader(m_shaderProgram, fragmentShader);
    g_gl.linkProgram(m_shaderProgram);

    GLint success = 0;
    g_gl.getProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        char infoLog[1024] = {};
        g_gl.getProgramInfoLog(m_shaderProgram, static_cast<GLsizei>(sizeof(infoLog)), nullptr, infoLog);
        diagnostics::RuntimeDiagnostics::error("platform.opengl", "opengl.program_link_failed",
                                               std::string("Program link failed: ") + infoLog);
        g_gl.deleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }

    g_gl.deleteShader(vertexShader);
    g_gl.deleteShader(fragmentShader);

    const uint32_t texturedVertexShader = compileShader(GL_VERTEX_SHADER, TEXTURED_VERTEX_SHADER);
    if (texturedVertexShader == 0) {
        return;
    }

    const uint32_t texturedFragmentShader = compileShader(GL_FRAGMENT_SHADER, TEXTURED_FRAGMENT_SHADER);
    if (texturedFragmentShader == 0) {
        g_gl.deleteShader(texturedVertexShader);
        return;
    }

    m_texturedShaderProgram = g_gl.createProgram();
    g_gl.attachShader(m_texturedShaderProgram, texturedVertexShader);
    g_gl.attachShader(m_texturedShaderProgram, texturedFragmentShader);
    g_gl.linkProgram(m_texturedShaderProgram);

    success = 0;
    g_gl.getProgramiv(m_texturedShaderProgram, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        char infoLog[1024] = {};
        g_gl.getProgramInfoLog(m_texturedShaderProgram, static_cast<GLsizei>(sizeof(infoLog)), nullptr, infoLog);
        diagnostics::RuntimeDiagnostics::error("platform.opengl", "opengl.textured_program_link_failed",
                                               std::string("Textured program link failed: ") + infoLog);
        g_gl.deleteProgram(m_texturedShaderProgram);
        m_texturedShaderProgram = 0;
    }

    g_gl.deleteShader(texturedVertexShader);
    g_gl.deleteShader(texturedFragmentShader);
}

void OpenGLRenderer::setupDrawBuffers() {
    if (m_shaderProgram == 0) {
        return;
    }

    g_gl.genVertexArrays(1, &m_vao);
    g_gl.genBuffers(1, &m_vbo);

    g_gl.bindVertexArray(m_vao);
    g_gl.bindBuffer(GL_ARRAY_BUFFER, m_vbo);
    g_gl.bufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    g_gl.enableVertexAttribArray(0);
    g_gl.vertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImmediateColorVertex),
                             reinterpret_cast<void*>(offsetof(ImmediateColorVertex, x)));

    g_gl.enableVertexAttribArray(1);
    g_gl.vertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ImmediateColorVertex),
                             reinterpret_cast<void*>(offsetof(ImmediateColorVertex, r)));

    g_gl.bindBuffer(GL_ARRAY_BUFFER, 0);
    g_gl.bindVertexArray(0);

    m_immediatePipelineReady = (m_vao != 0 && m_vbo != 0);

    if (m_texturedShaderProgram == 0) {
        return;
    }

    g_gl.genVertexArrays(1, &m_texturedVao);
    g_gl.genBuffers(1, &m_texturedVbo);

    if (m_texturedVao == 0 || m_texturedVbo == 0) {
        return;
    }

    g_gl.bindVertexArray(m_texturedVao);
    g_gl.bindBuffer(GL_ARRAY_BUFFER, m_texturedVbo);
    g_gl.bufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    g_gl.enableVertexAttribArray(0);
    g_gl.vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                             reinterpret_cast<void*>(offsetof(SpriteVertex, position)));

    g_gl.enableVertexAttribArray(1);
    g_gl.vertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                             reinterpret_cast<void*>(offsetof(SpriteVertex, uv)));

    g_gl.enableVertexAttribArray(2);
    g_gl.vertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                             reinterpret_cast<void*>(offsetof(SpriteVertex, color)));

    g_gl.bindBuffer(GL_ARRAY_BUFFER, 0);
    g_gl.bindVertexArray(0);

    m_texturedPipelineReady = (m_texturedVao != 0 && m_texturedVbo != 0);
}

void OpenGLRenderer::processFrameCommands(const std::vector<FrameRenderCommand>& commands) {
    for (const auto& cmd : commands) {
        processFrameCommand(cmd);
    }
}

void OpenGLRenderer::processCommands(const std::vector<std::shared_ptr<RenderCommand>>& commands) {
    for (const auto& cmd : commands) {
        if (cmd != nullptr) {
            processCommand(*cmd);
        }
    }
}

void OpenGLRenderer::processFrameCommand(const FrameRenderCommand& command) {
    switch (command.type) {
    case RenderCmdType::Sprite: {
        const auto* spriteData = command.tryGet<SpriteRenderData>();
        if (spriteData == nullptr) {
            break;
        }

        SpriteCommand legacy;
        legacy.zOrder = command.zOrder;
        legacy.x = command.x;
        legacy.y = command.y;
        legacy.textureId = spriteData->textureId;
        legacy.srcX = spriteData->srcX;
        legacy.srcY = spriteData->srcY;
        legacy.width = spriteData->width;
        legacy.height = spriteData->height;
        legacy.opacity = spriteData->opacity;
        drawSpriteCommand(legacy);
        break;
    }
    case RenderCmdType::Tile: {
        const auto* tileData = command.tryGet<TileRenderData>();
        if (tileData == nullptr) {
            break;
        }

        TileCommand legacy;
        legacy.zOrder = command.zOrder;
        legacy.x = command.x;
        legacy.y = command.y;
        legacy.tilesetId = tileData->tilesetId;
        legacy.tileIndex = tileData->tileIndex;
        drawTileCommand(legacy);
        break;
    }
    case RenderCmdType::Text: {
        const auto* textData = command.tryGet<TextRenderData>();
        if (textData == nullptr) {
            break;
        }

        TextCommand legacy;
        legacy.zOrder = command.zOrder;
        legacy.x = command.x;
        legacy.y = command.y;
        legacy.text = textData->text;
        legacy.fontFace = textData->fontFace;
        legacy.fontSize = textData->fontSize;
        legacy.maxWidth = textData->maxWidth;
        legacy.r = textData->r;
        legacy.g = textData->g;
        legacy.b = textData->b;
        legacy.a = textData->a;
        drawTextCommand(legacy);
        break;
    }
    case RenderCmdType::Rect: {
        const auto* rectData = command.tryGet<RectRenderData>();
        if (rectData == nullptr) {
            break;
        }

        RectCommand legacy;
        legacy.zOrder = command.zOrder;
        legacy.x = command.x;
        legacy.y = command.y;
        legacy.w = rectData->w;
        legacy.h = rectData->h;
        legacy.r = rectData->r;
        legacy.g = rectData->g;
        legacy.b = rectData->b;
        legacy.a = rectData->a;
        drawRectCommand(legacy);
        break;
    }
    case RenderCmdType::Clear:
        break;
    default:
        break;
    }
}

void OpenGLRenderer::processCommand(const RenderCommand& command) {
    switch (command.type) {
    case RenderCmdType::Sprite:
        drawSpriteCommand(static_cast<const SpriteCommand&>(command));
        break;
    case RenderCmdType::Tile:
        drawTileCommand(static_cast<const TileCommand&>(command));
        break;
    case RenderCmdType::Text:
        drawTextCommand(static_cast<const TextCommand&>(command));
        break;
    case RenderCmdType::Rect:
        drawRectCommand(static_cast<const RectCommand&>(command));
        break;
    case RenderCmdType::Clear:
        break;
    default:
        break;
    }
}

void OpenGLRenderer::submitImmediateBatch(const std::vector<float>& vertices) const {
    if (!m_immediatePipelineReady || vertices.empty()) {
        return;
    }

    g_gl.useProgram(m_shaderProgram);
    g_gl.bindVertexArray(m_vao);
    g_gl.bindBuffer(GL_ARRAY_BUFFER, m_vbo);
    g_gl.bufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(),
                    GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / kImmediateVertexStride));
    g_gl.bindBuffer(GL_ARRAY_BUFFER, 0);
    g_gl.bindVertexArray(0);
}

void OpenGLRenderer::submitTexturedBatch(const SpriteDrawData& batch) const {
    if (batch.vertices.empty()) {
        return;
    }

    // Texture ID 1 is a valid first in-memory texture in fresh snapshot contexts.
    const bool hasBoundTexture = batch.textureId != 0 && glIsTexture(batch.textureId) == GL_TRUE;
    if (!hasBoundTexture) {
        std::vector<ImmediateColorVertex> vertices;
        vertices.reserve(batch.vertices.size());

        for (const auto& vertex : batch.vertices) {
            vertices.push_back(makeVertex(vertex.position[0], vertex.position[1], m_viewportWidth, m_viewportHeight,
                                          vertex.color[0], vertex.color[1], vertex.color[2], vertex.color[3]));
        }

        submitImmediateBatch(flattenVertices(vertices));
        return;
    }

    if (!m_texturedPipelineReady || m_texturedShaderProgram == 0) {
        return;
    }

    g_gl.useProgram(m_texturedShaderProgram);
    g_gl.bindVertexArray(m_texturedVao);
    g_gl.bindBuffer(GL_ARRAY_BUFFER, m_texturedVbo);
    g_gl.bufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(batch.vertices.size() * sizeof(SpriteVertex)),
                    batch.vertices.data(), GL_DYNAMIC_DRAW);

    const GLint viewportUniform = g_gl.getUniformLocation(m_texturedShaderProgram, "uViewportSize");
    if (viewportUniform >= 0) {
        g_gl.uniform2f(viewportUniform, static_cast<float>(std::max(m_viewportWidth, 1)),
                       static_cast<float>(std::max(m_viewportHeight, 1)));
    }

    const GLint textureUniform = g_gl.getUniformLocation(m_texturedShaderProgram, "uTexture");
    if (textureUniform >= 0) {
        g_gl.uniform1i(textureUniform, 0);
    }

    g_gl.activeTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, batch.textureId);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(batch.vertices.size()));
    glBindTexture(GL_TEXTURE_2D, 0);

    g_gl.bindBuffer(GL_ARRAY_BUFFER, 0);
    g_gl.bindVertexArray(0);
}

void OpenGLRenderer::drawSpriteCommand(const SpriteCommand& command) {
    if (const auto texture = resolveTextureHandle(command.textureId)) {
        submitTexturedBatch(buildSpriteCommandBatch(command, *texture));
        return;
    }

    if (m_runtimeAssetMode == RuntimeAssetMode::Release) {
        emitUnresolvedTextureDiagnostic("opengl.sprite_texture_unresolved_release", "sprite", command.textureId);
        return;
    }

    const float width = static_cast<float>(std::max(command.width, 1));
    const float height = static_cast<float>(std::max(command.height, 1));
    const uint32_t hash = stableHash(command.textureId);
    auto fillColor = colorFromHash(hash, std::max(normalizeSpriteOpacity(command.opacity), 0.15f));
    auto accentColor = colorFromHash(hash ^ 0x00A5A5A5u, std::min(fillColor[3] + 0.1f, 1.0f));

    // Bounded truthfulness: unresolved sprite IDs still fall back to deterministic
    // placeholder quads derived from texture identity.
    submitImmediateBatch(buildPlaceholderQuadBatch(command.x, command.y, width, height, m_viewportWidth,
                                                   m_viewportHeight, fillColor, accentColor));
}

void OpenGLRenderer::drawTileCommand(const TileCommand& command) {
    if (const auto texture = resolveTextureHandle(command.tilesetId)) {
        submitTexturedBatch(buildTileCommandBatch(command, *texture));
        return;
    }

    if (m_runtimeAssetMode == RuntimeAssetMode::Release) {
        emitUnresolvedTextureDiagnostic("opengl.tileset_texture_unresolved_release", "tileset", command.tilesetId);
        return;
    }

    constexpr float kTileSize = 48.0f;
    const uint32_t hash = stableHash(command.tilesetId) ^ (static_cast<uint32_t>(command.tileIndex) * 2654435761u);
    auto fillColor = colorFromHash(hash, 1.0f);
    auto accentColor = colorFromHash(hash ^ 0x005A5A5Au, 1.0f);

    // Bounded truthfulness: unresolved tile IDs still render as deterministic
    // placeholder quads derived from tileset identity and tile index.
    submitImmediateBatch(buildPlaceholderQuadBatch(command.x, command.y, kTileSize, kTileSize, m_viewportWidth,
                                                   m_viewportHeight, fillColor, accentColor));
}

void OpenGLRenderer::drawTextCommand(const TextCommand& command) {
    submitImmediateBatch(buildTextBatch(command, m_viewportWidth, m_viewportHeight));
}

void OpenGLRenderer::drawRectCommand(const RectCommand& command) {
    submitImmediateBatch(buildRectBatch(command, m_viewportWidth, m_viewportHeight));
}

std::shared_ptr<GLTexture> OpenGLRenderer::resolveTextureHandle(const std::string& id) {
    if (id.empty()) {
        return nullptr;
    }

    if (auto it = m_textures.find(id); it != m_textures.end()) {
        if (it->second != nullptr && it->second->handle != 0 && glIsTexture(it->second->handle) == GL_TRUE) {
            return it->second;
        }
        m_textures.erase(it);
    }

    auto meta = TextureRegistry::getInstance().getTexture(id);
    if (meta == nullptr || meta->filePath.empty()) {
        return nullptr;
    }

    if (!loadTexture(id, meta->filePath)) {
        return nullptr;
    }

    auto loaded = m_textures.find(id);
    return loaded != m_textures.end() ? loaded->second : nullptr;
}

void OpenGLRenderer::emitUnresolvedTextureDiagnostic(const char* code, const std::string& role,
                                                     const std::string& id) const {
    const std::string resolvedId = id.empty() ? "<empty>" : id;
    diagnostics::RuntimeDiagnostics::error("platform.opengl", code,
                                           "Release renderer could not resolve " + role + " texture '" + resolvedId +
                                               "'; deterministic placeholder rendering is disabled.");
}

bool OpenGLRenderer::loadTexture(const std::string& id, const std::string& filePath) {
    if (id.empty() || filePath.empty()) {
        return false;
    }

    auto texture = AssetLoader::loadTexture(filePath);
    return registerTextureHandle(id, texture);
}

bool OpenGLRenderer::registerTextureHandle(const std::string& id, const std::shared_ptr<Texture>& texture) {
    if (id.empty() || texture == nullptr || texture->getId() == 0) {
        return false;
    }

    auto handle = std::make_shared<GLTexture>();
    handle->handle = texture->getId();
    handle->width = texture->getWidth();
    handle->height = texture->getHeight();
    handle->owner = texture;
    m_textures[id] = std::move(handle);
    return true;
}

} // namespace urpg
