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
// Sends trace messages to a separately running TraceServer process over the network
//
class NetworkTraceHandler : public I_TraceHandler
{
	// definitions
	//-------------
	RTTR_ENABLE(I_TraceHandler)

	static int32 const s_Timeout;

	// construct destruct
	//--------------------
public:
	NetworkTraceHandler(std::string const& traceClientName) : m_TraceClientName(traceClientName) {}
	virtual ~NetworkTraceHandler(); 

	// interface
	//-----------
	virtual bool Initialize() override;
	virtual void OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message) override;

	// functionality
	//---------------
	void UpdateClientName(std::string const& traceClientName);

	// utility
	//---------
private:
	TracePackage::E_Type ReceivePackage(std::vector<uint8>& buffer);


	// Data
	///////

	RefPtr<network::I_Socket> m_Socket;
	std::string m_TraceClientName;
};


} // namespace core
} // namespace et
