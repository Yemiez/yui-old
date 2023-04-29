#include "Selector.h"

#include <cassert>
#include <iostream>
#include <sstream>

#include "../ymd/DocumentNode.h"
#include "../ymd/Node.h"

yui::Selector::SimpleSelector::SimpleSelector(SimpleSelector &&other) noexcept
        : m_parts(std::move(other.m_parts)), m_pseudo_class(other.m_pseudo_class), m_relation(other.m_relation) {
    other.m_relation = Relation::None;
    other.m_pseudo_class = PseudoClass::None;
}

yui::Selector::SimpleSelector::SimpleSelector(std::vector<Part> parts, PseudoClass p, Relation r)
        : m_parts(std::move(parts)), m_pseudo_class(p), m_relation(r) {}

void yui::Selector::SimpleSelector::append(Part p) {
    m_parts.emplace_back(std::move(p));
}

void yui::Selector::SimpleSelector::append(Type t, std::string s) {
    m_parts.emplace_back(Part{ std::move(s), t });
}

void yui::Selector::SimpleSelector::set_relation(Relation relation) {
    m_relation = relation;
}

void yui::Selector::SimpleSelector::set_pseudo_class(PseudoClass pc) {
    m_pseudo_class = pc;
}

std::string yui::Selector::SimpleSelector::relation_to_string(Relation relation) {
    switch (relation) {
    case Relation::ImmediateChild:
        return ">";
    case Relation::Descendant:
        return " ";
    case Relation::AdjacentSibling:
        return "+";
    case Relation::None:
        break;
    }

    return "";
}

void yui::Selector::append(SimpleSelector v) {
    m_complex_selectors.emplace_back(std::move(v));
}

uint32_t yui::Selector::weight() const {
    auto weight = 0u;

    for (const auto &simple : m_complex_selectors) {
        for (const auto &part : simple.parts()) {
            switch (part.type) {
            case SimpleSelector::Type::Universal:
                ++weight;
                break;
            case SimpleSelector::Type::TagName:
                weight += 10;
                break;
            case SimpleSelector::Type::Id:
                weight += 100;
                break;
            case SimpleSelector::Type::Class:
                weight += 10;
                break;
            case SimpleSelector::Type::Invalid:
                break;
            }
        }

        if (simple.has_pseudo_class()) {
            weight += 50;
        }
    }

    return weight;
}

bool yui::Selector::match(Node &node) const {
    assert(!m_complex_selectors.empty());
    return match(node, m_complex_selectors.size() - 1);
}

std::string yui::Selector::to_string() const {
    std::stringstream ss;

    for (const auto &simple : m_complex_selectors) {
        if (simple.has_relation()) {
            ss << yui::Selector::SimpleSelector::relation_to_string(simple.relation()) << " ";
        }

        for (const auto &part : simple.parts()) {
            switch (part.type) {
            case SimpleSelector::Type::Universal:
                ss << "*";
                break;
            case SimpleSelector::Type::TagName:
                ss << part.value;
                break;
            case SimpleSelector::Type::Id:
                ss << "#" << part.value;
                break;
            case SimpleSelector::Type::Class:
                ss << "." << part.value;
                break;
            case SimpleSelector::Type::Invalid:
                break;
            }
        }

        if (simple.has_pseudo_class()) {
            switch (simple.pseudo_class()) {
            case SimpleSelector::PseudoClass::Hover:
                ss << ":hover";
                break;
            case SimpleSelector::PseudoClass::None:
                break;
            case SimpleSelector::PseudoClass::Focus:
                ss << ":focus";
                break;
            }
        }

        ss << " ";
    }

    return ss.str();
}

bool yui::Selector::match(Node &node, uint32_t selector_index) const {
    const auto &selector = m_complex_selectors[selector_index];

    for (const auto &part : selector.parts()) {
        switch (part.type) {
        case SimpleSelector::Type::Invalid:
            return false;
        case SimpleSelector::Type::Universal:
            break;
        case SimpleSelector::Type::TagName:
            if (!(part.is_tag_name() && part.value == node.tag_name())) {
                return false;
            }
            break;
        case SimpleSelector::Type::Id:
            if (!node.has_attribute("id")) {
                return false;
            }

            if (node.attribute("id") != part.value) {
                return false;
            }
            break;
        case SimpleSelector::Type::Class:
            if (!node.has_class(part.value)) {
                return false;
            }
            break;
        }
    }

    switch (selector.pseudo_class()) {
    case SimpleSelector::PseudoClass::None:
        break;
    case SimpleSelector::PseudoClass::Hover:
        if (!node.hovered()) {
            return false;
        }
        break;
    case SimpleSelector::PseudoClass::Focus:
        if (!node.focused()) {
            return false;
        }
        break;
    }

    switch (selector.relation()) {
    case SimpleSelector::Relation::None:
        return true;
    case SimpleSelector::Relation::ImmediateChild:
        if (selector_index == 0 || node.parent() == nullptr) {
            return false;
        }

        return match(*node.parent(), selector_index - 1);
    case SimpleSelector::Relation::Descendant:
        if (selector_index == 0 || node.parent() == nullptr) {
            return false;
        }

        for (auto *node_ptr = node.parent(); node_ptr != nullptr; node_ptr = node_ptr->parent()) {
            if (match(*node_ptr, selector_index - 1)) {
                return true;
            }
        }
        break;
    case SimpleSelector::Relation::AdjacentSibling:
        if (selector_index == 0 || node.parent() == nullptr) {
            return false;
        }

        auto *parent = node.parent();
        const auto index_in_children = parent->index_of_child(&node);

        if (index_in_children == 0 || index_in_children == static_cast<uint32_t>(-1)) {
            return false;
        }

        return match(*parent->children()[index_in_children - 1], selector_index - 1);
    }

    return false;
}
