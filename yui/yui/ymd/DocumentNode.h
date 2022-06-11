#pragma once
#include "Node.h"
#include "../yss/StylesheetDeclaration.h"

namespace yui {
	class Selector;

	class Stylesheet
	{
	public:
		Stylesheet(std::vector<StylesheetDeclaration> declarations, std::string source_file);

		const std::vector<StylesheetDeclaration>& declarations() const { return m_declarations; }
		std::vector<StylesheetDeclaration>& declarations() { return m_declarations; }
		const std::string& source_file() const { return m_source_file; };
	private:
		std::vector<StylesheetDeclaration> m_declarations{};
		std::string m_source_file{};
	};

	class DocumentNode : public Node
	{
	public:
		static constexpr auto DOCUMENT_TAG_NAME = "doc";
	public:
		DocumentNode();

		Node* query(const Selector& selector);
		std::vector<Node*> query_all(const Selector& selector);

		Node* get_node_by_id(const std::string& id);

		bool load_stylesheet(const std::string& file);
		bool reload_stylesheet(const std::string& file);

		std::vector<StylesheetDeclaration*> matching_styles(Node&); 
	public:
		// Nodes
		void update_node_id_reference(const std::string&, Node*);
		void remove_existing_id_reference(Node*);

		// UI States
		void set_current_ui_state_node(NodeUiState ui_state, Node* node);
		const Node* current_ui_state_node(NodeUiState) const;
		Node* current_ui_state_node(NodeUiState);

		template<typename Callable>
		auto traverse(yui::Node *node, Callable&& c) -> decltype(c(node))
		{
			auto result = c(node);

			if (result != nullptr) {
				return result;
			}

			for (auto *child : node->children()) {
				result = traverse(child, std::forward<Callable>(c));

				if (result) {
					return result;
				}
			}

			return static_cast<decltype(c(node))>(nullptr);
		}

		const std::vector<Stylesheet>& stylesheets() const { return m_stylesheets; };

	private:
		using NodeIdMap = std::map<std::string, Node*>;
		using NodeIdIterator = NodeIdMap::iterator;
		NodeIdMap m_id_map{};
		std::map<NodeUiState, Node*> m_current_ui_nodes{
			{NodeUiState::None, nullptr},
			{NodeUiState::Hovered, nullptr},
			{NodeUiState::Focused, nullptr},
		};
		std::vector<Stylesheet> m_stylesheets{};
	};
	
}
