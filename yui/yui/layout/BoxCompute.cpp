#include "BoxCompute.h"
#include "Box.h"
#include "../ymd/Node.h"
#include "../yss/Computed.h"

void yui::layout::BoxCompute::compute_rows(LayoutNode &box) {
    for (auto i = 0u, sz = box.children().size(); i < sz; ++i) {
        auto *prev = i == 0 ? nullptr : box.children().at(i - 1);
        auto *child = box.children().at(i);

        const auto margin = child->dom_node()->computed().margin();
        const auto prev_margin = prev ? prev->dom_node()->computed().margin() : ComputedMargin{ 0, 0, 0, 0 };
        const auto prev_rect = prev ? prev->absolute_rect() : Rect{ 0, 0, 0, 0 };

        if (child->is_inline() || child->is_input()) {
            child->compute();

            if (prev && prev->is_inline()) {
                // TODO: Consider virtual lines.
                // Adjust current position by X instead of Y as usually done
                // This is because multiply repeating inline elements should be rendered as one "row".
                child->set_position(
                        {
                                prev_rect.right + prev_margin.right + margin.left,
                                prev_rect.top
                        }
                );
                continue;
            }

            // Prev was box or nonexistent
            if (prev) {
                // Adjust Y position to bottom of previous item + previous margin bottom + current margin top.
                // TODO: Here is where we would use either prev_rect.right or prev_rect.bottom
                // TODO: based on the current direction.
                child->set_position(
                        {
                                box.inner_position().x + margin.left,
                                prev_rect.bottom + prev_margin.bottom + margin.top
                        }
                );
            } else {
                child->set_position(
                        {
                                box.inner_position().x + margin.left,
                                box.inner_position().y + margin.top
                        }
                );
            }
        } else {
            // Compute next box size.
            if (prev) {
                child->set_position(
                        {
                                box.inner_position().x + margin.left,
                                prev_rect.bottom + prev_margin.bottom + margin.top
                        }
                );
            } else {
                child->set_position(
                        {
                                box.inner_position().x + margin.left,
                                box.inner_position().y + margin.top
                        }
                );
            }

            child->compute();
        }
    }

    glm::ivec2 size{ 0, 0 };
    auto first_inline_idx = static_cast<uint32_t>(-1);
    std::vector<std::pair<uint32_t, uint32_t>> inline_ranges{ };

    for (auto i = 0u, sz = box.children().size(); i < sz; ++i) {
        auto *prev = i == 0 ? nullptr : box.children().at(i - 1);
        auto *child = box.children().at(i);

        if (child->is_inline() && (!prev || !prev->is_inline())) {
            first_inline_idx = i;
        }

        if (prev && prev->is_inline() && !child->is_inline()) {
            inline_ranges.emplace_back(first_inline_idx, i);
            first_inline_idx = static_cast<uint32_t>(-1);
        }

        if (child->is_inline()) {
            continue;
        }

        // Box remaining
        size.y += child->size_with_padding().y;
        size.y += child->dom_node()->computed().margin().top + child->dom_node()->computed().margin().bottom;

        auto x_accum = child->dom_node()->computed().margin().left + child->size_with_padding().x
                + child->dom_node()->computed().margin().right;
        if (x_accum > size.x) {
            size.x = x_accum;
        }
    }

    if (first_inline_idx != static_cast<uint32_t>(-1)) {
        inline_ranges.emplace_back(first_inline_idx, box.children().size());
    }

    for (auto [i, sz] : inline_ranges) {
        auto x_accumulated = 0;
        auto max_y = 0;
        auto max_bottom_margin = 0;

        for (auto j = i; j < sz; ++j) {
            auto *child = box.children().at(j);
            if (child->size_with_padding().y > max_y) {
                max_y = child->size_with_padding().y;
            }

            auto margin = child->dom_node()->computed().margin();

            if (margin.bottom > max_bottom_margin) {
                max_bottom_margin = child->dom_node()->computed().margin().bottom;
            }

            x_accumulated += margin.left + child->size_with_padding().x + margin.right;
        }

        size.y += max_y + max_bottom_margin;

        if (x_accumulated > size.x) {
            size.x = x_accumulated;
        }
    }

    box.set_size(size);
}

void yui::layout::BoxCompute::compute_columns(Box &box) {

}
