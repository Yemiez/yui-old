#include "DocumentWidget.h"
#include <algorithm>
#include <stack>
#include "Box.h"
#include "Inline.h"
#include "InlineBox.h"
#include "Input.h"
#include "LayoutTreeDumper.h"
#include "Textarea.h"
#include "../Window.h"
#include "../ymd/DocumentNode.h"
#include "../yss/StyleHelper.h"

yui::layout::DocumentWidget::DocumentWidget(DocumentNode *doc, Window *wnd)
        : m_dom_document(doc), m_window(wnd) {
    m_dom_node = reinterpret_cast<yui::Node *>(doc);
}

void yui::layout::DocumentWidget::set_dom_document(DocumentNode *node) {
    m_dom_document = node;
}

void yui::layout::DocumentWidget::paint(yui::Painter &painter) {
    BENCHMARK_BEGIN
                        for (auto &child : m_children) {
                            child->paint(painter);
                        }
    BENCHMARK_END
}

void yui::layout::DocumentWidget::update(float dt) {
    //compute();
    LayoutNode::update(dt);

    if (m_dirty_layout) {
        auto new_cursor = ComputedCursorMode::None;
        for (auto *dirty : m_dirty_nodes) {
            dirty->dom_node()->compute_styles();
            dirty->compute();

            if (dirty->dom_node()->hovered() && dirty->dom_node()->computed().cursor() != ComputedCursorMode::None) {
                new_cursor = dirty->dom_node()->computed().cursor();
            }
        }

        switch (new_cursor) {
        case ComputedCursorMode::Input:
            m_window->use_input_cursor();
            break;
        case ComputedCursorMode::Hand:
            m_window->use_hand_cursor();
            break;
        case ComputedCursorMode::None:
        default:
            m_window->use_arrow_cursor();
            break;
        }

        m_dirty_nodes.clear();
        m_dirty_layout = false;
    }
}

void yui::layout::DocumentWidget::compute() {
    BENCHMARK_BEGIN
                        LayoutNode::compute();

                const auto margin = m_children[0]->dom_node()->computed().margin();
                m_size = m_children[0]->size_with_padding() + glm::ivec2{
                        margin.left + margin.right,
                        margin.top + margin.bottom
                };
    BENCHMARK_END
}

void yui::layout::DocumentWidget::clear_layout_tree() {
    std::vector<LayoutNode *> nodes{ };
    flatten(nodes);

    for (auto *node : nodes) {
        delete node;
    }
    m_children.clear();
}

yui::layout::LayoutNode *yui::layout::DocumentWidget::fragment_allocate_or_take_parent(Node *dom_node) {
    if ((m_construction_stack.top()->is_inline() && !m_construction_stack.top()->is_inline_box())
            && dom_node->is_fragment()) {
        auto *inl = dynamic_cast<Inline *>(m_construction_stack.top());
        inl->append_fragment(*dynamic_cast<TextFragmentNode *>(dom_node));
        return inl;
    }

    auto *inl = new Inline();
    if (dom_node->is_fragment()) {
        inl->append_fragment(*dynamic_cast<TextFragmentNode *>(dom_node));
    }
    inl->set_dom_node(dom_node);
    return inl;
}

yui::layout::LayoutNode *yui::layout::DocumentWidget::allocate_display_node(ComputedDisplay display, Node *node) {
    if (display == ComputedDisplay::Block) {
        return allocate_box_context(node);
    } else if (display == ComputedDisplay::None) {
        return nullptr;
    }

    return fragment_allocate_or_take_parent(node);
}

yui::layout::LayoutNode *yui::layout::DocumentWidget::allocate_box_context(Node *dom_node) {
    auto *box = new Box();
    box->set_dom_node(dom_node);
    return box;
}

yui::layout::LayoutNode *yui::layout::DocumentWidget::allocate_input_node(Node *dom_node) {
    auto *node = new Input();
    node->set_dom_node(dom_node);
    return node;
}

yui::layout::LayoutNode *yui::layout::DocumentWidget::allocate_textarea_node(Node *dom_node) {
    auto *textarea = new Textarea();
    textarea->set_dom_node(dom_node);
    return textarea;
}

yui::layout::LayoutNode *yui::layout::DocumentWidget::rearrange_into_box(Node *dom_node) {
    auto *box = new InlineBox();
    box->set_document_widget(this);

    // Fetch the currently top of stack parent.
    auto top_inline = m_construction_stack.top();
    auto *previous_parent = top_inline->parent();
    box->set_dom_node(top_inline->dom_node());

    m_construction_stack.pop();
    box->insert_child(top_inline);

    // Re assign parent structure
    previous_parent->replace_child(top_inline, box);
    box->set_parent(previous_parent);

    top_inline->set_parent(box);
    m_construction_stack.push(box);

    // Next node
    auto *next = allocate_layout_node(dom_node);
    next->set_dom_node(dom_node);
    next->set_parent(box);
    next->set_document_widget(this);
    box->insert_child(next);

    return next;
}

yui::layout::LayoutNode *yui::layout::DocumentWidget::allocate_layout_node(Node *dom_node) {
    if (dom_node->is_display_inline() &&
            (m_construction_stack.top()->dom_node()->is_display_inline() &&
                    m_construction_stack.top()->is_inline() &&
                    !m_construction_stack.top()->is_inline_box())) {
        return rearrange_into_box(dom_node);
    }

    if (dom_node->is_fragment()) {
        return fragment_allocate_or_take_parent(dom_node);
    }
    if (dom_node->tag_name() == "input") {
        return allocate_input_node(dom_node);
    }
    if (dom_node->tag_name() == "textarea") {
        return allocate_textarea_node(dom_node);
    }

    return allocate_display_node(dom_node->computed().display(), dom_node);
}

bool yui::layout::DocumentWidget::has_loaded_font(const std::string &font_name, int font_size) const {
    return std::find_if(
            m_loaded_fonts.begin(), m_loaded_fonts.end(), [&](auto *font) {
                return font->path() == font_name && font->font_size() == font_size;
            }
    ) != m_loaded_fonts.end();
}

yui::FontResource *yui::layout::DocumentWidget::loaded_font(const std::string &cs, int font_size) {
    const auto it = std::find_if(
            m_loaded_fonts.begin(), m_loaded_fonts.end(), [&](auto *font) {
                return font->path() == cs && font->font_size() == static_cast<uint32_t>(font_size);
            }
    );

    if (it == m_loaded_fonts.end()) {
        return nullptr;
    }

    return *it;
}

yui::FontResource *yui::layout::DocumentWidget::default_font() {
    if (m_loaded_fonts.empty()) { return nullptr; }
    return m_loaded_fonts[0];
}

void yui::layout::DocumentWidget::did_reload_stylesheets() {
    m_dom_document->compute_styles();
    load_fonts();
    compute();
}

const yui::layout::LayoutNode *yui::layout::DocumentWidget::find_layout_node(Node *dom_node) const {
    const LayoutNode *result = nullptr;
    traverse(
            [&](const LayoutNode *node) {
                if (node->dom_node() == dom_node) {
                    result = node;
                }
            }
    );
    return result;
}

void yui::layout::DocumentWidget::construct_layout_tree() {
    // Compute styles
    clear_layout_tree();
    m_dom_document->compute_styles();

    m_construction_stack.push(this);
    for (auto *child : dom_node()->children()) {
        construct_layout_tree(child);
    }
    m_construction_stack = { };

    // Load fonts after construction of layout tree.
    load_fonts();
    compute();
}

void yui::layout::DocumentWidget::invalidate_node(LayoutNode &node) {
    m_dirty_layout = true;
    m_dirty_nodes.emplace_back(&node);
}

void yui::layout::DocumentWidget::invalidate() {
    m_dirty_layout = true;
    m_dirty_nodes.emplace_back(this);
}

void yui::layout::DocumentWidget::mouse_move(double x, double y) {
    BENCHMARK_BEGIN
                        LayoutNode *deepest_hit = nullptr;
                std::vector<LayoutNode *> changed_nodes{ };

                for (auto *child : m_children) {
                    if (child->on_mouse_move({ static_cast<int>(x), static_cast<int>(y) })) {
                        break;
                    }
                }
    BENCHMARK_END
}

void yui::layout::DocumentWidget::mouse_left_down(int mouse_x, int mouse_y) {
    for (auto *child : m_children) {
        if (child->on_left_mouse_down({ mouse_x, mouse_y })) {
            break;
        }
    }
}

void yui::layout::DocumentWidget::mouse_left_up(int mouse_x, int mouse_y) {
    for (auto *child : m_children) {
        if (child->on_left_mouse_up({ mouse_x, mouse_y })) {
            break;
        }
    }
}

bool yui::layout::DocumentWidget::on_key_up(int key, int scan, int mods) {
#ifdef BENCHMARK
    if (key == GLFW_KEY_F1) {
        Application::the().profiler().report(std::cout, false);
        return true;
    }
#endif

    return LayoutNode::on_key_up(key, scan, mods);
}

void yui::layout::DocumentWidget::construct_layout_tree(Node *dom_node) {
    auto stack_to_vec = [](std::stack<LayoutNode *> copy) {
        std::vector<LayoutNode *> copied{ };

        while (!copy.empty()) {
            copied.emplace_back(copy.top());
            copy.pop();
        }

        std::reverse(copied.begin(), copied.end());
        return copied;
    };

    auto *node = allocate_layout_node(dom_node);
    if (node == nullptr) {
        return; // this node is hidden or otherwise unavailable. Skip children.
    }

    node->set_document_widget(this);

    if (!node->dom_node()) {
        node->set_dom_node(dom_node);
    }

    auto *parent = m_construction_stack.top();

    if (node->parent() == nullptr) {
        parent->insert_child(node);
    }

    auto do_pop = false;
    if (node != m_construction_stack.top()) {
        m_construction_stack.push(node);
        do_pop = true;
    }

    for (auto *child : dom_node->children()) {
        construct_layout_tree(child);
    }

    if (do_pop) { m_construction_stack.pop(); }
}

void yui::layout::DocumentWidget::flatten(std::vector<LayoutNode *> &nodes) {
    traverse(
            [&](LayoutNode *node) {
                nodes.emplace_back(node);
            }
    );
}

void yui::layout::DocumentWidget::load_fonts() {
    BENCHMARK_BEGIN
                        traverse(
                                [&](LayoutNode *node) {
                                    auto text = node->dom_node()->computed().text();

                                    if (text.font_name.empty() || text.font_size == 0) {
                                        return;
                                    }

                                    if (loaded_font(text.font_name, text.font_size) != nullptr) {
                                        return;
                                    }

                                    auto *font = window()->resource_loader().load_font(text.font_name, text.font_size);

                                    if (font == nullptr) {
                                        Application::the().report_error(
                                                fmt::format(
                                                        "Could not load font {} (with size={})",
                                                        text.font_name,
                                                        text.font_size
                                                )
                                        );
                                        return;
                                    }

                                    m_loaded_fonts.emplace_back(font);
                                }
                        );
    BENCHMARK_END
}
