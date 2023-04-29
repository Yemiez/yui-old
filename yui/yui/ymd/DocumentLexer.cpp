#include "DocumentLexer.h"

#include <algorithm>
#include <sstream>
#include "../Util.h"

yui::DocumentToken::DocumentToken(StreamPosition start, StreamPosition end, std::string content, DocumentTokenType type)
        : m_start_position(start), m_end_position(end), m_content(std::move(content)), m_type(type) {}

std::string yui::DocumentToken::to_string() const {
    std::stringstream ss;
    ss << "DocumentToken{ "
            << "start=" << m_start_position.to_string() << ", "
            << "end=" << m_end_position.to_string() << ", "
            << "type=" << type_to_string(m_type) << ", "
            << "content=" << yui::escape_content(m_content) << " }";
    return ss.str();
}

const char *yui::DocumentToken::type_to_string(DocumentTokenType type) {
    switch (type) {
#define DOCUMENT_TOKEN_ENUMERATOR_(x, y) case DocumentTokenType::x: return #x;
    DOCUMENT_TOKEN_ENUMERATOR
#undef DOCUMENT_TOKEN_ENUMERATOR_
    }

    return "none";
}

yui::DocumentLexError::DocumentLexError(StreamPosition where, std::string message)
        : m_position(where), m_message(std::move(message)) {}

yui::DocumentLexer::DocumentLexer(std::string buffer)
        : m_reader(std::move(buffer)) {
    lex();
}

bool yui::DocumentLexer::reached_eof() const {
    return m_cursor >= m_tokens.size();
}

yui::DocumentToken yui::DocumentLexer::next() {
    if (reached_eof()) { return { }; }
    return m_tokens[m_cursor++];
}
yui::DocumentToken yui::DocumentLexer::peek() const {
    if (reached_eof()) { return { }; }
    return m_tokens[m_cursor];
}

void yui::DocumentLexer::lex() {
    while (!m_reader.reached_eof()) {
        auto return_state{ LexerState::Data };
        switch (m_state) {
        case LexerState::Data:
            return_state = lex_data_state();
            break;

            // Open tag stuff
        case LexerState::InOpenTag:
            return_state = lex_in_open_tag();
            break;
        case LexerState::InOpenTagAfterName:
            return_state = lex_in_open_tag_after_name();
            break;
        case LexerState::InOpenTagAfterAttributeName:
            return_state = lex_in_open_tag_after_attribute_name();
            break;
        case LexerState::InOpenTagAfterAttributeEqual:
            return_state = lex_in_open_tag_after_attribute_equal();
            break;
        case LexerState::InOpenTagAfterAttributeQuoteStart:
            return_state = lex_in_open_tag_after_attribute_quote_start();
            break;

        case LexerState::InClosingTag:
            return_state = lex_in_closing_tag();
            break;

        case LexerState::InClosingTagAfterName:
            return_state = lex_in_closing_tag_after_name();
            break;

        }

        if (return_state == LexerState::Eof) {
            break;
        } else if (return_state == LexerState::Error) {
            continue; // Continue...
        } else {
            m_state = return_state;
        }
    }
}

yui::DocumentLexer::LexerState yui::DocumentLexer::lex_data_state() {
    if (m_reader.reached_eof()) {
        return emit_eof();
    }

    const auto start = m_reader.position();
    const auto character = m_reader.consume();

    if (character == 0) {
        emit_error(start, "Unexpected null character");
        emit_token(start, m_reader.position(), character, DocumentTokenType::Character);
        return LexerState::Error;
    } else if (character == '<' && m_reader.peek() == '/') {
        // Close
        m_reader.consume();
        emit_token(start, m_reader.position(), "</", DocumentTokenType::ClosingTagOpen);
        return LexerState::InClosingTag;
    } else if (character == '<') {
        // Open
        emit_token(start, m_reader.position(), "<", DocumentTokenType::OpenTagOpen);
        return LexerState::InOpenTag;
    }

    // Else
    emit_token(start, m_reader.position(), character, DocumentTokenType::Character);
    return LexerState::Data; // Continue in data mode.
}

yui::DocumentLexer::LexerState yui::DocumentLexer::lex_in_open_tag() {
    const auto start = m_reader.position();
    auto character = m_reader.consume();

    if (!valid_tag_name_character(character)) {
        return LexerState::Error;
    }

    std::string buffer{ };
    buffer.reserve(16);
    do {
        buffer.push_back(character);
        character = m_reader.consume();
    }
    while (valid_tag_name_character(character) && !m_reader.reached_eof());
    m_reader.unwind();

    if (m_reader.reached_eof()) {
        return emit_eof();
    }

    if (character != '>' && character != ' ') {
        emit_error(
                m_reader.position(),
                "Unexpected character encountered while reading tag name (this may produce undefined tokens after tag name)"
        );
    }

    emit_token(start, m_reader.position(), std::move(buffer), DocumentTokenType::OpenTagName);
    return LexerState::InOpenTagAfterName;
}

yui::DocumentLexer::LexerState yui::DocumentLexer::lex_in_open_tag_after_name() {
    auto start = m_reader.position();
    auto character = m_reader.consume();

    while (!m_reader.reached_eof() && ::isspace(std::clamp(character, static_cast<char>(0), static_cast<char>(127)))) {
        start = m_reader.position();
        character = m_reader.consume();
    }

    if (m_reader.reached_eof()) {
        emit_error(m_reader.position(), "Unexpected eof");
        return emit_eof();
    }

    if (character == '/' && m_reader.peek(1) == '>') {
        m_reader.consume();
        emit_token(start, m_reader.position(), "/>", DocumentTokenType::OpenTagSelfClosing);
        return LexerState::Data;
    }

    if (character == '>') {
        emit_token(start, m_reader.position(), character, DocumentTokenType::OpenTagClose);
        return LexerState::Data;
    }

    // Only possible remaining thing to parse would be attributes
    if (!valid_attribute_name_character(character)) {
        emit_error(m_reader.position(), "Unexpected character when trying to read tag attribute");
        return LexerState::Error;
    }

    start = m_reader.position();
    std::string buffer{ };
    buffer.reserve(16);
    do {
        buffer.push_back(character);
        character = m_reader.consume();
    }
    while (valid_attribute_name_character(character) && !m_reader.reached_eof());
    m_reader.unwind();

    if (m_reader.reached_eof()) {
        return emit_eof();
    }

    emit_token(start, m_reader.position(), buffer, DocumentTokenType::OpenTagAttributeName);
    return LexerState::InOpenTagAfterAttributeName;
}

yui::DocumentLexer::LexerState yui::DocumentLexer::lex_in_open_tag_after_attribute_name() {
    auto start = m_reader.position();
    auto character = m_reader.consume();

    while (!m_reader.reached_eof() && ::isspace(std::clamp(character, static_cast<char>(0), static_cast<char>(127)))) {
        start = m_reader.position();
        character = m_reader.consume();
    }

    if (m_reader.reached_eof()) {
        return emit_eof(); // bad boy
    }

    if (character != '=') {
        emit_error(m_reader.position(), "Unexpected character after attribute name");
        return LexerState::Error;
    }

    emit_token(start, m_reader.position(), character, DocumentTokenType::OpenTagAttributeEqual);
    return LexerState::InOpenTagAfterAttributeEqual;
}

yui::DocumentLexer::LexerState yui::DocumentLexer::lex_in_open_tag_after_attribute_equal() {
    auto start = m_reader.position();
    auto character = m_reader.consume();

    while (!m_reader.reached_eof() && ::isspace(std::clamp(character, static_cast<char>(0), static_cast<char>(127)))) {
        m_reader.consume();
        start = m_reader.position();
        character = m_reader.consume();
    }

    if (m_reader.reached_eof()) {
        return emit_eof(); // bad boy
    }

    if (character != '"') {
        emit_error(m_reader.position(), "Unexpected character after attribute name");
        return LexerState::Error;
    }

    emit_token(start, m_reader.position(), character, DocumentTokenType::OpenTagAttributeQuoteStart);
    return LexerState::InOpenTagAfterAttributeQuoteStart;
}

yui::DocumentLexer::LexerState yui::DocumentLexer::lex_in_open_tag_after_attribute_quote_start() {
    const auto start = m_reader.position();
    const auto character = m_reader.consume();

    if (character == '\\') {
        auto result{ '\\' };
        const auto escapee = m_reader.consume();
        switch (escapee) {
        case '\\':
            result = '\\';
            break;
        case '"':
            result = '"';
            break;
        }

        emit_token(start, m_reader.position(), result, DocumentTokenType::OpenTagAttributeValueFragment);
        return LexerState::InOpenTagAfterAttributeQuoteStart;
    }

    if (character == '"') {
        // We reached the end!
        emit_token(start, m_reader.position(), character, DocumentTokenType::OpenTagAttributeQuoteEnd);
        return LexerState::InOpenTagAfterName;
    }

    emit_token(start, m_reader.position(), character, DocumentTokenType::OpenTagAttributeValueFragment);
    return LexerState::InOpenTagAfterAttributeQuoteStart;
}

yui::DocumentLexer::LexerState yui::DocumentLexer::lex_in_closing_tag() {
    const auto start = m_reader.position();
    auto character = m_reader.consume();

    if (!valid_tag_name_character(character)) {
        return LexerState::Error;
    }

    std::string buffer{ };
    buffer.reserve(16);
    do {
        buffer.push_back(character);
        character = m_reader.consume();
    }
    while (valid_tag_name_character(character) && !m_reader.reached_eof());
    m_reader.unwind();

    if (m_reader.reached_eof()) {
        return emit_eof();
    }

    emit_token(start, m_reader.position(), std::move(buffer), DocumentTokenType::ClosingTagName);
    return LexerState::InClosingTagAfterName;
}

yui::DocumentLexer::LexerState yui::DocumentLexer::lex_in_closing_tag_after_name() {
    const auto start = m_reader.position();
    const auto character = m_reader.consume();

    if (character != '>') {
        emit_error(
                m_reader.position(),
                "Unexpected character encountered while reading closing tag name (this may produce undefined tokens after tag name)"
        );
        return LexerState::Error;
    }

    emit_token(start, m_reader.position(), character, DocumentTokenType::ClosingTagClose);
    return LexerState::Data;
}

void yui::DocumentLexer::emit_token(StreamPosition start, StreamPosition end, std::string s, DocumentTokenType t) {
    m_tokens.emplace_back(start, end, std::move(s), t);
}

void yui::DocumentLexer::emit_token(StreamPosition start, StreamPosition end, char c, DocumentTokenType t) {
    std::string s;
    s.push_back(c);
    m_tokens.emplace_back(start, end, s, t);
}

void yui::DocumentLexer::emit_error(StreamPosition s, std::string m) {
    m_errors.emplace_back(s, std::move(m));
}

yui::DocumentLexer::LexerState yui::DocumentLexer::emit_eof() {
    emit_error(m_reader.position(), "Unexpected eof");
    emit_token(m_reader.position(), m_reader.position(), 0, DocumentTokenType::Eof);
    return LexerState::Eof;
}

bool yui::DocumentLexer::valid_tag_name_character(char c) {
    return (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_';
}

bool yui::DocumentLexer::valid_attribute_name_character(char c) {
    return (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_';
}

