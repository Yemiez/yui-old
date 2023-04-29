#include "Node.h"

#include "DocumentNode.h"
#include "../Util.h"
#include "../yss/StyleHelper.h"

yui::Node::Node(Node *parent, DocumentNode *document)
        : m_parent(parent), m_document(document) {
}

bool yui::Node::has_attribute(const std::string &k) const {
    return m_attributes.find(k) != m_attributes.end();
}

const std::string &yui::Node::attribute(const std::string &k) const {
    return m_attributes.at(k);
}

std::string &yui::Node::attribute(const std::string &k) {
    return m_attributes.at(k);
}

void yui::Node::set_parent(Node *parent) {
    m_parent = parent;
    // TODO: remove from possible old parent.
}

void yui::Node::set_document(DocumentNode *document) {
    m_document = document;
    // TODO: Remove from possible old document
}

void yui::Node::set_tag_name(std::string tag_name) {
    m_tag_name = std::move(tag_name);
}

void yui::Node::set_attribute(const std::string &key, std::string value) {
    if (key == "class") {
        add_class(std::move(value));
    } else {
        m_attributes[key] = std::move(value);

        if (key == "id") {
            document()->update_node_id_reference(m_attributes.at(key), this);
        }
    }
}

void yui::Node::compute_styles() {
    const auto matching = m_document->matching_styles(*this);
    const auto merged = StyleHelper::merge(matching);

    if (merged) {
        m_computed = ImmutableComputedValues::immutable_from_style(*merged, *this);
    }

    // Inherit parent styles first
    if (m_parent) {
        auto inherited = m_computed.mutable_();
        inherited.inherit(m_parent->m_computed);
        m_computed = inherited.immutable();
    }

    for (auto &child : m_children) {
        child->compute_styles();
    }
}

void yui::Node::append_child(Node *node) {
    m_children.emplace_back(node);
}

yui::Node *yui::Node::remove_node_and_take_ownership(Node *) {
    // TODO
    return nullptr;
}

void yui::Node::remove_node(Node *) {
    // TODO
}

uint32_t yui::Node::index_of_child(Node *node) const {
    for (uint32_t i = 0; i < m_children.size(); ++i) {
        if (m_children[i] == node) {
            return i;
        }
    }

    return static_cast<uint32_t>(-1);
}

bool yui::Node::has_class(const std::string &key) const {
    return std::find_if(
            m_class_list.begin(),
            m_class_list.end(),
            [&key](const auto &a) { return a == key; }
    ) != m_class_list.end();
}

bool yui::Node::children_are_fragments() const {
    for (auto *child : m_children) {
        if (!child->is_fragment()) { return false; }
        if (!child->children_are_fragments()) { return false; }
    }

    return true;
}

void yui::Node::add_class(std::string class_names) {
    if (class_names.find(' ') != std::string::npos) {
        for (auto &&name : yui::split(std::move(class_names), ' ')) {
            m_class_list.emplace_back(std::move(name));
        }
    } else {
        m_class_list.emplace_back(std::move(class_names));
    }
}

bool yui::Node::set_hovered(bool value) {
    auto *current = document()->current_ui_state_node(NodeUiState::Hovered);

    if (!value && current != this) {
        document()->set_current_ui_state_node(NodeUiState::Hovered, nullptr);
        return current != nullptr;
    } else if (value && current != this) {
        document()->set_current_ui_state_node(NodeUiState::Hovered, this);
        return true;
    }

    return false;
}

bool yui::Node::set_focused(bool value) {
    auto *current = document()->current_ui_state_node(NodeUiState::Focused);

    if (!value && current != this) {
        document()->set_current_ui_state_node(NodeUiState::Focused, nullptr);
        return true;
    } else if (value && current != this) {
        document()->set_current_ui_state_node(NodeUiState::Focused, this);
        return true;
    }

    return false;
}

bool yui::Node::hovered() const {
    return document()->current_ui_state_node(NodeUiState::Hovered) == this;
}

bool yui::Node::focused() const {
    return document()->current_ui_state_node(NodeUiState::Focused) == this;
}

void yui::TextFragmentNode::set_text(std::string text) {
    m_text = std::move(text);
}
