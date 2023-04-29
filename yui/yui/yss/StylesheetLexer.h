#pragma once
#include <string>
#include <vector>
#include "../Stream.h"

namespace yui {

#define STYLESHEET_TOKEN_ENUMERATOR \
	STYLESHEET_TOKEN_ENUMERATOR_(None, none) \
	STYLESHEET_TOKEN_ENUMERATOR_(Comma, comma) \
	STYLESHEET_TOKEN_ENUMERATOR_(CompoundDelimiter, compound_delimiter) \
	STYLESHEET_TOKEN_ENUMERATOR_(Universal, universal) \
	STYLESHEET_TOKEN_ENUMERATOR_(TagName, tag_name) \
	STYLESHEET_TOKEN_ENUMERATOR_(IdRef, id_ref) \
	STYLESHEET_TOKEN_ENUMERATOR_(ClassName, class_name) \
	STYLESHEET_TOKEN_ENUMERATOR_(PseudoIdentifier, pseudo_identifier) \
	STYLESHEET_TOKEN_ENUMERATOR_(Plus, plus) \
	STYLESHEET_TOKEN_ENUMERATOR_(Greater, greater) \
	STYLESHEET_TOKEN_ENUMERATOR_(BraceOpen, brace_open) \
	STYLESHEET_TOKEN_ENUMERATOR_(PropertyName, property_name) \
	STYLESHEET_TOKEN_ENUMERATOR_(Colon, colon) \
	STYLESHEET_TOKEN_ENUMERATOR_(PropertyValue, property_value) \
	STYLESHEET_TOKEN_ENUMERATOR_(PropertyColorValue, property_color_value) \
	STYLESHEET_TOKEN_ENUMERATOR_(PropertyPixelValue, property_pixel_value) \
	STYLESHEET_TOKEN_ENUMERATOR_(PropertyPercentageValue, property_percentage_value) \
	STYLESHEET_TOKEN_ENUMERATOR_(SemiColon, semi_colon) \
	STYLESHEET_TOKEN_ENUMERATOR_(BraceClose, brace_close) \
	STYLESHEET_TOKEN_ENUMERATOR_(Eof, eof)
	
	enum class StylesheetTokenType
	{
#define STYLESHEET_TOKEN_ENUMERATOR_(x, ...) x,
		STYLESHEET_TOKEN_ENUMERATOR
#undef STYLESHEET_TOKEN_ENUMERATOR_
	};

	class StylesheetToken
	{
	public:
		StylesheetToken() = default;
		StylesheetToken(StreamPosition start, StreamPosition end, std::string content, StylesheetTokenType);
		[[nodiscard]] StreamPosition start_position() const { return m_start_position; }
		[[nodiscard]] StreamPosition end_position() const { return m_end_position; }
		[[nodiscard]] const std::string& content() const { return m_content; }
		[[nodiscard]] StylesheetTokenType type() const { return m_type; }
		static const char* type_to_string(StylesheetTokenType);
		[[nodiscard]] std::string to_string() const;

#define STYLESHEET_TOKEN_ENUMERATOR_(x, y) bool is_##y() const { return m_type == StylesheetTokenType::x; }
		STYLESHEET_TOKEN_ENUMERATOR
#undef STYLESHEET_TOKEN_ENUMERATOR_

		[[nodiscard]] bool is_relation_operator() const { return is_greater() || is_plus(); }
		[[nodiscard]] bool is_any_property_value() const
		{
			return is_property_value() || is_property_color_value() || 
				is_property_pixel_value() || is_property_percentage_value();
		}
		[[nodiscard]] bool is_any_selector() const
		{
			return is_universal() || is_tag_name() || is_id_ref() || is_class_name();
		}
		
	private:
		StreamPosition m_start_position{};
		StreamPosition m_end_position{};
		std::string m_content{};
		StylesheetTokenType m_type{StylesheetTokenType::None};
	};

	class StylesheetLexError
	{
	public:
		StylesheetLexError(StreamPosition, std::string);
		[[nodiscard]] StreamPosition position() const { return m_position; }
		[[nodiscard]] const std::string& message() const { return m_message; }
		[[nodiscard]] std::string to_string() const;
	private:
		StreamPosition m_position{};
		std::string m_message{};
	};

	class StylesheetLexer
	{
	private:
		enum class LexState
		{
			Data,
			Eof,
			Error,
			ExpectingSelectorPart,
			ExpectingTagName,
			ExpectingIdRef,
			ExpectingClassName,
			ExpectingOp,
			ExpectingPseudo,
			InRuleBody,
			ExpectingColon,
			ExpectingPropertyValue,
			ExpectingColor,
			ExpectingNumber,
			ExpectingString,
			ExpectingSemiColon,
		};
	public:
		static const auto ERROR_THRESHOLD = 8;
		explicit StylesheetLexer(std::string);

		[[nodiscard]] bool has_errors() const { return !m_errors.empty(); }
		[[nodiscard]] const std::vector<StylesheetLexError>& errors() const { return m_errors; }

		[[nodiscard]] const std::vector<StylesheetToken>& tokens() const { return m_tokens; }
		
	private:
		void lex();
		LexState lex_data();
		LexState lex_expecting_selector();
		LexState lex_tag_name();
		LexState lex_id_ref();
		LexState lex_class_name();
		LexState lex_op();
		LexState lex_pseudo();
		LexState lex_in_rule_body();
		LexState lex_expecting_colon();
		LexState lex_expecting_property_value();
		LexState lex_expecting_number();
		LexState lex_expecting_color();
		LexState lex_expecting_string();
		LexState lex_expecting_semicolon();
		
		
		void throw_away_until_newln();
		void throw_away_whitespace();

		static bool is_valid_start_character(char);
		static bool is_valid_character(char);
		
		void emit_token(StreamPosition start, StreamPosition end, std::string content, StylesheetTokenType);
		void emit_token(StreamPosition start, StreamPosition end, char character, StylesheetTokenType);
		void emit_error(StreamPosition, std::string);
	private:	
		yui::StreamReader<> m_reader;
		std::vector<StylesheetLexError> m_errors{};
		std::vector<StylesheetToken> m_tokens{};
		LexState m_lex_state{LexState::Data};
	};


}
