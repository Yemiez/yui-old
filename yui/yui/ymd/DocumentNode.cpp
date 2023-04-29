#include "DocumentNode.h"
#include <utility>
#include <filesystem>
#include "../Util.h"
#include "../yss/StylesheetParser.h"

yui::Stylesheet::Stylesheet(std::vector<StylesheetDeclaration> declarations, std::string source_file)
        : m_declarations(std::move(declarations)), m_source_file(std::move(source_file)) {}

yui::DocumentNode::DocumentNode()
        : Node(nullptr, this) {
    set_tag_name(DocumentNode::DOCUMENT_TAG_NAME);
}

yui::Node *yui::DocumentNode::query(const Selector &selector) {
    return traverse(
            this, [&selector](Node *node) -> Node * {
                if (selector.match(*node)) {
                    return node;
                }

                return nullptr;
            }
    );
}

std::vector<yui::Node *> yui::DocumentNode::query_all(const Selector &selector) {
    std::vector<yui::Node *> nodes{ };
    traverse(
            this, [&](Node *node) -> Node * {
                if (selector.match(*node)) {
                    nodes.emplace_back(node);
                }

                return nullptr;
            }
    );
    return nodes;
}

yui::Node *yui::DocumentNode::get_node_by_id(const std::string &id) {
    const auto iterator = m_id_map.find(id);

    if (iterator == m_id_map.end()) {
        return nullptr;
    }

    return iterator->second;
}

bool yui::DocumentNode::load_stylesheet(const std::string &file) {
    if (!std::filesystem::is_regular_file(file)) {
        return false;
    }

    StylesheetParser parser{ StylesheetLexer{ yui::read_file(file) } };

    if (parser.declarations().empty()) {
        return false;
    }

    m_stylesheets.emplace_back(std::move(parser.declarations()), file);
    return true;
}

bool yui::DocumentNode::reload_stylesheet(const std::string &file) {
    auto it = std::find_if(
            m_stylesheets.begin(), m_stylesheets.end(), [&file](const Stylesheet &sheet) {
                return sheet.source_file() == file;
            }
    );

    if (it == m_stylesheets.end()) {
        return false;
    }

    m_stylesheets.erase(it);
    return load_stylesheet(file);
}

std::vector<yui::StylesheetDeclaration *> yui::DocumentNode::matching_styles(Node &node) {
    std::vector<yui::StylesheetDeclaration *> result;

    for (auto &stylesheet : m_stylesheets) {
        for (auto &decl : stylesheet.declarations()) {
            if (decl.match(node)) {
                result.emplace_back(&decl);
            }
        }
    }

    return result;
}

void yui::DocumentNode::update_node_id_reference(const std::string &id, Node *node) {
    remove_existing_id_reference(node);
    m_id_map[id] = node;
}

void yui::DocumentNode::remove_existing_id_reference(Node *node) {
    const auto iterator = std::find_if(
            m_id_map.begin(), m_id_map.end(), [&](const auto &pair) {
                return pair.second == node;
            }
    );

    if (iterator == m_id_map.end()) { return; }
    m_id_map.erase(iterator);
}

void yui::DocumentNode::set_current_ui_state_node(NodeUiState ui_state, Node *node) {
    m_current_ui_nodes[ui_state] = node;
}

const yui::Node *yui::DocumentNode::current_ui_state_node(NodeUiState ui_state) const {
    return m_current_ui_nodes.at(ui_state);
}

yui::Node *yui::DocumentNode::current_ui_state_node(NodeUiState ui_state) {
    return m_current_ui_nodes.at(ui_state);
}
