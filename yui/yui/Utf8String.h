#pragma once
#include <string>
#include <vector>
#include "Optional.h"

#include "Types.h"
#include <algorithm>

namespace yui {
	class Utf8String
	{
	public:
		struct Utf8CharacterInfo
		{
			u32 code_point;
			unsigned short length_in_bytes;
			const char* begin() const { return reinterpret_cast<const char*>(&code_point); }
			const char* end() const { return reinterpret_cast<const char*>(&code_point) + length_in_bytes; }
		};
		using VectorString = std::vector<Utf8CharacterInfo>;

		using Iterator = VectorString::iterator;
		using ConstIterator = VectorString::const_iterator;
	public:
		explicit Utf8String(const char* str);
		explicit Utf8String(const char8_t* str);
		explicit Utf8String(std::string_view str);
		explicit Utf8String(const std::string& str);
		Utf8String(const Utf8String& other);
		Utf8String(Utf8String&& other) noexcept;
		Utf8String() = default;

		void clear();
		void resize(u32 i);
		void resize(u32 i, const Utf8CharacterInfo& value);
		Utf8CharacterInfo* data() { return m_string.data(); }
		u32 length() const { return m_string.size(); }
		bool empty() const { return m_string.empty(); }
		bool contains(const Utf8String& substring) const;
		bool contains(const std::string& substring) const;
		bool contains(const std::string_view& substring) const;

		Iterator begin() { return m_string.begin(); }
		Iterator end() { return m_string.end(); }
		ConstIterator begin() const { return m_string.cbegin(); }
		ConstIterator end() const { return m_string.cend(); }
		ConstIterator cbegin() const { return m_string.cbegin(); }
		ConstIterator cend() const { return m_string.cend(); }

		// Convert to a UTF-8 byte string (this is the RAW UTF-8 encoded string)
		std::string to_byte_string() const;

		std::string to_printable_string() const;

		u32 code_point_at(u32 idx) const { return at(idx).code_point; }
		const Utf8CharacterInfo& at(u32 idx) const { return m_string.at(idx); }
		Utf8CharacterInfo& at(u32 idx) { return m_string.at(idx); }

		// Decodes the code point and appends it.
		void push_back(u32 code_point);
		void push_back(Utf8CharacterInfo code_point);

		// Assumes that this is ascii (otherwise use push_back with u32 to perform proper decoding).
		void push_back(char byte);

		void append(const Utf8String&);
		void append(const ConstIterator& begin, const ConstIterator& end);

		Iterator insert(u32 where, const Utf8String&);
		Iterator insert(const Iterator& where, const Utf8String&);
		Iterator insert(u32 where, u32 code_point);
		Iterator insert(const Iterator& where, u32 code_point);
		Iterator erase(u32 where);
		Iterator erase(u32 from, u32 to);
		Iterator erase(const Iterator& where);
		Iterator erase(const Iterator& from, const Iterator& to);
		


		// single code point / character
		ConstIterator find(char) const;
		ConstIterator find(char8_t) const;
		ConstIterator find(u32 code_point) const;
		ConstIterator find(const Utf8CharacterInfo& decoded_code_point) const;
		ConstIterator find(u32 offset, char) const;
		ConstIterator find(u32 offset, char8_t) const;
		ConstIterator find(u32 offset, u32 code_point) const;
		ConstIterator find(u32 offset, const Utf8CharacterInfo& decoded_code_point) const;
		Iterator find(char);
		Iterator find(char8_t);
		Iterator find(u32 code_point);
		Iterator find(const Utf8CharacterInfo& decoded_code_point);
		Iterator find(u32 offset, char);
		Iterator find(u32 offset, char8_t);
		Iterator find(u32 offset, u32 code_point);
		Iterator find(u32 offset, Utf8CharacterInfo& decoded_code_point);

		// Entire string of code points / characters
		ConstIterator find(const char*) const;
		ConstIterator find(const char8_t*) const;
		ConstIterator find(const Utf8String&) const;
		ConstIterator find(u32 offset, const char*) const;
		ConstIterator find(u32 offset, const char8_t*) const;
		ConstIterator find(u32 offset, const Utf8String&) const;
		Iterator find(const char*);
		Iterator find(const char8_t*);
		Iterator find(const Utf8String&);
		Iterator find(u32 offset, const char*);
		Iterator find(u32 offset, const char8_t*);
		Iterator find(u32 offset, const Utf8String&);

		template<typename Callable>
		ConstIterator find_if(Callable predicate) const;
		template<typename Callable>
		ConstIterator find_if(u32 offset, Callable predicate) const;
		template<typename Callable>
		Iterator find_if(Callable predicate);
		template<typename Callable>
		Iterator find_if(u32 offset, Callable predicate);
		
		static Optional<Utf8CharacterInfo> decode(u32 code_point);
	public: // Modifying operators
		Utf8String& operator=(const Utf8String& rhs);
		Utf8String& operator=(Utf8String&& rhs) noexcept;
		Utf8String& operator=(const char* rhs);
		Utf8String& operator=(const char8_t* rhs);
		Utf8String& operator=(const std::string& rhs);
		Utf8String& operator=(const std::string_view& rhs);

	public: // Comparison operator
		bool operator==(const Utf8String& rhs) const;
		bool operator==(const char* rhs) const;
		bool operator==(const char8_t* rhs) const;
		bool operator==(const std::string& rhs) const;
		bool operator==(const std::string_view& rhs) const;
		bool operator!=(const Utf8String& rhs) const;
		bool operator!=(const char* rhs) const;
		bool operator!=(const char8_t* rhs) const;
		bool operator!=(const std::string& rhs) const;
		bool operator!=(const std::string_view& rhs) const;
	private:
		static Optional<Utf8CharacterInfo> decode(const char** cs, u32& size);
		void create_from_byte_stream(const char* str, u32 size);
		static bool decode_first_byte(unsigned char byte, uint16_t& out_length, u32& out_value);
	private:
		VectorString m_string{};
	};

	template <typename Callable>
	Utf8String::ConstIterator Utf8String::find_if(Callable predicate) const
	{
		return std::find_if(begin(), end(), predicate);
	}

	template <typename Callable>
	Utf8String::ConstIterator Utf8String::find_if(u32 offset, Callable predicate) const
	{
		return std::find_if(begin() + offset, end(), predicate);
	}

	template <typename Callable>
	Utf8String::Iterator Utf8String::find_if(Callable predicate)
	{
		return std::find_if(begin(), end(), predicate);
	}

	template <typename Callable>
	Utf8String::Iterator Utf8String::find_if(u32 offset, Callable predicate)
	{
		return std::find_if(begin() + offset, end(), predicate);
	}

}
