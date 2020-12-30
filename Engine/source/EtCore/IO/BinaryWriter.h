#pragma once


namespace et {
namespace core {


//-------------------------
// BinaryWriter
//
// Utility class for writing binary content
//
class BinaryWriter final
{
	// definitions
	//-------------
public:
	static size_t const s_InvalidBufferPos;

	// construct destruct
	//--------------------
	BinaryWriter(std::vector<uint8>& buffer);
	~BinaryWriter() = default;

	// functionality
	//---------------
	void SetBufferPosition(size_t const pos);

	void FormatBuffer(size_t const size, uint8 const data);

	template <typename TDataType>
	void Write(TDataType const& data);
	void WriteData(uint8 const* const data, size_t const size);

	// accessors
	//-----------
	size_t GetBufferPosition() const { return m_BufferPosition; }


	// Data
	///////

private:
	size_t m_BufferPosition = s_InvalidBufferPos;

	std::vector<uint8>* m_BinData = nullptr;
};


} // namespace core
} // namespace et


#include "BinaryWriter.inl"
