#pragma once

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/JsonParser.h>


namespace et {
namespace core {


//===================
// JSON Deserializer
//===================


//---------------------------------------
// JsonDeserializer::DeserializeFromData
//
// Create the reflected type from a json string (provided as a data vector)
// Returns false if deserialization is unsuccsesful. 
//
template<typename T>
bool JsonDeserializer::DeserializeFromData(std::vector<uint8> const& data, T& outObject)
{
	// Read the string into a json parser
	JSON::Parser parser = JSON::Parser(FileUtil::AsText(data));

	// if we don't have a root object parsing json was unsuccesful
	JSON::Object* root = parser.GetRoot();
	if (!root)
	{
		LOG("DeserializeFromData > unable to parse string to JSON!", Warning);
		return false;
	}

	return Deserialize(root, outObject);
}

//-----------------------------
// JsonDeserializer::Serialize
//
// Create a JSON::Object from the templated type using reflection data. Returns nullptr if serialization is unsuccsesful.
//
template<typename TDataType>
bool JsonDeserializer::Deserialize(JSON::Object* const parentObj, TDataType& outObject)
{
	// if this is a non pointer object we can deserialize directly into an instance, avoiding copying data
	rttr::type const callingType = rttr::type::get<TDataType>();
	if (TypeInfo::IsBasic(callingType))
	{
		TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(callingType);
		if (ti.m_Kind == TypeInfo::E_Kind::Class)
		{
			return DeserializeRoot(rttr::instance(outObject), ti, parentObj);
		}
	}

	// in all other cases we will deserialize a copied variant
	rttr::variant deserializedValue = outObject;
	if (DeserializeRoot(deserializedValue, callingType, parentObj))
	{
		if (deserializedValue.is_type<TDataType>())
		{
			outObject = deserializedValue.get_value<TDataType>();
			return true;
		}
		else if (deserializedValue.convert(outObject))
		{
			return true;
		}
	}

	return false;
}


} // namespace core
} // namespace et

