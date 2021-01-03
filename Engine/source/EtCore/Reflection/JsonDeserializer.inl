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
// Returns nullptr if deserialization is unsuccsesful. 
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

	Deserialize(root, outObject);

	return true;
}

//-----------------------------
// JsonDeserializer::Serialize
//
// Create a JSON::Object from the templated type using reflection data. Returns nullptr if serialization is unsuccsesful.
//
template<typename TDataType>
void JsonDeserializer::Deserialize(JSON::Object* const parentObj, TDataType& outObject)
{
	rttr::type const valueType = rttr::type::get<TDataType>();
	ET_ASSERT(valueType.is_valid());

	// polymorphic
	rttr::type localType = valueType;
	bool isNull;
	JSON::Value const* jVal = parentObj;
	if (!ExtractPointerValueType(localType, jVal, isNull) || isNull)
	{
		return;
	}

	// root value or internal pointer content ?
	if (localType == valueType)
	{
		FromJsonRecursive(outObject, parentObj);
	}
	else
	{
		// find the right constructor for our type
		rttr::constructor ctor = localType.get_constructor();
		ET_ASSERT(ctor.is_valid(), "Failed to get a valid constructor from type '%s", localType.get_name().data());
		rttr::variant var = ctor.invoke();

		// deserialize
		ObjectFromJsonRecursive(jVal, var, localType);
		ET_ASSERT(var.is_valid());

		if (localType != valueType)
		{
			var.convert(rttr::type(valueType));
		}

		outObject = var.get_value<TDataType>();
	}
}


} // namespace core
} // namespace et

