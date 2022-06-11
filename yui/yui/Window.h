#pragma once
#include <functional>
#include <string>
#include "Painter.h"
#include "ResourceLoader.h"
#include "Clipboard.h"

struct GLFWwindow;

namespace yui {

	class Window
	{
	public:
		Window();
		virtual ~Window() = default;
		bool create(int width, int height, const std::string& title, bool vsync = false);
		
		const ResourceLoader& resource_loader() const { return m_resource_loader; }
		ResourceLoader& resource_loader() { return m_resource_loader; }
		
		const Painter& painter() const { return m_painter; }
		Painter& painter() { return m_painter; }

		GLFWwindow* glfw_window() const { return m_glfw; }
		int width() const { return m_width; }
		int height() const { return m_height; }
		const std::string& title() const { return m_title; }

		double scroll_x() const { return m_scroll_x; }
		double scroll_y() const { return m_scroll_y; }

		int mouse_x() const { return m_mouse_x; }
		int mouse_y() const { return m_mouse_y; }

		bool vsync() const { return m_vsync; }
		bool visible() const;
		void show();
		void hide();

		void set_closing(bool cond);
		bool closing() const { return m_closing; }

		void request_attention() const;
		void make_focused() const;

		uint32_t window_id() const { return m_window_id; }
		void set_window_id(uint32_t);

		Color clear_color() const { return m_clear_color; }
		void set_clear_color(Color color);

		int fps() const { return m_fps; }

		void use_arrow_cursor() const;
		void use_input_cursor() const;
		void use_hand_cursor() const;

		const Clipboard& clipboard() const { return m_clipboard; }
		Clipboard& clipboard() { return m_clipboard; }

		// events functions
	public:
		using KeyCode = int;
		using ScanCode = int;
		using KeyMods = int;
		std::function<void(Window*, Painter*)> on_paint{};
		std::function<void(Window*, Painter*)> on_init{}; // Use this event to initialize Shader & Load a default font.
		std::function<void(Window*, float)> on_update{};
		std::function<void(Window*)> on_gained_focus{};
		std::function<void(Window*)> on_lost_focus{};
		std::function<void(Window*)> on_user_close{};
		std::function<void(Window*, KeyCode, ScanCode, KeyMods)> on_key_down{};
		std::function<void(Window*, KeyCode, ScanCode, KeyMods)> on_key_up{};
		std::function<void(Window*, int)> on_input{};
		std::function<void(Window*, int, int)> on_resize{};
		std::function<void(Window*, double, double)> on_mouse_move{};
		std::function<void(Window*, double, double)> on_mouse_scroll{};
		std::function<void(Window*, int, int)> on_mouse_down{};
		std::function<void(Window*, int, int)> on_mouse_up{};
		std::function<void(Window*)> on_mouse_left_down{};
		std::function<void(Window*)> on_mouse_left_up{};
		std::function<void(Window*)> on_mouse_right_down{};
		std::function<void(Window*)> on_mouse_right_up{};

	public:
		// Called from event handler code
		void gained_focus();
		void lost_focus();
		void user_close();
		void key_down(int key, int scancode, int mods);
		void key_up(int key, int scancode, int mods);
		void input(unsigned codepoint);
		void resize(int width, int height);
		void mouse_move(double x, double y);
		void mouse_scroll(double delta_x, double delta_y);
		void mouse_down(int button, int mods);
		void mouse_up(int button, int mods);
		

	private:
		// Called from application
		virtual void initialize();
		virtual void update(float);
		virtual void paint();
		void setup_environment();
		void update_fps(int fps);
		friend class Application;
	private:
		uint32_t m_window_id{0};
		GLFWwindow* m_glfw{nullptr};
		int m_width{};
		int m_height{};
		int m_mouse_x{};
		int m_mouse_y{};
		std::string m_title{};
		bool m_closing{false};
		bool m_vsync{false};
		Painter m_painter;
		ResourceLoader m_resource_loader;
		double m_scroll_x{};
		double m_scroll_y{};
		Color m_clear_color{255, 255, 255};
		int m_fps{0};
		bool m_did_init{false};
		// Cursors
		GLFWcursor *m_arrow_cursor{nullptr},
			*m_input_cursor{nullptr},
			*m_hand_cursor{nullptr};
		Clipboard m_clipboard{*this};
	};
	
}