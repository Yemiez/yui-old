#pragma once
#include <stack>
#include <string>
#include "LayoutNode.h"
#include "../io/Profiler.h"
#include "../yss/StylesheetDeclaration.h"

namespace yui {
enum class ComputedDisplay;
class Window;
class FontResource;
class DocumentNode;
}

namespace yui::layout {

class DocumentWidget : public LayoutNode {
public:
    DocumentWidget() = default;
    DocumentWidget(DocumentNode *, Window *);

    [[nodiscard]] Window *window() const { return m_window; }

    [[nodiscard]] DocumentNode *dom_document() const { return m_dom_document; }
    void set_dom_document(DocumentNode *node);

    void paint(yui::Painter &) override;
    void update(float dt) override;
    void compute() override;
    [[nodiscard]] bool has_loaded_font(const std::string &font_name, int font_size) const;
    FontResource *loaded_font(const std::string &cs, int font_size);
    FontResource *default_font();
    void did_reload_stylesheets();

    [[nodiscard]] bool is_root() const override { return true; }

    // Find a layout node from dom node
    const LayoutNode *find_layout_node(Node *node) const;

    [[nodiscard]] const char *layout_name() const override { return "document"; }

    template<typename Callable>
    void traverse(Callable &&callable);

    template<typename Callable>
    void traverse(Callable &&callable) const;

    template<typename Callable>
    void traverse_cancelable(Callable &&callable);

    template<typename Callable>
    void traverse_cancelable(Callable &&callable) const;
public:
    void construct_layout_tree();

    void invalidate_node(LayoutNode &);
    void invalidate();

public:
    void mouse_move(double x, double y);
    void mouse_left_down(int mouse_x, int mouse_y);
    void mouse_left_up(int mouse_x, int mouse_y);
    bool on_key_up(int key, int scan, int mods) override;
    // Private functions
private:
    LayoutNode *fragment_allocate_or_take_parent(Node *dom_node);
    LayoutNode *allocate_display_node(ComputedDisplay, Node *node);
    LayoutNode *allocate_box_context(Node *dom_node);
    LayoutNode *allocate_input_node(Node *dom_node);
    LayoutNode *allocate_textarea_node(Node *dom_node);
    LayoutNode *rearrange_into_box(Node *dom_node);
    LayoutNode *allocate_layout_node(Node *dom_node);
    void clear_layout_tree();
    void construct_layout_tree(Node *);
    void flatten(std::vector<LayoutNode *> &nodes);

    template<typename Callable>
    void traverse_children(LayoutNode *node, Callable &&callable);

    template<typename Callable>
    void traverse_children(const LayoutNode *node, Callable &&callable) const;

    template<typename Callable>
    void traverse_children_cancelable(LayoutNode *node, Callable &&callable);
    template<typename Callable>
    void traverse_children_cancelable(const LayoutNode *node, Callable &&callable) const;

    void load_fonts();
private:
    DocumentNode *m_dom_document{ nullptr };
    Window *m_window{ nullptr };
    std::stack<LayoutNode *> m_construction_stack{ };
    std::vector<FontResource *> m_loaded_fonts{ };
    bool m_dirty_layout{ false };
    std::vector<LayoutNode *> m_dirty_nodes{ };
};

template<typename Callable>
void DocumentWidget::traverse(Callable &&callable) {
    traverse_children(this, callable);
}

template<typename Callable>
void DocumentWidget::traverse_children(LayoutNode *node, Callable &&callable) {
    for (auto *child : node->children()) {
        callable(child);
        traverse_children(child, std::forward<Callable>(callable));
    }
}

template<typename Callable>
void DocumentWidget::traverse(Callable &&callable) const {
    traverse_children(this, callable);
}

template<typename Callable>
void DocumentWidget::traverse_cancelable(Callable &&callable) {
    traverse_children_cancelable(this, callable);
}

template<typename Callable>
void DocumentWidget::traverse_cancelable(Callable &&callable) const {
    traverse_children_cancelable(this, callable);
}

template<typename Callable>
void DocumentWidget::traverse_children(const LayoutNode *node, Callable &&callable) const {
    for (const auto *child : node->children()) {
        callable(child);
        traverse_children(child, std::forward<Callable>(callable));
    }
}

template<typename Callable>
void DocumentWidget::traverse_children_cancelable(LayoutNode *node, Callable &&callable) {
    for (auto *child : node->children()) {
        if (callable(child)) {
            traverse_children_cancelable(child, callable);
        }
    }
}

template<typename Callable>
void DocumentWidget::traverse_children_cancelable(const LayoutNode *node, Callable &&callable) const {
    for (const auto *child : node->children()) {
        if (callable(child)) {
            traverse_children_cancelable(child, callable);
        }
    }
}

}
