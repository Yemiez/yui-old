#pragma once
#include <string>

#include "LayoutNode.h"

namespace yui {
class TextFragmentNode;
}

namespace yui::layout {
// FIXME: Inline shouldn't have it's own text field.
// FIXME: It should be relying on the value from the current dom node.


class Inline : public LayoutNode {
public:
    [[nodiscard]] const yui::Utf8String &text() const { return m_text; }
    void append_fragment(TextFragmentNode &fragment);
    [[nodiscard]] const std::vector<std::string> &text_fragments() const { return m_text_fragments; }

    void paint(yui::Painter &) override;
    void compute() override;
    [[nodiscard]] bool is_inline() const override { return true; }
    [[nodiscard]] bool hit_test(int x, int y) const override;

    [[nodiscard]] const char *layout_name() const override { return "inline"; }
private:
    yui::Utf8String m_text{ };
    std::vector<std::string> m_text_fragments{ };
    std::vector<TextFragmentNode *> m_dom_fragments{ };
};

}
