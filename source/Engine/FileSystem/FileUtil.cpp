#include "stdafx.hpp"
#include "FileUtil.h"

#include <limits>

std::vector<std::string> FileUtil::ParseLines( std::string raw )
{
	std::vector<std::string> result;
	std::vector<std::string> tokens{ "\r\n", "\n\r", "\n", "\r" };
	while(raw.size())
	{
		int32 closestIdx = std::numeric_limits<int32>::max();
		uint32 tokenSize = 0;
		for(auto token : tokens)
		{
			int32 index = raw.find( token );
			if(index != std::string::npos && index < closestIdx)
			{
				closestIdx = index;
				tokenSize = token.size();
			}
		}
		if(closestIdx != std::string::npos)
		{
			result.push_back( raw.substr( 0, closestIdx ) );
			raw = raw.substr( closestIdx + tokenSize );
			if(raw.size() == 0)result.push_back( raw );
		}
		else
		{
			result.push_back( raw );
			raw = "";
		}
	}
	return result;
}
