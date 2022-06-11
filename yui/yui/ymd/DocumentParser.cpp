#include "DocumentParser.h"
#include "DocumentLexer.h"
#include "DocumentNode.h"
#include "Node.h"
#include "../Util.h"

yui::DocumentParseError::DocumentParseError(DocumentToken token, std::string msg)
	: m_token(token), m_message(std::move(msg))
{}

yui::DocumentParser::DocumentParser(DocumentLexer lexer)
	: m_lexer(std::move(lexer))
{
	construct_tree();
}

yui::DocumentNode* yui::DocumentParser::release_document()
{
	auto *tmp = m_document;
	m_document = nullptr;
	return tmp;
}

bool yui::DocumentParser::is_self_closing(const std::string& cs)
{
	static std::map<std::string, bool> elements {
		{"area", true},
		{"base", true},
		{"br", true},
		{"col", true},
		{"embed", true},
		{"hr", true},
		{"img", true},
		{"input", true},
		{"textarea", true},
		{"link", true},
		{"meta", true},
		{"source", true},
		{"track", true},
		{"wbr", true},
	};
	return elements.contains(cs);
}

void yui::DocumentParser::construct_tree()
{
	// Create document
	m_document = new DocumentNode();
	m_working_stack.push(m_document); // Doc should always be first in and last out.
	m_history.emplace_back(m_document);

	auto exit_out_of_node = [&]() {
		if (m_working_stack.empty()) {
			// We've reached the end!
			will_change_state(ParserState::Done);
			change_state(ParserState::Done);
		}
		else {
			will_change_state(ParserState::InTagBody);
			change_state(ParserState::InTagBody);
		}
	};
	
	auto close_current_tag = [&]() {
		m_history.emplace_back(m_working_stack.top());
		m_working_stack.pop();
		exit_out_of_node();
	};
	
	while (!m_lexer.reached_eof()) {
		auto token = m_lexer.next();

		switch (token.type()) {
			case yui::DocumentTokenType::None:
		case yui::DocumentTokenType::Eof:
			will_change_state(ParserState::Done);
			change_state(ParserState::Done);
			break;
			
		case yui::DocumentTokenType::Character:
			if (m_parser_state == ParserState::InTagBody) {
				// Text fragment
				m_buffer.append(token.content());
			}
			break;
		case yui::DocumentTokenType::OpenTagOpen: // <
			break;
		case yui::DocumentTokenType::OpenTagName: // div
			{
				will_change_state(ParserState::AfterTagName);
				append_new_node(token);
				change_state(ParserState::AfterTagName);
			}
			break;
		case yui::DocumentTokenType::OpenTagSelfClosing:
			{
				close_current_tag();
			}
			break;
		case yui::DocumentTokenType::OpenTagClose: // >
			{
				if (is_self_closing(m_working_stack.top()->tag_name())) {
					close_current_tag();
				}
				else {
					will_change_state(ParserState::InTagBody);
					change_state(ParserState::InTagBody);	
				}
			}
			break;
		case yui::DocumentTokenType::OpenTagAttributeName:
			{
				will_change_state(ParserState::InTagAttributeDeclaration);
				m_current_attribute = token.content();
				change_state(ParserState::InTagAttributeDeclaration);
			}
			break;
		case yui::DocumentTokenType::OpenTagAttributeEqual: 
			break;
		case yui::DocumentTokenType::OpenTagAttributeQuoteStart: 
			break;
		case yui::DocumentTokenType::OpenTagAttributeValueFragment:
			m_buffer.append(token.content());
			break;
		case yui::DocumentTokenType::OpenTagAttributeQuoteEnd:
			{
				working_parent()->set_attribute(m_current_attribute, m_buffer);
				will_change_state(ParserState::AfterTagName);
				change_state(ParserState::AfterTagName);
			}
			break;
		case yui::DocumentTokenType::ClosingTagOpen: // </
			{
				will_change_state(ParserState::InClosingTag);
				change_state(ParserState::InClosingTag);
			}
			break;
		case yui::DocumentTokenType::ClosingTagName: // div
			{
				m_buffer = token.content();
			}
			break;
		case yui::DocumentTokenType::ClosingTagClose: // >
			{
				if (m_history.back()->tag_name() == m_buffer && is_self_closing(m_buffer)) {
					// this element was auto-closed.
					exit_out_of_node();
				}
				else {
					auto *parent = working_parent();

					if (m_buffer != parent->tag_name()) {
						omit_error(token, "Closing tag doesn't match the top of the working node stack. Closing anyway.");
					}

					close_current_tag();
				}
			}
			break;
		}

		if (m_parser_state == ParserState::Done) {
			break;
		}
	}
}

void yui::DocumentParser::will_change_state(ParserState new_state)
{
	if (m_parser_state == ParserState::InTagBody && !m_buffer.empty()) {
		yui::trim(m_buffer);

		if (!m_buffer.empty()) {
			// Move the old fragment buffer into inserted node.
			append_text_fragment(std::move(m_buffer));
		}
	}

	// Always clear.
	m_buffer.clear();
}

void yui::DocumentParser::change_state(ParserState new_state)
{
	m_parser_state = new_state;
}

void yui::DocumentParser::append_text_fragment(std::string text)
{
	auto* fragment = new TextFragmentNode(working_parent(), m_document, std::move(text));
	working_parent()->append_child(fragment);
}

void yui::DocumentParser::append_new_node(const DocumentToken& token)
{
	if (token.content() == "doc") {
		if (m_parser_state == ParserState::ExpectingDocument) {
			return;
		}

		// Omit an error.
		omit_error(token, "Found another <doc> tag inside of an already opened document. Treating as regular node.");
	}
	
	auto *node = new Node(working_parent(), m_document);
	node->set_tag_name(token.content());

	// Add it to parent.
	working_parent()->append_child(node);
	
	m_working_stack.push(node);
}

yui::Node* yui::DocumentParser::working_parent()
{
	return m_working_stack.top();
}

void yui::DocumentParser::omit_error(const DocumentToken& token, std::string message)
{
	m_errors.emplace_back(DocumentParseError{token, std::move(message)});
}
