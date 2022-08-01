#pragma once
#include "NetworkTypes.h"


namespace et {
namespace core {

namespace network {


//----------
// I_Socket
//
// Platform independent network socket wrapper
// also contains static functions to access platform dependent network functionality
//
class I_Socket 
{
	// static
	//--------
public:
	// Init / Deinit socket library before using other socket functions
	static void IncrementUseCount(); 
	static void DecrementUseCount(); 

	// Per platform socket factory
	static RefPtr<I_Socket> Create(E_AddressFamily const family, E_SocketType const type, E_Protocol const protocol);

	// Utility functions
	static int32 PortNtoH(T_Port const port);
	static std::string GetAddressString(SocketAddress const& addr);
	static std::vector<AddressInfo> GetAddressInfo(char const* const address, 
		char const* const port, 
		E_AddressFamily const family, 
		E_SocketType const socketType,
		T_AddressFlags const flags = E_AddressFlags::AF_None,
		E_Protocol const protocol = E_Protocol::Any);

	static int32 Poll(T_PollDescs& pollDescriptors, int32 const timeout); // timeout in ms, or -1 for infinity

	static std::string GetHostName(); 
	static void GetHostByName(std::vector<SocketAddress>& outAddresses, std::string const& hostName);

	// construct destruct
	//--------------------
protected:
	I_Socket() = default;

public:
	virtual ~I_Socket() = default; // closes the socket

	// functionality
	//---------------
	virtual bool Connect(Endpoint const& endpoint) = 0;
	virtual bool Bind(Endpoint const& endpoint) = 0;
	virtual bool Listen(int32 const backlog) = 0;
	virtual RefPtr<I_Socket> Accept(Endpoint& outEndpoint, bool& wouldBlock) = 0; // can block

	virtual int32 Send(char const* const msg, int32 const length) const = 0;
	virtual int32 Receive(char* const outBuffer, int32 const length) const = 0; // can block

	virtual void SetAddressReuseEnabled(bool const enabled) = 0;
	virtual void SetBlocking(bool const blocking) = 0;

	// accessors
	//-----------
	virtual bool GetPeerName(Endpoint& outEndpoint) const = 0;
};


} // namespace network

} // namespace core
} // namespace et
