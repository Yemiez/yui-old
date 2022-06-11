#pragma once
#include <cstdint>
#include <vector>
#include <glm/vec2.hpp>

#include "../Utf8String.h"

namespace yui {
	class Clipboard;
	class Utf8String;
}

namespace yui::layout {

	class EditorEngine;
	enum class EditorEngineType
	{
		SingleLine,
		MultiLine
	};

	class TextPosition
	{
	public:
		TextPosition() = default;
		TextPosition(uint32_t line, uint32_t column)
			: m_line(line), m_column(column)
		{}
		
		auto line() const { return m_line; }
		auto column() const { return m_column; }
		void set_line(uint32_t line) { m_line = line; }
		void set_column(uint32_t column) { m_column = column; }

		bool operator==(const TextPosition& rhs) const { return m_line == rhs.m_line && m_column == rhs.m_column; }
		bool operator!=(const TextPosition& rhs) const { return !(*this == rhs); }
		bool operator<(const TextPosition& rhs) const { return m_line < rhs.m_line || (m_line == rhs.m_line && m_column < rhs.m_column); }
		bool operator<=(const TextPosition& rhs) const { return (*this == rhs) || (*this < rhs); }
		bool operator>(const TextPosition& rhs) const { return m_line > rhs.m_line || (m_line == rhs.m_line && m_column > rhs.m_column); }
		bool operator>=(const TextPosition& rhs) const { return (*this == rhs) || (*this > rhs); }
	private:
		uint32_t m_line{0u};
		uint32_t m_column{0};
	};

	class TextRange
	{
	public:
		TextRange() = default;
		TextRange(TextPosition start, TextPosition end)
			: m_start(start), m_end(end)
		{}
		
		TextRange normalized() const
		{
			auto tmp{*this};
			if (tmp.start() > tmp.end()) {
				std::swap(tmp.m_start, tmp.m_end);
			}
			return tmp;
		}
		TextPosition& start() { return m_start; }
		const TextPosition& start() const { return m_start; }
		const TextPosition& end() const { return m_end; }
		TextPosition& end() { return m_end; }
		void set_start(TextPosition pos) { m_start = pos; }
		void set_end(TextPosition pos) { m_end = pos; }
		void set(TextPosition start, TextPosition end) { m_start = start; m_end = end; }
		bool operator==(const TextRange& rhs) const { return m_start == rhs.m_start && m_end == rhs.m_end; }
		bool operator!=(const TextRange& rhs) const { return !(*this == rhs); }
		bool contains(const TextPosition& position) const { return position >= m_start && position <= m_end; }
		bool is_same_line() const { return m_start.line() == m_end.line(); }
	private:
		TextPosition m_start{};
		TextPosition m_end{};
	};

	class TextDocument;
	class TextDocumentLine
	{
	public:
		TextDocumentLine(TextDocument* document);
		TextDocumentLine(TextDocument* document, Utf8String);

		const Utf8String& text() const { return m_text; }
		Utf8String& text() { return m_text; }
		uint32_t length() const { return m_text.length(); }
		bool empty() const { return m_text.empty(); }
	private:
		TextDocument *m_document{nullptr};
		Utf8String m_text{};
	};
	
	class TextDocument
	{
	public:
		class Client
		{
		public:
			virtual ~Client() = default;
			virtual void text_document_did_insert_line(uint32_t index) {}
			virtual void text_document_did_remove_line(uint32_t index) {}
			virtual void text_document_did_change() {}
			virtual void text_document_did_set_caret(const TextPosition&) {}

			virtual glm::ivec2 document_request_client_text_size(const Utf8String&) { return {}; };
		};
		
	public:
		TextDocument(EditorEngine* editor);

		// Document client.
		void connect_client(Client* client) { m_client = client; }
		void disconnect_client() { m_client = nullptr; }

		const auto* client() const { return m_client; }
		auto* client() { return m_client; }
		
		uint32_t line_count() const { return m_lines.size(); }

		const auto& line(uint32_t idx) const { return m_lines.at(idx); }
		auto& line(uint32_t idx) { return m_lines.at(idx); }
		const auto& lines() const { return m_lines; }
		auto& lines() { return m_lines; }
	public: // Text functions
		void reset_to(const Utf8String& content);
		void append_line(Utf8String content); // with content
		void append_line(); // empty
		void insert_line_before(uint32_t where, Utf8String content); // with content
		void insert_line_before(uint32_t where); // empty
		void insert_line_after(uint32_t where, Utf8String content); // with content
		void insert_line_after(uint32_t where); // empty
		void remove_line(uint32_t where);

		// Insert single code point
		TextPosition insert(const TextPosition& where, int code_point);
		TextPosition insert(const TextPosition& where, const Utf8String& text);
		TextPosition erase(const TextPosition& where);
		TextPosition erase_range(TextRange range);
		
		Utf8String text() const;
		Utf8String text_in_range(TextRange) const;

		uint32_t code_point_at(const TextPosition&) const;

		TextRange range_for_entire_document() const;
		TextRange range_for_entire_line(uint32_t index) const;
		TextPosition start_of_line(uint32_t index) const;
		TextPosition end_of_line(uint32_t index) const;
		TextPosition next_position_after(const TextPosition&) const;
		TextPosition previous_position_after(const TextPosition&) const;
		TextPosition next_position_up(const TextPosition&) const;
		TextPosition next_position_down(const TextPosition&) const;
		TextPosition next_word_break_after(const TextPosition&) const;
		TextPosition previous_word_break_after(const TextPosition&) const;

		bool contains(const TextPosition&) const;
		
	public:
		void notify_did_insert_line(uint32_t);
		void notify_did_remove_line(uint32_t);
		void notify_did_change();
	private:
		Client *m_client{nullptr};
		std::vector<TextDocumentLine> m_lines{};
		EditorEngine *m_editor{nullptr};
	};
	
	class EditorEngine
	{
	public:
		virtual ~EditorEngine() = default;
		EditorEngine() = delete;
		EditorEngine(const EditorEngine&) = delete;
		EditorEngine(EditorEngine&&) = default;
		EditorEngine(EditorEngineType type);

		bool is_single_line() const { return m_type == EditorEngineType::SingleLine; }
		bool is_multi_line() const { return m_type == EditorEngineType::MultiLine; }
		const TextDocument& text_document() const { return m_document; }
		TextDocument& text_document() { return m_document; }
		
		bool handle_input(int code_point);
		bool handle_key_down(int key, int scan, int mods);
		bool handle_key_up(int key, int scan, int mods);

		void attach_clipboard(Clipboard& clipboard);

		virtual void editor_caret_did_change() {}
		virtual void editor_selection_did_change() {}

		bool has_selection() const { return m_selection.start() != m_selection.end(); }
	protected:
		EditorEngineType m_type{EditorEngineType::SingleLine};
		TextDocument m_document;
		TextPosition m_caret{};
		TextRange m_selection{};
		Clipboard *m_clipboard{nullptr};
	};
	
}
