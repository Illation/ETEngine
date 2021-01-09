#pragma once


namespace et {
namespace core {


//=====================
// Binary Deserializer
//=====================


//-----------------------------------------
// BinaryDeserializer::DeserializeFromData
//
// Create the reflected type from an etbin file (provided as a data vector)
// Returns false if deserialization is unsuccsesful. 
//
template<typename TDataType>
bool BinaryDeserializer::DeserializeFromData(std::vector<uint8> const& data, TDataType& outObject)
{
	// if this is a non pointer object we can deserialize directly into an instance, avoiding copying data
	rttr::type const callingType = rttr::type::get<TDataType>();
	if (TypeInfo::IsBasic(callingType))
	{
		TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(callingType);
		if (ti.m_Kind == TypeInfo::E_Kind::Class)
		{
			return DeserializeRoot(rttr::instance(outObject), ti, data);
		}
	}

	// in all other cases we will deserialize a copied variant
	rttr::variant deserializedValue = outObject;
	if (DeserializeRoot(deserializedValue, callingType, data))
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
