#include "Textarea.h"

#include "DocumentWidget.h"
#include "PainterUtilities.h"
#include "../ResourceLoader.h"
#include "../Window.h"
#include "../ymd/Node.h"

yui::layout::Textarea::Textarea()
	: EditorEngine(EditorEngineType::MultiLine)
{
	text_document().connect_client(this);
}

void yui::layout::Textarea::paint(yui::Painter& painter)
{
	PainterUtilities::paint_background(painter, *this);
	PainterUtilities::paint_borders(painter, *this);

	auto *font = PainterUtilities::get_font(*this);

	if (!font) {
		return;
	}

	if (has_selection()) {
		auto range = m_selection.normalized();

		for (auto i = range.start().line(); i <= range.end().line(); ++i) {
			const auto& line = text_document().line(i);

			if (line.empty() && i != range.start().line()) {
				// TODO: Do this properly
				painter.fill_rect(inner_position().x, inner_position().y + (m_character_size.y * i), m_character_size.x, m_character_size.y, { 4, 97, 208, 255 });
				continue;
			}
			
			const auto selection_start = i == range.start().line() ? range.start().column() : 0;
			auto selection_end = i == range.end().line() ? range.end().column() : line.length();

			if (selection_end > line.length()) {
				selection_end = line.length();
			}

			auto view_start = position_to_screen({ i, selection_start });
			auto view_end = position_to_screen({ i, selection_end });

			painter.fill_rect(view_start.x, view_start.y, view_end.x - view_start.x, m_character_size.y, { 4, 97, 208, 255 });
		}
	}
	
	auto pos = inner_position();
	for (const auto &line : text_document().lines()) {		
		painter.text(line.text(), dom_node()->computed().text().color, pos.x, pos.y, *font);
		pos.y += m_character_size.y;
	}

	if (dom_node()->focused()) {
		if (m_caret_beam <= 0.5f) {
			auto caret_position = position_to_screen(m_caret);
			painter.line(caret_position.x, caret_position.y, caret_position.x, caret_position.y + m_character_size.y, { 255, 255, 255, 255 });
		}
	}
}

void yui::layout::Textarea::update(float dt)
{
	m_caret_beam += dt;

	if (m_caret_beam > 1.0f) {
		m_caret_beam = 0.0f;
	}
}

void yui::layout::Textarea::compute()
{
	if (dom_node()->has_attribute("cols")) {
		m_columns = std::stoul(dom_node()->attribute("cols")); 
	}

	if (dom_node()->has_attribute("rows")) {
		m_rows = std::stoul(dom_node()->attribute("rows"));
	}

	auto *font = PainterUtilities::get_font(*this);

	if (!font) {
		return;
	}

	m_character_size = document_widget()->window()->painter().text_size(")", *font);
	m_size.x = m_columns * m_character_size.x;
	m_size.y = m_rows * m_character_size.y;

	if (text_document().line_count() == 0 && dom_node()->has_attribute("value")) {
		text_document().reset_to(Utf8String{dom_node()->attribute("value")});
	}

	attach_clipboard(document_widget()->window()->clipboard());
}

bool yui::layout::Textarea::on_input(int code_point)
{
	if (dom_node()->focused()) {
		return handle_input(code_point);
	}
	
	return false;
}

bool yui::layout::Textarea::on_key_down(int key, int scan, int mods)
{
	if (dom_node()->focused()) {
		return handle_key_down(key, scan, mods);
	}
	return false;
}

bool yui::layout::Textarea::on_key_up(int key, int scan, int mods)
{
	if (dom_node()->focused()) {
		return handle_key_up(key, scan, mods);
	}
	
	return false;
}

glm::ivec2 yui::layout::Textarea::document_request_client_text_size(const Utf8String& text)
{
	auto* font = PainterUtilities::get_font(*this);
	if (!font) return {};
	return document_widget()->window()->painter().text_size(text, *font);
}

void yui::layout::Textarea::editor_caret_did_change()
{
	m_caret_beam = 0.f;
}

glm::ivec2 yui::layout::Textarea::position_to_screen(const TextPosition& caret) const
{
	return inner_position() + glm::ivec2{
		caret.column() * m_character_size.x,
		caret.line() * m_character_size.y,
	};
}

yui::layout::TextPosition yui::layout::Textarea::screen_to_position(glm::ivec2 pt) const
{
	pt -= inner_position();
	glm::ivec2 caret{};
	TextPosition result{};
	
	for (auto y = m_scroll_y; y < m_rows && y < text_document().line_count(); ++y) {
		const auto& line = text_document().line(y);
		for (auto x = m_scroll_x; x < m_columns && x <= line.length(); ++x) {
			if (pt.x >= caret.x && pt.x <= caret.x + m_character_size.x &&
				pt.y >= caret.y && pt.y <= caret.y + m_character_size.y) {
				result.set_column(x);
				result.set_line(y);
				return result;
			}

			caret.x += m_character_size.x;
		}

		if (pt.y >= caret.y && pt.y <= caret.y + m_character_size.y) {
			return text_document().end_of_line(y);
		}

		caret.x = 0;
		caret.y += m_character_size.y;
	}

	if (pt.y > caret.y) {
		return text_document().range_for_entire_document().end();
	}
	
	return {};
}

void yui::layout::Textarea::on_click(glm::ivec2 mouse_position)
{
	m_caret = screen_to_position(mouse_position);
}

