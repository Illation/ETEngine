#pragma once

// Inline functions
//////////////////////


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
		JSON::Object* root = static_cast<JSON::Object*>(SerializeToJson(serialObject));
		if (root)
		{
			// then write it to a string
			JSON::Writer writer(false);
			if (writer.Write(root))
			{
				// convert that string to a byte array
				fileContent = FileUtil::FromText(writer.GetResult()); // #todo this shouldn't copy twice, pass content byte vector as reference
				serializeSuccess = true;
			}
			else
			{
				LOG("SerializeToFile > unable to write JSON DOM to string", Warning);
			}
		}
		else
		{
			LOG("SerializeToFile > unable to serialize object to JSON DOM!", Warning);
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
// SerializeToJson
//
// Create a JSON::Object from the templated type using reflection data. Returns nullptr if serialization is unsuccsesful.
//
template<typename T>
JSON::Value* SerializeToJson(T const& serialObject)
{
	rttr::instance inst(serialObject);
	if (!inst.is_valid())
	{
		LOG("SerializeToJson > Couldn't create a valid instance from the object to serialize!", Warning);
		return nullptr;
	}

	JSON::Value* outObject = nullptr;

	if (ToJsonRecursive(inst, outObject))
	{
		JSON::Object* root = new JSON::Object();

		root->value.emplace_back(inst.get_type().get_name().to_string(), outObject);

		return static_cast<JSON::Value*>(root);
	}

	SafeDelete(outObject);
	return nullptr;
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
	std::string const content(FileUtil::AsText(file->Read()));

	// We can now close the file again
	SafeDelete(file);

	// for now json is the only supported format
	if (ext == "json")
	{		
		return DeserializeFromJsonString(content, outObject);
	}

	LOG("DeserializeFromFile > File type '" + ext + std::string("' not supported!"), Warning);
	return false;
}

//---------------------------------
// DeserializeFromJsonResource
//
// Create the reflected type from a json string that is compiled into the app
// Returns nullptr if deserialization is unsuccsesful. 
//
template<typename T>
bool DeserializeFromJsonResource(std::string const& resourcePath, T& outObject)
{
	// get binary data from compiled resource
	std::vector<uint8> data;
	if (!FileUtil::GetCompiledResource(resourcePath, data))
	{
		LOG("DeserializeFromJsonResource > couldn't get data from resource '" + resourcePath + std::string("'"), LogLevel::Warning);
		return false;
	}

	// read binary resource into a string
	std::string const content(FileUtil::AsText(data));

	return DeserializeFromJsonString(content, outObject);
}

//---------------------------------
// DeserializeFromJsonString
//
// Create the reflected type from a json string
// Returns nullptr if deserialization is unsuccsesful. 
//
template<typename T>
bool DeserializeFromJsonString(std::string const& jsonString, T& outObject)
{
	// Read the string into a json parser
	JSON::Parser parser = JSON::Parser(jsonString);

	// if we don't have a root object parsing json was unsuccesful
	JSON::Object* root = parser.GetRoot();
	if (!root)
	{
		LOG("DeserializeFromJsonString > unable to parse string to JSON!", Warning);
		return false;
	}

	return DeserializeFromJson(root, outObject);
}

//---------------------------------
// DeserializeFromJson
//
// Create the reflected type from JSON data. 
// Returns nullptr if deserialization is unsuccsesful. 
// Assumes parentObj is the parent of the object we are deserializing
//
template<typename T>
bool DeserializeFromJson(JSON::Object* const parentObj, T& outObject)
{
	// Get the Serialized type from our template typename
	rttr::type objectType = rttr::type::get<T>();
	if (!objectType.is_valid())
	{
		LOG("DeserializeFromJson > type is invalid!", Warning);
		return nullptr;
	}

	// get the name of our type
	std::string typeName = objectType.get_name().to_string();

	// try finding a json value in its parent by the typename
	auto foundJObjectIt = std::find_if(parentObj->value.begin(), parentObj->value.end(), [typeName](JSON::Pair const& el)
	{
		return el.first == typeName;
	});

	if (foundJObjectIt == parentObj->value.cend())
	{
		LOG("DeserializeFromJson > Couldn't find '" + typeName + std::string("' in provided json object parent!"), Warning);
		return nullptr;
	}

	// try creating an instance from the value
	FromJsonRecursive(outObject, foundJObjectIt->second);

	// copy construct our return object from the value of the variant
	return true;
}

} // namespace serialization