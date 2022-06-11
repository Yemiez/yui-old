#pragma once

namespace yui::layout {
	class LayoutNode;

	class InlineCompute
	{
	public:
		static void compute(LayoutNode* node);
		static void compute_inline_box(LayoutNode* node);
	};
	
}