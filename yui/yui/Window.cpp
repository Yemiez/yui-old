#include "Window.h"

#include <iostream>
#include "includes.h"
#include "Application.h"
#include "Util.h"
#include "log/Registry.h"

// callback definitions
void callback_window_close(GLFWwindow*);
void callback_window_focus(GLFWwindow*, int);
void callback_window_key(GLFWwindow* wnd, int key, int scancode, int action, int mods);
void callback_window_character(GLFWwindow* wnd, unsigned code_point);
void callback_window_size(GLFWwindow* wnd, int width, int height);
void callback_window_cursor_pos(GLFWwindow* wnd, double x, double y);
void callback_window_scroll(GLFWwindow* wnd, double delta_x, double delta_y);
void callback_window_mouse_button(GLFWwindow* wnd, int button, int action, int mods);

yui::Window::Window()
	: m_painter(this), m_resource_loader(this)
{}

bool yui::Window::create(int width, int height, const std::string& title, bool vsync)
{
	if (!Application::initialized()) {
		return false;
	}

	glfwWindowHint(GLFW_VISIBLE, 0);
	//glfwWindowHint(GLFW_FLOATING, 1);
	auto *wnd = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	
	if (wnd == nullptr) {
		return false;
	}
	
	m_width = width;
	m_height = height;
	m_title = title;
	m_glfw = wnd;
	m_vsync = vsync;
	glfwSetWindowUserPointer(wnd, static_cast<void*>(this));

	// Setup event functions
	glfwSetWindowCloseCallback(wnd, callback_window_close);
	glfwSetWindowFocusCallback(wnd, callback_window_focus);
	glfwSetKeyCallback(wnd, callback_window_key);
	glfwSetCharCallback(wnd, callback_window_character);
	glfwSetWindowSizeCallback(wnd, callback_window_size);
	glfwSetCursorPosCallback(wnd, callback_window_cursor_pos);
	glfwSetScrollCallback(wnd, callback_window_scroll);
	glfwSetMouseButtonCallback(wnd, callback_window_mouse_button);

	m_arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_input_cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	m_hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	use_arrow_cursor();
	
	// Application & Painter
	Application::the().register_window(this);

	log::success("Created window with id {} (name {}, size {} {})", m_window_id, m_title, m_width, m_height);
	return true;
}

bool yui::Window::visible() const
{
	return !!glfwGetWindowAttrib(m_glfw, GLFW_VISIBLE);
}

void yui::Window::show()
{
	glfwShowWindow(m_glfw);
}

void yui::Window::hide()
{
	glfwHideWindow(m_glfw);
}

void yui::Window::set_closing(bool cond)
{
	m_closing = cond;
}

void yui::Window::request_attention() const
{
	glfwRequestWindowAttention(m_glfw);
}

void yui::Window::make_focused() const
{
	glfwFocusWindow(m_glfw);
}

void yui::Window::set_window_id(uint32_t id)
{
	m_window_id = id;
}

void yui::Window::set_clear_color(Color color)
{
	m_clear_color = color;
}

void yui::Window::use_arrow_cursor() const
{
	glfwSetCursor(m_glfw, m_arrow_cursor);
}

void yui::Window::use_input_cursor() const
{
	glfwSetCursor(m_glfw, m_input_cursor);
}

void yui::Window::use_hand_cursor() const
{
	glfwSetCursor(m_glfw, m_hand_cursor);
}

void yui::Window::initialize()
{
	if (!m_did_init && on_init) {
		m_did_init = true;
		on_init(this, &m_painter);
	}
}

void yui::Window::update(float dt)
{
	if (on_update) on_update(this, dt);

	m_scroll_x = m_scroll_y = 0.0;
}

void yui::Window::paint()
{
	//glEnable(GL_SCISSOR_TEST);
	//glScissor(342, 20, 180, 18);
	m_painter.clear(m_clear_color);
	//glDisable(GL_SCISSOR_TEST);
	
	if (on_paint) on_paint(this, &m_painter);

	// Render all the draw calls.
	m_painter.render();
}


void yui::Window::gained_focus()
{
	if (on_gained_focus) on_gained_focus(this);
}

void yui::Window::lost_focus()
{
	if (on_lost_focus) on_lost_focus(this);
}

void yui::Window::user_close()
{
	if (on_user_close) on_user_close(this);
}

void yui::Window::key_down(int key, int scancode, int mods)
{
	if (on_key_down) on_key_down(this, key, scancode, mods);
}

void yui::Window::key_up(int key, int scancode, int mods)
{
	if (on_key_up) on_key_up(this, key, scancode, mods);
}

void yui::Window::input(unsigned codepoint)
{
	if (on_input) on_input(this, codepoint);
}


void yui::Window::mouse_move(double x, double y)
{
	m_mouse_x = static_cast<int>(x);
	m_mouse_y = static_cast<int>(y);

	Application::the().profiler().run(__FUNCTION__, [&]() {
		if (on_mouse_move) on_mouse_move(this, x, y);
	});
}

void yui::Window::mouse_scroll(double delta_x, double delta_y)
{
	if (on_mouse_scroll) on_mouse_scroll(this, delta_x, delta_y);

	m_scroll_x = delta_x;
	m_scroll_y = delta_y;
}

void yui::Window::mouse_down(int button, int mods)
{
	if (on_mouse_down) {
		on_mouse_down(this, button, mods);
	}
	
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (on_mouse_left_down) {
			on_mouse_left_down(this);
		}
	}
	else {
		if (on_mouse_right_down) {
			on_mouse_right_down(this);
		}
	}
}

void yui::Window::mouse_up(int button, int mods)
{
	if (on_mouse_up) {
		on_mouse_up(this, button, mods);
	}
	
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (on_mouse_left_up) {
			on_mouse_left_up(this);
		}
	}
	else {
		if (on_mouse_right_up) {
			on_mouse_right_up(this);
		}
	}
}

void yui::Window::resize(int w, int h)
{
	m_width = w;
	m_height = h;
	Application::the().window_resize(this, w, h);
}

void yui::Window::setup_environment()
{
	painter().make_context();

	auto glew_error = glewInit();
	if (glew_error != GLEW_OK) {
		Application::the().report_error(yui::fmt("Could not initialize glew: %s", glewGetErrorString(glew_error)));
		throw std::exception();
	}
	
	glfwSwapInterval(0);

	painter().present();
}

void yui::Window::update_fps(int fps)
{
	m_fps = fps;
}

// CALLBACKS

#define GET_WINDOW() \
	auto* window = static_cast<yui::Window*>(glfwGetWindowUserPointer(wnd)); \
	if (window == nullptr) { \
		std::cerr << "Window has been unregistered but still receiving events?" << std::endl; \
		return; \
	}

void callback_window_close(GLFWwindow* wnd)
{
	GET_WINDOW();

	window->set_closing(true);
	window->user_close();
}
void callback_window_focus(GLFWwindow* wnd, int focused)
{
	GET_WINDOW();
	if (focused == 1) {
		window->gained_focus();
	}
	else {
		window->lost_focus();
	}
}

void callback_window_key(GLFWwindow* wnd, int key, int scancode, int action, int mods)
{
	GET_WINDOW();
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		window->key_down(key, scancode, mods);
	}
	else {
		window->key_up(key, scancode, mods);
	}
}

void callback_window_character(GLFWwindow* wnd, unsigned code_point)
{
	GET_WINDOW();
	window->input(code_point);
}

void callback_window_size(GLFWwindow* wnd, int width, int height)
{
	GET_WINDOW();
	window->resize(width, height);
}

void callback_window_cursor_pos(GLFWwindow* wnd, double x, double y)
{
	GET_WINDOW();
	window->mouse_move(x, y);
}

void callback_window_scroll(GLFWwindow* wnd, double delta_x, double delta_y)
{
	GET_WINDOW();
	window->mouse_scroll(delta_x, delta_y);
}

void callback_window_mouse_button(GLFWwindow* wnd, int button, int action, int mods)
{
	GET_WINDOW();
	if (action == GLFW_PRESS) {
		window->mouse_down(button, mods);
	}
	else {
		window->mouse_up(button, mods);
	}
}
