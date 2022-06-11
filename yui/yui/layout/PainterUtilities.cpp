#include "PainterUtilities.h"

#include <algorithm>


#include "DocumentWidget.h"
#include "Inline.h"
#include "LayoutNode.h"
#include "Textarea.h"
#include "../ResourceLoader.h"
#include "../Util.h"
#include "../ymd/Node.h"

static std::map<uint32_t, float> g_beam_storage = {};

void yui::layout::PainterUtilities::paint_background(Painter& p, LayoutNode& node)
{
	auto computed = node.dom_node()->computed();
	if (computed.background_color() != Color{0, 0, 0, 0}) {
		p.fill_rect(
			node.position().x, 
			node.position().y, 
			node.size_with_padding().x, 
			node.size_with_padding().y, 
			computed.background_color()
		);
	}
}

void yui::layout::PainterUtilities::paint_borders(Painter& p, LayoutNode& node)
{
	auto computed = node.dom_node()->computed();
	if (computed.border().width > 0) {
		p.outline_rect(
			node.position().x, 
			node.position().y, 
			node.size_with_padding().x, 
			node.size_with_padding().y, 
			computed.border().color,
			static_cast<float>(computed.border().width)
		);
	}
}

void yui::layout::PainterUtilities::paint_inline_text(Painter& p, LayoutNode& node)
{
	auto *font = get_font(node);

	if (!font) {
		return;
	}

	[[maybe_unused]]bool is_emoji = font->path().find("Emoji") != std::string::npos;
	const auto computed_text = node.dom_node()->computed().text();

	auto text = node.as<Inline>()->text();

	if (node.parent()->is_inline_box()) {
		p.text(text, computed_text.color, node.position().x, node.position().y, *font);
	}
	else {
		glm::ivec2 pos{
			node.position().x + (node.size_with_padding().x / 2) - (node.size().x / 2),
			node.position().y + (node.size_with_padding().y / 2) - (node.size().y / 2)
		};
	
		p.text(text, computed_text.color, pos.x, pos.y, *font);
	}
	
	
}

yui::FontResource* yui::layout::PainterUtilities::get_font(LayoutNode& node)
{
	const auto computed = node.dom_node()->computed();
	const auto computed_text = computed.text();
	return node.document_widget()->loaded_font(computed_text.font_name, computed_text.font_size);
}
