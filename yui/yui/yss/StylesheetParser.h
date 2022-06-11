#pragma once
#include <string>
#include "StylesheetLexer.h"
#include "StylesheetDeclaration.h"
#include "../Stream.h"

namespace yui {
	class StylesheetDeclaration;

	class StylesheetParseError
	{
	public:
		StylesheetParseError() = default;
		StylesheetParseError(const StylesheetParseError&) = default;
		StylesheetParseError(StylesheetParseError&&) = default;
		StylesheetParseError(StylesheetToken, std::string);

		// Which token caused the error?
		const StylesheetToken& token() const { return m_token; }

		// Get the message of the error.
		const std::string& message() const { return m_message; }
		
	private:
		StylesheetToken m_token{};
		std::string m_message{};
	};
	
	class StylesheetParser
	{
	private:
		enum class ParseState
		{
			Error,
			ExpectingSelectorPart,
			ExpectingProperties,
		};
		
	public:
		StylesheetParser(const StylesheetLexer&);

		bool has_errors() const { return !m_errors.empty(); }
		const std::vector<StylesheetParseError>& errors() const { return m_errors; }

		const std::vector<StylesheetDeclaration>& declarations() const { return m_declarations; }
		std::vector<StylesheetDeclaration>& declarations() { return m_declarations; }
	private:
		void parse();
		ParseState parse_expecting_selector_part();
		ParseState parse_expecting_properties();

		void emit_error(StylesheetToken, std::string);
	private:
		StreamReader<std::vector<StylesheetToken>> m_reader;
		std::vector<StylesheetDeclaration> m_declarations{};
		std::vector<StylesheetParseError> m_errors{};
		StylesheetDeclaration *m_current_declaration{nullptr};
		yui::Selector m_current_selector{};
		yui::Selector::SimpleSelector m_current_simple{};
		std::vector<yui::Selector> m_current_selectors{};
		ParseState m_parser_state{ParseState::ExpectingSelectorPart};
	};
	
}
