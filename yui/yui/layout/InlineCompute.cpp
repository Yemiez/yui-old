#include "InlineCompute.h"


#include "BoxCompute.h"
#include "DocumentWidget.h"
#include "Inline.h"
#include "LayoutNode.h"
#include "../Window.h"
#include "../ymd/Node.h"

void yui::layout::InlineCompute::compute(LayoutNode* node)
{
	const auto text = node->as<Inline>()->text();
	auto *font = node->document_widget()->loaded_font(
		node->dom_node()->computed().text().font_name,
		node->dom_node()->computed().text().font_size
	);

	if (font == nullptr) {
		return;
	}

	node->set_size(node->document_widget()->window()->painter().text_size(text, *font));
}

void yui::layout::InlineCompute::compute_inline_box(LayoutNode* node)
{
	// Initial size
	compute(node);
	auto initial_size = node->size();

	BoxCompute::compute_rows(*node);

	/*
	for (auto child : node->children()) {
		child->compute();
		initial_size.x += child->size().x + child->dom_node()->computed().text().spacing;

		if (child->previous()) {
			child->set_position({ child->previous()->position().x + child->previous()->size().x + child->dom_node()->computed().text().spacing, node->position().y });
		}
		else {
			child->set_position({ node->position().x, node->position().y });
		}
	}

	node->set_size(initial_size);
	*/
}
