#include "stdafx.hpp"
#include "JSONparser.h"

#include <functional>
#include <cwctype>
#include <locale>

JSONparser::JSONparser(const std::string &textFile)
{
	m_Root = new JSONobject();


}

JSONparser::~JSONparser()
{
	delete m_Root;
	m_Root = nullptr;
}

void JSONparser::MoveToNonWhitespace(const std::string &textFile)
{
	//std::string::iterator it = std::find_if(textFile.begin(), textFile.end(), [](const char& c) { return !std::isspace(c); });
}
