#pragma once


namespace et {
namespace core {


//---------------------
// BinaryWriter::Write
//
// Write the object into the buffer at the current buffer position
template <typename TDataType>
void BinaryWriter::Write(TDataType const& data)
{
	ET_ASSERT(m_BufferPosition + sizeof(TDataType) <= m_BinData->size());
	memcpy(m_BinData->data() + m_BufferPosition, &data, sizeof(TDataType));
	m_BufferPosition += sizeof(TDataType);
}


} // namespace core
} // namespace et
