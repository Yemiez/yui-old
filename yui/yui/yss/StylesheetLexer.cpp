#include "StylesheetLexer.h"


#include <sstream>
#include <utility>

#include "../Util.h"

yui::StylesheetToken::StylesheetToken(StreamPosition start, StreamPosition end, std::string content, StylesheetTokenType type)
	: m_start_position(start), m_end_position(end), m_content(std::move(content)), m_type(type)
{}

const char* yui::StylesheetToken::type_to_string(StylesheetTokenType type)
{
	switch (type) {
#define _STYLESHEET_TOKEN_ENUMERATOR_(x, ...) case StylesheetTokenType::x: return #x;
		STYLESHEET_TOKEN_ENUMERATOR
#undef _STYLESHEET_TOKEN_ENUMERATOR_
	}

	return "none";
}

std::string yui::StylesheetToken::to_string() const
{
	std::stringstream ss;
	ss << "StylesheetToken { "
		<< "start=" << m_start_position.to_string() << ", "
		<< "end=" << m_end_position.to_string() << ", "
		<< "type=" << type_to_string(m_type) << ", "
		<< "content=" << yui::escape_content(m_content) << " }";
	return ss.str();
}

yui::StylesheetLexError::StylesheetLexError(StreamPosition pos, std::string msg)
	: m_position(pos), m_message(std::move(msg))
{}

std::string yui::StylesheetLexError::to_string() const
{
	std::stringstream ss;
	ss << "Error at " << m_position.to_string() << ": " << m_message;
	return ss.str();
}

yui::StylesheetLexer::StylesheetLexer(std::string buffer)
	: m_reader(std::move(buffer))
{
	lex();
}

void yui::StylesheetLexer::lex()
{
	while (!m_reader.reached_eof()) {
		auto return_state = LexState::Error;
		switch (m_lex_state) {
		case LexState::Data:
			return_state = lex_data();
			break;

		case LexState::ExpectingSelectorPart:
			return_state = lex_expecting_selector();
			break;

		case LexState::ExpectingTagName:
			return_state = lex_tag_name();
			break;

		case LexState::ExpectingIdRef:
			return_state = lex_id_ref();
			break;

		case LexState::ExpectingClassName:
			return_state = lex_class_name();
			break;
		case LexState::ExpectingOp:
			return_state = lex_op();
			break;
		case LexState::ExpectingPseudo:
			return_state = lex_pseudo();
			break;

		case LexState::InRuleBody:
			return_state = lex_in_rule_body();
			break;

		case LexState::ExpectingColon:
			return_state = lex_expecting_colon();
			break;

		case LexState::ExpectingPropertyValue:
			return_state = lex_expecting_property_value();
			break;

		case LexState::ExpectingNumber:
			return_state = lex_expecting_number();
			break;
		case LexState::ExpectingColor:
			return_state = lex_expecting_color();
			break;
		case LexState::ExpectingString:
			return_state = lex_expecting_string();
			break;
		case LexState::ExpectingSemiColon:
			return_state = lex_expecting_semicolon();
			break;
		}

		if (return_state == LexState::Eof) {
			break;
		}
		else if (return_state == LexState::Error) {
			if (m_errors.size() > ERROR_THRESHOLD) {
				emit_error(m_reader.position(), "Too many errors encountered, stopping.");
				break;
			}
		}
		else {
			m_lex_state = return_state;
		}
	}
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_data()
{
	if (m_reader.peek() == '/' && m_reader.peek(1) == '/') {
		throw_away_until_newln();
		return LexState::Data;
	}

	if (::isspace(m_reader.peek())) {
		const auto start = m_reader.position();
		throw_away_whitespace();
		emit_token(start, m_reader.position(), ' ', StylesheetTokenType::CompoundDelimiter);
		return LexState::Data;
	}

	return LexState::ExpectingSelectorPart;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_expecting_selector()
{
	const auto start = m_reader.position();
	const auto character = m_reader.peek();

	if (character == ',') {
		m_reader.consume();
		emit_token(start, m_reader.position(), ',', StylesheetTokenType::Comma);
		return LexState::Data;
	}
	if (character == '#') {
		return LexState::ExpectingIdRef;
	}
	if (character == '.') {
		return LexState::ExpectingClassName;
	}
	if (character == '>' || character == '+') {
		return LexState::ExpectingOp;
	}
	if (character == ':') {
		return LexState::ExpectingPseudo;
	}
	if (character == '*') {
		m_reader.consume();
		emit_token(start, m_reader.position(), character, StylesheetTokenType::Universal);
		return LexState::Data;
	}
	if (is_valid_start_character(character)) {
		return LexState::ExpectingTagName;
	}
	if (character == '{') {
		m_reader.consume();
		emit_token(start, m_reader.position(), character, StylesheetTokenType::BraceOpen);
		return LexState::InRuleBody;
	}

	return LexState::Error;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_tag_name()
{
	const auto start = m_reader.position();
	std::string tag_name{};
	m_reader.consume_until_false([](auto c) {
		return yui::StylesheetLexer::is_valid_character(c);
	}, tag_name);

	emit_token(start, m_reader.position(), std::move(tag_name), StylesheetTokenType::TagName);
	return LexState::Data;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_id_ref()
{
	const auto start = m_reader.position();
	m_reader.consume(); // consume #
	std::string id_ref{};
	m_reader.consume_until_false([](auto c) {
		return yui::StylesheetLexer::is_valid_character(c);
	}, id_ref);

	emit_token(start, m_reader.position(), std::move(id_ref), StylesheetTokenType::IdRef);
	return LexState::Data;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_class_name()
{
	const auto start = m_reader.position();
	m_reader.consume(); // consume .
	std::string class_name{};
	m_reader.consume_until_false([](auto c) {
		return yui::StylesheetLexer::is_valid_character(c);
	}, class_name);

	emit_token(start, m_reader.position(), std::move(class_name), StylesheetTokenType::ClassName);
	return LexState::Data;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_op()
{
	const auto start = m_reader.position();
	const auto ch = m_reader.consume();
	emit_token(start, m_reader.position(), ch, ch == '>' ? StylesheetTokenType::Greater : StylesheetTokenType::Plus);
	return LexState::Data;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_pseudo()
{
	const auto start = m_reader.position();
	m_reader.consume(); // consume :
	std::string pseudo{};
	m_reader.consume_until_false([](auto c) {
		return yui::StylesheetLexer::is_valid_character(c);
	}, pseudo);

	emit_token(start, m_reader.position(), std::move(pseudo), StylesheetTokenType::PseudoIdentifier);
	return LexState::Data;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_in_rule_body()
{
	throw_away_whitespace();
	const auto start = m_reader.position();

	if (m_reader.peek() == '}') {
		emit_token(start, m_reader.position(), m_reader.consume(), StylesheetTokenType::BraceClose);
		return LexState::Data;
	}
	
	if (!is_valid_start_character(m_reader.peek())) {
		emit_error(m_reader.position(), yui::fmt("Invalid start character for property name %c", m_reader.peek()));
		return LexState::Error;
	}

	std::string property_name{};
	m_reader.consume_until_false([](auto c) {
		return yui::StylesheetLexer::is_valid_character(c);
	}, property_name);

	emit_token(start, m_reader.position(), std::move(property_name), StylesheetTokenType::PropertyName);
	return LexState::ExpectingColon;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_expecting_colon()
{
	throw_away_whitespace();

	const auto start = m_reader.position();
	const auto ch = m_reader.consume(); // Always consume this character.
	if (ch != ':') {
		return LexState::Error;
	}

	emit_token(start, m_reader.position(), ch, StylesheetTokenType::Colon);
	return LexState::ExpectingPropertyValue;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_expecting_property_value()
{
	throw_away_whitespace();

	const auto ch = m_reader.peek();
	if (ch == '#') {
		return LexState::ExpectingColor;
	}
	if (::isdigit(ch)) {
		return LexState::ExpectingNumber;
	}
	
	return LexState::ExpectingString;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_expecting_number()
{
	const auto start = m_reader.position();
	std::string part{};
	m_reader.consume_until_false([](auto c) {
		return c >= '0' && c <= '9';
	}, part);

	auto type = StylesheetTokenType::PropertyPixelValue;
	const auto ch = m_reader.peek();
	if (ch == 'p' && m_reader.peek(1) == 'x') {
		part.append("px");
		m_reader.consume();
		m_reader.consume();
	}
	else if (ch == '%') {
		part.push_back('%');
		m_reader.consume();
		type = StylesheetTokenType::PropertyPercentageValue;
	}
	else if (ch != ';' && ch != '}' && !::isspace(ch)) {
		emit_error(m_reader.position(), yui::fmt("Unknown character after number value: %d", static_cast<int>(ch)));
	}

	emit_token(start, m_reader.position(), part, type);
	return LexState::ExpectingSemiColon;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_expecting_color()
{
	const auto start = m_reader.position();
	std::string value{};
	m_reader.consume_until_false([](auto c) {
		return yui::StylesheetLexer::is_valid_character(c) || c == '#';
	}, value);

	emit_token(start, m_reader.position(), std::move(value), StylesheetTokenType::PropertyColorValue);
	return LexState::ExpectingSemiColon;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_expecting_string()
{
	const auto start = m_reader.position();
	std::string value{};
	m_reader.consume_until_false([](auto c) {
		return c != ';';
	}, value);

	emit_token(start, m_reader.position(), std::move(value), StylesheetTokenType::PropertyValue);
	return LexState::ExpectingSemiColon;
}

yui::StylesheetLexer::LexState yui::StylesheetLexer::lex_expecting_semicolon()
{
	throw_away_whitespace();

	auto start = m_reader.position();
	const auto ch = m_reader.consume();

	if (ch == ';') {
		emit_token(start, m_reader.position(), ch, StylesheetTokenType::SemiColon);
		return LexState::InRuleBody;
	}
	else if (ch == '}') {
		emit_token(start, m_reader.position(), ch, StylesheetTokenType::BraceClose);
		return LexState::Data;
	}
	
	// error
	emit_error(m_reader.position(),  yui::fmt("Unknown character when expecting semicolon or closing brace: %d %c", static_cast<int>(ch), ch));
	return LexState::Error;
}

void yui::StylesheetLexer::throw_away_until_newln()
{
	m_reader.consume_until_false([](auto c) {
		return c != '\n';
	});
}

void yui::StylesheetLexer::throw_away_whitespace()
{
	m_reader.consume_until_false([](auto c) {
		return ::isspace(c);
	});
}

bool yui::StylesheetLexer::is_valid_start_character(char c)
{
	return (c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		c == '_';
}

bool yui::StylesheetLexer::is_valid_character(char c)
{
	return (c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		(c >= '0' && c <= '9') ||
		c == '_' || c == '-';
}

void yui::StylesheetLexer::emit_token(StreamPosition start, StreamPosition end, std::string content, StylesheetTokenType type)
{
	m_tokens.emplace_back(StylesheetToken{start, end, std::move(content), type});
}

void yui::StylesheetLexer::emit_token(StreamPosition start, StreamPosition end, char character, StylesheetTokenType type)
{
	std::string s;
	s.push_back(character);
	emit_token(start, end, std::move(s), type);
}

void yui::StylesheetLexer::emit_error(StreamPosition pos, std::string msg)
{
	m_errors.emplace_back(StylesheetLexError{pos, std::move(msg)});
}
