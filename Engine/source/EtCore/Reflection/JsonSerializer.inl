#pragma once

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/JsonDom.h>
#include <EtCore/IO/JsonWriter.h>


namespace et {
namespace core {


//=================
// JSON Serializer
//=================


//---------------------------
// JsonSerializer::Serialize
//
// Serialize the object into a byte array containing JSON text
//
template<typename TDataType>
bool JsonSerializer::SerializeToData(TDataType const& serialObject, std::vector<uint8>& data)
{
	JSON::Object* root = static_cast<JSON::Object*>(Serialize(serialObject));
	if (root)
	{
		// then write it to a string
		JSON::Writer writer(!m_IsVerbose);
		if (writer.Write(root))
		{
			// convert that string to a byte array
			data = FileUtil::FromText(writer.GetResult()); // #todo this shouldn't copy twice, pass content byte vector as reference
			return true;
		}
		else
		{
			LOG("JsonSerializer::SerializeToData > unable to write JSON DOM to string", Warning);
		}
	}
	else
	{
		LOG("JsonSerializer::SerializeToData > unable to serialize object to JSON DOM!", Warning);
	}

	return false;
}

//---------------------------
// JsonSerializer::Serialize
//
// Create a JSON::Object from the templated type using reflection data. Returns nullptr if serialization is unsuccsesful.
//
template<typename TDataType>
JSON::Value* JsonSerializer::Serialize(TDataType const& serialObject)
{
	rttr::instance inst(serialObject);
	if (!inst.is_valid())
	{
		LOG("SerializeToJson > Couldn't create a valid instance from the object to serialize!", Warning);
		return nullptr;
	}

	JSON::Value* outObject = nullptr;

	rttr::type const callingType = rttr::type::get(serialObject);
	if (ToJsonRecursive(inst, outObject, callingType))
	{
		if (callingType.is_pointer())
		{
			return outObject;
		}
		else
		{
			JSON::Object* root = new JSON::Object();

			root->value.emplace_back(inst.get_type().get_name().to_string(), outObject);

			return static_cast<JSON::Value*>(root);
		}
	}

	delete outObject;
	return nullptr;
}


} // namespace core
} // namespace et
