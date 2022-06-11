#include "Box.h"


#include "BoxCompute.h"
#include "Inline.h"
#include "PainterUtilities.h"
#include "../ymd/Node.h"

void yui::layout::Box::paint(yui::Painter& painter)
{
	PainterUtilities::paint_background(painter, *this);
	PainterUtilities::paint_borders(painter, *this);

	for (auto *child : m_children) {
		child->paint(painter);
	}
}

void yui::layout::Box::compute()
{
	// TODO: Use direction to calculate box size.
	// TODO: Considering min, max, and preferred width/height.
	[[maybe_unused]]const auto direction = dom_node()->computed().layout();

	// TODO tomorrow: padding-x is only adding padding on the right side (because we should be using inner_position())

	if (direction == ComputedLayout::Rows) {
		BoxCompute::compute_rows(*this);
	}
	else {
		BoxCompute::compute_columns(*this);
	}
}

bool yui::layout::Box::hit_test(int x, int y) const
{
	return x >= m_position.x && x <= m_position.x + size_with_padding().x &&
		y >= m_position.y && y <= m_position.y + size_with_padding().y;
}
