#ifdef ET_PLATFORM_LINUX

#include "stdafx.h"
#include "Socket_Linux.h"


namespace et {
namespace core {

namespace network {


//========
// Socket
//========


//------------------
// I_Socket::Create
//
// Instatiate a linux socket
//
RefPtr<I_Socket> I_Socket::Create(E_AddressFamily const, E_SocketType const, E_Protocol const)
{
	ET_ERROR("Not implemented");
	return nullptr;
}

//--------------------------
// I_Socket::GetAddressInfo
//
std::vector<AddressInfo> I_Socket::GetAddressInfo(char const* const, // address
	char const* const, // port
	E_AddressFamily const, 
	E_SocketType const, 
	T_AddressFlags const,
	E_Protocol const)
{
	ET_ERROR("Not Implemented!");
	return std::vector<AddressInfo>();
}

//==============
// Linux Socket
//==============


//-------------------
// LinuxSocket::Bind
//
bool LinuxSocket::Bind(Endpoint const&)
{
	ET_ERROR("Not Implemented!");
	return false;
}

//---------------------
// LinuxSocket::Listen
//
bool LinuxSocket::Listen(int32 const)
{
	ET_ERROR("Not Implemented!");
	return false;
}

//-------------------------------------
// LinuxSocket::SetAddressReuseEnabled
//
void LinuxSocket::SetAddressReuseEnabled(bool const)
{
	ET_ERROR("Not Implemented!");
}


} // namespace network

} // namespace core
} // namespace et

#endif // ET_PLATFORM_LINUX
