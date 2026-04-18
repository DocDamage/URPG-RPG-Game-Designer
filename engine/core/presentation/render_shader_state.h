#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::render {

/**
 * @brief Shader attribute types for the input assembler.
 */
enum class ShaderAttributeType {
    Position,
    Normal,
    TexCoord,
    Color,
    Tangent,
    InstanceData
};

/**
 * @brief Blending modes for the pipeline state.
 */
enum class BlendMode {
    Opaque,
    AlphaBlend,
    Additive,
    Multiply
};

/**
 * @brief Depth test configurations.
 */
enum class DepthTest {
    None,
    Read,
    Write,
    ReadWrite
};

/**
 * @brief Complete Shader Pipeline State (ADR-007).
 * This represents the immutable state required to bind a shader to the hardware.
 */
struct ShaderPipelineState {
    std::string shaderId;
    BlendMode blendMode = BlendMode::Opaque;
    DepthTest depthTest = DepthTest::ReadWrite;
    bool cullBackface = true;
    bool wireframe = false;
};

/**
 * @brief Uniform/Constant buffer slot definition.
 */
struct ShaderUniform {
    std::string name;
    uint32_t size;
    uint32_t binding;
};

/**
 * @brief A shader program definition including pass requirements.
 */
struct ShaderProgram {
    std::string vS; // Vertex Shader Code/Path
    std::string pS; // Pixel/Fragment Shader Code/Path
    std::vector<ShaderAttributeType> attributes;
    std::vector<ShaderUniform> uniforms;
};

/**
 * @brief Manages the lifecycle of shader states and binding (ADR-007).
 */
class ShaderPassManager {
public:
    /**
     * @brief Computes a unique hash for the pipeline state to minimize state changes.
     */
    static uint64_t ComputeStateHash(const ShaderPipelineState& state) {
        // Simplified hash for demonstration
        uint64_t hash = 0;
        hash |= static_cast<uint64_t>(state.blendMode) << 0;
        hash |= static_cast<uint64_t>(state.depthTest) << 4;
        hash |= (state.cullBackface ? 1ULL : 0ULL) << 8;
        return hash;
    }

    /**
     * @brief Resolves which shader to use for a specific render intent and tier.
     */
    static std::string ResolveShaderForTier(const std::string& baseShader, uint32_t tier) {
        if (tier == 0) return baseShader + "_LOD_LOW"; // Heritage path
        if (tier >= 2) return baseShader + "_PBR_ADVANCED"; // High-end path
        return baseShader + "_LIT"; // Standard path
    }
};

} // namespace urpg::render
