#pragma once

class FileUtil
{
public:
	static bool ParseLine( std::string &input, std::string &extractedLine );
	static std::vector<std::string> ParseLines( std::string raw );
};