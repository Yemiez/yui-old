#include "Utf8String.h"
#include "Util.h"

#define NOMINMANX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

yui::Utf8String::Utf8String(const char *str) {
    create_from_byte_stream(str, strlen(str));
}

yui::Utf8String::Utf8String(const char8_t *str) {
    create_from_byte_stream(reinterpret_cast<const char *>(str), strlen(reinterpret_cast<const char *>(str)));
}

yui::Utf8String::Utf8String(const std::string_view str) {
    create_from_byte_stream(str.data(), str.length());
}

yui::Utf8String::Utf8String(const std::string &str) {
    create_from_byte_stream(str.c_str(), str.length());
}

yui::Utf8String::Utf8String(const Utf8String &other)
        : m_string(other.m_string) {}

yui::Utf8String::Utf8String(Utf8String &&other) noexcept
        : m_string(std::move(other.m_string)) {}

void yui::Utf8String::clear() {
    m_string.clear();
}

void yui::Utf8String::resize(u32 i) {
    m_string.resize(i);
}

void yui::Utf8String::resize(u32 i, const Utf8CharacterInfo &value) {
    m_string.resize(i, value);
}

bool yui::Utf8String::contains(const Utf8String &substring) const {
    if (substring.length() == length()) { return *this == substring; }
    if (substring.length() > length()) { return false; }

    for (auto i = 0u; i < length() && i + substring.length() < length(); ++i) {
        auto found{ true };
        for (auto j = 0u; j < substring.length(); ++j) {
            if (substring.code_point_at(j) != code_point_at(i)) {
                found = false;
                break;
            }
        }

        if (found) { return true; }
    }

    return false;
}

bool yui::Utf8String::contains(const std::string &substring) const {
    return contains(Utf8String{ substring });
}

bool yui::Utf8String::contains(const std::string_view &substring) const {
    return contains(Utf8String{ substring });
}

std::string yui::Utf8String::to_byte_string() const {
    // TODO: Cache this on demand?
    std::string str{ };
    str.reserve(length());

    for (auto code_point : m_string) {
        str.append(code_point.begin(), code_point.end());
    }

    return str;
}

std::string yui::Utf8String::to_printable_string() const {
    std::string str{ };
    str.reserve(length());

    for (const auto &code_point : *this) {
        if (code_point.length_in_bytes == 1) {
            if (::isprint(*code_point.begin())) {
                str.push_back(*code_point.begin());
            } else {
                str.append(yui::escape_char(*code_point.begin()));
            }
        } else {
            str.append(yui::escape_code_point(code_point));
        }
    }
    return str;
}

void yui::Utf8String::push_back(u32 code_point) {
    Utf8CharacterInfo info{ .code_point = code_point };

    auto tmp{ 0u };
    if (!decode_first_byte(*info.begin(), info.length_in_bytes, tmp)) {
        return;
    }
    m_string.emplace_back(info);
}

void yui::Utf8String::push_back(Utf8CharacterInfo code_point) {
    m_string.emplace_back(std::move(code_point));
}

void yui::Utf8String::push_back(char byte) {
    m_string.emplace_back(Utf8CharacterInfo{ .code_point = static_cast<u32>(byte), .length_in_bytes = 1 });
}

void yui::Utf8String::append(const Utf8String &other) {
    append(other.begin(), other.end());
}

void yui::Utf8String::append(const ConstIterator &begin, const ConstIterator &end) {
    m_string.insert(m_string.end(), begin, end);
}

yui::Utf8String::Iterator yui::Utf8String::insert(u32 where, const Utf8String &str) {
    return m_string.insert(begin() + where, str.begin(), str.end());
}

yui::Utf8String::Iterator yui::Utf8String::insert(const Iterator &where, const Utf8String &str) {
    return m_string.insert(where, str.begin(), str.end());
}

yui::Utf8String::Iterator yui::Utf8String::insert(u32 where, u32 code_point) {
    auto decoded = decode(code_point);

    if (!decoded.has_value()) {
        return end();
    }

    return m_string.insert(m_string.begin() + where, decoded.value());
}

yui::Utf8String::Iterator yui::Utf8String::insert(const Iterator &where, u32 code_point) {
    auto decoded = decode(code_point);

    if (!decoded.has_value()) {
        return end();
    }

    return m_string.insert(where, decoded.value());
}

yui::Utf8String::Iterator yui::Utf8String::erase(u32 where) {
    return m_string.erase(m_string.begin() + where);
}

yui::Utf8String::Iterator yui::Utf8String::erase(u32 from, u32 to) {
    return m_string.erase(m_string.begin() + from, m_string.begin() + to);
}

yui::Utf8String::Iterator yui::Utf8String::erase(const Iterator &where) {
    return m_string.erase(where);
}

yui::Utf8String::Iterator yui::Utf8String::erase(const Iterator &from, const Iterator &to) {
    return m_string.erase(from, to);
}

yui::Utf8String::ConstIterator yui::Utf8String::find(const char c) const {
    return find_if(
            [&c](const auto &decoded) {
                return decoded.length_in_bytes == 1 && static_cast<char>(decoded.code_point) == c;
            }
    );
}

yui::Utf8String::ConstIterator yui::Utf8String::find(const char8_t c) const {
    return find(static_cast<char>(c));
}

yui::Utf8String::ConstIterator yui::Utf8String::find(u32 code_point) const {
    return find_if(
            [&code_point](const auto &decoded) {
                return decoded.code_point == code_point;
            }
    );
}

yui::Utf8String::ConstIterator yui::Utf8String::find(const Utf8CharacterInfo &decoded_code_point) const {
    return find_if(
            [&decoded_code_point](const auto &decoded) {
                return decoded_code_point.code_point == decoded.code_point;
            }
    );
}

yui::Utf8String::ConstIterator yui::Utf8String::find(u32 offset, const char c) const {
    return find_if(
            offset, [&c](const auto &decoded) {
                return decoded.length_in_bytes == 1 && static_cast<char>(decoded.code_point) == c;
            }
    );
}

yui::Utf8String::ConstIterator yui::Utf8String::find(u32 offset, const char8_t uc) const {
    return find(offset, static_cast<char>(uc));
}

yui::Utf8String::ConstIterator yui::Utf8String::find(u32 offset, u32 code_point) const {
    return find_if(
            offset, [&code_point](const auto &decoded) {
                return decoded.code_point == code_point;
            }
    );
}

yui::Utf8String::ConstIterator yui::Utf8String::find(u32 offset, const Utf8CharacterInfo &decoded_code_point) const {
    return find_if(
            offset, [&decoded_code_point](const auto &decoded) {
                return decoded_code_point.code_point == decoded.code_point;
            }
    );
}

yui::Utf8String::Iterator yui::Utf8String::find(const char c) {
    return find_if(
            [&c](const auto &decoded) {
                return decoded.length_in_bytes == 1 && static_cast<char>(decoded.code_point) == c;
            }
    );
}

yui::Utf8String::Iterator yui::Utf8String::find(const char8_t c) {
    return find(static_cast<char>(c));
}

yui::Utf8String::Iterator yui::Utf8String::find(u32 code_point) {
    return find_if(
            [&code_point](const auto &decoded) {
                return decoded.code_point == code_point;
            }
    );
}

yui::Utf8String::Iterator yui::Utf8String::find(const Utf8CharacterInfo &decoded_code_point) {
    return find_if(
            [&decoded_code_point](const auto &decoded) {
                return decoded_code_point.code_point == decoded.code_point;
            }
    );
}

yui::Utf8String::Iterator yui::Utf8String::find(u32 offset, char c) {
    return find_if(
            offset, [&c](const auto &decoded) {
                return decoded.length_in_bytes == 1 && static_cast<char>(decoded.code_point) == c;
            }
    );
}

yui::Utf8String::Iterator yui::Utf8String::find(u32 offset, char8_t uc) {
    return find(offset, static_cast<char>(uc));
}

yui::Utf8String::Iterator yui::Utf8String::find(u32 offset, u32 code_point) {
    return find_if(
            offset, [&code_point](const auto &decoded) {
                return decoded.code_point == code_point;
            }
    );
}

yui::Utf8String::Iterator yui::Utf8String::find(u32 offset, Utf8CharacterInfo &decoded_code_point) {
    return find_if(
            offset, [&decoded_code_point](const auto &decoded) {
                return decoded_code_point.code_point == decoded.code_point;
            }
    );
}

yui::Utf8String::ConstIterator yui::Utf8String::find(const char *c_str) const {
    return find(0, c_str);
}

yui::Utf8String::ConstIterator yui::Utf8String::find(const char8_t *u_str) const {
    return find(0, u_str);
}

yui::Utf8String::ConstIterator yui::Utf8String::find(const Utf8String &str) const {
    return find(0, str);
}

yui::Utf8String::ConstIterator yui::Utf8String::find(u32 offset, const char *c_str) const {
    return find(offset, Utf8String{ c_str });
}

yui::Utf8String::ConstIterator yui::Utf8String::find(u32 offset, const char8_t *u_str) const {
    return find(offset, Utf8String{ u_str });
}

yui::Utf8String::ConstIterator yui::Utf8String::find(u32 offset, const Utf8String &str) const {
    if (str.length() == length()) { return *this == str ? begin() : end(); }
    if (str.length() > length()) { return end(); }

    for (auto i = begin() + offset; i != end() && i + str.length() != end(); ++i) {
        auto found{ true };
        for (const auto &j : str) {
            if (i->code_point != j.code_point) {
                found = false;
                break;
            }
        }

        if (found) { return i; }
    }

    return end();
}

yui::Utf8String::Iterator yui::Utf8String::find(const char *c_str) {
    return find(0, c_str);
}

yui::Utf8String::Iterator yui::Utf8String::find(const char8_t *u_str) {
    return find(0, u_str);
}

yui::Utf8String::Iterator yui::Utf8String::find(const Utf8String &str) {
    return find(0, str);
}

yui::Utf8String::Iterator yui::Utf8String::find(u32 offset, const char *c_str) {
    return find(offset, Utf8String{ c_str });
}

yui::Utf8String::Iterator yui::Utf8String::find(u32 offset, const char8_t *u_str) {
    return find(offset, Utf8String{ u_str });
}

yui::Utf8String::Iterator yui::Utf8String::find(u32 offset, const Utf8String &str) {
    if (str.length() == length()) { return *this == str ? begin() : end(); }
    if (str.length() > length()) { return end(); }

    for (auto i = begin() + offset; i != end() && i + str.length() != end(); ++i) {
        auto found{ true };
        for (const auto &j : str) {
            if (i->code_point != j.code_point) {
                found = false;
                break;
            }
        }

        if (found) { return i; }
    }

    return end();
}

yui::Utf8String &yui::Utf8String::operator=(const Utf8String &rhs) = default;

yui::Utf8String &yui::Utf8String::operator=(Utf8String &&rhs) noexcept {
    m_string = std::move(rhs.m_string);
    return *this;
}

yui::Utf8String &yui::Utf8String::operator=(const char *rhs) {
    create_from_byte_stream(rhs, strlen(rhs));
    return *this;
}

yui::Utf8String &yui::Utf8String::operator=(const char8_t *rhs) {
    create_from_byte_stream(reinterpret_cast<const char *>(rhs), strlen(reinterpret_cast<const char *>(rhs)));
    return *this;
}

yui::Utf8String &yui::Utf8String::operator=(const std::string &rhs) {
    create_from_byte_stream(rhs.c_str(), rhs.length());
    return *this;
}

yui::Utf8String &yui::Utf8String::operator=(const std::string_view &rhs) {
    create_from_byte_stream(rhs.data(), rhs.length());
    return *this;
}

bool yui::Utf8String::operator==(const Utf8String &rhs) const {
    if (length() != rhs.length()) { return false; }

    for (auto i = 0u; i < length(); ++i) {
        if (code_point_at(i) != rhs.code_point_at(i)) {
            return false;
        }
    }

    return true;
}

bool yui::Utf8String::operator==(const char *rhs) const {
    return *this == Utf8String{ rhs };
}

bool yui::Utf8String::operator==(const char8_t *rhs) const {
    return *this == Utf8String{ rhs };
}

bool yui::Utf8String::operator==(const std::string &rhs) const {
    return *this == Utf8String{ rhs };
}

bool yui::Utf8String::operator==(const std::string_view &rhs) const {
    return *this == Utf8String{ rhs };
}

bool yui::Utf8String::operator!=(const Utf8String &rhs) const {
    return !(*this == rhs);
}

bool yui::Utf8String::operator!=(const char *rhs) const {
    return !(*this == rhs);
}

bool yui::Utf8String::operator!=(const char8_t *rhs) const {
    return !(*this == rhs);
}

bool yui::Utf8String::operator!=(const std::string &rhs) const {
    return !(*this == rhs);
}

bool yui::Utf8String::operator!=(const std::string_view &rhs) const {
    return !(*this == rhs);
}

yui::Optional<yui::Utf8String::Utf8CharacterInfo> yui::Utf8String::decode(const char **cs, u32 &size) {
    uint16_t length{ 0u };
    auto value{ 0u };

    if (!decode_first_byte(**cs, length, value)) {
        return { };
    }

    if (size < length) {
        return { }; // Error again.
    }

    for (auto i = 1u; i < length; ++i) {
        value <<= 6;
        value |= (*cs)[i] & 63;
    }

    *cs += length;
    size -= length;
    return Utf8CharacterInfo{ .code_point = value, .length_in_bytes = length };
}

yui::Optional<yui::Utf8String::Utf8CharacterInfo> yui::Utf8String::decode(u32 code_point) {
    uint16_t length{ 0 };
    auto value{ 0u };

    if (!decode_first_byte(static_cast<unsigned char>(code_point), length, value)) {
        return { };
    }

    return Utf8CharacterInfo{ .code_point=code_point, .length_in_bytes=length };
}

void yui::Utf8String::create_from_byte_stream(const char *str, u32 size) {
    m_string.clear();
    m_string.reserve(size); // reserve what we might need.

    const auto *end = str + size;

    while (str != end) {
        auto decoded = decode(&str, size);

        if (!decoded.has_value()) {
            ++str; // Skip this byte...
            continue;
        }
        m_string.emplace_back(decoded.value());
    }
}

bool yui::Utf8String::decode_first_byte(unsigned char byte, uint16_t &out_length, u32 &out_value) {
    if ((byte & 128) == 0) {
        out_value = byte;
        out_length = 1;
        return true;
    }
    if ((byte & 64) == 0) {
        return false;
    }
    if ((byte & 32) == 0) {
        out_value = byte & 31;
        out_length = 2;
        return true;
    }
    if ((byte & 16) == 0) {
        out_value = byte & 15;
        out_length = 3;
        return true;
    }
    if ((byte & 8) == 0) {
        out_value = byte & 7;
        out_length = 4;
        return true;
    }

    return true;
}
