#ifdef ET_PLATFORM_WIN

#include "stdafx.h"
#include "Socket_Win.h"


namespace et {
namespace core {

namespace network {


//==========================
// Network Type Conversions
//==========================


//---------------------
//
int32 ToWinSock(E_AddressFamily const family)
{
	switch (family)
	{
	case E_AddressFamily::Unspecified:		return AF_UNSPEC;
	case E_AddressFamily::InterNetwork:		return AF_INET;
	case E_AddressFamily::InterNetwork6:	return AF_INET6;
	}

	ET_ERROR("Unhandled address family");
	return 0;
}

//---------------------
//
int32 ToWinSock(E_SocketType const type)
{
	switch (type)
	{
	case E_SocketType::Stream:		return SOCK_STREAM;
	case E_SocketType::Datagram:	return SOCK_DGRAM;
	}

	ET_ERROR("Unhandled socket type");
	return 0;
}

//---------------------
//
int32 ToWinSock(E_Protocol const protocol)
{
	switch (protocol)
	{
	case E_Protocol::Any:	return 0;
	case E_Protocol::Tcp:	return IPPROTO_TCP;
	case E_Protocol::Udp:	return IPPROTO_UDP;
	}

	ET_ERROR("Unhandled protocol");
	return 0;
}

//---------------------
//
int32 ToWinSock(E_AddressFlags const flags)
{
	int32 ret = 0;

	if (static_cast<T_AddressFlags>(flags) & E_AddressFlags::AF_Passive)
	{
		ret |= AI_PASSIVE;
	}

	return ret;
}

//---------------------
//
// returns the address length
//
int32 ToWinSock(Endpoint const& ep, sockaddr_storage& addr)
{
	memset(&addr, 0, sizeof(addr));

	switch (ep.m_Address.m_Family)
	{
	case E_AddressFamily::InterNetwork:
	{
		sockaddr_in addrIn;
		memset(&addrIn, 0, sizeof(addrIn));

		addrIn.sin_family = AF_INET;
		addrIn.sin_addr.S_un.S_addr = static_cast<u_long>(ep.m_Address.m_Ip4.m_Address);
		addrIn.sin_port = ep.m_Port;

		addr = *reinterpret_cast<sockaddr_storage*>(&addrIn);
		return sizeof(addrIn);
	}

	case E_AddressFamily::InterNetwork6:
	{
		sockaddr_in6 addrIn;
		memset(&addrIn, 0, sizeof(addrIn));

		addrIn.sin6_family = AF_INET6;
		std::copy(std::begin(ep.m_Address.m_Ip6.m_Bytes), std::end(ep.m_Address.m_Ip6.m_Bytes), std::begin(addrIn.sin6_addr.u.Byte));
		addrIn.sin6_port = ep.m_Port;

		// flowinfo not set
		// scope not set

		addr = *reinterpret_cast<sockaddr_storage*>(&addrIn);
		return sizeof(addrIn);
	}
	}

	ET_ERROR("Unhandled address family!");
	return 0;
}

//---------------------
//
SHORT ToWinSock(E_PollEvent const pollEvent)
{
	SHORT ret = 0;

	if (static_cast<T_PollEvent>(pollEvent) & E_PollEvent::PE_In)
	{
		ret |= POLLIN;
	}

	if (static_cast<T_PollEvent>(pollEvent) & E_PollEvent::PE_Out)
	{
		ret |= POLLOUT;
	}

	if (static_cast<T_PollEvent>(pollEvent) & E_PollEvent::PE_Priority)
	{
		ret |= POLLPRI;
	}

	if (static_cast<T_PollEvent>(pollEvent) & E_PollEvent::PE_Error)
	{
		ret |= POLLERR;
	}

	if (static_cast<T_PollEvent>(pollEvent) & E_PollEvent::PE_Disconnected)
	{
		ret |= POLLHUP;
	}

	if (static_cast<T_PollEvent>(pollEvent) & E_PollEvent::PE_InvalidSocket)
	{
		ret |= POLLNVAL;
	}

	return ret;
}


//---------------------
//
E_AddressFamily ToAddressFamily(int32 const family)
{
	switch (family)
	{
	case AF_UNSPEC:		return E_AddressFamily::Unspecified;
	case AF_INET:		return E_AddressFamily::InterNetwork;
	case AF_INET6:		return E_AddressFamily::InterNetwork6;
	}

	ET_ERROR("Unhandled address family");
	return E_AddressFamily::Unspecified;
}

//---------------------
//
E_SocketType ToSocketType(int32 const type)
{
	switch (type)
	{
	case SOCK_STREAM:		return E_SocketType::Stream;
	case SOCK_DGRAM:		return E_SocketType::Datagram;
	}

	ET_ERROR("Unhandled socket type");
	return E_SocketType::Invalid;
}

//---------------------
//
E_Protocol ToProtocol(int32 const protocol)
{
	switch (protocol)
	{
	case 0:				return E_Protocol::Any;
	case IPPROTO_TCP:	return E_Protocol::Tcp;
	case IPPROTO_UDP:	return E_Protocol::Udp;
	}

	ET_ERROR("Unhandled protocol");
	return E_Protocol::Invalid;
}

//---------------------
//
void ToEndpoint(sockaddr const& addr, Endpoint& outEndpoint)
{
	if (addr.sa_family == AF_INET)
	{
		outEndpoint.m_Address.m_Family = E_AddressFamily::InterNetwork;

		sockaddr_in const& addrIn = *reinterpret_cast<sockaddr_in const*>(&addr);
		outEndpoint.m_Address.m_Ip4 = *reinterpret_cast<IpAddress4 const*>(&(addrIn.sin_addr));
		outEndpoint.m_Port = addrIn.sin_port;
	}
	else if (addr.sa_family == AF_INET6)
	{
		outEndpoint.m_Address.m_Family = E_AddressFamily::InterNetwork6;

		sockaddr_in6 const& addrIn = *reinterpret_cast<sockaddr_in6 const*>(&addr);
		outEndpoint.m_Address.m_Ip6 = *reinterpret_cast<IpAddress6 const*>(&(addrIn.sin6_addr));
		outEndpoint.m_Port = addrIn.sin6_port;
	}
	else
	{
		ET_ERROR("Unhandled in address");
		outEndpoint.m_Address.m_Family = E_AddressFamily::Unspecified;
	}
}

//---------------------
//
T_PollEvent ToPollEvent(SHORT const pollEvent)
{
	T_PollEvent ret = E_PollEvent::PE_None;

	if (pollEvent & POLLIN)
	{
		ret |= E_PollEvent::PE_In;
	}

	if (pollEvent & POLLOUT)
	{
		ret |= E_PollEvent::PE_Out;
	}

	if (pollEvent & POLLPRI)
	{
		ret |= E_PollEvent::PE_Priority;
	}

	if (pollEvent & POLLERR)
	{
		ret |= E_PollEvent::PE_Error;
	}

	if (pollEvent & POLLHUP)
	{
		ret |= E_PollEvent::PE_Disconnected;
	}

	if (pollEvent & POLLNVAL)
	{
		ret |= E_PollEvent::PE_InvalidSocket;
	}

	return ret;
}


//========
// Socket
//========


static size_t s_Instances;


//-----------------------------
// I_Socket::IncrementUseCount
//
void I_Socket::IncrementUseCount()
{
	if (s_Instances++ == 0u)
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			ET_ERROR("Failed to initialize WinSock, error code: %i", WSAGetLastError());
		}
		else
		{
			ET_TRACE_I(ET_CTX_CORE, "WinSock initialized");
		}
	}
}

//-----------------------------
// I_Socket::DecrementUseCount
//
void I_Socket::DecrementUseCount()
{
	if (--s_Instances == 0u)
	{
		if (WSACleanup() != 0)
		{
			ET_ERROR("Failed to deinitialize WinSock, error code: %i", WSAGetLastError());
		}
		else
		{
			ET_TRACE_I(ET_CTX_CORE, "WinSock deinitialized");
		}
	}
}

//------------------
// I_Socket::Create
//
// Instatiate a Windows socket
//
RefPtr<I_Socket> I_Socket::Create(E_AddressFamily const family, E_SocketType const type, E_Protocol const protocol)
{
	SOCKET const socketHandle = socket(ToWinSock(family), ToWinSock(type), ToWinSock(protocol));
	if (socketHandle == -1)
	{
		ET_ERROR("Error creating Win Socket");
		return nullptr;
	}

	return et::Create<WindowsSocket>(socketHandle);
}

//--------------------
// I_Socket::PortNtoH
//
int32 I_Socket::PortNtoH(T_Port const port)
{
	return static_cast<int32>(ntohs(port));
}

//----------------------------
// I_Socket::GetAddressString
//
std::string I_Socket::GetAddressString(SocketAddress const& addr)
{
	char remoteIp[INET6_ADDRSTRLEN];
	char const* const res = inet_ntop(ToWinSock(addr.m_Family), reinterpret_cast<void const*>(&addr.m_Ip6), remoteIp, INET6_ADDRSTRLEN);
	if (res == nullptr)
	{
		ET_ERROR("Error converting address string: %i", WSAGetLastError());
		return std::string();
	}

	return res;
}

//--------------------------
// I_Socket::GetAddressInfo
//
std::vector<AddressInfo> I_Socket::GetAddressInfo(char const* const address,
	char const* const port,
	E_AddressFamily const family,
	E_SocketType const socketType,
	T_AddressFlags const flags,
	E_Protocol const protocol)
{
	std::vector<AddressInfo> ret;

	addrinfo hints;
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = ToWinSock(family);
	hints.ai_socktype = ToWinSock(socketType);
	hints.ai_flags = ToWinSock(static_cast<E_AddressFlags>(flags));
	hints.ai_protocol = ToWinSock(protocol);

	int32 status;
	addrinfo* infoList = nullptr;
	if ((status = getaddrinfo(address, port, &hints, &infoList)) != 0)
	{
		ET_ERROR("Failed to get address info list: %s", gai_strerror(status));
		return ret;
	}

	addrinfo* currentInfo = nullptr;
	for (currentInfo = infoList; currentInfo != nullptr; currentInfo = currentInfo->ai_next)
	{
		ret.push_back(AddressInfo());
		AddressInfo& info = ret.back();

		info.m_SocketType = ToSocketType(currentInfo->ai_socktype);
		info.m_Protocol = ToProtocol(currentInfo->ai_protocol);

		ET_ASSERT(currentInfo->ai_addr->sa_family == currentInfo->ai_family);
		ToEndpoint(*currentInfo->ai_addr, info.m_Endpoint);
	}

	freeaddrinfo(infoList);

	return ret;
}

//----------------
// I_Socket::Poll
//
int32 I_Socket::Poll(std::vector<PollDesc>& pollDescriptors, int32 const timeout)
{
	std::vector<pollfd> pfds;
	pfds.reserve(pollDescriptors.size());

	for (PollDesc const& desc : pollDescriptors)
	{
		pfds.push_back(pollfd());
		pfds.back().fd = reinterpret_cast<WindowsSocket const*>(desc.m_Socket.Get())->GetImpl();
		pfds.back().events = ToWinSock(static_cast<E_PollEvent>(desc.m_InFlags));
	}

	int32 const ret = WSAPoll(pfds.data(), static_cast<ULONG>(pfds.size()), timeout);
	ET_ASSERT(ret != SOCKET_ERROR);

	ET_ASSERT_PARANOID(pfds.size() == pollDescriptors.size());
	for (size_t idx = 0u; idx < pfds.size(); ++idx)
	{
		pollDescriptors[idx].m_Events = ToPollEvent(pfds[idx].revents);
	}

	return ret;
}


//================
// Windows Socket
//================


//----------------------
// WindowsSocket::c-tor
//
WindowsSocket::WindowsSocket(SOCKET const handle) 
	: I_Socket()
	, m_Handle(handle)
{
	IncrementUseCount();
}

//----------------------
// WindowsSocket::d-tor
//
WindowsSocket::~WindowsSocket()
{
	if (closesocket(m_Handle) < 0)
	{
		ET_ERROR("Error closing Win Socket!");
	}

	DecrementUseCount();
}

//------------------------
// WindowsSocket::Connect
//
bool WindowsSocket::Connect(Endpoint const& endpoint)
{
	sockaddr_storage addr;
	int32 const addrLength = ToWinSock(endpoint, addr);
	if (connect(m_Handle, reinterpret_cast<sockaddr const*>(&addr), addrLength) == SOCKET_ERROR)
	{
		ET_ERROR("Failed to connect socket to address: %i", WSAGetLastError());
		return false;
	}

	return true;
}

//---------------------
// WindowsSocket::Bind
//
bool WindowsSocket::Bind(Endpoint const& endpoint)
{
	sockaddr_storage addr;
	int32 const addrLength = ToWinSock(endpoint, addr);
	if (bind(m_Handle, reinterpret_cast<sockaddr const*>(&addr), addrLength) == SOCKET_ERROR)
	{
		ET_ERROR("Failed to bind socket to address: %i", WSAGetLastError());
		return false;
	}

	return true;
}

//-----------------------
// WindowsSocket::Listen
//
bool WindowsSocket::Listen(int32 const backlog)
{
	if (listen(m_Handle, backlog) == SOCKET_ERROR)
	{
		ET_ERROR("Failed to listen with socket: %i", WSAGetLastError());
		return false;
	}

	return true;
}

//-----------------------
// WindowsSocket::Listen
//
RefPtr<I_Socket> WindowsSocket::Accept(Endpoint& outEndpoint, bool& wouldBlock)
{
	sockaddr_storage remoteaddr;
	socklen_t addrlen = sizeof(remoteaddr);

	SOCKET const newHandle = accept(m_Handle, reinterpret_cast<sockaddr*>(&remoteaddr), &addrlen);
	if (newHandle == SOCKET_ERROR)
	{
		int32 const error = WSAGetLastError();
		if (error != EWOULDBLOCK)
		{
			ET_ERROR("Failed to accept new connection: %i", error);
			wouldBlock = false;
		}
		else
		{
			wouldBlock = true;
		}
		
		return nullptr;
	}

	wouldBlock = false;
	ToEndpoint(*reinterpret_cast<sockaddr*>(&remoteaddr), outEndpoint);

	return et::Create<WindowsSocket>(newHandle);
}

//---------------------
// WindowsSocket::Send
//
int32 WindowsSocket::Send(char const* const msg, int32 const length) const
{
	int32 const numBytes = send(m_Handle, msg, length, 0);
	if (numBytes == SOCKET_ERROR)
	{
		ET_ERROR("Error sending data: %i", WSAGetLastError());
	}

	return numBytes;
}

//------------------------
// WindowsSocket::Receive
//
int32 WindowsSocket::Receive(char* const outBuffer, int32 const length) const
{
	int32 const numBytes = recv(m_Handle, outBuffer, length, 0);
	if (numBytes == SOCKET_ERROR)
	{
		ET_ERROR("Error receiving data: %i", WSAGetLastError());
	}

	return numBytes;
}

//---------------------------------------
// WindowsSocket::SetAddressReuseEnabled
//
void WindowsSocket::SetAddressReuseEnabled(bool const enabled)
{
	BOOL const optVal = enabled ? TRUE : FALSE;
	if (setsockopt(m_Handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char const*>(&optVal), sizeof(BOOL)) == SOCKET_ERROR)
	{
		ET_ERROR("Failed to set address reuse: %i", WSAGetLastError());
	}
}

//----------------------------
// WindowsSocket::SetBlocking
//
void WindowsSocket::SetBlocking(bool const blocking)
{
	u_long mode = blocking ? 1u : 0u;
	if (ioctlsocket(m_Handle, FIONBIO, &mode) == SOCKET_ERROR)
	{
		ET_ERROR("Failed to set blocking mode: %i", WSAGetLastError());
	}
}

//----------------------------
// WindowsSocket::GetPeerName
//
bool WindowsSocket::GetPeerName(Endpoint& outEndpoint) const
{
	sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);

	if (getpeername(m_Handle, reinterpret_cast<sockaddr*>(&addr), &addrlen) == SOCKET_ERROR)
	{
		ET_ERROR("Couldn't get peer name: %i", WSAGetLastError());
		return false;
	}

	ToEndpoint(*reinterpret_cast<sockaddr*>(&addr), outEndpoint);
	return true;
}


} // namespace network

} // namespace core
} // namespace et

#endif // ET_PLATFORM_WIN
