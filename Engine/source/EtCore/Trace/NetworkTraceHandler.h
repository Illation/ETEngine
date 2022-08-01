#pragma once
#include "TraceHandlerInterface.h"

#include "TracePackage.h"


namespace et {
namespace core {


namespace network {
	class I_Socket;
}


//---------------------
// NetworkTraceHandler
//
// Sends trace messages to a seperately running TraceServer process over the network
//
class NetworkTraceHandler : public I_TraceHandler
{
	RTTR_ENABLE(I_TraceHandler)

public:
	virtual ~NetworkTraceHandler(); 

	virtual bool Initialize() override;
	virtual void OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message) override;

	// utility
	//---------
private:
	bool ReceiveAllBytes(std::vector<uint8>& buffer) const;

	TracePackage::E_Type ReceivePackage(std::vector<uint8>& buffer);


	// Data
	///////

	RefPtr<network::I_Socket> m_Socket;
};


} // namespace core
} // namespace et
