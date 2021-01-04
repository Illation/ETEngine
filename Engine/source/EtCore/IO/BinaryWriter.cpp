#include "stdafx.h"
#include "BinaryWriter.h"


namespace et {
namespace core {


//===============
// Binary Writer
//===============


// static
size_t const BinaryWriter::s_InvalidBufferPos = std::numeric_limits<size_t>::max();


//---------------------
// BinaryWriter::c-tor
//
BinaryWriter::BinaryWriter(std::vector<uint8>& buffer)
	: m_BinData(&buffer)
	, m_BufferPosition(0u)
{ }

//---------------------------------
// BinaryWriter::SetBufferPosition
//
void BinaryWriter::SetBufferPosition(size_t const pos)
{
	ET_ASSERT(pos < m_BinData->size());
	m_BufferPosition = pos;
}

//----------------------------
// BinaryWriter::FormatBuffer
//
void BinaryWriter::FormatBuffer(size_t const size, uint8 const data)
{
	m_BinData->assign(size, data);
}

//----------------------------------
// BinaryWriter::IncreaseBufferSize
//
void BinaryWriter::IncreaseBufferSize(size_t const size, uint8 const data)
{
	m_BinData->resize(m_BinData->size() + size, data);
}

//-------------------------
// BinaryWriter::WriteData
//
void BinaryWriter::WriteData(uint8 const* const data, size_t const size)
{
	ET_ASSERT(m_BufferPosition + size <= m_BinData->size());
	memcpy(m_BinData->data() + m_BufferPosition, data, size);
	m_BufferPosition += size;
}

//-------------------------------
// BinaryWriter::WriteNullString
//
void BinaryWriter::WriteNullString(std::string const& str)
{
	WriteString(str);
	Write<uint8>(0u);
}

//---------------------------
// BinaryWriter::WriteString
//
// Write a string where the reader must know the size already
//
void BinaryWriter::WriteString(std::string const& str)
{
	WriteData(reinterpret_cast<uint8 const*>(str.data()), str.size());
}


} // namespace core
} // namespace et
