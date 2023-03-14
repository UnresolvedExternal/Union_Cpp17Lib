#pragma once

#include <sstream>

template <class T>
string StdToString(const T& value)
{
	std::ostringstream out;
	out << value;
	return out.str().c_str();
}

template <class T>
T StdFromString(const string& text)
{
	std::istringstream in{ text.GetVector() };
	T value;
	in >> value;
	return value;
}

template <>
string StdToString(const string& value)
{
	return value;
}

template <>
string StdFromString(const string& value)
{
	return value;
}

CCmd& operator<<(CCmd& cmd, const char* value)
{
	return cmd << string{ value };
}

CCmd& operator<<(CCmd& cmd, const std::string& value)
{
	return cmd << value.c_str();
}