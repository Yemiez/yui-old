#pragma once
#include <string>
#include <vector>

namespace yui {
class Node;

class Selector {
public:
    class SimpleSelector {
    public:
        enum class PseudoClass {
            None,
            Hover,
            Focus,
        };

        enum class Relation {
            None,
            ImmediateChild,
            Descendant,
            AdjacentSibling,
        };

        enum class Type {
            Invalid,
            Universal,
            TagName,
            Id,
            Class,
        };

        struct Part {
            std::string value{ };
            Type type{ Type::Invalid };
            [[nodiscard]] bool is_universal() const { return type == Type::Universal; }
            [[nodiscard]] bool is_tag_name() const { return type == Type::TagName; }
            [[nodiscard]] bool is_id() const { return type == Type::Id; }
            [[nodiscard]] bool is_class() const { return type == Type::Class; }
        };

    public:
        SimpleSelector() = default;
        SimpleSelector(const SimpleSelector &) = default;
        SimpleSelector(SimpleSelector &&) noexcept;
        explicit SimpleSelector(std::vector<Part> parts, PseudoClass = PseudoClass::None, Relation = Relation::None);

        [[nodiscard]] const std::vector<Part> &parts() const { return m_parts; }
        void append(Part);
        void append(Type, std::string);

        // The relation to the previous selector.
        // e.g. for "#div1 > #div2 > #div3" then #div1 has no relation, but both #div2 and #div3 has relation = ImmediateChild.
        void set_relation(Relation);
        [[nodiscard]] Relation relation() const { return m_relation; }
        [[nodiscard]] bool has_relation() const { return m_relation != Relation::None; }

        void set_pseudo_class(PseudoClass);
        [[nodiscard]] PseudoClass pseudo_class() const { return m_pseudo_class; }
        [[nodiscard]] bool has_pseudo_class() const { return m_pseudo_class != PseudoClass::None; }
        static std::string relation_to_string(Relation relation);
    private:
        std::vector<Part> m_parts{ };
        PseudoClass m_pseudo_class{ PseudoClass::None };
        Relation m_relation{ Relation::None };
    };

    using ComplexSelectors = std::vector<SimpleSelector>;
public:
    Selector() = default;

    void append(SimpleSelector);

    [[nodiscard]] uint32_t weight() const;

    [[nodiscard]] const ComplexSelectors &complex_selectors() const { return m_complex_selectors; }
    ComplexSelectors &complex_selectors() { return m_complex_selectors; }

    bool match(Node &) const;
    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] bool empty() const { return m_complex_selectors.empty(); }

private:
    bool match(Node &, uint32_t selector_index) const;

private:
    ComplexSelectors m_complex_selectors{ };
};

}
