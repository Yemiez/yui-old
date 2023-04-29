#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <glm/vec2.hpp>

#include "../yss/Computed.h"
#include "../yss/StylesheetDeclaration.h"

namespace yui {
class DocumentNode;
class Node;
using AttributeMap = std::map<std::string, std::string>;
using NodeList = std::vector<Node *>;

using ClassName = std::string;
using ClassList = std::vector<ClassName>;

enum class NodeUiState {
    None,
    Hovered,
    Focused,
};

class Node {
public:
    explicit Node(Node *parent, DocumentNode *document = nullptr);
    virtual ~Node() = default;

    // Getters
    Node *parent() { return m_parent; }
    [[nodiscard]] const Node *parent() const { return m_parent; }
    DocumentNode *document() { return m_document; }
    [[nodiscard]] const DocumentNode *document() const { return m_document; }
    [[nodiscard]] const NodeList &children() const { return m_children; }
    [[nodiscard]] const std::string &tag_name() const { return m_tag_name; }
    [[nodiscard]] const AttributeMap &attributes() const { return m_attributes; }
    [[nodiscard]] bool has_attribute(const std::string &) const;
    [[nodiscard]] const std::string &attribute(const std::string &) const;
    std::string &attribute(const std::string &);
    [[nodiscard]] const ImmutableComputedValues &computed() const { return m_computed; }

    // Setters
    void set_parent(Node *parent);
    void set_document(DocumentNode *document);
    void set_tag_name(std::string);
    void set_attribute(const std::string &, std::string);
    virtual void compute_styles();

    // Tree modifying functions
    void append_child(Node *);

    // Removes a node from the tree (below this node) and grants the callee ownership.
    Node *remove_node_and_take_ownership(Node *);

    // Removes a node from the tree (below this node) and frees it from memory.
    void remove_node(Node *);
    uint32_t index_of_child(Node *) const;

    [[nodiscard]] bool has_class(const std::string &) const;
    [[nodiscard]] const ClassList &class_list() const { return m_class_list; }
    [[nodiscard]] bool children_are_fragments() const;
    void add_class(std::string);

    // Returns true if it changed anything, otherwise false
    bool set_hovered(bool value);

    // Returns true if it changed anything, otherwise false
    bool set_focused(bool value);

    [[nodiscard]] bool hovered() const;
    [[nodiscard]] bool focused() const;

    [[nodiscard]] bool is_display_inline() const {
        return computed().display() == ComputedDisplay::Inline;
    }

    [[nodiscard]] virtual bool is_fragment() const { return false; }
private:
    Node *m_parent{ nullptr };
    DocumentNode *m_document{ nullptr };
    NodeList m_children{ };
    std::string m_tag_name{ };
    AttributeMap m_attributes{ };
    ClassList m_class_list{ };
    NodeUiState m_ui_state{ NodeUiState::None };
    ImmutableComputedValues m_computed{ };
};

class TextFragmentNode : public Node {
public:
    static constexpr auto TEXT_FRAGMENT_TAG_NAME = "#text-fragment";

public:
    TextFragmentNode(Node *parent, DocumentNode *document, std::string text)
            : Node(parent, document), m_text(std::move(text)) {
        set_tag_name(TEXT_FRAGMENT_TAG_NAME);
    }

    [[nodiscard]] const std::string &text() const { return m_text; }
    void set_text(std::string);
    [[nodiscard]] bool is_fragment() const override { return true; }
private:
    std::string m_text{ };
};

}
