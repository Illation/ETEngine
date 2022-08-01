#include "stdafx.h"
#include "NetworkTraceHandler.h"

#include <EtCore/Network/Socket.h>


namespace et {
namespace core {


//=======================
// Network Trace Handler
//=======================


//----------------------------
// NetworkTraceHandler::d-tor
//
// socket dereferencing should close the connection
//
NetworkTraceHandler::~NetworkTraceHandler()
{
	core::network::I_Socket::DecrementUseCount(); // Might deinitit socket library if we're not using it elsewhere
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
}

//--------------------------------------
// NetworkTraceHandler::ReceiveAllBytes
//
// use formated buffer size for byte count
//
bool NetworkTraceHandler::ReceiveAllBytes(std::vector<uint8>& buffer) const
{
	int32 bytesLeft = static_cast<int32>(buffer.size());

	while (bytesLeft >= 0)
	{
		int32 const received = m_Socket->Receive(reinterpret_cast<char*>(&buffer[0]), bytesLeft);
		if (received < 0)
		{
			ET_LOG_W(ET_CTX_CORE, "Error receiving bytes from trace server!");
			break;
		}

		bytesLeft -= received;
	}

	return (bytesLeft == 0);
}

//-------------------------------------
// NetworkTraceHandler::ReceivePackage
//
TracePackage::E_Type NetworkTraceHandler::ReceivePackage(std::vector<uint8>& buffer)
{
	static std::vector<uint8> headerBuffer(TracePackage::GetHeaderSize());

	if (!ReceiveAllBytes(headerBuffer))
	{
		ET_LOG_W(ET_CTX_CORE, "Failed to receive header data!");
		return TracePackage::E_Type::Invalid;
	}

	uint16 packageSize = 0u;
	TracePackage::E_Type const packageType = TracePackage::ReadHeader(headerBuffer, packageSize);

	buffer.resize(static_cast<size_t>(packageSize));
	if (!ReceiveAllBytes(buffer))
	{
		ET_LOG_W(ET_CTX_CORE, "Failed to receive buffer data!");
		buffer.clear();
		return TracePackage::E_Type::Invalid;
	}

	return packageType;
}



} // namespace core
} // namespace et
