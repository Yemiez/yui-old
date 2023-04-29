#include "Clipboard.h"
#include "includes.h"
#include "Window.h"

yui::Clipboard::Clipboard(Window &parent)
        : m_window(parent) {}

yui::Utf8String yui::Clipboard::content() const {
    auto content = Utf8String{ glfwGetClipboardString(m_window.glfw_window()) };
    spdlog::debug("Retrieved clipboard: {}", content.to_printable_string());
    return content;
}

void yui::Clipboard::set_content(const Utf8String &content) {
    spdlog::debug("Setting clipboard content to: {}", content.to_printable_string());
    glfwSetClipboardString(m_window.glfw_window(), content.to_byte_string().c_str());
}

void yui::Clipboard::set_content(const char *content) {
    spdlog::debug("Setting clipboard content to: {}", content);
    glfwSetClipboardString(m_window.glfw_window(), content);
}

void yui::Clipboard::set_content(const std::string &content) {
    set_content(content.c_str());
}
