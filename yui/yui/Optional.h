#pragma once
#include <type_traits>

namespace yui {

template<typename T>
class Optional {
public:
    Optional() = default;
    template<typename...Params>
    Optional(Params &&...params)
            : m_has_value(true), m_value(std::forward<Params>(params)...) {}
    Optional(const Optional<T> &other) = default;
    Optional(Optional<T> &&other) noexcept = default;
    Optional(const T &other)
            : m_has_value(true), m_value(other) {}
    Optional(T &&other)
            : m_has_value(true), m_value(std::move(other)) {}

    [[nodiscard]] bool has_value() const { return m_has_value; }
    [[nodiscard]] const T &value() const { return m_value; }
    [[nodiscard]] T &value() { return m_value; }
    [[nodiscard]] T value_or(const T &other) { return has_value() ? m_value : other; }
private:
    bool m_has_value{ false };
    T m_value{ };
};

}
