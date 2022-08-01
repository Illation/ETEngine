#pragma once


namespace et {
namespace core {


//--------------
// TracePackage
//
// Protocol for sending and receiving trace messages through a network
//
class TracePackage final
{
	// definitions
	//----------------

public:
	//----------------
	// E_Type
	//
	enum class E_Type : uint8
	{
		Invalid = 0u,

		ConnectionAcknowledged	= 1u,
		ClientName				= 2u,
		HasClient				= 3u,
		ContextName				= 4u,
		ContextsDone			= 5u,
		InitSuccess				= 6u,
		InitFailed				= 7u,
		TraceMessage			= 8u
	};

	static std::string const s_TraceServerPort;
	static std::string const s_ProtocolVersion;

	// write functions - output in network byte order
	//------------------------------------------------
	static std::vector<uint8> WriteConnectionAcknowledged();

	// read functions - input in network byte order
	//----------------------------------------------
	static size_t GetHeaderSize();
	static E_Type ReadHeader(std::vector<uint8> const& inBuffer, uint16& outPackageSize);

	static bool ReadConnectionAcknowledged(std::vector<uint8> const& inBuffer);
};


} // namespace core
} // namespace et

