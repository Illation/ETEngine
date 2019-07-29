#include "CompiledDataGenerator.h"

#include <Engine/stdafx.h>

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>


//---------------------------------
// GenerateCompilableResource
//
// Generates a header and source file containing a data array that can get compiled into the executable
//
void GenerateCompilableResource(std::vector<uint8> const& data, std::string const& name, std::string const& path)
{
	// Ensure the generated file directory exists
	Directory* dir = new Directory(path, nullptr, true); 

	// consistency of var names
	std::string compiledDataName("s_CompiledData_" + name);

	// file access flags for generated source files
	FILE_ACCESS_FLAGS outFlags;
	outFlags.SetFlags(FILE_ACCESS_FLAGS::FLAGS::Create | FILE_ACCESS_FLAGS::FLAGS::Exists);

	// create the header file
	File* header = new File(name + std::string(".h"), dir);
	header->Open(FILE_ACCESS_MODE::Write, outFlags);
	if (!(header->Write(FileUtil::FromText(generator_detail::GetHeaderString(name, compiledDataName)))))
	{
		LOG("GenerateCompilableResource > Couldn't write header file", LogLevel::Warning);
	}

	// cleanup header
	header->Close();
	delete header;
	header = nullptr;

	// create the source file
	File* source = new File(name + std::string(".cpp"), dir);
	source->Open(FILE_ACCESS_MODE::Write, outFlags);
	if (!(source->Write(FileUtil::FromText(generator_detail::GetSourceString(data, name, compiledDataName)))))
	{
		LOG("GenerateCompilableResource > Couldn't write source file", LogLevel::Warning);
	}

	// cleanup source
	source->Close();
	delete source;
	source = nullptr;

	// cleanup directory
	delete dir;
	dir = nullptr;
}


namespace generator_detail {


//---------------------------------
// GetHeaderString
//
// Returns a string for the file header
//
std::string GetHeaderString(std::string const& name, std::string const& compiledDataName)
{
	return std::string(
		"#pragma once\n"
		"\n"
		"namespace generated {\n"
		"\n"
		"struct wrapper\n"
		"{\n"
		"\tstatic unsigned char const ") + compiledDataName + std::string("[];\n"
		"};\n"
		"\n"
		"} // namespace generated\n"
		"\n"
		"unsigned char const* GetCompiledData_") + name + std::string("();\n"
		);
}

//---------------------------------
// GetHeaderString
//
// Returns a string for the file header
//
std::string GetSourceString(std::vector<uint8> const& data, std::string const& name, std::string const& compiledDataName)
{
	// #include "name.h"
	std::string ret("#include \"");
	ret += name + std::string(".h\"\n\n");

	// begin of array
	ret += 
		"\n"
		"unsigned char const* GetCompiledData_" + name + std::string("()\n"
		"{\n"
		"\treturn generated::wrapper::") + compiledDataName + std::string(";\n"
		"}\n"
		"\n"
		"namespace generated { \n"
		"\n"
		"unsigned char const wrapper::") + compiledDataName + std::string("[] = {");

	// end of file
	std::string eof(
		" };\n"
		"\n"
		"} // namespace generated\n");

	// make sure our string is allocated enough space in one go
	static size_t const s_NumHexChars = 6u;// number of characters per byte
	static size_t const s_StringSizeBuffer = 1024u;// number of characters per byte
	ret.reserve(ret.size() + data.size() * s_NumHexChars + eof.size() + s_StringSizeBuffer);

	// add the bytes
	static size_t const s_BytesPerLine = 16u;
	static unsigned char s_HexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	for (size_t dataIdx = 0u; dataIdx < data.size(); ++dataIdx)
	{
		// insert new line or space
		if (dataIdx % s_BytesPerLine == 0u)
		{
			ret += "\n\t";
		}
		else if (dataIdx % (s_BytesPerLine / 2) == 0u)
		{
			ret += "    ";
		}
		else
		{
			ret += " ";
		}

		// indicate hex number
		ret += "0x";

		// generate hex code
		uint8 const currentByte = data[dataIdx];
		uint8 const bigEndian = currentByte >> 4u;
		uint8 const smallEndian = currentByte & 0x0F;

		ret += s_HexChars[bigEndian];
		ret += s_HexChars[smallEndian];

		// insert delim
		if (dataIdx != data.size() - 1)
		{
			ret += ",";
		}
	}

	// add file end
	ret += eof;

	// should move string data instead of copy
	return ret;
}


} // namespace generator_detail

