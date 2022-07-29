#pragma once

#include <EtCore/IO/BinaryReader.h>
#include <EtCore/IO/Endianness.h>


namespace et {
namespace core {


//-------------------------
// NetworkReader
//
// Thin wrapper around a binary reader for cases where converting from network byte order is important 
// 	- we wrap instead of extending for performance reasons
//
class NetworkReader final
{

	// construct destruct
	//--------------------
public:
	NetworkReader() : m_Reader(), m_SwapByteOrder(!IsArchitectureBigEndian()) {}
	~NetworkReader() = default;

	// functionality
	//---------------
	void Open(std::vector<uint8> const& binaryContent, size_t const start = 0u, size_t const count = 0u) { m_Reader.Open(binaryContent, start, count); }
	void Close() { m_Reader.Close(); }

	void SetBufferPosition(size_t const pos) { m_Reader.SetBufferPosition(pos); }
	void MoveBufferPosition(size_t const move) { m_Reader.MoveBufferPosition(move); }

	// accessors
	//-----------
	size_t GetBufferPosition() const { return m_Reader.GetBufferPosition(); }
	size_t GetBufferSize() const { return m_Reader.GetBufferSize(); }
	bool Exists() const { return m_Reader.Exists(); }

	template<typename TDataType>
	TDataType Read();

	uint8 const* GetCurrentDataPointer() const { return m_Reader.GetCurrentDataPointer(); }

	template<typename TStringLengthType>
	std::string ReadString() { return m_Reader.ReadString<TStringLengthType>(); }
	std::string ReadString(size_t const size) { return m_Reader.ReadString(size); }
	std::string ReadNullString() { return m_Reader.ReadNullString(); }


	// Data
	///////

private:
	BinaryReader m_Reader;
	bool m_SwapByteOrder = false;
};


} // namespace core
} // namespace et

#include "NetworkReader.inl"
