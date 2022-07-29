#pragma once

#include <EtCore/IO/BinaryWriter.h>
#include <EtCore/IO/Endianness.h>


namespace et {
namespace core {


//-------------------------
// NetworkWriter
//
// Thin wrapper around a binary writer for cases where converting to and from network byte order is important
//  we wrap around it instead of integrating for better performance
//
class NetworkWriter final
{
	// definitions
	//-------------
public:
	// construct destruct
	//--------------------
	NetworkWriter(std::vector<uint8>& buffer) : m_Writer(buffer), m_SwapEndian(!IsArchitectureBigEndian()) {}
	~NetworkWriter() = default;

	// functionality
	//---------------
	void SetBufferPosition(size_t const pos) { m_Writer.SetBufferPosition(pos); }

	void FormatBuffer(size_t const size, uint8 const data = 0u) { m_Writer.FormatBuffer(size, data); }
	void IncreaseBufferSize(size_t const size, uint8 const data = 0u) { m_Writer.IncreaseBufferSize(size, data); }

	template <typename TDataType>
	void Write(TDataType const& data) { m_Writer.Write(m_SwapEndian ? SwapEndian(data) : data); }

	void WriteNullString(std::string const& str) { m_Writer.WriteNullString(str); }
	void WriteString(std::string const& str) { m_Writer.WriteString(str); } // no terminator

	// accessors
	//-----------
	size_t GetBufferPosition() const { return m_Writer.GetBufferPosition(); }


	// Data
	///////

private:
	BinaryWriter m_Writer;
	bool m_SwapEndian = false;
};


} // namespace core
} // namespace et
