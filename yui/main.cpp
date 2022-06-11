#define GUI_TEST

#ifdef GUI_TEST
#include "yui/Application.h"
#include "yui/Window.h"
#include "yui/layout/DocumentWidget.h"
#include "yui/layout/LayoutTreeDumper.h"
#include "yui/ymd/DocumentNode.h"
#include "yui/ymd/DocumentParser.h"
#include "yui/yss/StyleHelper.h"
#include "yui/yss/StylesheetLexer.h"
#include <thread>
#else
#include "yui/Vector.h"
#include "yui/Utf8String.h"
#include "yui/Util.h"
#include <Windows.h>
#include <cstdio>
#endif
#include "yui/log/Registry.h"

int main(int argc, char **argv)
{
#ifdef GUI_TEST
	yui::log::Registry registry;

	auto logger = std::make_shared<yui::log::Logger>(
		"yui",
		std::vector<std::shared_ptr<yui::log::Sink>>{
			std::make_shared<yui::log::StdoutSink>()
		}
	);
	
	yui::Application app{argc, argv};
	auto window = std::make_shared<yui::Window>();
	
	if (!window->create(363, 120, "Hello world #1")) {
		return 0;
	}
	
	auto parse_document = [](const std::string& name) {
		yui::DocumentParser parser{yui::DocumentLexer{yui::read_file(name)}};
		return parser.release_document();
	};

	auto *document = parse_document("./assets/content/document.ymd");
	document->load_stylesheet("./assets/content/style.yss");
	auto widget = std::make_unique<yui::layout::DocumentWidget>(document, window.get());

	window->on_init = [&widget](yui::Window* sender, yui::Painter* painter) {
		sender->set_clear_color({64, 64, 64});
		// Load default stuff
		auto *shader = sender->resource_loader().load_shader("./assets/shaders/basic");
		painter->set_shader(shader);

		// Construct layout tree
		widget->construct_layout_tree();

		// "FAke" resize the window to fit content.
		glfwSetWindowSize(sender->glfw_window(), widget->size_with_padding().x, widget->size_with_padding().y);
	};

	auto time_passed = 0.f;
	window->on_update = [&widget, &time_passed](yui::Window* sender, float dt) {
		time_passed += dt;
		if (time_passed > 1.f) {
			time_passed = 0.f;
			//widget->dom_document()->reload_stylesheet("./assets/content/style.yss");
			//widget->did_reload_stylesheets();
		}
		widget->update(dt);
	};

	window->on_mouse_left_down = [&widget](yui::Window* sender) {
		widget->mouse_left_down(sender->mouse_x(), sender->mouse_y());
	};

	window->on_mouse_left_down = [&widget](yui::Window* sender) {
		widget->mouse_left_up(sender->mouse_x(), sender->mouse_y());
	};

	window->on_input = [&widget](yui::Window *sender, int code_point) {
		widget->on_input(code_point);
	};

	window->on_key_down = [&widget](yui::Window *sender, int key, int scan, int mods) {
		widget->on_key_down(key, scan, mods);
	};

	window->on_key_up = [&](yui::Window *sender, int key, int scan, int mods) {
		widget->on_key_up(key, scan, mods);
		
		if (key == GLFW_KEY_F11) {
			sender->painter().set_debug(!sender->painter().debug());
		}

		if (key == GLFW_KEY_INSERT && mods & GLFW_MOD_SHIFT) {
			auto dom_node = widget->dom_document()->current_ui_state_node(yui::NodeUiState::Hovered);

			if (dom_node != nullptr) {
				auto *layout_node = widget->find_layout_node(dom_node);

				if (layout_node != nullptr) {
					std::cout << "Dump:\n" << yui::layout::LayoutTreeDumper::dump(layout_node);
				}
			}
		}
		else if (key == GLFW_KEY_INSERT) {
			const auto dump = yui::layout::LayoutTreeDumper::dump(widget.get());
			std::cout << "Dump:\n" << dump;
		}
	};

	window->on_mouse_move = [&widget](yui::Window *sender, double x, double y) {
		widget->mouse_move(x, y);

	};

	window->on_paint = [&](yui::Window* sender, yui::Painter* painter) {
		widget->paint(*painter);
		/*
		auto ypos = 20.f;
		auto xpos = 20.f;
		auto off_x = painter->text_size("Hello 100").x + 10.f;;
		
		for (auto i = 0; i < 100; ++i) {
			auto text = yui::fmt("Hello %d", i);
			painter->text(text, {255, 255, 255}, xpos, ypos);

			auto size = painter->text_size(text);
			ypos += size.y + 10.f;

			if (ypos + size.y + 10.f > sender->height()) {
				ypos = 20.f;
				xpos += off_x;
			}
		}
		*/
	};

	window->show();
	return app.exec();
#else
	return 0;
#endif
	
}
