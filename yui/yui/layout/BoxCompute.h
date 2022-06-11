#pragma once
namespace yui::layout {
	class LayoutNode;
	class Box;

	class BoxCompute
	{
	public:
		static void compute_rows(LayoutNode& box);
		static void compute_columns(Box& box);
	};
	
}