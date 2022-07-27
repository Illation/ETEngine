#pragma once
#ifdef ET_PLATFORM_WIN

#include <EtCore/Network/Socket.h>

#include <Ws2tcpip.h>
#include <WinSock2.h>


namespace et {
namespace core {

namespace network {


//---------------
// WindowsSocket
//
// Windows implementation for a network socket
//
class WindowsSocket final : public I_Socket
{
	// construct destruct
	//--------------------
public:
	WindowsSocket(SOCKET const handle);
	~WindowsSocket();

	// socket interface
	//------------------
public:
	// functionality
	bool Connect(Endpoint const& endpoint) override;
	bool Bind(Endpoint const& endpoint) override;
	bool Listen(int32 const backlog) override;
	RefPtr<I_Socket> Accept(Endpoint& outEndpoint, bool& wouldBlock) override;

	int32 Send(char const* const msg, int32 const length) const override;
	int32 Receive(char* const outBuffer, int32 const length) const override; 

	void SetAddressReuseEnabled(bool const enabled) override;
	void SetBlocking(bool const blocking) override;

	// accessors
	bool GetPeerName(Endpoint& outEndpoint) const override;

	// utility
	//---------
	SOCKET GetImpl() const { return m_Handle; }


	// Data
	///////

private:
	SOCKET m_Handle;
};


} // namespace network

} // namespace core
} // namespace et

#endif // ET_PLATFORM_WIN
