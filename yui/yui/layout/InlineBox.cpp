#include "InlineBox.h"

#include "InlineCompute.h"
#include "PainterUtilities.h"

void yui::layout::InlineBox::paint(yui::Painter& painter)
{
	PainterUtilities::paint_background(painter, *this);
	PainterUtilities::paint_borders(painter, *this);
	for (auto &child : m_children) {
		child->paint(painter);
	}
}

void yui::layout::InlineBox::compute()
{
	InlineCompute::compute_inline_box(this);
}
