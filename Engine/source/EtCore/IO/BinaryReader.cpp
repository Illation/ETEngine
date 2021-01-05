#include "stdafx.h"
#include "BinaryReader.h"

#include <sstream> 


namespace et {
namespace core {


//===============
// Binary Reader
//===============


// static
size_t const BinaryReader::s_InvalidBufferPos = std::numeric_limits<size_t>::max();


//---------------------
// BinaryReader::Open
//
void BinaryReader::Open(std::vector<uint8> const& binaryContent, size_t const start, size_t const count)
{
	Close();

	if (start >= binaryContent.size())
	{
		ET_ASSERT(false, "Start position '" ET_FMT_SIZET "' is larger than buffer size '" ET_FMT_SIZET "'", start, binaryContent.size());
		return;
	}
	else
	{
		m_BufferStart = start;
	}

	if (count == 0u)
	{
		m_BufferSize = binaryContent.size() - start;
	}
	else if (start + count > (uint32)binaryContent.size())
	{
		ET_ASSERT(false, "Count '" ET_FMT_SIZET "' is larger than content size '" ET_FMT_SIZET "'", count, binaryContent.size() - start);
		return;
	}

	m_BufferPosition = m_BufferStart;

	m_BinData = &binaryContent;
}

//---------------------
// BinaryReader::Close
//
void BinaryReader::Close()
{
	m_BufferPosition = s_InvalidBufferPos;
	m_BufferStart = 0u;
	m_BufferSize = s_InvalidBufferPos;

	m_BinData = nullptr; // we don't own the binary data
}

//---------------------------------
// BinaryReader::SetBufferPosition
//
void BinaryReader::SetBufferPosition(size_t const pos)
{
	ET_ASSERT(Exists());
	ET_ASSERT(pos <= m_BufferSize);

	m_BufferPosition = m_BufferStart + pos;
}

//----------------------------------
// BinaryReader::MoveBufferPosition
//
void BinaryReader::MoveBufferPosition(size_t const move)
{
	ET_ASSERT(Exists());
	ET_ASSERT(m_BufferPosition + move < m_BufferStart + m_BufferSize);

	m_BufferPosition += move;
}

//---------------------------------
// BinaryReader::GetBufferPosition
//
size_t BinaryReader::GetBufferPosition() const
{
	ET_ASSERT(Exists());
	return m_BufferPosition - m_BufferStart;
}

//------------------------
// BinaryReader::ReadData
//
void BinaryReader::ReadData(uint8* const data, size_t const size)
{
	ET_ASSERT(Exists(), "BinaryReader doesn't exist! Unable to read binary data...");
	ET_ASSERT(m_BufferPosition + size <= m_BufferStart + m_BufferSize);

	memcpy(data, m_BinData->data() + m_BufferPosition, size);
	m_BufferPosition += size;
}

//--------------------------
// BinaryReader::ReadString
//
std::string BinaryReader::ReadString(size_t const size)
{
	ET_ASSERT(Exists(), "BinaryReader doesn't exist! Unable to read binary data...");
	ET_ASSERT(m_BufferPosition + size <= m_BufferStart + m_BufferSize);

	uint8 const* const data = m_BinData->data() + m_BufferPosition;
	m_BufferPosition += size;

	return std::string(reinterpret_cast<char const*>(data), size);
}

//------------------------------
// BinaryReader::ReadNullString
//
// Not safe, but will assert if there is no string terminator within the buffer
//
std::string BinaryReader::ReadNullString()
{
	ET_ASSERT(Exists(), "BinaryReader doesn't exist! Unable to read binary data...");

	ET_ASSERT(std::find(m_BinData->cbegin() + m_BufferPosition, m_BinData->cbegin() + m_BufferStart + m_BufferSize, '\0')
		!= m_BinData->cbegin() + m_BufferStart + m_BufferSize);

	std::string const ret(reinterpret_cast<char const*>(m_BinData->data() + m_BufferPosition));
	m_BufferPosition += ret.size() + 1u;

	return ret;
}


} // namespace core
} // namespace et
