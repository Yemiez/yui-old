#include "Input.h"
#include <algorithm>
#include "DocumentWidget.h"
#include "InlineCompute.h"
#include "PainterUtilities.h"
#include "../Window.h"
#include "../ymd/Node.h"

yui::layout::Input::Input()
        : EditorEngine(EditorEngineType::SingleLine) {
    text_document().connect_client(this);
}

void yui::layout::Input::paint(yui::Painter &painter) {
    PainterUtilities::paint_background(painter, *this);
    PainterUtilities::paint_borders(painter, *this);

    auto text = text_document().text_in_range(
            {
                    TextPosition{ 0, m_scroll },
                    TextPosition{ 0, m_scroll + m_size_value },
            }
    );

    auto *font = PainterUtilities::get_font(*this);

    if (has_selection()) {
        auto selection = m_selection.normalized();

        if (selection.start().column() < m_scroll) {
            selection.start().set_column(m_scroll);
        }

        if (selection.end().column() > m_scroll + m_size_value) {
            selection.end().set_column(m_scroll + m_size_value);
        }

        auto start = position_to_screen(selection.start());
        auto end = position_to_screen(selection.end());

        painter.fill_rect(start.x, start.y, end.x - start.x, m_character_size.y, { 4, 97, 208, 255 });
    }

    if (!text.empty()) {
        painter.text(text, dom_node()->computed().text().color, inner_position().x, inner_position().y, *font);
    } else if (!m_placeholder.empty() && !dom_node()->focused()) {
        painter.text(m_placeholder, { 255, 255, 255, 100 }, inner_position().x, inner_position().y, *font);
    }

    if (dom_node()->focused()) {
        if (m_caret_beam <= 0.5f) {
            auto caret_position = position_to_screen(m_caret);
            painter.line(
                    caret_position.x,
                    caret_position.y,
                    caret_position.x,
                    caret_position.y + m_character_size.y,
                    { 255, 255, 255, 255 }
            );
        }
    }
}

void yui::layout::Input::update(float dt) {
    m_caret_beam += dt;

    if (m_caret_beam > 1.0f) {
        m_caret_beam = 0.0f;
    }
}

void yui::layout::Input::compute() {
    attach_clipboard(document_widget()->window()->clipboard());
    auto &bytes_value = dom_node()->attribute("value");
    auto &size_value = dom_node()->attribute("size");
    auto size = std::stoul(size_value);

    if (!size) {
        size = 6;
    }

    auto *font = PainterUtilities::get_font(*this);
    assert(font);
    m_character_size = document_widget()->window()->painter().text_size(")", *font);

    m_size_value = size;
    m_size.x = m_character_size.x * size;
    m_size.y = m_character_size.y;

    if (text_document().line_count() == 0) {
        text_document().reset_to(Utf8String{ bytes_value });
    }

    if (dom_node()->has_attribute("placeholder")) {
        m_placeholder = dom_node()->attribute("placeholder");
    }
}

bool yui::layout::Input::on_key_down(int key, int scan, int mods) {
    if (dom_node()->focused()) {
        return handle_key_down(key, scan, mods);
    }

    return false;
}

bool yui::layout::Input::on_key_up(int key, int scan, int mods) {
    if (dom_node()->focused()) {
        return handle_key_up(key, scan, mods);
    }

    return false;
}

bool yui::layout::Input::on_input(int code_point) {
    if (dom_node()->focused()) {
        return handle_input(code_point);
    }

    return false;
}

void yui::layout::Input::text_document_did_change() {

}

glm::ivec2 yui::layout::Input::document_request_client_text_size(const Utf8String &text) {
    auto *font = PainterUtilities::get_font(*this);
    if (!font) { return { }; }
    return document_widget()->window()->painter().text_size(text, *font);
}

void yui::layout::Input::editor_caret_did_change() {
    m_caret_beam = 0.f;
    if (m_caret.column() < m_scroll) {
        --m_scroll;
    } else if (m_caret.column() - m_scroll > m_size_value) {
        m_scroll++;
    }

    auto content_length = text_document().line(0).length();
    auto max_scroll = content_length > m_size_value ? content_length - m_size_value : 0;

    if (m_scroll > max_scroll) {
        m_scroll = max_scroll;
    }
}

glm::ivec2 yui::layout::Input::position_to_screen(const TextPosition &caret) const {
    const auto actual_column = caret.column() - m_scroll;
    auto pos = inner_position();
    pos.x += actual_column * m_character_size.x;
    return pos;
}

yui::layout::TextPosition yui::layout::Input::screen_to_position(glm::ivec2 pt) const {
    pt -= inner_position();
    auto position = glm::ivec2{ 0, 0 };
    TextPosition pos{ 0, 0 };

    for (auto i = m_scroll; i < m_size_value && i < text_document().line(0).length(); ++i) {
        if (pt.x >= position.x && pt.x <= position.x + m_character_size.x) {
            pos.set_column(i);
            break;
        }

        position.x += m_character_size.x;
    }

    return pos;
}

void yui::layout::Input::on_click(glm::ivec2 mouse_position) {
    m_caret = screen_to_position(mouse_position);
}
