#pragma once


namespace et {
namespace core {


//---------------------
// BinaryWriter::Write
//
// Write the object into the buffer at the current buffer position
//
template <typename TDataType>
void BinaryWriter::Write(TDataType const& data)
{
	ET_ASSERT(m_BufferPosition + sizeof(TDataType) <= m_BinData->size());
	memcpy(m_BinData->data() + m_BufferPosition, &data, sizeof(TDataType));
	m_BufferPosition += sizeof(TDataType);
}

//---------------------------
// BinaryWriter::WriteVector
//
template <uint8 n, class T>
void BinaryWriter::WriteVector(math::vector<n, T> const& vec)
{
	WriteData(reinterpret_cast<uint8 const*>(vec.data.data()), sizeof(T) * n);
}


} // namespace core
} // namespace et
