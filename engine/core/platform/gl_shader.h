#pragma once

#include <string>
#include <vector>
#include <map>

namespace urpg {

/**
 * @brief Manages OpenGL Shader Programs (Vertex + Fragment).
 */
class Shader {
public:
    Shader() = default;
    ~Shader();

    /**
     * @brief Compiles and links the shader program from source.
     */
    bool compile(const char* vertexSource, const char* fragmentSource);

    /**
     * @brief Activates the shader program.
     */
    void use();

    /**
     * @brief Set uniform values.
     */
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setMat4(const std::string& name, const float* matrix);

    uint32_t getProgramId() const { return m_programId; }

private:
    uint32_t m_programId = 0;
    bool checkCompileErrors(uint32_t shader, std::string type);
};

} // namespace urpg
