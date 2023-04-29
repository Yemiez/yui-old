#pragma once
#include <string>

namespace yui::layout {
class LayoutNode;

class LayoutTreeDumper {
public:
    static std::string dump(const LayoutNode *root);

private:
    static void dump(const LayoutNode *node, int indentation, std::ostream &output);
};

}
