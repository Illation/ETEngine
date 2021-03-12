#pragma once

#include "TypeInfoRegistry.h"


namespace et { namespace core { namespace JSON {
	struct Value;
	struct Object;
} } } 


namespace et {
namespace core {


//------------------
// JsonDeserializer
//
// Class that contains all functionality to deserialize a reflected object from a JSON DOM
//
class JsonDeserializer final 
{
	// construct
	//-----------
public:
	JsonDeserializer() = default;

	// functionality
	//---------------
	template<typename T>
	bool DeserializeFromData(std::vector<uint8> const& data, T& outObject);

	template<typename TDataType>
	bool Deserialize(JSON::Object* const parentObj, TDataType& outObject);

	// utility 
	//---------
private:
	bool DeserializeRoot(rttr::variant& var, rttr::type const callingType, JSON::Object const* const parentObj);
	bool DeserializeRoot(rttr::instance& inst, TypeInfo const& ti, JSON::Object const* const parentObj);

	// general
	bool ReadVariant(rttr::variant& var, rttr::type const callingType, JSON::Value const* const jVal);
	bool ReadBasicVariant(rttr::variant& var, TypeInfo const& ti, JSON::Value const* const jVal);

	// atomic
	bool ReadArithmeticType(rttr::variant& var, HashString const typeId, JSON::Value const* const jVal);
	bool ReadEnum(rttr::variant& var, rttr::type const enumType, JSON::Value const* const jVal);
	bool ReadVectorType(rttr::variant& var, TypeInfo const& ti, JSON::Value const* const jVal);
	bool ReadHash(rttr::variant& var, JSON::Value const* const jVal);

	// complex
	bool ReadSequentialContainer(rttr::variant& var, JSON::Value const* const jVal);
	bool ReadAssociativeContainer(rttr::variant& var, JSON::Value const* const jVal);
	bool ReadObject(rttr::variant& var, TypeInfo const& ti, JSON::Value const* const jVal);
	bool ReadObjectProperties(rttr::instance& inst, TypeInfo const& ti, JSON::Object const* const jObj);
};


} // namespace core
} // namespace et


#include "JsonDeserializer.inl"
