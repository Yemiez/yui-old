#pragma once
#include "Inline.h"
#include "LayoutNode.h"

namespace yui::layout {

	class InlineBox : public Inline
	{
	public:
		void paint(yui::Painter&) override;
		void compute() override;

		bool is_inline_box() const override { return true; }
		bool is_inline() const override { return true; }
		const char* layout_name() const override { return "inline-box"; }
	private:
		
	};
	
}
