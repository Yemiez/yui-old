#include "EditorEngine.h"
#include "../includes.h"
#include "../Application.h"
#include "../Clipboard.h"

yui::layout::TextDocumentLine::TextDocumentLine(TextDocument* document)
	: m_document(document)
{}

yui::layout::TextDocumentLine::TextDocumentLine(TextDocument* document, Utf8String text)
	: m_document(document), m_text(std::move(text))
{}

yui::layout::TextDocument::TextDocument(EditorEngine* editor)
	: m_editor(editor)
{
}

void yui::layout::TextDocument::reset_to(const Utf8String& content)
{
	m_lines.clear();

	Utf8String current{};

	for (const auto &cp : content) {
		if (cp.code_point == '\n') {
			m_lines.emplace_back(this, current);
			current.clear();
		}
		else {
			current.push_back(cp);
		}
	}

	if (!current.empty()) {
		m_lines.emplace_back(this, current);
	}

	if (m_lines.empty()) {
		m_lines.emplace_back(this);
	}
}

void yui::layout::TextDocument::append_line(Utf8String content)
{
	m_lines.emplace_back(TextDocumentLine{this, std::move(content)});
	notify_did_insert_line(line_count() - 1);
}

void yui::layout::TextDocument::append_line()
{
	m_lines.emplace_back(TextDocumentLine{this});
	notify_did_insert_line(line_count() - 1);
}

void yui::layout::TextDocument::insert_line_before(uint32_t where, Utf8String content)
{
	m_lines.insert(m_lines.begin() + where, TextDocumentLine{this, std::move(content)});
	notify_did_insert_line(where);
}

void yui::layout::TextDocument::insert_line_before(uint32_t where)
{
	m_lines.insert(m_lines.begin() + where, TextDocumentLine{this});
	notify_did_insert_line(where);
}

void yui::layout::TextDocument::insert_line_after(uint32_t where, Utf8String content)
{
	if (m_lines.size() - 1 == where) {
		m_lines.insert(m_lines.end(), TextDocumentLine{this, std::move(content)});
	}
	else {
		m_lines.insert(m_lines.begin() + where + 1, TextDocumentLine{this, std::move(content)});
	}
}

void yui::layout::TextDocument::insert_line_after(uint32_t where)
{
	if (m_lines.size() - 1 == where) {
		m_lines.insert(m_lines.end(), TextDocumentLine{this});
	}
	else {
		m_lines.insert(m_lines.begin() + where + 1, TextDocumentLine{this});
	}
}

void yui::layout::TextDocument::remove_line(uint32_t where)
{
	m_lines.erase(m_lines.begin() + where);
	notify_did_remove_line(where);
}

yui::layout::TextPosition yui::layout::TextDocument::insert(const TextPosition& where, int code_point)
{
	auto &line = this->line(where.line());
	auto iterator = line.text().insert(where.column(), code_point);

	if (iterator == line.text().end()) {
		return where; // No change.
	}

	notify_did_change();
	return next_position_after(where);
}

yui::layout::TextPosition yui::layout::TextDocument::insert(const TextPosition& where, const Utf8String& text)
{
	auto& line = this->line(where.line());
	auto iterator = line.text().insert(where.column(), text);

	notify_did_change();

	return { where.line(), where.column() + text.length() };
}

yui::layout::TextPosition yui::layout::TextDocument::erase(const TextPosition& where)
{
	auto &line = this->line(where.line());

	if (where.column() == 0 && line_count() == 0) {
		return where;
	}

	if (where.column() == 0) {
		if (line.empty() && line_count() > 1) {
			remove_line(where.line());

			if (where.line() == 0) {
				return end_of_line(0);
			}
			return end_of_line(where.line() - 1);
		}

		if (where.line() == 0) {
			return where;
		}

		auto content = line.text();
		remove_line(where.line());

		auto next = end_of_line(where.line() - 1);
		insert(next, content);
		return next;
	}

	auto iterator = line.text().erase(where.column() - 1);
	notify_did_change();
	return previous_position_after(where);
}

yui::layout::TextPosition yui::layout::TextDocument::erase_range(TextRange range)
{
	range = range.normalized();

	if (range.start().line() > line_count() ||
		range.end().line() > line_count()) {
		return {};
	}

	if (line_count() == 0) 
		return {};

	if (range == range_for_entire_document()) {
		m_lines.clear();
		m_lines.emplace_back(TextDocumentLine{this});
		return {0, 0};
	}

	auto end = range.end().line();
	for (auto i = range.start().line(); i <= end && end != 0; --end) {
		auto& line = m_lines.at(i);
		
		const auto selection_start = i == range.start().line() ? range.start().column() : 0;
		auto selection_end = i == range.end().line() ? range.end().column() : line.length();

		if (selection_end > line.length()) {
			selection_end = line.length();
		}
		line.text().erase(selection_start, selection_end);

		if (line.text().empty() && line_count() > 1) {
			// remove the line, keep I the same.
			m_lines.erase(m_lines.begin() + i);
		}
		else {
			++i;
		}
	}

	return range.start();
}

yui::Utf8String yui::layout::TextDocument::text() const
{
	yui::Utf8String text;
	for (const auto &line : m_lines) {
		text.append(line.text());
	}
	return text;
}

yui::Utf8String yui::layout::TextDocument::text_in_range(TextRange range) const
{
	Utf8String text{};
	range = range.normalized();

	if (range.start().line() > line_count() ||
		range.end().line() > line_count()) {
		return {};
	}

	if (m_lines.empty()) {
		return {};
	}
	
	for (auto i = range.start().line(); i <= range.end().line(); ++i) {
		const auto& line = m_lines.at(i);

		if (line.empty()) {
			continue;
		}
		
		const auto selection_start = i == range.start().line() ? range.start().column() : 0;
		auto selection_end = i == range.end().line() ? range.end().column() : line.length();

		if (selection_end > line.length()) {
			selection_end = line.length();
		}
		text.append(line.text().begin() + selection_start, line.text().begin() + selection_end);
		if (i != range.end().line()) text.push_back('\n');
	}
	
	return text;
}

uint32_t yui::layout::TextDocument::code_point_at(const TextPosition& position) const
{
	if (line_count() <= position.line()) {
		return 0;
	}

	const auto& l = line(position.line());

	if (l.length() <= position.column()) {
		return 0;
	}

	return l.text().code_point_at(position.column());
}

yui::layout::TextRange yui::layout::TextDocument::range_for_entire_document() const
{
	return { {0, 0}, { line_count() - 1, line(line_count() - 1).length() } };
}

yui::layout::TextRange yui::layout::TextDocument::range_for_entire_line(uint32_t index) const
{
	if (line_count() <= index) {
		return {};
	}
	
	const auto &line = this->line(index);
	return {
		{ index, 0 },
		{ index, line.length() },
	};
}

yui::layout::TextPosition yui::layout::TextDocument::start_of_line(uint32_t index) const
{
	if (line_count() <= index) {
		return {};
	}
	return { index, 0 };
}

yui::layout::TextPosition yui::layout::TextDocument::end_of_line(uint32_t index) const
{
	if (line_count() <= index) {
		return {};
	}
	return { index, line(index).length() };
}

yui::layout::TextPosition yui::layout::TextDocument::next_position_after(const TextPosition& position) const
{
	if (line_count() <= position.line()) {
		return {};
	}

	const auto& line = this->line(position.line());
	if (position.column() == line.length()) {
		if (position.line() == line_count() - 1) {
			return position;
		}

		return { position.line() + 1, 0 };
	}
	return { position.line(), position.column() + 1 };
}

yui::layout::TextPosition yui::layout::TextDocument::previous_position_after(const TextPosition& position) const
{
	if (line_count() <= position.line()) {
		return {};
	}

	if (position.column() == 0) {
		if (position.line() == 0) return position; // no change
		
		return { position.line() - 1, this->line(position.line() - 1).length() };
	}

	return { position.line(), position.column() - 1 };
}

yui::layout::TextPosition yui::layout::TextDocument::next_position_up(const TextPosition& position) const
{
	if (position.line() == 0) {
		return position; // Perhaps wrap around?
	}

	TextPosition next{ position.line() - 1, position.column() };

	if (next.column() > line(next.line()).length()) {
		next.set_column(line(next.line()).length());
	}

	return next;
}

yui::layout::TextPosition yui::layout::TextDocument::next_position_down(const TextPosition& position) const
{
	if (position.line() == line_count() - 1) {
		return position; // Perhaps wrap around?
	}

	TextPosition next{ position.line() + 1, position.column() };

	if (next.column() > line(next.line()).length()) {
		next.set_column(line(next.line()).length());
	}

	return next;
}

yui::layout::TextPosition yui::layout::TextDocument::next_word_break_after(const TextPosition& position) const
{
	const auto start_was_alpha = ::isalpha(code_point_at(position)) || code_point_at(position) == '_';

	auto consume_until = [this](TextPosition pos, auto predicate) -> TextPosition {
		while (contains(pos)) {
			if (predicate(code_point_at(pos)))
				break;
			pos = next_position_after(pos);
		}
		return pos;
	};

	if (start_was_alpha) {
		return consume_until(position, [this](auto p) {
			return !::isalnum(p);
		});
	}
	
	return consume_until(next_position_after(position), [this](auto p) {
		return ::isalnum(p) || p == '_' || p == '-' || p == '(' || p == ')' ||
			p == '[' || p == ']' || p == '{' || p == '}' || p == '.'  || p == ':' ||
			p == ';' || p == '#';
	});
}

yui::layout::TextPosition yui::layout::TextDocument::previous_word_break_after(const TextPosition& position) const
{
	const auto start_was_alpha = ::isalpha(code_point_at(position)) || code_point_at(position) == '_';

	auto consume_until = [this](TextPosition pos, auto predicate) -> TextPosition {
		pos = previous_position_after(pos);
		while (contains(pos)) {
			if (predicate(code_point_at(pos)))
				break;
			pos = previous_position_after(pos);
		}
		return pos;
	};

	if (start_was_alpha) {
		return consume_until(position, [this](auto p) {
			return !::isalnum(p);
		});
	}
	
	return consume_until(position, [this](auto p) {
		return ::isalnum(p) || p == '_' || p == '-' || p == '(' || p == ')' ||
			p == '[' || p == ']' || p == '{' || p == '}' || p == '.'  || p == ':' ||
			p == ';' || p == '#';
	});
}

bool yui::layout::TextDocument::contains(const TextPosition& position) const
{
	return range_for_entire_document().contains(position);
}

void yui::layout::TextDocument::notify_did_insert_line(uint32_t index)
{
	if (m_client) m_client->text_document_did_insert_line(index);	
}

void yui::layout::TextDocument::notify_did_remove_line(uint32_t index)
{
	if (m_client) m_client->text_document_did_remove_line(index);	
}

void yui::layout::TextDocument::notify_did_change()
{
	if (m_client) m_client->text_document_did_change();
}

yui::layout::EditorEngine::EditorEngine(EditorEngineType type)
	: m_type(type), m_document(this)
{
}

bool yui::layout::EditorEngine::handle_input(int code_point)
{
	if (has_selection()) {
		m_caret = text_document().erase_range(m_selection);
		m_selection = {};
		editor_caret_did_change();
	}
	
	m_caret = text_document().insert(m_caret, code_point);
	editor_caret_did_change();
	return false;
}

bool yui::layout::EditorEngine::handle_key_down(int key, int scan, int mods)
{
	/* Input (i.e. ctrl + c) */

	// Copy
	if (key == GLFW_KEY_C && mods & GLFW_MOD_CONTROL && m_clipboard) {
		auto content = text_document().text_in_range(m_selection);
		printf("Copy: %s\n", content.to_byte_string().c_str());
		m_clipboard->set_content(content);
		return true;
	}

	// Cut
	if (key == GLFW_KEY_X && mods & GLFW_MOD_CONTROL && m_clipboard) {
		TextRange range{};
		if (has_selection()) {
			range = m_selection;
			m_selection = {};
		}
		else {
			range = text_document().range_for_entire_line(m_caret.line());
		}

		auto content = text_document().text_in_range(range);
		m_clipboard->set_content(content);
		printf("Cut: %s\n", content.to_byte_string().c_str());
		m_caret = text_document().erase_range(range);
		editor_caret_did_change();
		return true;
	}
	
	// Paste
	if (key == GLFW_KEY_V && mods & GLFW_MOD_CONTROL && m_clipboard) {
		auto content = m_clipboard->content();
		printf("Clipboard: %s\n", content.to_byte_string().c_str());
		m_caret = text_document().insert(m_caret, content);
		editor_caret_did_change();
		return true;
	}

	/* Text selection */

	// Move selection 1 character right.
	if (key == GLFW_KEY_RIGHT && mods & GLFW_MOD_SHIFT) {
		if (!has_selection()) {
			m_selection.set_start(m_caret);
			m_selection.set_end(m_caret);
		}

		m_selection.set_end(text_document().next_position_after(m_selection.end()));
		printf(
			"Selection: { %d, %d } { %d, %d }\n", m_selection.start().line(), m_selection.start().column(),
			m_selection.end().line(), m_selection.end().column()
		);

		// Move caret to the right
		m_caret = text_document().next_position_after(m_caret);
		editor_caret_did_change();
		return true;
	}
	// Move selection 1 character left
	if (key == GLFW_KEY_LEFT && mods & GLFW_MOD_SHIFT) {
		if (!has_selection()) {
			m_selection.set_start(m_caret);
			m_selection.set_end(m_caret);
		}

		m_selection.set_end(text_document().previous_position_after(m_selection.end()));
		printf(
			"Selection: { %d, %d } { %d, %d }\n", m_selection.start().line(), m_selection.start().column(),
			m_selection.end().line(), m_selection.end().column()
		);

		// Move caret to the left
		m_caret = text_document().previous_position_after(m_caret);
		editor_caret_did_change();
		return true;
	}

	// Move selection down
	if (key == GLFW_KEY_DOWN && mods & GLFW_MOD_SHIFT && is_multi_line()) {
		if (!has_selection()) {
			m_selection.set_start(m_caret);
			m_selection.set_end(m_caret);
		}

		m_caret = text_document().next_position_down(m_caret);
		editor_caret_did_change();

		m_selection.set_end(m_caret);
		editor_selection_did_change();
		return true;
	}

	// Move selection up
	if (key == GLFW_KEY_UP && mods & GLFW_MOD_SHIFT && is_multi_line()) {
		if (!has_selection()) {
			m_selection.set_start(m_caret);
			m_selection.set_end(m_caret);
		}

		m_caret = text_document().next_position_up(m_caret);
		editor_caret_did_change();

		m_selection.set_end(m_caret);
		editor_selection_did_change();
		return true;
	}

	// Select all
	if (key == GLFW_KEY_A && mods & GLFW_MOD_CONTROL) {
		m_selection = text_document().range_for_entire_document();
		m_caret = text_document().end_of_line(text_document().line_count() - 1);
		editor_selection_did_change();
	}

	/* Caret control */
	if (key == GLFW_KEY_RIGHT) {
		if (mods & GLFW_MOD_CONTROL) {
			// Move to next word
			m_caret = text_document().next_word_break_after(m_caret);
		}
		else {
			m_caret = text_document().next_position_after(m_caret);
		}

		m_selection = {};
		editor_caret_did_change();
		return true;
	}
	if (key == GLFW_KEY_LEFT) {
		if (mods & GLFW_MOD_CONTROL) {
			// Move to next word
			m_caret = text_document().previous_word_break_after(m_caret);
		}
		else {
			m_caret = text_document().previous_position_after(m_caret);
		}
		
		m_selection = {};
		editor_caret_did_change();
		return true;
	}
	if (key == GLFW_KEY_DOWN && is_multi_line()) {
		m_selection = {};
		m_caret = text_document().next_position_down(m_caret);
		editor_caret_did_change();
		return true;
	}
	if (key == GLFW_KEY_UP && is_multi_line()) {
		m_selection = {};
		m_caret = text_document().next_position_up(m_caret);
		editor_caret_did_change();
		return true;
	}
	
	/* Erase & Text */
	if (key == GLFW_KEY_BACKSPACE) {
		if (has_selection()) {
			m_caret = text_document().erase_range(m_selection);
			m_selection = {};
		}
		else {
			m_caret = text_document().erase(m_caret);
		}
		
		editor_caret_did_change();
		return true;
	}

	if (key == GLFW_KEY_ENTER) {
		auto range = text_document().range_for_entire_line(m_caret.line());

		if (m_caret.column() != range.end().column()) {
			range.start().set_column(m_caret.column());

			auto text_for_next_line = text_document().text_in_range(range);
			m_caret = text_document().erase_range(range);
			editor_caret_did_change();

			m_document.insert_line_after(m_caret.line(), std::move(text_for_next_line));
			m_caret = text_document().next_position_after(m_caret);
			editor_caret_did_change();
			return true;
		}
		
		m_document.insert_line_after(m_caret.line());
		m_caret = text_document().next_position_after(m_caret);
		editor_caret_did_change();
		return true;
	}

	if (key == GLFW_KEY_TAB) {
		for (auto i = 0u; i < 4; ++i) {
			m_caret = text_document().insert(m_caret, ' ');
			editor_caret_did_change();
		}
		return true;
	}
	
	return false;
}

bool yui::layout::EditorEngine::handle_key_up(int key, int scan, int mods)
{
	return false;
}

void yui::layout::EditorEngine::attach_clipboard(Clipboard& clipboard)
{
	m_clipboard = &clipboard;
}
