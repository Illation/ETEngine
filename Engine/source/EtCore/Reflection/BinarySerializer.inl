#pragma once


namespace et {
namespace core {


//===================
// Binary Serializer
//===================


//-----------------------------------
// BinarySerializer::SerializeToData
//
// Forward data to non templated functions
//
template<typename TDataType>
bool BinarySerializer::SerializeToData(TDataType const& serialObject, std::vector<uint8>& outData)
{
	rttr::variant var(serialObject);
	if (!var.is_valid())
	{
		ET_ERROR("couldn't get valid instance from serialObject");
		return false;
	}

	return SerializeRoot(var, outData);
}


} // namespace core
} // namespace et

