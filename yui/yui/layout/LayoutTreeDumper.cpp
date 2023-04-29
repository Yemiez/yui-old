#include "LayoutTreeDumper.h"

#include <sstream>

#include "Inline.h"
#include "LayoutNode.h"
#include "../ymd/Node.h"
#include "../Util.h"

std::string yui::layout::LayoutTreeDumper::dump(const LayoutNode *root) {
    std::ostringstream ss{ };
    ss << "Layout tree:\n";
    dump(root, 0, ss);
    return ss.str();
}

void yui::layout::LayoutTreeDumper::dump(const LayoutNode *node, int indentation, std::ostream &output) {
    for (auto i = 0; i < indentation; ++i) { output << ' '; }

    output << "> #" << node->id() << " " << node->layout_name();

    if (node->is_inline() && !dynamic_cast<const Inline *>(node)->text().empty()) {
        output << " fragments={";

        for (const auto &fragment : dynamic_cast<const Inline *>(node)->text_fragments()) {
            output << " '" << yui::escape_content(fragment) << "',";
        }

        output << " }";
    }

    output << " dom=" << node->dom_node()->tag_name();

    if (node->dom_node()->has_attribute("id")) {
        output << "#" << node->dom_node()->attribute("id");
    }

    output << " size={ " << node->size_with_padding().x << ", " << node->size_with_padding().y << " }";
    output << " pos={ " << node->position().x << ", " << node->position().y << " }";

    if (!node->dom_node()->class_list().empty()) {
        for (const auto &name : node->dom_node()->class_list()) {
            output << '.' << name;
        }
    }

    output << '\n';
    for (auto *child : node->children()) {
        dump(child, indentation + 2, output);
    }
}
