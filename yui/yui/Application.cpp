#include "log/Registry.h"
#include "Application.h"
#include <cassert>
#include <iostream>
#include "Window.h"
#include "io/Profiler.h"

static yui::Application* g_application = nullptr;

void app_error_callback(int, const char* msg)
{
	yui::log::critical("GLFW Error {}", msg);
}

yui::Application::Application(int argc, char** argv)
	: m_arguments(argv + 1, argv + argc)
{
	glfwSetErrorCallback(app_error_callback);
	if (!glfwInit()) {
		report_error("Could not initialize glfw");
		throw std::exception();
	}

	auto error = FT_Init_FreeType(&m_freetype_library);

	if (error) {
		report_error("Could not initialize freetype");
		throw std::exception();
	}

	g_application = this;
	log::success("Application initialized");
}

int yui::Application::exec()
{
	glEnable(GL_TEXTURE_2D);
	io::Profiler<> bm;
	while (!m_halting) {
		glfwPollEvents();
		for (auto it = m_window_map.begin(); it != m_window_map.end();) {
			auto &[_, instance] = *it;

			// Is the window closing? Should be do something with it?
			if (instance.window->closing()) {
				instance.window->painter().make_context();
				instance.window->painter().delete_buffers();
				glfwDestroyWindow(instance.window->glfw_window());
				it = m_window_map.erase(it);

				if (m_window_map.empty()) {
					halt(0);
					break;
				}
				continue;
			}

			// Refresh window
			do_window_refresh(instance);
			++it;
		}
	}
	return 0;
}

void yui::Application::halt(int exit_code)
{
	m_exit_code = exit_code;
	m_halting = true;
}

void yui::Application::register_window(yui::Window* window)
{
	auto id = m_window_counter++;
	
	auto &instance = m_window_map[id] = WindowInstance{
		.id = id,
		.window = window,
		.glfw = window->glfw_window(),
		.original_title = window->title()
	};
	window->set_window_id(id);
	instance.timer.update();

	// Simple and quick window draw.
	instance.window->setup_environment();
	instance.window->painter().create_buffers();
	log::success("Window({}) registered to Application", id);
}

void yui::Application::report_error(const std::string& message)
{
	log::error(message);
}

bool yui::Application::initialized()
{
	return g_application != nullptr;
}

yui::Application& yui::Application::the()
{
	assert(g_application);
	return *g_application;
}

void yui::Application::window_resize(Window* window, int x, int y)
{
	auto& instance = m_window_map.at(window->window_id());
	do_window_refresh(instance);
}

void yui::Application::window_refresh(Window* window)
{
	auto& instance = m_window_map.at(window->window_id());
	do_window_refresh(instance);
}

void yui::Application::do_window_refresh(WindowInstance& instance)
{
	// Update the current drawing context.
	instance.window->painter().make_context();

	// Initialize window
	if (!instance.initialized) {
		instance.window->initialize();
		instance.initialized = true;
	}

	// Update
	const auto delta = instance.timer.update();
	instance.window->update(delta);

	// Setup viewport & paint.
	instance.window->painter().setup_viewport(instance.window->width(), instance.window->height());
	instance.window->paint();
	instance.window->painter().present();

	instance.passed_time += delta;
	if (instance.passed_time > 1.f) {
		instance.fps = instance.timer.get_frames_per_second();
		instance.passed_time = 0.f;
		instance.window->update_fps(instance.fps);
		auto title = yui::fmt("%s | FPS %d", instance.original_title.c_str(), instance.fps);
		glfwSetWindowTitle(instance.window->glfw_window(), title.c_str());
	}
}
