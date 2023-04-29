#pragma once
#include "Input.h"

namespace yui {
class Painter;
class FontResource;
}

namespace yui::layout {

class Textarea;
class LayoutNode;

class PainterUtilities {
public:
    static void paint_background(Painter &, LayoutNode &);
    static void paint_borders(Painter &, LayoutNode &);
    static void paint_inline_text(Painter &, LayoutNode &);
    static FontResource *get_font(LayoutNode &);
private:
};

}
