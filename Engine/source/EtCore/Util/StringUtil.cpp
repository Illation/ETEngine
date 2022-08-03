#include "stdafx.h"
#include "StringUtil.h"

#include <cctype>
#include <locale>


namespace et {
namespace core {


//------------------------
// SeparateByWhitespace
//
// Get multiple c style strings
//
std::vector<std::string> SeparateByWhitespace(std::string const& inStr)
{
	std::vector<std::string> ret;

	bool isWhiteSpace = true;
	std::string curr;
	for (size_t i = 0u; i < inStr.size(); ++i)
	{
		if (!std::isspace(inStr[i]))
		{
			curr.push_back(inStr[i]);
			isWhiteSpace = false;
		}
		else
		{
			if (!isWhiteSpace)
			{
				ret.push_back(curr);
				curr.clear();
			}

			isWhiteSpace = true;
		}
	}

	if (!isWhiteSpace)
	{
		ret.push_back(curr);
	}

	return ret;
}


} // namespace core
} // namespace et
