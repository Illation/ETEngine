#pragma once

#include <rttr/type>

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Json/JsonDom.h>
#include <EtCore/FileSystem/Json/JsonParser.h>
#include <EtCore/FileSystem/Json/JsonWriter.h>


namespace et {
namespace core {


// This code is heavily based on https://github.com/rttrorg/rttr/tree/master/src/examples/json_serialization

//---------------------------------
// serialization
//
// This namespace will contain all functionality to serialize and deserialize things to various data formats, for now json
//
namespace serialization
{
	// Serialization
	//----------------

	template<typename T>
	bool SerializeToFile(std::string const& filePath, T const& serialObject);

	template<typename T>
	JSON::Value* SerializeToJson(T const& serialObject);

	// Deserialization
	//-----------------

	template<typename T>
	bool DeserializeFromFile(std::string const& filePath, T& outObject);

	template<typename T>
	bool DeserializeFromJsonResource(std::string const& resourcePath, T& outObject);

	template<typename T>
	bool DeserializeFromJsonString(std::string const& jsonString, T& outObject);

	template<typename T>
	bool DeserializeFromJson(JSON::Object* const parentObj, T& outObject);






	// Utility functions
	//---------------------

	// serialization
	bool ToJsonRecursive(rttr::instance const& inst, JSON::Value*& outJObject, rttr::type const& callingType);

	bool VariantToJsonValue(rttr::variant const& var, JSON::Value*& outVal);
	bool IsVectorType(rttr::type const type);
	bool IsAssetType(rttr::type const type);
	bool AtomicTypeToJsonValue(rttr::type const& valueType, rttr::variant  const& var, JSON::Value*& outVal);
	bool ArrayToJsonArray(const rttr::variant_sequential_view& view, JSON::Value*& outVal);
	bool AssociativeContainerToJsonArray(const rttr::variant_associative_view& view, JSON::Value*& outVal);

	// deserialization
	rttr::variant ExtractBasicTypes(JSON::Value const* const jVal);
	bool ArrayFromJsonRecursive(rttr::variant_sequential_view& view, JSON::Value const* const jVal);
	rttr::variant ExtractValue(JSON::Value const* const jVal, const rttr::type& valueType);
	bool AssociativeViewFromJsonRecursive(rttr::variant_associative_view& view, JSON::Value const* const jVal);
	void FromJsonValue(JSON::Value const* jVal, rttr::type &valueType, rttr::variant &var);
	bool ExtractPointerValueType(rttr::type &inOutValType, JSON::Value const* &inOutJVal, bool& isNull);
	void ObjectFromJsonRecursive(JSON::Value const* const jVal, rttr::instance const &inst, rttr::type &instType);

	void FromJsonRecursive(rttr::instance const inst, JSON::Value const* const jVal);

} // namespace serialization


} // namespace core
} // namespace et

#include "Serialization.inl"
