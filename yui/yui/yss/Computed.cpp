#include "Computed.h"

#include "StylesheetDeclaration.h"

yui::ImmutableComputedValues yui::ComputedValues::immutable_from_style(
        const StylesheetDeclaration &style,
        Node &dom_node
) {
    auto str_to_display = [](const auto &str) -> ComputedDisplay {
        if (str == "block") { return ComputedDisplay::Block; }
        if (str == "inline") { return ComputedDisplay::Inline; }
        if (str == "none") { return ComputedDisplay::None; }
        return DefaultValues::display();
    };

    auto str_to_align = [](const auto &str) -> ComputedAlign {
        if (str == "left") { return ComputedAlign::Left; }
        if (str == "center") { return ComputedAlign::Center; }
        if (str == "right") { return ComputedAlign::Right; }
        return DefaultValues::text_align();
    };

    auto str_to_layout = [](const auto &str) -> ComputedLayout {
        if (str == "rows") { return ComputedLayout::Rows; }
        if (str == "columns") { return ComputedLayout::Columns; }
        return DefaultValues::layout();
    };

    auto str_to_cursor = [](const auto &str) -> ComputedCursorMode {
        if (str == "input") { return ComputedCursorMode::Input; }
        if (str == "hand") { return ComputedCursorMode::Hand; }
        return ComputedCursorMode::None;
    };

    auto color_value = [&style](const auto &id) -> Color {
        return dynamic_cast<const StylesheetColorValue &>(style.property(id)).color();
    };

    auto length_value = [&style](const auto &id) -> int {
        return dynamic_cast<const StylesheetUnitSizeValue &>(style.property(id)).scalar();
    };

    auto string_value = [&style](const auto &id) -> const std::string & {
        return style.property(id).string();
    };

    MutableComputedValues computed{ };

    // Layout
    if (style.has_property(StylesheetPropertyId::Display)) {
        computed.set_display(str_to_display(string_value(StylesheetPropertyId::Display)));
    }

    if (style.has_property(StylesheetPropertyId::Layout)) {
        computed.set_layout(str_to_layout(string_value(StylesheetPropertyId::Layout)));
    }

    if (style.has_property(StylesheetPropertyId::MarginTop)) {
        computed.set_margin_top(length_value(StylesheetPropertyId::MarginTop));
    }
    if (style.has_property(StylesheetPropertyId::MarginLeft)) {
        computed.set_margin_left(length_value(StylesheetPropertyId::MarginLeft));
    }
    if (style.has_property(StylesheetPropertyId::MarginBottom)) {
        computed.set_margin_bottom(length_value(StylesheetPropertyId::MarginBottom));
    }
    if (style.has_property(StylesheetPropertyId::MarginRight)) {
        computed.set_margin_right(length_value(StylesheetPropertyId::MarginRight));
    }

    if (style.has_property(StylesheetPropertyId::PaddingX)) {
        computed.set_padding_x(length_value(StylesheetPropertyId::PaddingX));
    }
    if (style.has_property(StylesheetPropertyId::PaddingY)) {
        computed.set_padding_y(length_value(StylesheetPropertyId::PaddingY));
    }

    if (style.has_property(StylesheetPropertyId::Width)) {
        computed.set_width(length_value(StylesheetPropertyId::Width));
    }
    if (style.has_property(StylesheetPropertyId::Height)) {
        computed.set_height(length_value(StylesheetPropertyId::Height));
    }

    if (style.has_property(StylesheetPropertyId::MinWidth)) {
        computed.set_min_width(length_value(StylesheetPropertyId::MinWidth));
    }
    if (style.has_property(StylesheetPropertyId::MinHeight)) {
        computed.set_min_height(length_value(StylesheetPropertyId::MinHeight));
    }

    if (style.has_property(StylesheetPropertyId::MaxWidth)) {
        computed.set_max_width(length_value(StylesheetPropertyId::MaxWidth));
    }
    if (style.has_property(StylesheetPropertyId::MaxHeight)) {
        computed.set_max_height(length_value(StylesheetPropertyId::MaxHeight));
    }

    // Appearance
    if (style.has_property(StylesheetPropertyId::BackgroundColor)) {
        computed.set_background_color(color_value(StylesheetPropertyId::BackgroundColor));
    }
    if (style.has_property(StylesheetPropertyId::BorderSize)) {
        computed.set_border_width(length_value(StylesheetPropertyId::BorderSize));
    }
    if (style.has_property(StylesheetPropertyId::BorderColor)) {
        computed.set_border_color(color_value(StylesheetPropertyId::BorderColor));
    }

    // Text
    if (style.has_property(StylesheetPropertyId::FontName)) {
        computed.set_font_name(string_value(StylesheetPropertyId::FontName));
    }
    if (style.has_property(StylesheetPropertyId::FontSize)) {
        computed.set_font_size(length_value(StylesheetPropertyId::FontSize));
    }
    if (style.has_property(StylesheetPropertyId::TextColor)) {
        computed.set_text_color(color_value(StylesheetPropertyId::TextColor));
    }
    if (style.has_property(StylesheetPropertyId::TextAlign)) {
        computed.set_text_align(str_to_align(string_value(StylesheetPropertyId::TextAlign)));
    }

    // Misc
    if (style.has_property(StylesheetPropertyId::Cursor)) {
        computed.set_cursor(str_to_cursor(string_value(StylesheetPropertyId::Cursor)));
    }

    return computed.immutable();
}

void yui::MutableComputedValues::inherit(const ImmutableComputedValues &other) {
    if (!m_inherited.text.changed.color) {
        m_inherited.text.color = other.text().color;
    }
    if (!m_inherited.text.changed.font_name) {
        m_inherited.text.font_name = other.text().font_name;
    }
    if (!m_inherited.text.changed.font_size) {
        m_inherited.text.font_size = other.text().font_size;
    }

    if (other.cursor() != ComputedCursorMode::None && m_inherited.cursor == ComputedCursorMode::None) {
        m_inherited.cursor = other.cursor();
    }
}
