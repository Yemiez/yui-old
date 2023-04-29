#include "StylesheetParser.h"
#include <utility>
#include "../Util.h"

yui::StylesheetParseError::StylesheetParseError(StylesheetToken tok, std::string msg)
        : m_token(std::move(tok)), m_message(std::move(msg)) {}

yui::StylesheetParser::StylesheetParser(const StylesheetLexer &lexer)
        : m_reader(lexer.tokens()) {
    parse();
}

void yui::StylesheetParser::parse() {
    while (!m_reader.reached_eof()) {
        auto return_state = ParseState::Error;
        switch (m_parser_state) {
        case ParseState::ExpectingSelectorPart:
            return_state = parse_expecting_selector_part();
            break;

        case ParseState::ExpectingProperties:
            return_state = parse_expecting_properties();
            break;
        case ParseState::Error:
            break;
        }

        if (return_state == ParseState::Error) {
            continue;
        } else {
            m_parser_state = return_state;
        }
    }
}

yui::StylesheetParser::ParseState yui::StylesheetParser::parse_expecting_selector_part() {
    const auto &token = m_reader.consume();

    if (token.is_comma()) {
        if (!m_current_simple.parts().empty()) {
            m_current_selector.append(std::move(m_current_simple));
        }

        if (!m_current_selector.complex_selectors().empty()) {
            m_current_selectors.emplace_back(std::move(m_current_selector));
        }
    } else if (token.is_relation_operator() && m_current_selector.empty()) {
        emit_error(token, "Relation operator not allowed before selector part (id, class, etc)");
        return ParseState::Error;
    } else if (token.is_relation_operator()) {
        m_current_simple.set_relation(
                token.is_plus()
                ? Selector::SimpleSelector::Relation::AdjacentSibling
                : Selector::SimpleSelector::Relation::ImmediateChild
        );
    } else if (token.is_compound_delimiter()) {
        if (!m_current_simple.parts().empty()) {
            m_current_selector.append(std::move(m_current_simple));
        }
    } else if (token.is_any_selector()) {
        if (!m_current_selector.complex_selectors().empty()
                && m_current_simple.relation() == Selector::SimpleSelector::Relation::None) {
            m_current_simple.set_relation(Selector::SimpleSelector::Relation::Descendant);
        }

        auto type = Selector::SimpleSelector::Type::Invalid;

        if (token.is_tag_name()) {
            type = Selector::SimpleSelector::Type::TagName;
        } else if (token.is_id_ref()) {
            type = Selector::SimpleSelector::Type::Id;
        } else if (token.is_class_name()) {
            type = Selector::SimpleSelector::Type::Class;
        } else if (token.is_universal()) {
            type = Selector::SimpleSelector::Type::Universal;
        }

        m_current_simple.append(type, token.content());
    } else if (token.is_pseudo_identifier()) {
        if (m_current_simple.parts().empty()) {
            // Error
            emit_error(token, "Cannot have a pseudo class without a specifier first");
            return ParseState::Error;
        }

        if (token.content() == "hover") {
            m_current_simple.set_pseudo_class(
                    Selector::SimpleSelector::PseudoClass::Hover
            );
        } else if (token.content() == "focus") {
            m_current_simple.set_pseudo_class(
                    Selector::SimpleSelector::PseudoClass::Focus
            );
        } else {
            emit_error(token, fmt::format("Unknown pseudo class: {}", token.content()));
        }
    } else if (token.is_brace_open()) {
        if (!m_current_simple.parts().empty()) {
            m_current_selector.append(std::move(m_current_simple));
        }

        if (!m_current_selector.complex_selectors().empty()) {
            m_current_selectors.emplace_back(std::move(m_current_selector));
        }

        m_current_declaration = &m_declarations.emplace_back();
        m_current_declaration->set_selectors(std::move(m_current_selectors));
        m_current_selector.complex_selectors().clear();
        return ParseState::ExpectingProperties;
    } else {
        emit_error(token, "Unknown token when still expecting selector part..");
        return ParseState::Error;
    }

    return ParseState::ExpectingSelectorPart;
}

yui::StylesheetParser::ParseState yui::StylesheetParser::parse_expecting_properties() {
    auto token = m_reader.consume();

    if (token.is_semi_colon()) {
        return ParseState::ExpectingProperties;
    }
    if (token.is_brace_close()) {
        m_current_declaration = nullptr;
        return ParseState::ExpectingSelectorPart;
    }

    if (!token.is_property_name()) {
        emit_error(token, "expected property name!");
        return ParseState::Error;
    }

    auto name = token.content();

    token = m_reader.consume();

    if (!token.is_colon()) {
        emit_error(token, "Expected colon (:) after property name");
        return ParseState::Error;
    }

    token = m_reader.consume();

    if (!token.is_any_property_value()) {
        emit_error(token, "Expected value after property name and colon");
        return ParseState::Error;
    }

    auto value = token.content();

    token = m_reader.peek();
    if (!token.is_semi_colon() && !token.is_brace_close()) {
        emit_error(
                token,
                "The next token after property value is not a semi-colon or closing brace. This may lead to incorrect results."
        );
        return ParseState::Error;
    }

    m_current_declaration->set_property(name, std::move(value));
    return ParseState::ExpectingProperties;
}

void yui::StylesheetParser::emit_error(StylesheetToken tok, std::string msg) {
    m_errors.emplace_back(std::move(tok), std::move(msg));
}
