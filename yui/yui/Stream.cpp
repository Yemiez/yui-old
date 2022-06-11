#include "Stream.h"

/*
uint32_t yui::StreamReader<std::string, char>::size() const
{
	return m_container.length();
}

char& yui::StreamReader<std::string, char>::consume()
{
	if (reached_eof()) throw std::runtime_error("Attempted to consume past eof");
	auto& c = m_container.at(m_cursor++);
	if (c == '\n') {
		++m_row;
		m_col = 0;
	}
	else {
		++m_col;
	}
	return c;
}*/