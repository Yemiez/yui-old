#include "StylesheetDeclaration.h"

#include <cassert>
#include <numeric>
#include <utility>

#include "Selector.h"

yui::StylesheetValue::StylesheetValue(std::string value)
        : m_string(std::move(value)) {}

const std::string &yui::StylesheetValue::string() const { return m_string; }

void yui::StylesheetValue::set_string(std::string value) {
    m_string = std::move(value);
}

yui::StylesheetValue *yui::StylesheetValue::copy() const {
    return new StylesheetValue(m_string);
}

yui::StylesheetColorValue::StylesheetColorValue(std::string value)
        : StylesheetValue(std::move(value)), m_color(to_color(m_string)) {}

yui::StylesheetValue *yui::StylesheetColorValue::copy() const {
    return new StylesheetColorValue(m_string);
}

yui::Color yui::StylesheetColorValue::to_color(const std::string &hex_string) {
    auto hex_color{ 0 };
    auto delta{ 0 };

    if (hex_string[0] == '#') {
        delta = 1;
    }

    sscanf_s(&hex_string[0] + delta, "%x", &hex_color);

    if (hex_color == 0) { return { }; }

    Color color{ };

    if (strlen(&hex_string[0] + delta) == 8) {
        color.r = (hex_color >> 24) & 0xFF;
        color.g = (hex_color >> 16) & 0xFF;
        color.b = (hex_color >> 8) & 0xFF;
        color.a = (hex_color & 0xFF);
    } else {
        color.r = (hex_color >> 16) & 0xFF;
        color.g = (hex_color >> 8) & 0xFF;
        color.b = (hex_color & 0xFF);
    }

    return color;
}

std::string yui::StylesheetColorValue::to_string(Color) {
    return "";
}

yui::StylesheetUnitSizeValue::StylesheetUnitSizeValue(std::string value)
        : StylesheetValue(std::move(value)) {
    auto [scalar, unit] = to_pair(m_string);
    m_scalar = scalar;
    m_unit = unit;
}

yui::StylesheetUnitSizeValue::StylesheetUnitSizeValue(int scalar, Unit unit)
        : m_scalar(scalar), m_unit(unit) {
    m_string = to_string(m_scalar, m_unit);
}

void yui::StylesheetUnitSizeValue::set_string(std::string value) {
    m_string = std::move(value);
    // Set from value
    auto [scalar, unit] = to_pair(m_string);
    m_scalar = scalar;
    m_unit = unit;
}

void yui::StylesheetUnitSizeValue::set_unit(Unit unit) {
    if (m_unit != unit) {
        m_unit = unit;
        m_string = to_string(m_scalar, m_unit);
    }
}

void yui::StylesheetUnitSizeValue::set_scalar(int scalar) {
    if (m_scalar != scalar) {
        m_scalar = scalar;
        m_string = to_string(m_scalar, m_unit);
    }
}

std::string yui::StylesheetUnitSizeValue::to_string(int scalar, Unit unit) {
    auto str = std::to_string(scalar);

    switch (unit) {
    case Unit::Percentage:
        str.push_back('%');
        break;
    case Unit::Pixels:
        str.append("px");
        break;
    }
    return str;
}

std::pair<int, yui::StylesheetUnitSizeValue::Unit> yui::StylesheetUnitSizeValue::to_pair(std::string string) {
    std::string number_part{ };
    number_part.reserve(string.length());
    for (auto it = string.begin(); it != string.end();) {
        if (*it >= '0' && *it <= '9') {
            number_part.push_back(*it);
            it = string.erase(it);
        } else {
            break;
        }
    }
    auto number = std::atoi(number_part.c_str());
    if (string == "%") {
        return { number, Unit::Percentage };
    }
    return { number, Unit::Pixels };
}

yui::StylesheetValue *yui::StylesheetUnitSizeValue::copy() const {
    return new StylesheetUnitSizeValue(m_scalar, m_unit);
}

yui::StylesheetDeclaration::StylesheetDeclaration(std::vector<Selector> selectors)
        : m_selectors(std::move(selectors)) {}

yui::StylesheetDeclaration::StylesheetDeclaration(const StylesheetDeclaration &other)
        : m_selectors(other.m_selectors) {
    for (const auto &[id, ptr] : other.m_properties) {
        m_properties[id] = ptr->copy();
    }

    for (const auto &[id, ptr] : other.m_custom_properties) {
        m_custom_properties[id] = ptr->copy();
    }
}

yui::StylesheetDeclaration::StylesheetDeclaration(StylesheetDeclaration &&other) noexcept
        : m_selectors(std::move(other.m_selectors)), m_properties(std::move(other.m_properties)),
          m_custom_properties(std::move(other.m_custom_properties)) {}

yui::StylesheetDeclaration::~StylesheetDeclaration() {
    for (auto [_, ptr] : m_properties) {
        delete ptr;
    }

    for (auto [_, ptr] : m_custom_properties) {
        delete ptr;
    }

    m_properties.clear();
    m_custom_properties.clear();
}

bool yui::StylesheetDeclaration::match(Node &dom_node) const {
    return match(dom_node, nullptr);
}

bool yui::StylesheetDeclaration::match(Node &dom_node, const Selector **first_matching_selector) const {
    for (const auto &selector : m_selectors) {
        if (selector.match(dom_node)) {
            if (first_matching_selector != nullptr) {
                *first_matching_selector = &selector;
            }
            return true;
        }
    }
    return false;
}

uint32_t yui::StylesheetDeclaration::weight() const {
    return std::accumulate(
            m_selectors.cbegin(),
            m_selectors.cend(),
            0u,
            [](const auto &acc, const auto &sel) -> uint32_t {
                return acc + sel.weight();
            }
    );
}

bool yui::StylesheetDeclaration::has_property(StylesheetPropertyId id) const {
    return m_properties.find(id) != m_properties.end();
}

bool yui::StylesheetDeclaration::has_property(const std::string &name) const {
    const auto id = StylesheetDeclaration::property_id_from_string(name);

    if (id != StylesheetPropertyId::Unknown) {
        return has_property(id);
    }

    return m_custom_properties.find(name) != m_custom_properties.end();
}

const yui::StylesheetValue &yui::StylesheetDeclaration::property(StylesheetPropertyId id) const {
    return *m_properties.at(id);
}

const yui::StylesheetValue &yui::StylesheetDeclaration::property(const std::string &name) const {
    const auto id = StylesheetDeclaration::property_id_from_string(name);

    if (id != StylesheetPropertyId::Unknown) {
        return property(id);
    }

    return *m_custom_properties.at(name);
}

void yui::StylesheetDeclaration::set_property(StylesheetPropertyId id, std::string value) {
    const auto iterator = m_properties.find(id);

    if (iterator == m_properties.end()) {
        m_properties[id] = create_stylesheet_value(id, std::move(value));
    } else {
        m_properties[id]->set_string(std::move(value));
    }
}

void yui::StylesheetDeclaration::set_property(const std::string &name, std::string value) {
    const auto id = StylesheetDeclaration::property_id_from_string(name);

    if (id != StylesheetPropertyId::Unknown) {
        return set_property(id, std::move(value));
    }

    // Set?
    const auto iterator = m_custom_properties.find(name);

    if (iterator == m_custom_properties.end()) {
        m_custom_properties[name] = new StylesheetValue(std::move(value));
    } else {
        m_custom_properties[name]->set_string(std::move(value));
    }
}

void yui::StylesheetDeclaration::set_property_ptr(StylesheetPropertyId id, StylesheetValue *value) {
    const auto iterator = m_properties.find(id);

    if (iterator != m_properties.end()) {
        delete iterator->second;
    }
    m_properties[id] = value;
}

void yui::StylesheetDeclaration::set_property_ptr(const std::string &name, StylesheetValue *value) {
    const auto id = StylesheetDeclaration::property_id_from_string(name);

    if (id != StylesheetPropertyId::Unknown) {
        return set_property_ptr(id, value);
    }

    const auto iterator = m_custom_properties.find(name);

    if (iterator != m_custom_properties.end()) {
        delete iterator->second;
    }
    m_custom_properties[name] = value;
}

void yui::StylesheetDeclaration::unset_property(StylesheetPropertyId id) {
    const auto iterator = m_properties.find(id);

    if (iterator == m_properties.end()) {
        return;
    }

    delete iterator->second;
    m_properties.erase(iterator);
}

void yui::StylesheetDeclaration::unset_property(const std::string &name) {
    const auto id = StylesheetDeclaration::property_id_from_string(name);

    if (id != StylesheetPropertyId::Unknown) {
        return unset_property(id);
    }

    const auto iterator = m_custom_properties.find(name);

    if (iterator == m_custom_properties.end()) {
        return;
    }

    delete iterator->second;
    m_custom_properties.erase(iterator);
}

yui::StylesheetPropertyId yui::StylesheetDeclaration::property_id_from_string(const std::string &name) {
#define STYLESHEET_PROPERTIES_ENUMERATOR_(a, b, c, ...) \
    if (name == #a || name == #b || name == #c) return yui::StylesheetPropertyId::a;
    STYLESHEET_PROPERTIES_ENUMERATOR
#undef STYLESHEET_PROPERTIES_ENUMERATOR_

    return StylesheetPropertyId::Unknown;
}

yui::StylesheetValue *yui::StylesheetDeclaration::create_stylesheet_value(StylesheetPropertyId id, std::string value) {
#define STYLESHEET_PROPERTIES_ENUMERATOR_(a, b, c, className) if (id == StylesheetPropertyId::a) return new className(std::move(value));
    STYLESHEET_PROPERTIES_ENUMERATOR
#undef STYLESHEET_PROPERTIES_ENUMERATOR_

    assert(1);
    return nullptr;
}
