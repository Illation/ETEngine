#include "stdafx.h"
#include "TraceClient.h"

#include <EtCore/Network/NetworkUtil.h>


namespace et {
namespace trace {


//==============
// Trace Client
//==============


// reflection
//////////////
RTTR_REGISTRATION
{
	rttr::registration::enumeration<TraceClient::E_State>("TraceClientState") (
		rttr::value("Invalid", TraceClient::E_State::Invalid),
		rttr::value("InitialConnection", TraceClient::E_State::InitialConnection),
		rttr::value("MatchingProtocol", TraceClient::E_State::MatchingProtocol),
		rttr::value("Named", TraceClient::E_State::Named),
		rttr::value("Ready", TraceClient::E_State::Ready));
}

// static
uint64 const TraceClient::s_SetupTimeout = 10000; // 10 seconds


//--------------------
// TraceClient::c-tor
//
TraceClient::TraceClient(RefPtr<core::network::I_Socket> const socket, uint64 const timestamp)
	: m_Socket(socket)
	, m_Timestamp(timestamp)
{
	m_Socket->GetPeerName(m_Endpoint);
}

//---------------------------------
// TraceClient::HandleTracePackage
//
void TraceClient::HandleTracePackage(core::TracePackage::E_Type const type, std::vector<uint8> const& packageData, uint64 const timestamp)
{
	bool goodType = true;
	switch (m_State)
	{
	case E_State::InitialConnection:
		if (type == core::TracePackage::E_Type::ConnectionAcknowledged)
		{
			if (core::TracePackage::ReadConnectionAcknowledged(packageData))
			{
				if (core::network::SendAllBytes(m_Socket.Get(), core::TracePackage::WriteConnectionAcknowledged()))
				{
					m_State = E_State::MatchingProtocol;
					ET_LOG_I(ET_CTX_CORE, "Client '%s': Connection acknowledged.", GetName().c_str());
					break;
				}
				else
				{
					ET_LOG_W(ET_CTX_CORE, "Client '%s': Failed to send connection acknowledge data!", GetName().c_str());
				}
			}
			else
			{
				ET_LOG_W(ET_CTX_CORE, "Client '%s': Connection acknowledged package had incorrect data!", GetName().c_str());
			}

			m_State = E_State::Invalid;
		}
		else
		{
			goodType = false;
		}

		break;

	case E_State::MatchingProtocol:
		break;

	case E_State::Named:
		break;

	case E_State::Ready:
		break;

	default: return;
	}

	if (goodType)
	{
		m_Timestamp = timestamp;
	}
	else
	{
		ET_LOG_W(ET_CTX_TRACE,
			"Client '%s': Unexpected trace package type '%s' during state '%s'",
			GetName().c_str(),
			core::reflection::EnumString(type).c_str(),
			core::reflection::EnumString(m_State).c_str());
	}
}

//----------------------
// TraceClient::GetName
//
// Print IP address or name if we have it
//
std::string TraceClient::GetName() const
{
	return FS("%s:%i", core::network::I_Socket::GetAddressString(m_Endpoint.m_Address).c_str(), core::network::I_Socket::PortNtoH(m_Endpoint.m_Port));
}


} // namespace trace
} // namespace et
