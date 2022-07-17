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
			ET_TRACE_W(ET_CTX_CORE, "JsonSerializer::SerializeToData > unable to write JSON DOM to string");
		}
	}
	else
	{
		ET_TRACE_W(ET_CTX_CORE, "JsonSerializer::SerializeToData > unable to serialize object to JSON DOM!");
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
	rttr::variant var(serialObject);
	if (!var.is_valid())
	{
		ET_ASSERT(false, "couldn't get valid instance from serialObject");
		return nullptr;
	}

	JSON::Value* outVal = nullptr;
	if (!SerializeRoot(var, outVal))
	{
		ET_ASSERT(false, "Failed to serialize to JSON");
		delete outVal;
		return nullptr;
	}

	return outVal;
}


} // namespace core
} // namespace et
