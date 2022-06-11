#pragma once
#include <stack>
#include <string>

#include "DocumentLexer.h"

namespace yui {
	class DocumentNode;
	class Node;
	class DocumentLexer;

	class DocumentParseError
	{
	public:
		DocumentParseError() = default;
		DocumentParseError(const DocumentParseError&) = default;
		DocumentParseError(DocumentParseError&&) = default;
		DocumentParseError(DocumentToken, std::string);

		// Which token caused the error?
		const DocumentToken& token() const { return m_token; }

		// Get the message of the error.
		const std::string& message() const { return m_message; }
		
	private:
		DocumentToken m_token{};
		std::string m_message{};
	};
	
	class DocumentParser
	{
	private:
		enum class ParserState;
	public:
		DocumentParser() = delete;
		DocumentParser(const DocumentParser&) = delete;
		DocumentParser(DocumentParser&&) = delete;
		DocumentParser(DocumentLexer);

		// did any errors occur? Note that the Document tree may still be functional.
		bool has_error() const { return !m_errors.empty(); }

		// Get any DocumentParseErrors that occurred.
		const std::vector<DocumentParseError>& errors() const { return m_errors; }

		// Get the DocumentNode containing the parsed tree.
		const DocumentNode* document() const { return m_document; }

		// Get the DocumentNode* and release the DocumentParser from ownership.
		DocumentNode* release_document();
	private:
		static bool is_self_closing(const std::string& cs);
		void construct_tree();

		// for text fragments
		void will_change_state(ParserState);
		void change_state(ParserState);

		void append_text_fragment(std::string);
		void append_new_node(const DocumentToken& token);

		// Get the current parent that is being built.
		Node* working_parent();

		void omit_error(const DocumentToken& token, std::string);
	private:
		DocumentLexer m_lexer;
		DocumentNode* m_document{nullptr};
		std::vector<DocumentParseError> m_errors{};
		std::stack<Node*> m_working_stack{};
		std::vector<Node*> m_history{};
		std::string m_buffer{};
		std::string m_current_attribute{};
		enum class ParserState
		{
			ExpectingDocument,
			Error,
			Done,
			AfterTagName,
			InTagBody,
			InTagAttributeDeclaration,
			InClosingTag,
		} m_parser_state{ParserState::ExpectingDocument};
	};
	
}
