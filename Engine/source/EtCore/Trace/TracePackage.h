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
	enum class E_Type : uint8 // make sure to update the reflection when adding types, and for incompatible changes update the protocol version
	{
		Invalid = 0u,

		ConnectionAcknowledged	= 1u,
		ClientInfo				= 2u,
		HasClient				= 3u,
		ContextName				= 4u,
		ContextsDone			= 5u,
		InitSuccess				= 6u,
		TraceMessage			= 7u,
		UpdateClientName		= 8u
	};

	static std::string const s_TraceServerPort;
	static std::string const s_ProtocolVersion;

	// write functions - output in network byte order
	//------------------------------------------------
	static std::vector<uint8> WriteConnectionAcknowledged();
	static std::vector<uint8> WriteClientInfo(std::string const& clientName, uint16 const contextCount);
	static std::vector<uint8> WriteHasClient();
	static std::vector<uint8> WriteContextName(std::string const& contextName);
	static std::vector<uint8> WriteContextsDone();
	static std::vector<uint8> WriteInitSuccess();
	static std::vector<uint8> WriteTraceMessage(T_Hash const context,
		E_TraceLevel const level, 
		std::string const& timestamp, 
		std::string const& message);

	static std::vector<uint8> WriteUpdateClientName(std::string const& clientName);

	// read functions - input in network byte order
	//----------------------------------------------
	static size_t GetHeaderSize();
	static E_Type ReadHeader(std::vector<uint8> const& inBuffer, uint16& outPackageSize);

	static bool ReadConnectionAcknowledged(std::vector<uint8> const& inBuffer);
	static void ReadClientInfo(std::vector<uint8> const& inBuffer, std::string& outClientName, uint16& outContextCount);
	static void ReadContextName(std::vector<uint8> const& inBuffer, std::string& outContextName);
	static void ReadTraceMessage(std::vector<uint8> const& inBuffer, 
		T_Hash& outContext,
		E_TraceLevel& outLevel, 
		std::string& outTimestamp, 
		std::string& outMessage);

	static void ReadUpdateClientName(std::vector<uint8> const& inBuffer, std::string& outClientName);
};


} // namespace core
} // namespace et

