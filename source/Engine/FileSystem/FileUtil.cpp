#include "stdafx.hpp"
#include "FileUtil.h"

#include <limits>

static std::vector<std::string> newLineTokens{ "\r\n", "\n\r", "\n", "\r" };

bool FileUtil::ParseLine( std::string &input, std::string &extractedLine )
{
	if(input.size() == 0)return false;

	int32 closestIdx = std::numeric_limits<int32>::max();
	uint32 tokenSize = 0;
	for(auto token : newLineTokens)
	{
		int32 index = input.find( token );
		if(index != std::string::npos && index < closestIdx)
		{
			closestIdx = index;
			tokenSize = token.size();
		}
	}

	if(closestIdx != std::string::npos && static_cast<uint32>(closestIdx) < input.size())
	{
		extractedLine = input.substr( 0, closestIdx );
		input = input.substr( closestIdx + tokenSize );
		if(input.size() == 0) input = "";
		return true;
	}
	extractedLine = input;
	input = "";
	return true;
}

std::vector<std::string> FileUtil::ParseLines( std::string raw )
{
	std::vector<std::string> ret;
	std::string extractedLine;
	while(FileUtil::ParseLine(raw, extractedLine))
	{
		ret.push_back( extractedLine );
	}
	return ret;
}