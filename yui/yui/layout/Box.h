#pragma once
#include "LayoutNode.h"

namespace yui::layout {

	class Box : public LayoutNode
	{
	public:
		void paint(yui::Painter&) override;
		void compute() override;
		bool is_box() const override { return true; }

		bool hit_test(int x, int y) const override;

		const char* layout_name() const override { return "box"; }
	private:
		
	};
	
}
