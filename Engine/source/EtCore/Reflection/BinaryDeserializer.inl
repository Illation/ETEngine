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
	rttr::variant deserializedValue = outObject;
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

//------------------------------
// JsonDeserializer::ReadVector
//
template <uint8 n, class T>
math::vector<n, T> BinaryDeserializer::ReadVector()
{
	math::vector<n, T> ret;
	m_Reader.ReadData(reinterpret_cast<uint8*>(ret.data.data()), sizeof(T) * n);
	return ret;
}

//------------------------------
// JsonDeserializer::ReadMatrix
//
template <uint8 m, uint8 n, class T>
math::matrix<m, n, T> BinaryDeserializer::ReadMatrix()
{
	math::matrix<m, n, T> ret;
	m_Reader.ReadData(reinterpret_cast<uint8*>(ret.data), sizeof(T) * m * n);
	return ret;
}


} // namespace core
} // namespace et
