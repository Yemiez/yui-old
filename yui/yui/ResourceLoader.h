#pragma once
#include <string>
#include <vector>
#include "Application.h"
#include <glm/glm.hpp>
#include "Badge.h"

namespace yui {
class Utf8String;
struct Color;
class Painter;
class Window;

class FontResource {
private:
    struct CharacterTex;

public:
    FontResource(FT_Face, std::string, uint32_t pixel_size);
    ~FontResource();
    glm::ivec2 text_size(const std::string_view &, Badge<Painter>);
    glm::ivec2 text_size(const Utf8String &, Badge<Painter>);
    CharacterTex *character(unsigned code_point);

    void set_font_size(uint32_t);
    uint32_t font_size() const { return m_pixel_size; }

    const std::string &path() const { return m_font_path; }
private:
    FT_Face m_face{ };
    std::string m_font_path{ };
    uint32_t m_pixel_size{ };
    std::map<unsigned, CharacterTex *> m_textures{ };

    struct CharacterTex {
        uint32_t texture_id;
        glm::ivec2 size;
        glm::ivec2 bearing;
        long advance;
    };

    unsigned m_vao{ 0 }, m_vbo{ 0 };
};

class Shader {
public:
    Shader(const std::string &vertex_path, const std::string &fragment_path);

    [[nodiscard]] uint32_t program_id() const { return m_program_id; }

    void use() const {
        glUseProgram(m_program_id);
    }
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(m_program_id, name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(m_program_id, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(m_program_id, name.c_str()), value);
    }
    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(m_program_id, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(m_program_id, name.c_str()), x, y);
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(m_program_id, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(m_program_id, name.c_str()), x, y, z);
    }
    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(m_program_id, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(m_program_id, name.c_str()), x, y, z, w);
    }
    void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(m_program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(m_program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(m_program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    bool compile_status(uint32_t);
    bool link_status(uint32_t);

private:
    uint32_t m_program_id{ 0 };
    std::string m_compilation_error{ };
    std::string m_link_error{ };
};

class ShaderGuard {
public:
    explicit ShaderGuard(Shader &shader)
            : m_shader(shader) {
        glUseProgram(m_shader.program_id());
    }
    ~ShaderGuard() {
        glUseProgram(0);
    }

private:
    Shader &m_shader;
};

class ResourceLoader {
public:
    explicit ResourceLoader(Window *);
    ~ResourceLoader();

    Window *window() { return m_window; }
    [[nodiscard]] const Window *window() const { return m_window; }

    // Font
    void set_default_font(FontResource *font);
    [[nodiscard]] FontResource *default_font() const { return m_default_font; }
    FontResource *load_font(std::string, uint32_t pixel_size = 12);

    void remove_font(FontResource *);

    // will load vertex shader as path + ".vert" and fragment shader as path + ".frag"
    Shader *load_shader(const std::string &path);
private:
    Window *m_window;
    FontResource *m_default_font{ nullptr };
    std::vector<FontResource *> m_fonts{ };
    std::vector<Shader *> m_shaders{ };
};

}
