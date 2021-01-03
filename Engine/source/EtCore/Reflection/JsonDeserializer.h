#pragma once


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
	void Deserialize(JSON::Object* const parentObj, TDataType& outObject);

	// utility 
	//---------
private:
	rttr::variant ExtractBasicTypes(JSON::Value const* const jVal);
	bool ArrayFromJsonRecursive(rttr::variant_sequential_view& view, JSON::Value const* const jVal);
	rttr::variant ExtractValue(JSON::Value const* const jVal, const rttr::type& valueType);
	bool AssociativeViewFromJsonRecursive(rttr::variant_associative_view& view, JSON::Value const* const jVal);
	void FromJsonValue(JSON::Value const* jVal, rttr::type &valueType, rttr::variant &var);
	bool ExtractPointerValueType(rttr::type &inOutValType, JSON::Value const* &inOutJVal, bool& isNull);
	void ObjectFromJsonRecursive(JSON::Value const* const jVal, rttr::instance const &inst, rttr::type &instType);

	void FromJsonRecursive(rttr::instance const inst, JSON::Object const* const parentObj);
};


} // namespace core
} // namespace et


#include "JsonDeserializer.inl"
