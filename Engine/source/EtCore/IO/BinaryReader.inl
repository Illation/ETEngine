#pragma once


namespace et {
namespace core {


//---------------------
// BinaryReader::Read
//
// Read a section of data and move the buffer position to the next item
//  - returns a copy of the data - useful for small values
//
template<typename TDataType>
TDataType BinaryReader::Read()
{
	ET_ASSERT(Exists(), "BinaryReader doesn't exist! Unable to read binary data...");
	ET_ASSERT(m_BufferPosition + sizeof(TDataType) <= m_BufferStart + m_BufferSize);

	TDataType const value(*reinterpret_cast<TDataType const*>(m_BinData->data() + m_BufferPosition));

	m_BufferPosition += sizeof(TDataType);
	return value;
}

//------------------------
// BinaryReader::ReadRef
//
// Read a section of data and move the buffer position to the next item
//  - returns a const ref to the data without copying, so best for data that is larger than pointer size
//
template<typename TDataType>
TDataType const& BinaryReader::ReadRef()
{
	ET_ASSERT(Exists(), "BinaryReader doesn't exist! Unable to read binary data...");
	ET_ASSERT(m_BufferPosition + sizeof(TDataType) <= m_BufferStart + m_BufferSize);

	uint8 const* const data = m_BinData->data() + m_BufferPosition;
	m_BufferPosition += sizeof(TDataType);

	return *reinterpret_cast<TDataType const*>(data);
}

//--------------------------
// BinaryReader::ReadString
//
// Read x characters into a string, where x is defined by a number of TStringLengthType that is read at the first position
//
template<typename TStringLengthType>
std::string BinaryReader::ReadString()
{
	size_t const stringLength = static_cast<size_t>(Read<TStringLengthType>());

	ET_ASSERT(m_BufferPosition + stringLength < m_BufferStart + m_BufferSize);

	uint8 const* const data = m_BinData->data() + m_BufferPosition;
	m_BufferPosition += stringLength;

	return std::string(reinterpret_cast<char const*>(data), stringLength);
}


} // namespace core
} // namespace et

