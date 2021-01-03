#pragma once

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>

#include "JsonSerializer.h"
#include "JsonDeserializer.h"


// Inline functions
//////////////////////


namespace et {
namespace core {

namespace serialization {


//---------------------------------
// SerializeToFile
//
// Write a file from the templated type using reflection data. Returns false if serialization is unsuccsesful.
//
template<typename T>
bool SerializeToFile(std::string const& filePath, T const& serialObject)
{
	// Open the file
	File* file = new File(filePath, nullptr);
	std::string const ext(file->GetExtension());

	// convert the object to the files data format, depending on the extension
	bool serializeSuccess = false;
	std::vector<uint8> fileContent;
	if (ext == "json")
	{
		// first convert to the json document object model
		JsonSerializer serializer;
		if (serializer.SerializeToData(serialObject, fileContent))
		{
			serializeSuccess = true;
		}
	}
	else
	{
		LOG("SerializeToFile > File type '" + ext + std::string("' not supported!"), Warning);
	}

	// write the content to the file
	if (serializeSuccess)
	{
		FILE_ACCESS_FLAGS outFlags;
		outFlags.SetFlags(FILE_ACCESS_FLAGS::FLAGS::Create | FILE_ACCESS_FLAGS::FLAGS::Exists); // create a new file or overwrite the existing one
		if (!file->Open(FILE_ACCESS_MODE::Write, outFlags))
		{
			LOG("SerializeToFile > unable to open file '" + filePath + std::string("' for writing!"), Warning);
			serializeSuccess = false;
		}

		if (serializeSuccess && !file->Write(fileContent))
		{
			LOG("SerializeToFile > Writing content to file failed!", Warning);
			serializeSuccess = false;
		}
	}

	SafeDelete(file);
	return serializeSuccess;
}


//---------------------------------
// DeserializeFromJson
//
// Create the reflected type from a file, the file type is determined by the extension
// Returns nullptr if deserialization is unsuccsesful. 
//
template<typename T>
bool DeserializeFromFile(std::string const& filePath, T& outObject)
{
	// Open the file
	File* file = new File(filePath, nullptr);
	if (!file->Open(FILE_ACCESS_MODE::Read))
	{
		LOG("DeserializeFromFile > unable to open file '" + filePath + std::string("'!"), Warning);
		return false;
	}

	// extract the necessary information
	std::string const ext(file->GetExtension());
	std::vector<uint8> const content(file->Read());

	// We can now close the file again
	SafeDelete(file);

	// for now json is the only supported format
	if (ext == "json")
	{		
		JsonDeserializer deserializer;
		return deserializer.DeserializeFromData(content, outObject);
	}

	LOG("DeserializeFromFile > File type '" + ext + std::string("' not supported!"), Warning);
	return false;
}


} // namespace serialization

} // namespace core
} // namespace et
