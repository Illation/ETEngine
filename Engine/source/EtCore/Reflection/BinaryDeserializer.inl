#pragma once


namespace et {
namespace core {


//=====================
// Binary Deserializer
//=====================


//---------------------------------------
// JsonDeserializer::DeserializeFromData
//
// Create the reflected type from an etbin file (provided as a data vector)
// Returns false if deserialization is unsuccsesful. 
//
template<typename TDataType>
bool BinaryDeserializer::DeserializeFromData(std::vector<uint8> const& data, TDataType& outObject)
{
	rttr::variant deserializedValue;
	if (DeserializeRoot(deserializedValue, rttr::type::get<TDataType>(), data))
	{
		if (deserializedValue.is_type<TDataType>())
		{
			outObject = deserializedValue.get_value<TDataType>(); // check that we don't loose data after the variant goes out of scope
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
