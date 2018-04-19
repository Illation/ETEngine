//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.hpp"

#include "BinaryReader.hpp"

#include <sstream> 


BinaryReader::BinaryReader(void) :
	m_Exists(false),
	m_pReader(nullptr)
{
}

BinaryReader::~BinaryReader(void)
{
	Close();
}

std::string BinaryReader::ReadLongString()
{
	if (m_pReader == nullptr)
	{
		LOG("BinaryReader doesn't exist! Unable to read binary data...", Warning);
		return "";
	}

	auto stringLength = Read<uint32>();

	std::stringstream ss;
	for (uint32 i = 0; i<stringLength; ++i)
	{
		ss << Read<uint8>();
	}

	return (std::string)ss.str();
}

std::string BinaryReader::ReadNullString()
{
	if (m_pReader == nullptr)
	{
		LOG("BinaryReader doesn't exist! Unable to read binary data...", Warning);
		return "";
	}

	std::string buff;
	getline(*m_pReader, buff, '\0');

	return std::string(buff.begin(), buff.end());
}


std::string BinaryReader::ReadString()
{
	if (m_pReader == nullptr)
	{
		LOG("BinaryReader doesn't exist! Unable to read binary data...", Warning);
		return "";
	}

	int32 stringLength = (int32)Read<uint8>();

	std::stringstream ss;
	for (int32 i = 0; i<stringLength; ++i)
	{
		ss << Read<uint8>();
	}

	return (std::string)ss.str();
}

void BinaryReader::Open( const std::vector<uint8> &binaryContent, uint32 start, uint32 count )
{
	Close();

	if (start >= (uint32)binaryContent.size())
	{
		LOG("Start element of binary content for binary reader must be smaller than content element size", Error);
	}
	if (count == 0)
	{
		count = (uint32)binaryContent.size() - start;
	}
	if (start+count > (uint32)binaryContent.size())
	{
		LOG("Count outside of array bounds for binary reader", Error);
	}

	std::string data(&((char*)binaryContent.data())[start], count);
	m_pReader = new std::stringstream( data );
	m_Exists = true;
}

void BinaryReader::Close()
{
	SafeDelete(m_pReader);
	m_Exists = false;
}

int32 BinaryReader::GetBufferPosition()
{
	if (m_pReader)
	{
		return static_cast<int32>(m_pReader->tellg());
	}

	LOG("BinaryReader::GetBufferPosition> m_pReader doesn't exist", Warning);
	return -1;
}

bool BinaryReader::SetBufferPosition(int32 pos)
{
	if (m_pReader)
	{
		m_pReader->seekg(pos);
		return true;
	}

	LOG("BinaryReader::SetBufferPosition> m_pReader doesn't exist", Warning);
	return false;
}

bool BinaryReader::MoveBufferPosition(int32 move)
{
	auto currPos = GetBufferPosition();
	if (currPos>0)
	{
		return SetBufferPosition(currPos + move);
	}

	return false;
}
