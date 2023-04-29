#include "StyleHelper.h"

#include <algorithm>

std::unique_ptr<yui::StylesheetDeclaration> yui::StyleHelper::merge(std::vector<StylesheetDeclaration *> declarations) {
    if (declarations.empty()) {
        return nullptr;
    }

    // Resulting declaration will not have a selector of any sort.
    auto result = std::make_unique<yui::StylesheetDeclaration>();

    std::sort(
            declarations.begin(), declarations.end(), [](StylesheetDeclaration *a, StylesheetDeclaration *&b) {
                return a->weight() < b->weight();
            }
    );

    for (const auto &decl : declarations) {
        for (const auto &[id, value] : decl->properties()) {
            result->set_property_ptr(id, value->copy());
        }

        for (const auto &[name, value] : decl->custom_properties()) {
            result->set_property_ptr(name, value->copy());
        }
    }

    return result;
}
