#pragma once

class FileUtil
{
public:
	static std::string AsText( const std::vector<uint8> &data );
	static std::vector<uint8> FromText( const std::string &data );

	static bool ParseLine( std::string &input, std::string &extractedLine );
	static std::vector<std::string> ParseLines( std::string raw );

	static std::string ExtractPath(const std::string& fileName);
	static std::string ExtractName(const std::string& fileName);
};