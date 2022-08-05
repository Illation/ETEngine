#include "stdafx.h"
#include "FileUtil.h"

#include <limits>

#include <EtCore/Platform/PlatformUtil.h>


namespace et {
namespace core {


//==============================
// File Util
//==============================


// static variables
static std::vector<std::string> newLineTokens{ "\r\n", "\n\r", "\n", "\r" };
static std::vector<char> pathDelimiters{ '\\', '/' };
static char const s_PathDelimiter = '/';
std::string FileUtil::s_ExePath = std::string();
uint8 const* FileUtil::s_CompiledData = nullptr;


//---------------------------------
// FileUtil::AsText
//
// Converts a byte vector to an std::string
//
std::string FileUtil::AsText(const std::vector<uint8> &data)
{
	return std::string(data.begin(), data.end());
}

//---------------------------------
// FileUtil::AsText
//
// Converts a byte array to an std::string
//
void FileUtil::AsText(uint8 const* const data, uint64 const size, std::string& outText)
{
	outText.append(reinterpret_cast<char const*>(data), static_cast<size_t>(size));
}

//---------------------------------
// FileUtil::FromText
//
// Converts an std::string to a byte array
//
std::vector<uint8> FileUtil::FromText(const std::string &data)
{
	return std::vector<uint8>(data.begin(), data.end());
}

//---------------------------------
// FileUtil::ParseLine
//
// Removes one line from the input string and places it in the referenced extractedLine
//  - returns false if no line could be extracted
//
bool FileUtil::ParseLine(std::string &input, std::string &extractedLine)
{
	if (input.size() == 0)
	{
		return false;
	}

	int32 closestIdx = std::numeric_limits<int32>::max();
	uint32 tokenSize = 0;
	for (auto token : newLineTokens)
	{
		int32 index = (int32)input.find(token);
		if (index != std::string::npos && index < closestIdx)
		{
			closestIdx = index;
			tokenSize = (uint32)token.size();
		}
	}

	if (closestIdx != std::string::npos && static_cast<uint32>(closestIdx) < input.size())
	{
		extractedLine = input.substr(0, closestIdx);
		input = input.substr(closestIdx + tokenSize);
		if (input.size() == 0) input = "";
		return true;
	}
	extractedLine = input;
	input = "";
	return true;
}

//---------------------------------
// FileUtil::ParseLines
//
// Converts a raw string to a list of lines
//
std::vector<std::string> FileUtil::ParseLines(std::string raw)
{
	std::vector<std::string> ret;
	std::string extractedLine;
	while (FileUtil::ParseLine(raw, extractedLine))
	{
		ret.push_back(extractedLine);
	}
	return ret;
}

//---------------------------------
// FileUtil::ExtractPath
//
// Extract the path from a path with file name
//
std::string FileUtil::ExtractPath(std::string const& fileName)
{
	// if the string ends with a delimiter we treat the last directory as the name
	if (fileName.empty())
	{
		return fileName;
	}

	size_t pos = fileName.size() - 1;
	while (std::find(pathDelimiters.cbegin(), pathDelimiters.cend(), fileName[pos]) != pathDelimiters.cend())
	{
		--pos;
	}

	// Get the last path delimiter
	size_t closestIdx = 0u;
	for (char const token : pathDelimiters)
	{
		size_t index = fileName.rfind(token, pos);
		if (index != std::string::npos && index > closestIdx)
		{
			closestIdx = index;
		}
	}

	// if the closest "delimiter is not an actual delimiter", the file has no path attached
	if (std::find(pathDelimiters.cbegin(), pathDelimiters.cend(), fileName[closestIdx]) == pathDelimiters.cend())
	{
		return "";
	}

	return fileName.substr(0, closestIdx + 1);
}

//---------------------------------
// FileUtil::ExtractName
//
// Extract the filename from a path
//
std::string FileUtil::ExtractName(std::string const& fileName)
{
	// if the string ends with a delimiter we treat the last directory as the name
	if (fileName.empty())
	{
		return fileName;
	}

	size_t pos = fileName.size() - 1;
	while (std::find(pathDelimiters.cbegin(), pathDelimiters.cend(), fileName[pos]) != pathDelimiters.cend())
	{
		--pos;
	}

	// Get the last path delimiter
	size_t closestIdx = 0u;
	for (char const token : pathDelimiters)
	{
		size_t index = fileName.rfind(token, pos);
		if (index != std::string::npos && index > closestIdx)
		{
			closestIdx = index;
		}
	}

	// if the closest "delimiter is not an actual delimiter", the file has no path attached
	if (std::find(pathDelimiters.cbegin(), pathDelimiters.cend(), fileName[closestIdx]) == pathDelimiters.cend())
	{
		return fileName;
	}

	return fileName.substr(closestIdx + 1);
}

//---------------------------------
// FileUtil::ExtractName
//
// Extract the extension from a filename
//
std::string FileUtil::ExtractExtension(std::string const& fileName)
{
	std::size_t found = fileName.rfind(".");

	if (found != std::string::npos)
	{
		return fileName.substr(found + 1);
	}

	return std::string("");
}

//---------------------------------
// FileUtil::RemoveExtension
//
// Remove the extension from a filename
//
std::string FileUtil::RemoveExtension(std::string const& fileName)
{
	std::size_t found = fileName.rfind(".");

	if (found != std::string::npos)
	{
		return fileName.substr(0, found);
	}

	return fileName;
}

//---------------------------------
// FileUtil::SetExecutablePath
//
// Sets the base path the executable lives in 
//
void FileUtil::SetExecutablePath(std::string const& inPath)
{
	std::string path;
#ifdef ET_PLATFORM_WIN
	platform::GetExecutablePathName(path);
#else
	path = inPath;
#endif

	ET_ASSERT(IsAbsolutePath(path));

	// set the executable path to the part without the exe name
	s_ExePath = ExtractPath(path);
	UnifyPathDelimiters(s_ExePath);
}

//---------------------------------
// FileUtil::SetCompiledData
//
// Sets the compiled data, should only be called once at startup
//
void FileUtil::SetCompiledData(uint8 const* const data)
{
	s_CompiledData = data;
}

//---------------------------------
// FileUtil::RemoveExcessPathDelimiters
//
// Makes all path delimiters forward slashes
//
void FileUtil::UnifyPathDelimiters(std::string& path)
{
	// loop from the back so that our index stays valid as we remove elements
	for (size_t idx = path.size() - 1; idx > 0; --idx)
	{
		// check if the current character is a delimiter
		if (std::find(pathDelimiters.cbegin(), pathDelimiters.cend(), path[idx]) != pathDelimiters.cend())
		{
			path[idx] = s_PathDelimiter;
		}
	}
}

//---------------------------------
// FileUtil::RemoveExcessPathDelimiters
//
// Removes double / or \
//
void FileUtil::RemoveExcessPathDelimiters(std::string& path)
{
	// loop from the back so that our index stays valid as we remove elements
	for (size_t idx = path.size() - 1; idx > 0; --idx)
	{
		// check if the current character is a delimiter
		if (std::find(pathDelimiters.cbegin(), pathDelimiters.cend(), path[idx]) != pathDelimiters.cend())
		{
			// check if the next (reverse previous) character is also a delimiter
			if (std::find(pathDelimiters.cbegin(), pathDelimiters.cend(), path[idx - 1]) != pathDelimiters.cend())
			{
				// remove the current character before moving on to the next
				path.erase(idx, 1);
			}
		}
	}
}

//---------------------------------
// FileUtil::GetAbsolutePath
//
// Gets the absolute path relative to the executable - returns false if none where found
//
bool FileUtil::RemoveRelativePath(std::string& path)
{
	// get the first '/.' sequence
	size_t closestIdx = std::numeric_limits<size_t>::max();
	for (char const token : pathDelimiters)
	{
		size_t index = path.find(std::string(1, token) + std::string("."));
		if (index != std::string::npos && index < closestIdx)
		{
			closestIdx = index;
		}
	}

	// if none is found our work is done
	if (closestIdx > path.size())
	{
		return false;
	}

	// otherwise we check if we are referring to this directory or the parent directory
	size_t preCut = closestIdx;
	size_t postCut = closestIdx + 1;
	if (closestIdx + 2 < path.size() && path[closestIdx + 2] == '.')
	{
		// we have a parentDir
		postCut++;

		// find the index of the previous ("parent") delim
		size_t lastDelim = 0u;
		for (char const token : pathDelimiters)
		{
			size_t index = path.rfind(token, preCut - 1);
			if (index != std::string::npos && index > lastDelim)
			{
				lastDelim = index;
			}
		}

		// Check we found a parent directory
		if (lastDelim == 0u || lastDelim >= preCut - 1)
		{
			ET_TRACE_E(ET_CTX_CORE, "FileUtil::RemoveRelativePath > expected to find a parent directory to cut");
			return false;
		}

		preCut = lastDelim;
	}

	path = path.substr(0, preCut) + path.substr(postCut + 1);

	return true;
}

//---------------------------------
// FileUtil::SplitFirstDirectory
//
// Returns the first directory component, if any. The path variable will be modified to contain what is left
//
std::string FileUtil::SplitFirstDirectory(std::string& path)
{
	// get the first delimiter
	size_t closestIdx = std::numeric_limits<size_t>::max();
	for (char const token : pathDelimiters)
	{
		size_t index = path.find(token);
		if (index != std::string::npos && index < closestIdx)
		{
			closestIdx = index;
		}
	}

	// if none is found return an empty path
	if (closestIdx > path.size())
	{
		return "";
	}

	++closestIdx;

	std::string ret = path.substr(0, closestIdx);
	path = path.substr(closestIdx);
	return ret;
}

//---------------------------------
// FileUtil::GetAbsolutePath
//
// Gets the absolute path relative to the executable
//
std::string FileUtil::GetAbsolutePath(std::string const& path)
{
	// full path unedited
	std::string combinedPath;

	// if the path is already absolute we just trim without appending the exe dir
	if (IsAbsolutePath(path))
	{
		combinedPath = path;
	}
	else
	{
		combinedPath = GetExecutableDir() + path;
	}

	// remove excess delimiters first so that removing parent directories is reliable
	RemoveExcessPathDelimiters(combinedPath);

	// remove .. and . tokens until there are none left
	bool isRelative = true;
	while (isRelative)
	{
		isRelative = RemoveRelativePath(combinedPath);
	}

	UnifyPathDelimiters(combinedPath);
	return combinedPath;
}

//---------------------------------
// FileUtil::GetRelativePath
//
// Converts inPath to a path relative to rootPath - assumes both arguments paths are relative to the executable directory
//
std::string FileUtil::GetRelativePath(std::string const& inPath, std::string const& rootPath)
{
	// Make sure both paths have the same format
	std::string absIn = GetAbsolutePath(inPath);
	std::string absRoot = GetAbsolutePath(rootPath);

	// Find the first letter that is different
	size_t idx = 0u;
	for (;idx < std::min(absIn.size(), absRoot.size()); ++idx)
	{
		if (absIn[idx] != absRoot[idx])
		{
			break;
		}
	}

	// trace backwards to the last path delimiter - delimiters should be all the same character (s_PathDelimiter) now due to GetAbsolutePath()
	while (idx > 0u)
	{
		--idx;
		if (absIn[idx] == s_PathDelimiter)
		{
			break;
		}
	}

	// figure out how many times we need to go to the parent directory
	size_t searchStart = idx; 
	size_t numParentDirs = 0u;
	for (;;)
	{
		searchStart = absRoot.find(s_PathDelimiter, ++searchStart); // idx is on a path delimiter right now, so we start from the next character
		if (searchStart != std::string::npos) // if we found another delimiter we have more parent directories
		{
			++numParentDirs;
		}
		else
		{
			break; // if we can't find any more we have reached the end of the string
		}
	}

	// build our output path

	// first escape the root directory into the first common directory
	std::string retVal;
	while (numParentDirs > 0u)
	{
		retVal += "../";
		--numParentDirs;
	}
	
	// append the rest of our input path
	// if the current common index is a delimiter we skip that
	if (absIn[idx] == s_PathDelimiter)
	{
		++idx;
	}
	return retVal + absIn.substr(idx);
}

//---------------------------------
// FileUtil::GetAbsolutePath
//
// Gets the absolute path relative to the executable
//
bool FileUtil::IsAbsolutePath(std::string const& path)
{
#if defined(ET_PLATFORM_LINUX)

	return path[0] == '/' || path[0] == '~';

#elif defined(ET_PLATFORM_WIN)

	// if the path has a colon, it's absolute
	return path.find(":") != std::string::npos;

#else

	ET_TRACE_W(ET_CTX_CORE, "FileUtil::IsAbsolutePath > Checking if a path is absolute is not implemented for this platform!");
	return false;

#endif
}


} // namespace core
} // namespace et
