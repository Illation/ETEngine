#include "stdafx.h"
#include "NetworkTraceHandler.h"

#include "TraceService.h"

#include <EtCore/Network/Socket.h>
#include <EtCore/Network/NetworkUtil.h>


namespace et {
namespace core {


//=======================
// Network Trace Handler
//=======================


// static
int32 const NetworkTraceHandler::s_Timeout = 500; // ms


//----------------------------
// NetworkTraceHandler::d-tor
//
// socket dereferencing should close the connection
//
NetworkTraceHandler::~NetworkTraceHandler()
{
	core::network::I_Socket::DecrementUseCount(); // Might deinit socket library if we're not using it elsewhere
}

//---------------------------------
// NetworkTraceHandler::Initialize
//
// Open a connection to the trace server and prepare it to accept messages
//
bool NetworkTraceHandler::Initialize()
{
	core::network::I_Socket::IncrementUseCount(); // Init network library if it hasn't already happened

	// get the server network address
	//--------------------------------
	std::vector<network::SocketAddress> localAddresses; // we assume the trace server is running on localhost
	network::I_Socket::GetHostByName(localAddresses, network::I_Socket::GetHostName());

	// try to set up a connection with the server
	//--------------------------------------------
	network::Endpoint serverEp;
	for (network::SocketAddress const& localhost : localAddresses)
	{
		std::vector<network::AddressInfo> addresses = network::I_Socket::GetAddressInfo(network::I_Socket::GetAddressString(localhost).c_str(),
			TracePackage::s_TraceServerPort.c_str(),
			network::E_AddressFamily::Unspecified,
			network::E_SocketType::Stream);

		for (network::AddressInfo const& info : addresses)
		{
			m_Socket = network::I_Socket::Create(info.m_Endpoint.m_Address.m_Family, info.m_SocketType, info.m_Protocol);
			if (m_Socket == nullptr)
			{
				continue;
			}

			ET_LOG_I(ET_CTX_CORE, "Attempting connection @ '%s:%i'",
				network::I_Socket::GetAddressString(info.m_Endpoint.m_Address).c_str(),
				network::I_Socket::PortNtoH(info.m_Endpoint.m_Port));
			if (!m_Socket->Connect(info.m_Endpoint))
			{
				m_Socket = nullptr;
				continue;
			}

			serverEp = info.m_Endpoint;
			break;
		}
	}

	if (m_Socket == nullptr)
	{
		ET_LOG_W(ET_CTX_CORE, "Failed to connect to trace server!");
		return false;
	}

	ET_LOG_I(ET_CTX_CORE, "Connected to trace server @ '%s:%i'",
		network::I_Socket::GetAddressString(serverEp.m_Address).c_str(),
		network::I_Socket::PortNtoH(serverEp.m_Port));


	// Setup the connection
	//----------------------

	TracePackage::E_Type packageType = TracePackage::E_Type::Invalid;
	std::vector<uint8> packageBuffer;

	// let the server know we speak the same language
	if (!network::SendAllBytes(m_Socket.Get(), TracePackage::WriteConnectionAcknowledged()))
	{
		ET_LOG_W(ET_CTX_CORE, "Error sending connection acknowledge package!");
		return false;
	}

	// acknowledge
	packageType = ReceivePackage(packageBuffer);
	if (packageType != TracePackage::E_Type::ConnectionAcknowledged)
	{
		ET_LOG_W(ET_CTX_CORE, "Expected to receive connection acknowledge package!");
		return false;
	}

	if (!TracePackage::ReadConnectionAcknowledged(packageBuffer))
	{
		ET_LOG_W(ET_CTX_CORE, "Connection acknowledged package had incorrect data!");
		return false;
	}

	// a bit about us
	std::vector<std::string> contextNames = TraceService::GetContextContainer().GetAllContextNames();

	if (!network::SendAllBytes(m_Socket.Get(), TracePackage::WriteClientInfo(m_TraceClientName, static_cast<uint16>(contextNames.size()))))
	{
		ET_LOG_W(ET_CTX_CORE, "Error sending client info package!");
		return false;
	}

	// wait for server response
	packageType = ReceivePackage(packageBuffer);
	if (packageType != TracePackage::E_Type::HasClient)
	{
		ET_LOG_W(ET_CTX_CORE, "Expected server to confirm client info!");
		return false;
	}
	else
	{
		ET_ASSERT(packageBuffer.size() == 0u);
	}

	// now send all of the context names
	for (std::string const& contextName : contextNames)
	{
		if (!network::SendAllBytes(m_Socket.Get(), TracePackage::WriteContextName(contextName)))
		{
			ET_LOG_W(ET_CTX_CORE, "Error sending context name package!");
			return false;
		}
	}

	if (!network::SendAllBytes(m_Socket.Get(), TracePackage::WriteContextsDone()))
	{
		ET_LOG_W(ET_CTX_CORE, "Error sending context name package!");
		return false;
	}

	// wait for server response again - we should now be fully set up
	packageType = ReceivePackage(packageBuffer);
	if (packageType != TracePackage::E_Type::InitSuccess)
	{
		ET_LOG_W(ET_CTX_CORE, "Expected server to confirm successful initialization!");
		return false;
	}
	else
	{
		ET_ASSERT(packageBuffer.size() == 0u);
	}

	// Done
	//------

	ET_LOG_I(ET_CTX_CORE, "Network trace handler is ready");
	return true;
}

//-------------------------------------
// NetworkTraceHandler::OnTraceMessage
//
// Send the message to the trace server
//
void NetworkTraceHandler::OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message)
{
	if (m_Socket == nullptr)
	{
		return;
	}

	// Check socket status
	network::T_PollDescs pollDescriptors;
	pollDescriptors.push_back(network::PollDesc(m_Socket, network::E_PollEvent::PE_Out));
	int32 const pollCount = network::I_Socket::Poll(pollDescriptors, 0); // don't block at all
	if (pollCount == -1)
	{
		ET_WARNING("Error polling on trace socket!");
	}

	if (pollDescriptors.back().m_Events & core::network::E_PollEvent::PE_Disconnected)
	{
		m_Socket = nullptr;
		ET_WARNING("Trace server closed the connection!");
		return;
	}

	// now send the message
	if (!network::SendAllBytes(m_Socket.Get(), TracePackage::WriteTraceMessage(context.Get(), level, timestamp, message)))
	{
		ET_WARNING("Error sending trace message!");
	}
}

//---------------------------------------
// NetworkTraceHandler::UpdateClientName
//
// This can be useful if we need to initialize the trace handler before having a good client name available
//
void NetworkTraceHandler::UpdateClientName(std::string const& traceClientName)
{
	m_TraceClientName = traceClientName;
	if (!network::SendAllBytes(m_Socket.Get(), TracePackage::WriteUpdateClientName(traceClientName)))
	{
		ET_LOG_W(ET_CTX_CORE, "Error sending context name package!");
	}

	// server should let us know that it got the new client name
	std::vector<uint8> packageBuffer;
	if (ReceivePackage(packageBuffer) != TracePackage::E_Type::InitSuccess)
	{
		ET_LOG_W(ET_CTX_CORE, "Expected server to confirm successful initialization!");
	}
	else
	{
		ET_ASSERT(packageBuffer.size() == 0u);
	}
}

//-------------------------------------
// NetworkTraceHandler::ReceivePackage
//
TracePackage::E_Type NetworkTraceHandler::ReceivePackage(std::vector<uint8>& buffer)
{
	static std::vector<uint8> s_HeaderBuffer(TracePackage::GetHeaderSize());

	// first poll for network events so we can timeout if we don't receive anything
	network::T_PollDescs pollDescriptors;
	pollDescriptors.push_back(network::PollDesc(m_Socket, network::E_PollEvent::PE_In));
	int32 const pollCount = network::I_Socket::Poll(pollDescriptors, s_Timeout);

	if (pollCount != 1)
	{
		ET_LOG_W(ET_CTX_CORE, "Failed to poll for events on socket!");
		return TracePackage::E_Type::Invalid;
	}

	if (!(pollDescriptors.back().m_Events & core::network::E_PollEvent::PE_In))
	{
		ET_LOG_W(ET_CTX_CORE, "Didn't have package data to receive on the socket!");
		return TracePackage::E_Type::Invalid;
	}

	// now we can receive the header in order to figure out how many more bytes we need
	size_t bytesReceived;
	if (!network::ReceiveAllBytes(m_Socket.Get(), s_HeaderBuffer, bytesReceived))
	{
		ET_LOG_W(ET_CTX_CORE, "Failed to receive header data!");
		return TracePackage::E_Type::Invalid;
	}

	uint16 packageSize = 0u;
	TracePackage::E_Type const packageType = TracePackage::ReadHeader(s_HeaderBuffer, packageSize);

	// finally we can receive the package data
	buffer.resize(static_cast<size_t>(packageSize));
	if (!network::ReceiveAllBytes(m_Socket.Get(), buffer, bytesReceived))
	{
		ET_LOG_W(ET_CTX_CORE, "Failed to receive buffer data!");
		buffer.clear();
		return TracePackage::E_Type::Invalid;
	}

	return packageType;
}



} // namespace core
} // namespace et
