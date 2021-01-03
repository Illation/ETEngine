#pragma once


namespace et { namespace core { namespace JSON {
	struct Value;
} } }


namespace et {
namespace core {


//----------------
// JsonSerializer
//
// Class that contains all functionality to serialize a reflected object into the JSON DOM
//
class JsonSerializer final 
{
	// construct
	//-----------
public:
	JsonSerializer(bool const isVerbose = true) : m_IsVerbose(isVerbose) { }

	// functionality
	//---------------
	template<typename TDataType>
	bool SerializeToData(TDataType const& serialObject, std::vector<uint8>& data);

	template<typename TDataType>
	JSON::Value* Serialize(TDataType const& serialObject);

	// utility 
	//---------
private:
	bool ToJsonRecursive(rttr::instance const& inst, JSON::Value*& outJObject, rttr::type const& callingType);

	bool VariantToJsonValue(rttr::variant const& var, JSON::Value*& outVal);
	bool AtomicTypeToJsonValue(rttr::type const& valueType, rttr::variant const& var, JSON::Value*& outVal);
	bool ArrayToJsonArray(const rttr::variant_sequential_view& view, JSON::Value*& outVal);
	bool AssociativeContainerToJsonArray(const rttr::variant_associative_view& view, JSON::Value*& outVal);

	// Data
	///////

	bool m_IsVerbose = true;
};


} // namespace core
} // namespace et


#include "JsonSerializer.inl"
