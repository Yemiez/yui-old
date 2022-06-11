#pragma once
#include <cassert>
#include <fstream>
#include <string>
#include <cstdio>
#include <numeric>
#include <vector>

#include "Utf8String.h"

namespace yui {
	// trim from start (in place)
	static inline void ltrim(std::string &s) {
	    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
	        return !std::isspace(ch);
	    }));
	}

	// trim from end (in place)
	static inline void rtrim(std::string &s) {
	    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
	        return !std::isspace(ch);
	    }).base(), s.end());
	}

	// trim from both ends (in place)
	static inline void trim(std::string &s) {
	    ltrim(s);
	    rtrim(s);
	}

	// trim from start (copying)
	static inline std::string ltrim_copy(std::string s) {
	    ltrim(s);
	    return s;
	}

	// trim from end (copying)
	static inline std::string rtrim_copy(std::string s) {
	    rtrim(s);
	    return s;
	}

	// trim from both ends (copying)
	static inline std::string trim_copy(std::string s) {
	    trim(s);
	    return s;
	}

	static inline std::vector<std::string> split(std::string str, const char delimiter)
	{
		std::vector<std::string> result{};

		auto pos = str.find(delimiter);
		do {
			result.emplace_back(str.substr(0, pos));
			str.erase(0, pos + 1);
		} while((pos = str.find(delimiter)) != std::string::npos && !str.empty());

		if (!str.empty()) {
			result.emplace_back(std::move(str));
		}

		return result;
	}

	static inline std::string join(const std::vector<std::string>& strings, const char glue)
	{
		std::string result{};
		result.resize(std::accumulate(strings.begin(), strings.end(), 1, [](auto carry, auto s) { return carry + s.length(); }) + (strings.size()), '\0');

		uint32_t idx {0};

		for (const auto& str : strings) {
			for (const auto& c : str) {
				result[idx++] = c;
			}
			result[idx++] = glue;
		}

		return result;
	}
	
	static inline std::string read_file(const std::string& file)
	{
		std::ifstream stream{file};
		stream.unsetf(std::ios::skipws);
		return {
			std::istream_iterator<char>{stream},
			std::istream_iterator<char>{},
		};
	}

	extern Utf8String read_utf8_file(const std::string& filename);

	static std::string escape_char(char c)
	{
		switch(c) {
		case '\t':
			return "\\t";
		case '\n':
			return "\\n";
		case '\r':
			return "\\r";
		case '\b':
			return "\\b";
		default:
			std::string s{};
			s.push_back(c);
			return s;
		}
	}
	
	static std::string escape_code_point(const yui::Utf8String::Utf8CharacterInfo& cp)
	{
		if (cp.length_in_bytes == 1) {
			return escape_char(*cp.begin());
		}
		std::string result;
		result.append("\\u");
		for (const auto &c : cp) {
			char buf[256];
			sprintf(buf, "%02x", c);
			result.append(buf);
		}
		return result;
	}

	static inline std::string escape_content(const std::string& content)
	{
		std::string result{};
		result.reserve(content.length());

		for (const auto& c : content) {
			result.append(escape_char(c));
		}

		return result;
	}

	template<typename...Arguments>
	static inline std::string fmt(const char* fmt, Arguments&&...arguments)
	{
		size_t size = ::snprintf(nullptr, 0, fmt, arguments...);
		if (size == 0) {
			return {};
		}
		char *bufsz = new char[size+ 1];
		snprintf(bufsz, size + 1, fmt, arguments...);
		std::string result{bufsz};
		delete[] bufsz;
		return result;
	}
}
