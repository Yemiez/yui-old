#pragma once
#include "LayoutNode.h"

namespace yui::layout {

class Box : public LayoutNode {
public:
    void paint(yui::Painter &) override;
    void compute() override;
    [[nodiscard]] bool is_box() const override { return true; }

    [[nodiscard]] bool hit_test(int x, int y) const override;

    [[nodiscard]] const char *layout_name() const override { return "box"; }
private:

};

}
