#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <cstdio>

// from https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf


//---------------------------------------------------------------------------------
// string_format
//
// generate a string with formated arguments
//
template<typename ... Args>
std::string string_format(char const* format = nullptr, Args ... args)
{
	if (format == nullptr)
	{
		return std::string();
	}

	size_t size = snprintf(nullptr, 0, format, args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format, args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}


#define FS(...) string_format(__VA_ARGS__)
