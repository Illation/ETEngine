#pragma once


namespace et {
namespace core {


//-------------------------
// IsArchitectureBigEndian
//
// Check if the system the engine is running on is big endian (most significant byte stored first) or not
//  - based on https://stackoverflow.com/a/1001373/4414168
//
inline bool IsArchitectureBigEndian()
{
	static union 
	{
		uint32 i;
		char c[4];
	} const s_BInt = { 0x01020304 };

	return s_BInt.c[0] == 1;
}

//------------
// SwapEndian
//
// Can only swap endian for basic types, for classes it should be done for each member individually
//
template <typename TDataType>
inline typename std::enable_if_t<(sizeof(TDataType) > 1) && !std::is_enum<TDataType>::value, TDataType> SwapEndian(TDataType const data)
{
	static_assert(!std::is_class<TDataType>::value, "Can't swap endianness for classes!");

	TDataType ret;
	uint8* retPtr = reinterpret_cast<uint8*>(&ret);

	size_t bitShift = sizeof(TDataType) * 8u;
	do 
	{
		bitShift -= 8u;
		*retPtr++ = data >> bitShift;
	} while (bitShift > 0u);

	return ret;
}

//------------
// SwapEndian
//
// Can only swap endian for basic types, for classes it should be done for each member individually
//
template <typename TDataType>
inline typename std::enable_if_t<(sizeof(TDataType) > 1) && std::is_enum<TDataType>::value, TDataType> SwapEndian(TDataType const data)
{
	return static_cast<TDataType>(SwapEndian(static_cast<std::underlying_type<TDataType>::type>(data)));
}

//------------
// SwapEndian
//
// No need to swap the endian of a 1 byte data type
//
template <typename TDataType>
inline typename std::enable_if_t<(sizeof(TDataType) == 1), TDataType> SwapEndian(TDataType const data)
{
	static_assert(!std::is_class<TDataType>::value, "Can't swap endianness for classes!");

	return data;
}


} // namespace core
} // namespace et
