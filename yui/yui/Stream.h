#pragma once
#include <string>
#include <stdexcept>

namespace yui {

struct StreamPosition {
    uint32_t row{ 0u };
    uint32_t col{ 0u };

    [[nodiscard]] std::string to_string() const;
};

inline std::string StreamPosition::to_string() const {
    return "{ " + std::to_string(row) + ", " + std::to_string(col) + " }";
}

template<typename Container = std::string, typename Elem = typename Container::value_type>
class StreamReader {
public:
    explicit StreamReader(Container container)
            : m_container(std::move(container)) {}

    [[nodiscard]] bool reached_eof() const {
        return m_cursor >= size();
    }

    Elem &consume() {
        if (reached_eof()) { throw std::runtime_error("Attempted to consume past eof"); }
        ++m_col;
        return m_container.at(m_cursor++);
    }
    void unwind();
    const Elem &peek(int off = 0) const;
    [[nodiscard]] StreamPosition position() const { return { .row=m_row, .col=m_col }; }

    template<typename Callable>
    void consume_until_false(Callable &&fn) {
        while (!reached_eof() && fn(peek())) {
            consume();
        }
    }

    template<typename Callable>
    void consume_until_false(Callable &&fn, Container &output) {
        while (!reached_eof() && fn(peek())) {
            output.push_back(consume());
        }
    }

    [[nodiscard]] uint32_t size() const {
        return m_container.size();
    }

private:
    Container m_container{ };
    uint32_t m_row{ 0u };
    uint32_t m_col{ 0u };
    uint32_t m_cursor{ 0u };
};

template<typename Container, typename Elem>
void StreamReader<Container, Elem>::unwind() {
    // FIXME: Proper handling of lines with std::string.
    if (m_cursor != 0) {
        --m_cursor;
        --m_col;
    }
}

template<typename Container, typename Elem>
const Elem &StreamReader<Container, Elem>::peek(int off) const {
    const auto peek_offset = m_cursor + off;

    if (peek_offset >= size()) {
        throw std::runtime_error("Attempted to peek past eof");
    }

    return m_container[static_cast<uint32_t>(peek_offset)];
}
}
