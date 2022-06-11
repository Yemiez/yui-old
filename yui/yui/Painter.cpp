#include "Painter.h"

#include <algorithm>
#include <iostream>


#include "Util.h"
#include "Window.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

yui::Color yui::Color::lerp(Color other, float t) const
{
	return {
		static_cast<uint8_t>((fr() + (other.fr() - fr()) * t) * 255), 
		static_cast<uint8_t>((fg() + (other.fg() - fg()) * t) * 255), 
		static_cast<uint8_t>((fb() + (other.fb() - fb()) * t) * 255), 
		static_cast<uint8_t>((fa() + (other.fa() - fa()) * t) * 255), 
	};
}

yui::Color yui::Color::with_r(uint32_t v) const
{
	return {static_cast<uint8_t>(v), g, b, a};
}
yui::Color yui::Color::with_g(uint32_t v) const
{
	return {r, static_cast<uint8_t>(v), b, a};
}
yui::Color yui::Color::with_b(uint32_t v) const
{
	return {r, g, static_cast<uint8_t>(v), a};
}
yui::Color yui::Color::with_a(uint32_t v) const
{
	return {r, g, b, static_cast<uint8_t>(v)};
}

yui::Color yui::Color::with_r(float rr) const
{
	return {static_cast<uint8_t>(rr * 255.f), g, b, a};
}
yui::Color yui::Color::with_g(float gg) const
{
	return {r, static_cast<uint8_t>(gg * 255.f), b, a};
}
yui::Color yui::Color::with_b(float bb) const
{
	return {r, g, static_cast<uint8_t>(bb * 255.f), a};
}
yui::Color yui::Color::with_a(float aa) const
{
	return {r, g, b, static_cast<uint8_t>(aa * 255.f)};
}

void yui::DrawList::clear()
{
	commands.clear();
	vertex_buffer.clear();
	index_buffer.clear();
}

yui::DrawCmd& yui::DrawList::push(Vertex* vertices, size_t vertices_count, DrawIndex* indices, size_t indices_count,
								Primitive prim
)
{
	auto vbo_offset = vertex_buffer.size();
	vertex_buffer.insert(vertex_buffer.end(), vertices, vertices + vertices_count);

	DrawCmd cmd {
		.index = index_buffer.size(),
		.elements = indices_count,
		.primitive = prim
	};

	// Transform indices (add the vbo offset)
	std::transform(indices, indices + indices_count, indices, [&vbo_offset](auto &it) {
		return it + vbo_offset;
	});
	index_buffer.insert(index_buffer.end(), indices, indices + indices_count);
	return commands.emplace_back(cmd);
}

bool yui::DrawList::empty() const
{
	return commands.empty() || vertex_buffer.empty() || index_buffer.empty();
}

yui::Painter::Painter(Window* window)
	: m_window(window)
{
	glEnable(GL_LINE_SMOOTH);
}

void yui::Painter::set_debug(bool v)
{
	m_debug = v;
}

void yui::Painter::set_shader(Shader* shader)
{
	m_shader = shader;	
}

void yui::Painter::clear(Color c)
{
	glClearColor ( c.fr(), c.fg(), c.fb(), c.fa() );
	glClear(GL_COLOR_BUFFER_BIT);
	m_draw_list.clear();
}

void yui::Painter::render()
{
	if (debug()) {
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		auto vertices = yui::fmt(
			"Vertices: %d (%d bytes / %d kb)", 
			m_draw_list.vertex_buffer.size(),
			m_draw_list.vertex_buffer.size() * sizeof(Vertex),
			m_draw_list.vertex_buffer.size() * sizeof(Vertex) / 1024
		);
		auto indices = yui::fmt(
			"Indices: %d (%d bytes / %d kb)", 
			m_draw_list.index_buffer.size(),
			m_draw_list.index_buffer.size() * sizeof(DrawIndex),
			m_draw_list.index_buffer.size() * sizeof(DrawIndex) / 1024
		);

		const auto v_size = text_size(vertices);
		const auto i_size = text_size(indices);
		const auto max = glm::fvec2(glm::max(v_size, i_size));
		const auto padding = max.y + 15.f;
		const auto spacing = 10.f;

		const auto x = static_cast<float>(m_window->width()) - max.x - (padding * 2) - spacing;
		const auto y = spacing;
		auto draw_calls = m_draw_list.commands.size();
		
		fill_rect(
			x, 
			y, 
			max.x + (padding * 2), 
			(padding * 4) + 15.f, 
			{ 24, 24, 24, 255 }
		);

		text(vertices, {255, 255, 255}, x + padding, y + 15.f);
		text(indices, {255, 255, 255}, x + padding, y + 15.f + padding);
		text("FPS: " + std::to_string(m_window->fps()), {255, 255, 255}, x + padding, y + (padding * 2) + 15.f);
		text("Draw calls: " + std::to_string(draw_calls), {255, 255, 255}, x + padding, y + (padding * 3) + 15.f);
	}
	
	if (m_draw_list.empty()) {
		return;
	}

	if (m_shader) {
		glUseProgram(m_shader->program_id());
		m_shader->setMat4("ProjMtx", m_projection);
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindBuffer(GL_ARRAY_BUFFER, m_draw_list.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_draw_list.ibo);
	
	// Buffer and store attributes
	glBufferData(
		GL_ARRAY_BUFFER, 
		sizeof(Vertex) * m_draw_list.vertex_buffer.size(), 
		&m_draw_list.vertex_buffer[0], 
		GL_STREAM_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex), 
		reinterpret_cast<void*>(offsetof(Vertex, position)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, false, sizeof(Vertex), 
		reinterpret_cast<void*>(offsetof(Vertex, use_sampler)));
	
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), 
		reinterpret_cast<void*>(offsetof(Vertex, uv)));
	
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex), 
		reinterpret_cast<void*>(offsetof(Vertex, color)));
	
	// IBO
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER, 
		sizeof(DrawIndex) * m_draw_list.index_buffer.size(), 
		&m_draw_list.index_buffer[0], 
		GL_STREAM_DRAW
	);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (auto &cmd : m_draw_list.commands) {
		if (cmd.texture_id) {
			glBindTexture(GL_TEXTURE_2D, cmd.texture_id);
		}

		GLenum type = GL_TRIANGLES;
		switch (cmd.primitive) {
		case Primitive::Triangles:
			type = GL_TRIANGLES;
			break;
		case Primitive::Lines:
			type = GL_LINES;
			break;
		}


		auto is_clipped = false;
		if (cmd.clip_rect != glm::fvec4(0, 0, 0, 0)) {
			glEnable(GL_SCISSOR_TEST);
			const auto clip_rect = glm::ivec4(cmd.clip_rect);
			glScissor(
				clip_rect.x,
				m_window->height() - clip_rect.y - clip_rect.w, 
				clip_rect.z, 
				clip_rect.w
			);
			is_clipped = true;
		}

		glDrawElements(
			type, 
			cmd.elements, 
			GL_UNSIGNED_INT, 
			reinterpret_cast<const void*>(cmd.index * sizeof(DrawIndex))
		);

		if (cmd.texture_id) {
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		if (is_clipped) {
			glDisable(GL_SCISSOR_TEST);
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void yui::Painter::present()
{
	glfwSwapBuffers(m_window->glfw_window());
}

void yui::Painter::create_buffers()
{
	// Create and setup VBO
	glGenBuffers(1, &m_draw_list.vbo);
	glGenBuffers(1, &m_draw_list.ibo);
}

void yui::Painter::delete_buffers()
{
	// Delete the buffers
	glDeleteBuffers(1, &m_draw_list.vbo);
	glDeleteBuffers(1, &m_draw_list.ibo);
}

void yui::Painter::set_gl_color(const Color& c)
{
	glColor4f(c.fr(), c.fg(), c.fb(), c.fa());
}

void yui::Painter::setup_viewport(int w, int h)
{
	// set up view
	glViewport(0, 0, w, h);
	m_projection = glm::ortho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f);
	m_viewport = {w, h};
}

glm::ivec2 yui::Painter::text_size(const std::string_view& s)
{
	auto *default_font = m_window->resource_loader().default_font();

	if (default_font == nullptr) {
		Application::the().report_error("No font created yet");
		return {0, 0};
	}

	return text_size(s, *default_font);
}

glm::ivec2 yui::Painter::text_size(const std::string_view& s, FontResource& f)
{
	return f.text_size(s, {*this});
}

glm::ivec2 yui::Painter::text_size(const Utf8String& view)
{
	auto *default_font = m_window->resource_loader().default_font();

	if (default_font == nullptr) {
		Application::the().report_error("No font created yet");
		return {0, 0};
	}

	return text_size(view, *default_font);
}

glm::ivec2 yui::Painter::text_size(const Utf8String& view, FontResource& f)
{
	return f.text_size(view, {*this});
}

void yui::Painter::text(const std::string_view& s, const Color& color, float x, float y, glm::fvec4 clip_rect)
{
	auto *default_font = m_window->resource_loader().default_font();

	if (!default_font) {
		return;
	}

	return text(s, color, x, y, *default_font, clip_rect);
}

void yui::Painter::text(const std::string_view& s, const Color& color, int x, int y, glm::fvec4 clip_rect)
{
	text(s, color, static_cast<float>(x), static_cast<float>(y), clip_rect);
}

void yui::Painter::text(const Utf8String& view, const Color& color, float x, float y, glm::fvec4 clip_rect)
{
	auto *default_font = m_window->resource_loader().default_font();

	if (!default_font) {
		return;
	}

	text(view, color, x, y, *default_font, clip_rect);
}

void yui::Painter::text(const Utf8String& view, const Color& color, int x, int y, glm::fvec4 clip_rect)
{
	auto *default_font = m_window->resource_loader().default_font();

	if (!default_font) {
		return;
	}

	text(view, color, x, y, *default_font, clip_rect);
}

void yui::Painter::text(const Utf8String& view, const Color& color, int x, int y, FontResource& font, glm::fvec4 clip_rect)
{
	text(view, color, static_cast<float>(x), static_cast<float>(y), font, clip_rect);
}

void yui::Painter::text(const Utf8String& view, const Color& color, float x, float y, FontResource& font, glm::fvec4 clip_rect)
{
	// TODO: We need to optimize this to become one single draw call instead of a draw call
	// per character. This is currently going to be incredibly inefficient with huge chunks
	// of texts (which often appear in UIs).
	auto y_max = font.character('|')->size.y;
	
	for (const auto info : view) {
		auto* ch = font.character(info.code_point);

		if (ch == nullptr) {
			continue;
		}
		
		auto xpos = x + static_cast<float>(ch->bearing.x);
		auto ypos = y + (y_max - static_cast<float>(ch->bearing.y));
		auto w = ch->size.x;
		auto h = ch->size.y;

		Vertex vertices[] = {
			{ .position = {xpos, ypos + h}, .use_sampler=1, .uv={ 0.0f, 1.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {xpos + w, ypos}, .use_sampler=1, .uv={ 1.0f, 0.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {xpos, ypos}, .use_sampler=1, .uv={ 0.0f, 0.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },

			{ .position = {xpos, ypos + h}, .use_sampler=1, .uv={ 0.0f, 1.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {xpos + w, ypos + h}, .use_sampler=1, .uv={ 1.0f, 1.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {xpos + w, ypos}, .use_sampler=1, .uv={ 1.0f, 0.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
		};

		DrawIndex indices[] = {
			0, 1, 2,
			3, 4, 5
		};

		auto &cmd = m_draw_list.push(vertices, indices);
		cmd.texture_id = ch->texture_id;

		if (clip_rect != glm::fvec4{0.f, 0.f, 0.f, 0.f}) {
			cmd.clip_rect = clip_rect;
		}

		x += ch->advance >> 6;
	}
}

void yui::Painter::text(const std::string_view& s, const Color& color, float x, float y, FontResource& font, glm::fvec4 clip_rect)
{
	// TODO: We need to optimize this to become one single draw call instead of a draw call
	// per character. This is currently going to be incredibly inefficient with huge chunks
	// of texts (which often appear in UIs).
	auto y_max = font.character('|')->size.y;
	
	for (const auto &c : s) {
		auto* ch = font.character(c);

		if (ch == nullptr) {
			continue;
		}

		auto xpos = x + static_cast<float>(ch->bearing.x);
		auto ypos = y + (y_max - static_cast<float>(ch->bearing.y));
		auto w = ch->size.x;
		auto h = ch->size.y;

		Vertex vertices[] = {
			{ .position = {xpos, ypos + h}, .use_sampler=1, .uv={ 0.0f, 1.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {xpos + w, ypos}, .use_sampler=1, .uv={ 1.0f, 0.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {xpos, ypos}, .use_sampler=1, .uv={ 0.0f, 0.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },

			{ .position = {xpos, ypos + h}, .use_sampler=1, .uv={ 0.0f, 1.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {xpos + w, ypos + h}, .use_sampler=1, .uv={ 1.0f, 1.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {xpos + w, ypos}, .use_sampler=1, .uv={ 1.0f, 0.0f }, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
		};

		DrawIndex indices[] = {
			0, 1, 2,
			3, 4, 5
		};

		auto &cmd = m_draw_list.push(vertices, indices);
		cmd.texture_id = ch->texture_id;

		if (clip_rect != glm::fvec4{0.f, 0.f, 0.f, 0.f}) {
			cmd.clip_rect = clip_rect;
		}

		x += ch->advance >> 6;
	}
}

void yui::Painter::text(const std::string_view& s, const Color& color, int x, int y, FontResource& font, glm::fvec4 clip_rect)
{
	text(s, color, static_cast<float>(x), static_cast<float>(y), font, clip_rect);
}

void yui::Painter::fill_rect(float x, float y, float w, float h, const Color& color, glm::fvec4 clip_rect)
{
	Vertex vertices[] = {
		{ .position = {x, y + h}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
		{ .position = {x + w, y + h}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
		{ .position = {x + w, y}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
		{ .position = {x, y}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
	};

	DrawIndex indices[] = {
		0, 1, 2,
		2, 3, 0,
	};

	m_draw_list.push(vertices, indices);
}

void yui::Painter::fill_rect(int x, int y, int w, int h, const Color& color, glm::fvec4 clip_rect)
{
	fill_rect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), color, clip_rect);
}

void yui::Painter::outline_rect(float x, float y, float w, float h, const Color& color, float thickness, glm::fvec4 clip_rect)
{
	if (thickness > 1.0f) {
		auto x1 = x;
		auto x2 = x + w + thickness;
		auto y1 = y;
		auto y2 = y + h + thickness;

		Vertex vertices[] = {
			// Top
			{ .position = {x1, y1 + thickness}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x2, y1 + thickness}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x2, y1}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x1, y1}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },

			// left
			{ .position = {x1, y2 + thickness}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x1 + thickness, y2 + thickness}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x1 + thickness, y1 + thickness}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },

			// Bottom
			{ .position = {x2 + thickness, y2 + thickness}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x2 + thickness, y2}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x1 + thickness, y2}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },

			// Right
			{ .position = {x2, y2 + thickness}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x2, y1}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x2 + thickness, y1}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
		};

		DrawIndex indices[] = {
			// Top
			0, 1, 2,
			2, 3, 0,

			// Left
			0, 4, 5,
			5, 6, 0,

			// Bottom
			4, 7, 8,
			8, 9, 4,

			// Right
			7, 10, 11,
			11, 12, 7
		};

		auto& cmd = m_draw_list.push(vertices, indices);

		if (clip_rect != glm::fvec4{0.f, 0.f, 0.f, 0.f}) {
			cmd.clip_rect = clip_rect;
		}
	}
	else {
		Vertex vertices[] = {
			// Top
			{ .position = {x, y}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x + w, y}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x + w, y + h}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
			{ .position = {x, y + h}, .use_sampler=0, .color= {color.fr(), color.fg(), color.fb(), color.fa()} },
		};

		DrawIndex indices[] = {
			0, 1,
			1, 2,
			2, 3,
			0, 3
		};

		auto& cmd = m_draw_list.push(vertices, indices, Primitive::Lines);

		if (clip_rect != glm::fvec4{0.f, 0.f, 0.f, 0.f}) {
			cmd.clip_rect = clip_rect;
		}
	}
}

void yui::Painter::outline_rect(int x, int y, int w, int h, const Color& color, float thickness, glm::fvec4 clip_rect)
{
	outline_rect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), color, thickness, clip_rect);
}

void yui::Painter::line(float x1, float y1, float x2, float y2, const Color& color)
{
	Vertex vertices[] = {
		{ .position = {x1, y1}, .color={color.fr(), color.fg(), color.fb(), color.fa()} },
		{ .position = {x2, y2}, .color={color.fr(), color.fg(), color.fb(), color.fa()} },
	};
	DrawIndex indices[] = { 0, 1 };
	m_draw_list.push(vertices, indices, Primitive::Lines);
}

void yui::Painter::line(int x1, int y1, int x2, int y2, const Color& c)
{
	line(static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(x2), static_cast<float>(y2), c);
}

void yui::Painter::make_context()
{
	glfwMakeContextCurrent(m_window->glfw_window());
}
