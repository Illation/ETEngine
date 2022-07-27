#pragma once


namespace et {
namespace core {

namespace network {


//-----------------
// E_AddressFamily
//
// IP address format
//
enum class E_AddressFamily : uint8
{
	Unspecified = 0u,

	InterNetwork,	// IP v4
	InterNetwork6	// IP v6
};

//------------
// E_SocketType
//
enum class E_SocketType : uint8
{
	Invalid = 0u,

	Stream,
	Datagram
};

//------------
// E_Protocol
//
// Data transmission protocol
//
enum class E_Protocol : uint8
{
	Any = 0,
	Invalid = Any,

	Tcp,
	Udp
};

//-----------------
// E_AddressFlags
//
// flags for address parsing
//
typedef int32 T_AddressFlags;
enum E_AddressFlags : T_AddressFlags
{
	AF_None = 0,

	AF_Passive = 1 << 0
};



typedef uint16 T_Port;


//------------
// IpAddress4
//
struct IpAddress4 final
{
	IpAddress4() : m_Address(0u) {}

	union
	{
		uint8 m_Bytes[4];
		uint16 m_Words[2];
		uint32 m_Address;
	};
};

//------------
// IpAddress6
//
struct IpAddress6 final
{
	union
	{
		uint8 m_Bytes[16];
		uint16 m_Words[8];
	};
};

//---------------
// SocketAddress
//
struct SocketAddress final
{
	SocketAddress() { memset(&m_Ip6, 0, sizeof m_Ip6); }

	E_AddressFamily m_Family = E_AddressFamily::Unspecified;

	union
	{
		struct  
		{
			IpAddress4 m_Ip4;
			uint8 _paddingIp4[12];
		};

		IpAddress6 m_Ip6;
	};
};

//----------
// Endpoint
//
struct Endpoint final
{
	SocketAddress m_Address;
	T_Port m_Port = 0u; // in network byte order - use PortNtoH
};

//-------------
// AddressInfo
//
struct AddressInfo final
{
	E_SocketType m_SocketType = E_SocketType::Invalid;
	E_Protocol m_Protocol = E_Protocol::Any;
	Endpoint m_Endpoint;
};



class I_Socket;

//----------------
// E_PollEvent
//
// Bitfield for events on poll data
//
typedef uint8 T_PollEvent;
enum E_PollEvent : T_PollEvent
{
	PE_None = 0,

	// inputs
	PE_In				= 1 << 0,
	PE_Out				= 1 << 1,
	PE_Priority			= 1 << 2,

	// return fields
	PE_Error			= 1 << 3,
	PE_Disconnected		= 1 << 4,
	PE_InvalidSocket	= 1 << 5,

	PE_All = 0xFF
};

//----------
// PollDesc
//
struct PollDesc final
{
	RefPtr<I_Socket> m_Socket;
	T_PollEvent m_InFlags;
	T_PollEvent m_Events;
};


} // namespace network

} // namespace core
} // namespace et
