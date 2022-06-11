#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Utf8String.h"

namespace yui {
	class Utf8String;
	class Shader;
	class FontResource;
	class Window;

	struct Color
	{
		uint8_t r{};
		uint8_t g{};
		uint8_t b{};
		uint8_t a{255};

		Color lerp(Color, float t) const;
		float fr() const { return static_cast<float>(r) / 255.f; }
		float fg() const { return static_cast<float>(g) / 255.f; }
		float fb() const { return static_cast<float>(b) / 255.f; }
		float fa() const { return static_cast<float>(a) / 255.f; }

		Color with_r(uint32_t) const;
		Color with_g(uint32_t) const;
		Color with_b(uint32_t) const;
		Color with_a(uint32_t) const;
		Color with_r(float) const;
		Color with_g(float) const;
		Color with_b(float) const;
		Color with_a(float) const;

		bool operator==(const Color& other) const
		{
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}
		bool operator!=(const Color& other) const { return !(*this == other); }
	};

	using DrawIndex = unsigned int;

	struct Vertex
	{
		float position[2]{};
		float use_sampler{0};
		float uv[2]{};
		float color[4]{};
	};

	enum class Primitive
	{
		Triangles,
		Points,
		Lines,
		LineStrip,
		LineLoop,
		Polygon,
		TriangleStrip,
		TriangleFan,
		Quads,
		QuadStrip,
	};

	struct DrawCmd
	{
		DrawIndex index{0};
		uint32_t elements{0};
		Primitive primitive{Primitive::Triangles};
		unsigned int texture_id{0};
		glm::fvec4 clip_rect{0.f, 0.f, 0.f, 0.f};
	};

	struct DrawList
	{
		std::vector<DrawCmd> commands{};
		std::vector<Vertex> vertex_buffer{};
		std::vector<DrawIndex> index_buffer{};
		unsigned int vbo{0}, ibo{0};

		void clear();

		template<size_t v_size, size_t i_size>
		DrawCmd& push(Vertex (&vertices)[v_size], DrawIndex (&indices)[i_size], Primitive prim = Primitive::Triangles)
		{
			return push(vertices, v_size, indices, i_size, prim);
		}
		
		DrawCmd& push(Vertex *vertices, size_t vertices_count, DrawIndex* indices, size_t indices_count, Primitive prim = Primitive::Triangles);
		bool empty() const;
	};
	
	class Painter
	{
	public:
		Painter(Window*);
		Painter(const Painter&) = delete;
		Painter(Painter&&) = delete;
		virtual ~Painter() = default;

		// Misc
		bool debug() const { return m_debug; }
		void set_debug(bool v);
		Shader* shader() const { return m_shader; }
		void set_shader(Shader*);
		
		virtual void clear(Color); // clears screen with Color
		virtual void render(); // renders the current draw list
		virtual void setup_viewport(int w, int h);

		glm::ivec2 viewport() const { return m_viewport; }

		// Utility
		glm::ivec2 text_size(const std::string_view&);
		glm::ivec2 text_size(const std::string_view&, FontResource&);
		glm::ivec2 text_size(const Utf8String&);
		glm::ivec2 text_size(const Utf8String&, FontResource&);

		// These functions add commands to the current draw list.
		void text(const std::string_view&, const Color&, float x, float y, glm::fvec4 clip_rect = {});
		void text(const std::string_view&, const Color&, int x, int y, glm::fvec4 clip_rect = {});
		
		void text(const Utf8String&, const Color&, float x, float y, glm::fvec4 clip_rect = {});
		void text(const Utf8String&, const Color&, int x, int y, glm::fvec4 clip_rect = {});
		void text(const Utf8String&, const Color&, int x, int y, FontResource&, glm::fvec4 clip_rect = {});
		void text(const std::string_view&, const Color&, int x, int y, FontResource&, glm::fvec4 clip_rect = {});
		void text(const Utf8String&, const Color&, float x, float y, FontResource&, glm::fvec4 clip_rect = {});
		void text(const std::string_view&, const Color&, float x, float y, FontResource&, glm::fvec4 clip_rect = {});
		void fill_rect(float x, float y, float w, float h,  const Color&, glm::fvec4 clip_rect = {});
		void fill_rect(int x, int y, int w, int h,  const Color&, glm::fvec4 clip_rect = {});
		void outline_rect(float x, float y, float w, float h, const Color&, float thickness = 1.f, glm::fvec4 clip_rect = {});
		void outline_rect(int x, int y, int w, int h, const Color&, float thickness = 1.f, glm::fvec4 clip_rect = {});
		
		void line(float x1, float y1, float x2, float y2, const Color&);
		void line(int x1, int y1, int x2, int y2, const Color&);
		
		
		// Called by Application.
		void make_context(); // makes current glfw context
		void present(); // swaps buffers

		void create_buffers();
		void delete_buffers();
	private:
		void set_gl_color(const Color&);
	private:
		Window* m_window;
		glm::mat4 m_projection{};
		DrawList m_draw_list{};
		Shader *m_shader{};
		bool m_debug{false};
		glm::ivec2 m_viewport{0, 0};
	};
	
}
