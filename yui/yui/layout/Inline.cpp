#include "Inline.h"

#include <algorithm>

#include "DocumentWidget.h"
#include "InlineCompute.h"
#include "PainterUtilities.h"
#include "../Window.h"
#include "../ymd/Node.h"

void yui::layout::Inline::append_fragment(TextFragmentNode &fragment) {
    m_text_fragments.emplace_back(fragment.text());
    m_dom_fragments.emplace_back(&fragment);

    // Auto set text.
    m_text.clear();
    for (auto &text_fragment : m_text_fragments) {
        if (!m_text.empty()) { m_text.push_back(' '); }
        m_text.append(Utf8String{ text_fragment });
    }
}

void yui::layout::Inline::paint(yui::Painter &painter) {
    PainterUtilities::paint_background(painter, *this);
    PainterUtilities::paint_borders(painter, *this);
    PainterUtilities::paint_inline_text(painter, *this);
}

void yui::layout::Inline::compute() {
    InlineCompute::compute(this);
}

bool yui::layout::Inline::hit_test(int x, int y) const {
    if (m_size == glm::ivec2{ 0, 0 }) {
        return false;
    }

    return x >= m_position.x && x <= m_position.x + size_with_padding().x &&
            y >= m_position.y && y <= m_position.y + size_with_padding().y;
}
