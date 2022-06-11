#pragma once
#include <string>
#include <vector>
#include "../Stream.h"

namespace yui {

#define STYLESHEET_TOKEN_ENUMERATOR \
	_STYLESHEET_TOKEN_ENUMERATOR_(None, none) \
	_STYLESHEET_TOKEN_ENUMERATOR_(Comma, comma) \
	_STYLESHEET_TOKEN_ENUMERATOR_(CompoundDelimiter, compound_delimiter) \
	_STYLESHEET_TOKEN_ENUMERATOR_(Universal, universal) \
	_STYLESHEET_TOKEN_ENUMERATOR_(TagName, tag_name) \
	_STYLESHEET_TOKEN_ENUMERATOR_(IdRef, id_ref) \
	_STYLESHEET_TOKEN_ENUMERATOR_(ClassName, class_name) \
	_STYLESHEET_TOKEN_ENUMERATOR_(PseudoIdentifier, pseudo_identifier) \
	_STYLESHEET_TOKEN_ENUMERATOR_(Plus, plus) \
	_STYLESHEET_TOKEN_ENUMERATOR_(Greater, greater) \
	_STYLESHEET_TOKEN_ENUMERATOR_(BraceOpen, brace_open) \
	_STYLESHEET_TOKEN_ENUMERATOR_(PropertyName, property_name) \
	_STYLESHEET_TOKEN_ENUMERATOR_(Colon, colon) \
	_STYLESHEET_TOKEN_ENUMERATOR_(PropertyValue, property_value) \
	_STYLESHEET_TOKEN_ENUMERATOR_(PropertyColorValue, property_color_value) \
	_STYLESHEET_TOKEN_ENUMERATOR_(PropertyPixelValue, property_pixel_value) \
	_STYLESHEET_TOKEN_ENUMERATOR_(PropertyPercentageValue, property_percentage_value) \
	_STYLESHEET_TOKEN_ENUMERATOR_(SemiColon, semi_colon) \
	_STYLESHEET_TOKEN_ENUMERATOR_(BraceClose, brace_close) \
	_STYLESHEET_TOKEN_ENUMERATOR_(Eof, eof)
	
	enum class StylesheetTokenType
	{
#define _STYLESHEET_TOKEN_ENUMERATOR_(x, ...) x,
		STYLESHEET_TOKEN_ENUMERATOR
#undef _STYLESHEET_TOKEN_ENUMERATOR_
	};

	class StylesheetToken
	{
	public:
		StylesheetToken() = default;
		StylesheetToken(StreamPosition start, StreamPosition end, std::string content, StylesheetTokenType);
		StreamPosition start_position() const { return m_start_position; }
		StreamPosition end_position() const { return m_end_position; }
		const std::string& content() const { return m_content; }
		StylesheetTokenType type() const { return m_type; }
		static const char* type_to_string(StylesheetTokenType);
		std::string to_string() const;

#define _STYLESHEET_TOKEN_ENUMERATOR_(x, y) bool is_##y() const { return m_type == StylesheetTokenType::x; }
		STYLESHEET_TOKEN_ENUMERATOR
#undef _STYLESHEET_TOKEN_ENUMERATOR_

		bool is_relation_operator() const { return is_greater() || is_plus(); }
		bool is_any_property_value() const
		{
			return is_property_value() || is_property_color_value() || 
				is_property_pixel_value() || is_property_percentage_value();
		}
		bool is_any_selector() const
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
		StreamPosition position() const { return m_position; }
		const std::string& message() const { return m_message; }
		std::string to_string() const;
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
		StylesheetLexer(std::string);

		bool has_errors() const { return !m_errors.empty(); }
		const std::vector<StylesheetLexError>& errors() const { return m_errors; }

		const std::vector<StylesheetToken>& tokens() const { return m_tokens; }
		
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
