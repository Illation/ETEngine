#pragma once
#include <EtCore/Network/Socket.h>
#include <EtCore/Trace/TracePackage.h>
#include <EtCore/Trace/TraceContextContainer.h>


namespace et {
namespace trace {


//-------------
// TraceClient
//
// Data related to a connected client
//
class TraceClient
{
	// definitions
	//-------------
public:
	enum class E_State : uint8
	{
		Invalid = 0u,

		InitialConnection,
		MatchingProtocol,
		Named,

		Ready
	};

	static uint64 const s_SetupTimeout;

	// construct destruct
	//--------------------
	TraceClient(RefPtr<core::network::I_Socket> const socket, uint64 const timestamp);

	// functionality
	//---------------

	// set state to invalid on error
	void HandleTracePackage(core::TracePackage::E_Type const type, std::vector<uint8> const& packageData, uint64 const timestamp); 

	// accessors
	//-----------
	core::network::I_Socket const* GetSocket() const { return m_Socket.Get(); }
	core::network::Endpoint const& GetEndpoint() const { return m_Endpoint; }
	std::string GetName() const;
	E_State GetState() const { return m_State; }
	uint64 GetTimestamp() const { return m_Timestamp; }

	// utility
	//---------
private:
	void HandleTraceMessage(std::vector<uint8> const& packageData);


	// Data
	///////

	RefPtr<core::network::I_Socket> m_Socket;
	core::network::Endpoint m_Endpoint;

	std::string m_Name;

	E_State m_State = E_State::InitialConnection;
	uint64 m_Timestamp = 0u; // in milliseconds - update whenever state changes

	size_t m_ExpectedContextCount = 0u;
	core::TraceContextContainer m_ContextContainer;
};


} // namespace trace
} // namespace et
