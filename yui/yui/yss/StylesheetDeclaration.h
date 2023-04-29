#pragma once
#include <map>
#include <string>
#include "Selector.h"
#include "../Painter.h"

namespace yui {
class Selector;

#define STYLESHEET_PROPERTIES_ENUMERATOR \
        STYLESHEET_PROPERTIES_ENUMERATOR_(Display, display, display, StylesheetValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(Cursor, cursor, cursor, StylesheetValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(BackgroundColor, backgroundColor, background-color, StylesheetColorValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(TextColor, textColor, text-color, StylesheetColorValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(FontName, fontName, font-name, StylesheetValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(FontSize, fontSize, font-size, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(BorderColor, borderColor, border-color, StylesheetColorValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(BorderSize, borderSize, border-size, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(TextAlign, textAlign, text-align, StylesheetValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(Layout, layout, layout, StylesheetValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(MarginTop, marginTop, margin-top, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(MarginBottom, marginBottom, margin-bottom, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(MarginLeft, marginLeft, margin-left, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(MarginRight, marginRight, margin-right, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(PaddingX, paddingX, padding-x, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(PaddingY, paddingY, padding-y, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(Width, width, width, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(Height, height, height, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(MaxWidth, maxWidth, max-width, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(MaxHeight, maxHeight, max-height, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(MinWidth, minWidth, min-width, StylesheetUnitSizeValue) \
        STYLESHEET_PROPERTIES_ENUMERATOR_(MinHeight, minHeight, min-height, StylesheetUnitSizeValue)

// Standardized properties
enum class StylesheetPropertyId {
    Unknown,
#define STYLESHEET_PROPERTIES_ENUMERATOR_(en, ...) en,
    STYLESHEET_PROPERTIES_ENUMERATOR
#undef STYLESHEET_PROPERTIES_ENUMERATOR_
};

class StylesheetValue {
public:
    StylesheetValue() = default;
    explicit StylesheetValue(std::string);
    virtual ~StylesheetValue() = default;

    [[nodiscard]] const std::string &string() const;
    virtual void set_string(std::string);

    [[nodiscard]] virtual StylesheetValue *copy() const;
protected:
    std::string m_string{ };
};

class StylesheetColorValue : public StylesheetValue {
public:
    explicit StylesheetColorValue(std::string);

    [[nodiscard]] Color color() const { return m_color; }

    [[nodiscard]] StylesheetValue *copy() const override;
    static Color to_color(const std::string &);
    static std::string to_string(Color);
private:
    Color m_color{ };
};

class StylesheetUnitSizeValue : public StylesheetValue {
public:
    enum class Unit {
        Pixels,
        Percentage,
    };

public:
    explicit StylesheetUnitSizeValue(std::string);
    StylesheetUnitSizeValue(int, Unit);
    void set_string(std::string) override;

    [[nodiscard]] Unit unit() const { return m_unit; }
    [[nodiscard]] int scalar() const { return m_scalar; }
    void set_unit(Unit);
    void set_scalar(int);

    static std::string to_string(int, Unit);
    static std::pair<int, Unit> to_pair(std::string);

    [[nodiscard]] StylesheetValue *copy() const override;
private:
    int m_scalar{ };
    Unit m_unit{ Unit::Pixels };
};

class StylesheetDeclaration {
public:
    using PropertyMap = std::map<StylesheetPropertyId, StylesheetValue *>;
    using CustomPropertyMap = std::map<std::string, StylesheetValue *>;

public:
    explicit StylesheetDeclaration(std::vector<Selector> selectors);
    StylesheetDeclaration() = default;
    StylesheetDeclaration(const StylesheetDeclaration &);
    StylesheetDeclaration(StylesheetDeclaration &&) noexcept;
    ~StylesheetDeclaration();

    [[nodiscard]] const PropertyMap &properties() const { return m_properties; }
    [[nodiscard]] const CustomPropertyMap &custom_properties() const { return m_custom_properties; }

    [[nodiscard]] const std::vector<Selector> &selectors() const { return m_selectors; }
    void set_selectors(std::vector<Selector> selector) { m_selectors = std::move(selector); }

    bool match(Node &dom_node) const;
    bool match(Node &dom_node, const Selector **first_matching_selector) const;

    [[nodiscard]] uint32_t weight() const;

    [[nodiscard]] bool has_property(StylesheetPropertyId) const;
    [[nodiscard]] bool has_property(const std::string &) const;
    [[nodiscard]] const StylesheetValue &property(StylesheetPropertyId) const;
    [[nodiscard]] const StylesheetValue &property(const std::string &) const;

    void set_property(StylesheetPropertyId, std::string);
    void set_property(const std::string &, std::string);
    void set_property_ptr(StylesheetPropertyId, StylesheetValue *);
    void set_property_ptr(const std::string &, StylesheetValue *);
    void unset_property(StylesheetPropertyId);
    void unset_property(const std::string &);

    static StylesheetPropertyId property_id_from_string(const std::string &);
    static StylesheetValue *create_stylesheet_value(StylesheetPropertyId, std::string);
private:
    std::vector<Selector> m_selectors;
    PropertyMap m_properties{ }; // Properties, standardized.
    CustomPropertyMap m_custom_properties{ }; // Custom properties, unknown.
};

}
