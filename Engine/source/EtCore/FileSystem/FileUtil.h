#pragma once


namespace et {
namespace core {


//---------------------------------
// FileUtil
//
// Globally accessible utility for file paths
//
class FileUtil
{
public:
	static std::string AsText(const std::vector<uint8> &data);
	static void AsText(uint8 const* const data, uint64 const size, std::string& outText);
	static std::vector<uint8> FromText(const std::string &data);

	static bool ParseLine(std::string &input, std::string &extractedLine);
	static std::vector<std::string> ParseLines(std::string raw);

	static std::string ExtractPath(std::string const& fileName);
	static std::string ExtractName(std::string const& fileName);
	static std::string ExtractExtension(std::string const& fileName);
	static std::string RemoveExtension(std::string const& fileName);

	static void SetExecutablePath(std::string const& path);
	static std::string const& GetExecutableDir() { return s_ExePath; }

	static void SetCompiledData(uint8 const* const data);
	static uint8 const* GetCompiledData() { return s_CompiledData; }

	static void UnifyPathDelimiters(std::string& path);
	static void RemoveExcessPathDelimiters(std::string& path);
	static bool RemoveRelativePath(std::string& path);
	static std::string SplitFirstDirectory(std::string& path);
	static std::string GetAbsolutePath(std::string const& path);
	static std::string GetRelativePath(std::string const& inPath, std::string const& rootPath);

	static bool IsAbsolutePath(std::string const& path);

private:
	static std::string s_ExePath;
	static uint8 const* s_CompiledData;
};


} // namespace core
} // namespace et
