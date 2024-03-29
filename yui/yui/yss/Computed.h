#pragma once
#include <string>
#include "../Painter.h"

namespace yui {
class Node;
class StylesheetDeclaration;

enum class ComputedDisplay {
    Block,
    Inline,
    None,
};

enum class ComputedAlign {
    Left,
    Center,
    Right,
};

enum class ComputedLayout {
    Rows,
    Columns,
};

enum class ComputedCursorMode {
    None,
    Input,
    Hand,
};

struct ComputedText {
    std::string font_name{ };
    int font_size{ };
    Color color{ };
    int character_spacing{ };
    int line_height{ };
    struct {
        bool font_name{ }, font_size{ }, color{ }, character_spacing{ }, line_height{ };
    } changed;
};

struct ComputedMargin {
    int top{ }, left{ }, bottom{ }, right{ };
};

struct ComputedPadding {
    int x{ }, y{ };
};

struct ComputedBorder {
    int width{ };
    Color color{ };
};

class DefaultValues {
public:
    static ComputedCursorMode cursor() { return ComputedCursorMode::None; }
    static ComputedDisplay display() { return ComputedDisplay::Block; }
    static ComputedLayout layout() { return ComputedLayout::Rows; }
    static ComputedMargin margin() { return { 2, 2, 2, 2 }; }
    static ComputedPadding padding() { return { 5, 5 }; }
    static ComputedBorder border() { return { .width=0, .color={ 0, 0, 0, 0 } }; }
    static Color background_color() { return { 0, 0, 0, 0 }; }
    static ComputedAlign text_align() { return ComputedAlign::Center; }
    static ComputedText text() {
        return {
                .font_name={ }, .font_size= 0, .color{
                        255,
                        255,
                        255,
                        255
                }, .character_spacing=10
        };
    }
};

class ImmutableComputedValues;
class MutableComputedValues;

class ComputedValues {
public:
    static ImmutableComputedValues immutable_from_style(const StylesheetDeclaration &, Node &dom_node);

    [[nodiscard]] ComputedText text() const { return m_inherited.text; }
    [[nodiscard]] ComputedCursorMode cursor() const { return m_inherited.cursor; }
    [[nodiscard]] ComputedDisplay display() const { return m_noninherited.display; }
    [[nodiscard]] ComputedLayout layout() const { return m_noninherited.layout; }
    [[nodiscard]] ComputedMargin margin() const { return m_noninherited.margin; }
    [[nodiscard]] ComputedPadding padding() const { return m_noninherited.padding; }
    [[nodiscard]] ComputedBorder border() const { return m_noninherited.border; }
    [[nodiscard]] Color background_color() const { return m_noninherited.background_color; }
    [[nodiscard]] ComputedAlign text_align() const { return m_noninherited.text_align; }
    [[nodiscard]] int width() const { return m_noninherited.width; }
    [[nodiscard]] int height() const { return m_noninherited.height; }
    [[nodiscard]] int max_width() const { return m_noninherited.max_width; }
    [[nodiscard]] int max_height() const { return m_noninherited.max_height; }
    [[nodiscard]] int min_width() const { return m_noninherited.min_width; }
    [[nodiscard]] int min_height() const { return m_noninherited.min_height; }

    [[nodiscard]] ImmutableComputedValues immutable() const;
    [[nodiscard]] MutableComputedValues mutable_() const;
protected:
    struct {
        ComputedText text{ DefaultValues::text() };
        ComputedCursorMode cursor{ DefaultValues::cursor() };
        struct {
            bool text{ false };
            bool cursor{ false };
        } changed;
    } m_inherited;
    struct {
        ComputedDisplay display{ DefaultValues::display() };
        ComputedLayout layout{ DefaultValues::layout() };
        ComputedMargin margin{ DefaultValues::margin() };
        ComputedPadding padding{ DefaultValues::padding() };
        ComputedBorder border{ DefaultValues::border() };
        Color background_color{ DefaultValues::background_color() };
        ComputedAlign text_align{ DefaultValues::text_align() };
        int width{ 0 };
        int height{ 0 };
        int max_width{ 0 };
        int max_height{ 0 };
        int min_width{ 0 };
        int min_height{ 0 };
    } m_noninherited;
};

class ImmutableComputedValues final : public ComputedValues {
public:
};

class MutableComputedValues final : public ComputedValues {
public:
    void set_cursor(ComputedCursorMode cursor) {
        m_inherited.cursor = cursor;
        m_inherited.changed.cursor = true;
    }
    void set_display(ComputedDisplay display) { m_noninherited.display = display; }
    void set_layout(ComputedLayout layout) { m_noninherited.layout = layout; }
    void set_margin_top(int top) { m_noninherited.margin.top = top; }
    void set_margin_left(int left) { m_noninherited.margin.left = left; }
    void set_margin_bottom(int bottom) { m_noninherited.margin.bottom = bottom; }
    void set_margin_right(int right) { m_noninherited.margin.right = right; }
    void set_margin(ComputedMargin margin) { m_noninherited.margin = margin; }
    void set_padding_x(int x) { m_noninherited.padding.x = x; }
    void set_padding_y(int y) { m_noninherited.padding.y = y; }
    void set_padding(ComputedPadding padding) { m_noninherited.padding = padding; }
    void set_font_name(std::string name) {
        m_inherited.text.font_name = std::move(name);
        m_inherited.changed.text = true;
        m_inherited.text.changed.font_name = true;
    }
    void set_font_size(int size) {
        m_inherited.text.font_size = size;
        m_inherited.changed.text = true;
        m_inherited.text.changed.font_size = true;
    }
    void set_text_color(Color color) {
        m_inherited.text.color = color;
        m_inherited.changed.text = true;
        m_inherited.text.changed.color = true;
    }
    void set_border_width(int width) { m_noninherited.border.width = width; }
    void set_border_color(Color color) { m_noninherited.border.color = color; }
    void set_background_color(Color color) { m_noninherited.background_color = color; }
    void set_text_align(ComputedAlign align) { m_noninherited.text_align = align; }
    void set_width(int width) { m_noninherited.width = width; }
    void set_height(int height) { m_noninherited.height = height; }
    void set_max_width(int width) { m_noninherited.max_width = width; }
    void set_max_height(int height) { m_noninherited.max_height = height; }
    void set_min_width(int width) { m_noninherited.min_width = width; }
    void set_min_height(int height) { m_noninherited.min_height = height; }
    void inherit(const ImmutableComputedValues &);
};

inline ImmutableComputedValues ComputedValues::immutable() const {
    return { *this };
}

inline MutableComputedValues ComputedValues::mutable_() const {
    return { *this };
}

}
