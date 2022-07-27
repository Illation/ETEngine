#pragma once
#ifdef ET_PLATFORM_LINUX

#include <EtCore/Network/Socket.h>


namespace et {
namespace core {

namespace network {


//-------------
// LinuxSocket
//
// Linux implementation for a network socket
//
class LinuxSocket final : public I_Socket
{
	// construct destruct
	//--------------------
public:
	LinuxSocket() : I_Socket() {}
	~LinuxSocket() = default;

	// socket interface
	//------------------
public:
	bool Bind(Endpoint const& endpoint) override;
	bool Listen(int32 const backlog) override;

	void SetAddressReuseEnabled(bool const enabled) override;
};


} // namespace network

} // namespace core
} // namespace et

#endif // ET_PLATFORM_LINUX
