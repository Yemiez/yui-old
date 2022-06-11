#include "ResourceLoader.h"
#include "includes.h"
#include "Application.h"
#include "Painter.h"
#include "Util.h"
#include "Window.h"
#include "Utf8String.h"
#include "log/Registry.h"

yui::FontResource::FontResource(FT_Face face, std::string path, uint32_t pixel_size)
	: m_face(face), m_font_path(std::move(path)), m_pixel_size(pixel_size)
{
}

yui::FontResource::~FontResource()
{
	FT_Done_Face(m_face);

	for (auto [_, texture] : m_textures) {
		glDeleteTextures(1, &texture->texture_id);
		delete texture;
	}

	log::success("Deleted font face ('{}', {}px)", m_font_path, m_pixel_size);
}

glm::ivec2 yui::FontResource::text_size(const std::string_view& text, Badge<Painter> badge)
{
	glm::ivec2 size{0, 0};
	for (auto &c : text) {
		auto *ch = character(c);

		if (ch == nullptr) {
			continue;
		}

		size.x += ch->advance >> 6;
	}

	size.y = character('|')->size.y;

	return size;
}

glm::ivec2 yui::FontResource::text_size(const Utf8String& view, Badge<Painter>)
{
	glm::ivec2 size{0, 0};
	for (const auto& info : view) {
		auto *ch = character(info.code_point);

		if (ch == nullptr) {
			continue;
		}

		size.x += ch->advance >> 6;
	}

	size.y = character('|')->size.y;
	return size;
}

yui::FontResource::CharacterTex* yui::FontResource::character(unsigned code_point)
{
	const auto it = m_textures.find(code_point);

	if (it != m_textures.end()) {
		return it->second;
	}

	FT_Select_Charmap(m_face, ft_encoding_unicode);
	// create it.
	auto error = FT_Load_Char(m_face, code_point, FT_LOAD_RENDER);

	if (error) {
		Application::the().report_error("Could not load code point in FontResource::character()");
		return nullptr;
	}

	GLuint texture = 0u;
	// disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate texture
	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,//GL_BGRA_EXT,
        m_face->glyph->bitmap.width,
        m_face->glyph->bitmap.rows,
        0,
        GL_RED,//GL_BGRA_EXT,
        GL_UNSIGNED_BYTE,
        m_face->glyph->bitmap.buffer
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	auto *tex = new CharacterTex{
		.texture_id = texture,
		.size = glm::ivec2(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows),
		.bearing = glm::ivec2(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top),
		.advance = m_face->glyph->advance.x
	};
	log::debug(
		"Loaded glyph '{}' from ('{}', {})", 
		yui::escape_code_point(Utf8String::decode(code_point).value_or({})),
		m_font_path,
		m_pixel_size
	);
	return m_textures[code_point] = tex;
}

void yui::FontResource::set_font_size(uint32_t size)
{
	m_pixel_size = size;

	const auto error = FT_Set_Pixel_Sizes(m_face, 0, m_pixel_size);

	if (error) {
		Application::the().report_error(
			yui::fmt("Could not resize font to %d", size)
		);
	}
}

yui::Shader::Shader(const std::string& vertex_path, const std::string& fragment_path)
{
	auto vertex_code = yui::read_file(vertex_path);
	auto fragment_code = yui::read_file(fragment_path);

	auto vertex_code_sz = vertex_code.c_str();
	auto fragment_code_sz = fragment_code.c_str();
	
	// Create vertex shader
	const auto vertex_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_id, 1, &vertex_code_sz, nullptr);
	glCompileShader(vertex_id);

	if (!compile_status(vertex_id)) {
		Application::the().report_error(yui::fmt(
			"Failed to compile vertex shader, error: %s",
			m_compilation_error.c_str()
		));
		throw std::exception();
	}

	// Create fragment shader
	const auto fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_id, 1, &fragment_code_sz, nullptr);
	glCompileShader(fragment_id);

	if (!compile_status(fragment_id)) {
		Application::the().report_error(yui::fmt(
			"Failed to compile fragment shader, error: %s",
			m_compilation_error.c_str()
		));
		throw std::exception();
	}

	// Create program
	m_program_id = glCreateProgram();
	
	glAttachShader(m_program_id, vertex_id);
	glAttachShader(m_program_id, fragment_id);

	glLinkProgram(m_program_id);

	if (!link_status(m_program_id)) {
		Application::the().report_error(yui::fmt(
			"Failed to link shader, error: %s",
			m_link_error.c_str()
		));
		throw std::exception();
	}

	// Can be deleted now.
	glDeleteShader(vertex_id);
	glDetachShader(m_program_id, vertex_id);
	
	glDeleteShader(fragment_id);
	glDetachShader(m_program_id, fragment_id);
}

bool yui::Shader::compile_status(uint32_t shader)
{
	GLint success;
	char info_log[1024];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		glGetShaderInfoLog(shader, 1024, nullptr, info_log);
		m_compilation_error = info_log;
		return false;
	}

	m_compilation_error.clear();
	return true;
}

bool yui::Shader::link_status(uint32_t program_id)
{
	GLint success;
	char info_log[1024];
	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		glGetProgramInfoLog(program_id, 1024, nullptr, info_log);
		m_link_error = info_log;
		return false;
	}

	m_link_error.clear();
	return true;
}

yui::ResourceLoader::ResourceLoader(Window* window)
	: m_window(window)
{}

yui::ResourceLoader::~ResourceLoader()
{
	// TODO deallocate
	for (auto &font : m_fonts) {
		delete font;
		font = nullptr;
	}
	m_fonts.clear();
}

void yui::ResourceLoader::set_default_font(FontResource* font)
{
	m_default_font = font;
}

yui::FontResource* yui::ResourceLoader::load_font(std::string path, uint32_t pixel_size)
{
	FT_Face face;
	auto error = FT_New_Face(Application::the().freetype(), path.c_str(), 0, &face);

	if (error) {
		Application::the().report_error(
			yui::fmt("Could not load font %s, error: %s", path.c_str(), FT_Error_String(error))
		);
		return nullptr;
	}

	error = FT_Set_Pixel_Sizes(face, 0, pixel_size);

	if (error) {
		Application::the().report_error(
			yui::fmt("Could not load font %s, error: %s", path.c_str(), FT_Error_String(error))
		);
		FT_Done_Face(face);
		return nullptr;
	}
	
	auto* font = new FontResource(face, std::move(path), pixel_size);
	m_fonts.emplace_back(font);

	if (m_default_font == nullptr) {
		m_default_font = font;
	}

	log::success("Loaded font '{}' {}px", font->path(), font->font_size());
	return font;
}

void yui::ResourceLoader::remove_font(FontResource* removing)
{
	for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it) {

		if (*it == removing) {
			m_fonts.erase(it);
			break;
		}
	}

	delete removing;
}

yui::Shader* yui::ResourceLoader::load_shader(const std::string& path)
{
	try {
		auto *shader = new Shader(path + ".vert", path + ".frag");
		m_shaders.emplace_back(shader);
		log::success("Loaded vertex & fragment shader '{}'", path);
		return shader;
	}
	catch (std::exception&) {
		log::error("Could not load vertex & fragment shader '{}'", path);
		return nullptr;
	}
}
