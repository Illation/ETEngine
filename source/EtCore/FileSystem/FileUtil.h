#pragma once

//---------------------------------
// FileUtil
//
// Globally accessible utility for file paths
//
class FileUtil
{
public:
	static std::string AsText(const std::vector<uint8> &data);
	static std::vector<uint8> FromText(const std::string &data);

	static bool ParseLine(std::string &input, std::string &extractedLine);
	static std::vector<std::string> ParseLines(std::string raw);

	static bool GetCompiledResource(std::string const& path, std::vector<uint8>& data);

	static std::string ExtractPath(std::string const& fileName);
	static std::string ExtractName(std::string const& fileName);

	static void SetExecutablePath(std::string const& path);
	static std::string const& GetExecutableDir() { return s_ExePath; }

	static void UnifyPathDelimiters(std::string& path);
	static void RemoveExcessPathDelimiters(std::string& path);
	static bool RemoveRelativePath(std::string& path);
	static std::string GetAbsolutePath(std::string const& path);
	static std::string GetRelativePath(std::string const& inPath, std::string const& rootPath);

	static bool IsAbsolutePath(std::string const& path);

private:
	static std::string s_ExePath;
};