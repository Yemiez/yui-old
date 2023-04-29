#pragma once
#include <string>
#include <vector>
#include "../Stream.h"

namespace yui {

#define DOCUMENT_TOKEN_ENUMERATOR \
    DOCUMENT_TOKEN_ENUMERATOR_(None, none) \
    DOCUMENT_TOKEN_ENUMERATOR_(Eof, eof) \
    DOCUMENT_TOKEN_ENUMERATOR_(Character, character) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagOpen, open_tag_open) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagName, open_tag_name) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagClose, open_tag_close) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagSelfClosing, open_tag_self_closing) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagAttributeName, open_tag_attribute_name) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagAttributeEqual, open_tag_attribute_equal) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagAttributeQuoteStart, open_tag_attribute_quote_start) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagAttributeValueFragment, open_tag_attribute_value_fragment) \
    DOCUMENT_TOKEN_ENUMERATOR_(OpenTagAttributeQuoteEnd, open_tag_attribute_quote_end) \
    DOCUMENT_TOKEN_ENUMERATOR_(ClosingTagOpen, closing_tag_open) \
    DOCUMENT_TOKEN_ENUMERATOR_(ClosingTagName, closing_tag_name) \
    DOCUMENT_TOKEN_ENUMERATOR_(ClosingTagClose, closing_tag_close)

enum class DocumentTokenType {
#define DOCUMENT_TOKEN_ENUMERATOR_(x, y) x,
    DOCUMENT_TOKEN_ENUMERATOR
#undef DOCUMENT_TOKEN_ENUMERATOR_
};

class DocumentToken {
public:
    DocumentToken() = default;
    DocumentToken(const DocumentToken &) = default;
    DocumentToken(DocumentToken &&) = default;
    DocumentToken(StreamPosition start, StreamPosition end, std::string content, DocumentTokenType type);

    [[nodiscard]] StreamPosition start_position() const { return m_start_position; }
    [[nodiscard]] StreamPosition end_position() const { return m_start_position; }
    [[nodiscard]] const std::string &content() const { return m_content; }
    [[nodiscard]] DocumentTokenType type() const { return m_type; }

    // is_x functions
#define DOCUMENT_TOKEN_ENUMERATOR_(x, y) bool is_##y() const { return m_type == DocumentTokenType::x; }
    DOCUMENT_TOKEN_ENUMERATOR
#undef DOCUMENT_TOKEN_ENUMERATOR_

    [[nodiscard]] std::string to_string() const;

    static const char *type_to_string(DocumentTokenType);
private:
    StreamPosition m_start_position{ };
    StreamPosition m_end_position{ };
    std::string m_content{ };
    DocumentTokenType m_type{ };
};

class DocumentLexError {
public:
    DocumentLexError() = default;
    DocumentLexError(const DocumentLexError &) = default;
    DocumentLexError(DocumentLexError &&) = default;
    DocumentLexError(StreamPosition where, std::string message);

    [[nodiscard]] StreamPosition position() const { return m_position; }
    [[nodiscard]] const std::string &message() const { return m_message; }

private:
    StreamPosition m_position{ };
    std::string m_message{ };
};

class DocumentLexer {
public:
    DocumentLexer() = delete;
    DocumentLexer(const DocumentLexer &) = default;
    DocumentLexer(DocumentLexer &&) = default;
    explicit DocumentLexer(std::string);

    [[nodiscard]] bool reached_eof() const;
    DocumentToken next();
    [[nodiscard]] DocumentToken peek() const;

    [[nodiscard]] bool has_error() const { return !m_errors.empty(); }
    [[nodiscard]] const std::vector<DocumentLexError> &errors() const { return m_errors; }
    [[nodiscard]] const std::vector<DocumentToken> &tokens() const { return m_tokens; }

private:
    enum class LexerState {
        Eof,
        Error,
        Data,
        InOpenTag,
        InOpenTagAfterName,
        InOpenTagAfterAttributeName,
        InOpenTagAfterAttributeEqual,
        InOpenTagAfterAttributeQuoteStart,
        InClosingTag,
        InClosingTagAfterName,
    };

    void lex();
    LexerState lex_data_state();
    LexerState lex_in_open_tag();
    LexerState lex_in_open_tag_after_name();
    LexerState lex_in_open_tag_after_attribute_name();
    LexerState lex_in_open_tag_after_attribute_equal();
    LexerState lex_in_open_tag_after_attribute_quote_start();
    LexerState lex_in_closing_tag();
    LexerState lex_in_closing_tag_after_name();

    void emit_token(StreamPosition start, StreamPosition end, std::string, DocumentTokenType);
    void emit_token(StreamPosition start, StreamPosition end, char, DocumentTokenType);
    void emit_error(StreamPosition, std::string);
    LexerState emit_eof();

    static bool valid_tag_name_character(char);
    static bool valid_attribute_name_character(char);
private:
    StreamReader<> m_reader;
    std::vector<DocumentToken> m_tokens{ };
    std::vector<DocumentLexError> m_errors{ };
    uint32_t m_cursor{ 0 };
    LexerState m_state{ LexerState::Data };
};

}
