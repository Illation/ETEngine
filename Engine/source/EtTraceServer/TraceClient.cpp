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
					ET_LOG_I(ET_CTX_TRACE, "Client '%s': Connection acknowledged.", GetName().c_str());
					break;
				}
				else
				{
					ET_LOG_W(ET_CTX_TRACE, "Client '%s': Failed to send connection acknowledge data!", GetName().c_str());
				}
			}
			else
			{
				ET_LOG_W(ET_CTX_TRACE, "Client '%s': Connection acknowledged package had incorrect data!", GetName().c_str());
			}

			m_State = E_State::Invalid;
		}
		else
		{
			goodType = false;
		}

		break;

	case E_State::MatchingProtocol:
		if (type == core::TracePackage::E_Type::ClientInfo)
		{
			uint16 contextCount = 0u;
			std::string clientName;
			core::TracePackage::ReadClientInfo(packageData, clientName, contextCount);

			if (core::network::SendAllBytes(m_Socket.Get(), core::TracePackage::WriteHasClient()))
			{
				m_State = E_State::Named;
				ET_LOG_I(ET_CTX_TRACE, "Client '%s': Client info received - new name: %s", GetName().c_str(), clientName.c_str());

				m_ExpectedContextCount = static_cast<size_t>(contextCount);
				m_Name = clientName;

				break;
			}

			ET_LOG_W(ET_CTX_TRACE, "Client '%s': Failed to send client info confirmation!", GetName().c_str());
			m_State = E_State::Invalid;
		}
		else
		{
			goodType = false;
		}

		break;

	case E_State::Named:
		if (type == core::TracePackage::E_Type::ContextsDone)
		{
			if (!packageData.empty())
			{
				ET_LOG_W(ET_CTX_TRACE, "Client '%s': Expected contexts done to be an empty package!", GetName().c_str());
				m_State = E_State::Invalid;
				break;
			}

			if (m_ContextContainer.GetContextCount() != m_ExpectedContextCount)
			{
				ET_LOG_W(ET_CTX_TRACE,
					"Client '%s': Received contexts done signal, but context name count ["ET_FMT_SIZET"] didn't match expected count ["ET_FMT_SIZET"]!",
					m_ContextContainer.GetContextCount(),
					m_ExpectedContextCount,
					GetName().c_str());

				m_State = E_State::Invalid;
				break;
			}

			if (core::network::SendAllBytes(m_Socket.Get(), core::TracePackage::WriteInitSuccess()))
			{
				m_State = E_State::Ready;
				ET_LOG_I(ET_CTX_TRACE, "Client '%s': Connection setup and ready to receive messages", GetName().c_str());
				break;
			}

			ET_LOG_W(ET_CTX_TRACE, "Client '%s': Failed to send initialization success!", GetName().c_str());
			m_State = E_State::Invalid;
		}
		else if (type == core::TracePackage::E_Type::ContextName)
		{
			std::string contextName;
			core::TracePackage::ReadContextName(packageData, contextName);
			ET_LOG_I(ET_CTX_TRACE, "Client '%s': Received context name: %s", GetName().c_str(), contextName.c_str());

			m_ContextContainer.RegisterContext(contextName);
		}
		else
		{
			goodType = false;
		}

		break;

	case E_State::Ready:
		if (type == core::TracePackage::E_Type::TraceMessage)
		{
			HandleTraceMessage(packageData);
		}
		else if (type == core::TracePackage::E_Type::UpdateClientName)
		{
			std::string clientName;
			core::TracePackage::ReadUpdateClientName(packageData, clientName);

			if (core::network::SendAllBytes(m_Socket.Get(), core::TracePackage::WriteInitSuccess()))
			{
				ET_LOG_I(ET_CTX_TRACE, "Client '%s': Setting new client name: %s", GetName().c_str(), clientName.c_str());
				m_Name = clientName;
				break;
			}

			ET_LOG_W(ET_CTX_TRACE, "Client '%s': Failed to send initialization success after client name change!", GetName().c_str());
			m_State = E_State::Invalid;
		}
		else
		{
			goodType = false;
		}

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
	if (m_Name.empty())
	{
		return FS("%s:%i", core::network::I_Socket::GetAddressString(m_Endpoint.m_Address).c_str(), core::network::I_Socket::PortNtoH(m_Endpoint.m_Port));
	}
	else
	{
		return m_Name;
	}
}

//---------------------------------
// TraceClient::HandleTraceMessage
//
// Read an incoming trace message
//
void TraceClient::HandleTraceMessage(std::vector<uint8> const& packageData)
{
	T_Hash context;
	core::E_TraceLevel level;
	std::string timestamp;
	std::string message;
	core::TracePackage::ReadTraceMessage(packageData, context, level, timestamp, message);

	std::string const& contextName = m_ContextContainer.GetContextName(core::HashString(context));

	ET_LOG(ET_CTX_TRACE, level, false, 
		"%s%s [%s]> %s", 
		timestamp.empty() ? "" : FS("[%s]", timestamp.c_str()).c_str(), GetName().c_str(), contextName.c_str(), message.c_str());
}


} // namespace trace
} // namespace et
