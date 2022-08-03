#include "stdafx.h"
#include "TraceServer.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/Reflection/TypeInfoRegistry.h>
#include <EtCore/Trace/TracePackage.h>
#include <EtCore/Trace/ConsoleTraceHandler.h>
#include <EtCore/Trace/DebugOutputTraceHandler.h>
#include <EtCore/Network/NetworkUtil.h>


namespace et {
namespace trace {


//==================
// E T Trace Server
//==================


//--------------------
// TraceServer::c-tor
//
// Sets up according to commandline arguments
//
TraceServer::TraceServer(int32 const argc, char* const argv[])
{
	ET_UNUSED(argc);
	ET_UNUSED(argv);

	// Init stuff
	//------------
	core::TraceService::Initialize();
	core::TraceService::Instance()->AddHandler<core::ConsoleTraceHandler>();
#if ET_CT_IS_ENABLED(ET_CT_TRACE_DBG_OUT)
	core::TraceService::Instance()->AddHandler<core::DebugOutputTraceHandler>();
#endif

	core::TypeInfoRegistry::Instance().Initialize(); 

	ET_LOG_I(ET_CTX_TRACE, "E.T.Trace");
	ET_LOG_I(ET_CTX_TRACE, "/////////");
	ET_LOG_I(ET_CTX_TRACE, "");
	ET_LOG_I(ET_CTX_TRACE, " - version: %s", build::Version::s_Name.c_str());
	ET_LOG_I(ET_CTX_TRACE, "");

	m_Time.Start();

	// Setup Socket
	//--------------

	m_ReturnCode = E_ReturnCode::ErrorListening;

	core::network::I_Socket::IncrementUseCount(); // Init network library

	std::vector<core::network::AddressInfo> addresses = core::network::I_Socket::GetAddressInfo(nullptr,
		core::TracePackage::s_TraceServerPort.c_str(),
		core::network::E_AddressFamily::InterNetwork,
		core::network::E_SocketType::Stream,
		core::network::E_AddressFlags::AF_Passive);

	core::network::Endpoint listenerEp;
	for (core::network::AddressInfo const& info : addresses)
	{
		m_ListenerSocket = core::network::I_Socket::Create(info.m_Endpoint.m_Address.m_Family, info.m_SocketType, info.m_Protocol);
		if (m_ListenerSocket == nullptr)
		{
			continue;
		}

		m_ListenerSocket->SetAddressReuseEnabled(true);

		if (!m_ListenerSocket->Bind(info.m_Endpoint))
		{
			m_ListenerSocket = nullptr;
			continue;
		}

		listenerEp = info.m_Endpoint;
		break;
	}

	if (m_ListenerSocket == nullptr)
	{
		ET_FATAL("Couldn't create a listener socket for the trace server!");
	}

	if (!m_ListenerSocket->Listen(10))
	{
		ET_FATAL("Listener socket couldn't listen!");
	}

	m_ReturnCode = E_ReturnCode::Success;
	m_PollDescriptors.push_back(core::network::PollDesc(m_ListenerSocket, core::network::E_PollEvent::PE_In));

	ET_LOG_I(ET_CTX_TRACE, "Listening for connections @ '%s:%i'",
		core::network::I_Socket::GetAddressString(listenerEp.m_Address).c_str(),
		core::network::I_Socket::PortNtoH(listenerEp.m_Port));
}

//--------------------
// TraceServer::d-tor
//
TraceServer::~TraceServer()
{
	core::network::I_Socket::DecrementUseCount(); // Deinit network library
	core::TraceService::Destroy();
}

//------------------
// TraceServer::Run
//
// Run according to arguments passed to class
//
void TraceServer::Run()
{
	for (;;)
	{
		// we periodically check in even if there are no new events so we can remove stale clients
		int32 const pollCount = core::network::I_Socket::Poll(m_PollDescriptors, TraceClient::s_SetupTimeout); 
		if (pollCount == -1)
		{
			m_ReturnCode = E_ReturnCode::ErrorDuringExecution;
			ET_FATAL("Polling failed!");
		}

		m_Time.Update();
		uint64 const timestampMs = m_Time.Timestamp() / 1000;

		core::network::T_PollDescs pollDescriptorsCopy = m_PollDescriptors;
		for (core::network::PollDesc const& pollDesc : pollDescriptorsCopy)
		{
			if (pollDesc.m_Events & core::network::E_PollEvent::PE_Disconnected)
			{
				ET_ASSERT(pollDesc.m_Socket != m_ListenerSocket);

				RemoveClient(pollDesc.m_Socket.Get());
				continue;
			}

			if (pollDesc.m_Socket == m_ListenerSocket)
			{
				if (!(pollDesc.m_Events & core::network::E_PollEvent::PE_In))
				{
					continue;
				}

				core::network::Endpoint remoteEp;
				bool wouldBlock;
				RefPtr<core::network::I_Socket> newSocket = m_ListenerSocket->Accept(remoteEp, wouldBlock);
				if (newSocket == nullptr)
				{
					ET_ERROR("Failed to accept new connection");
				}
				else
				{
					TraceClient const& client = AddClient(newSocket, timestampMs);
				}
			}
			else
			{
				TraceClient& client = GetClient(pollDesc.m_Socket.Get());
				if (client.GetState() == TraceClient::E_State::Invalid)
				{
					RemoveClient(pollDesc.m_Socket.Get());
					continue;
				}

				if (!(pollDesc.m_Events & core::network::E_PollEvent::PE_In))
				{
					if ((client.GetState() != TraceClient::E_State::Ready) && ((timestampMs - client.GetTimestamp()) >= TraceClient::s_SetupTimeout))
					{
						ET_LOG_W(ET_CTX_TRACE, "%s took too long to setup, removing...", client.GetName().c_str());
						RemoveClient(pollDesc.m_Socket.Get());
					}

					continue;
				}

				ReceiveData(client, timestampMs);
			}
		}
	}
}

//--------------------------
// TraceServer::ReceiveData
//
void TraceServer::ReceiveData(TraceClient& client, uint64 const timestamp)
{
	static std::vector<uint8> s_HeaderBuffer(core::TracePackage::GetHeaderSize());

	size_t bytesReceived;
	if (!core::network::ReceiveAllBytes(client.GetSocket(), s_HeaderBuffer, bytesReceived))
	{
		if (bytesReceived == 0)
		{
			RemoveClient(client.GetSocket());
		}
		else
		{
			ET_ERROR("Error receiving data");
		}
	}
	else
	{
		uint16 packageSize = 0u;
		core::TracePackage::E_Type const packageType = core::TracePackage::ReadHeader(s_HeaderBuffer, packageSize);

		std::vector<uint8> packageBuffer;
		if (packageSize > 0u)
		{
			packageBuffer.resize(static_cast<size_t>(packageSize));
			if (!core::network::ReceiveAllBytes(client.GetSocket(), packageBuffer, bytesReceived))
			{
				ET_WARNING("Failed to receive package data!");
				return;
			}
		}

		client.HandleTracePackage(packageType, packageBuffer, timestamp);
	}
}

//------------------------
// TraceServer::AddClient
//
TraceClient& TraceServer::AddClient(RefPtr<core::network::I_Socket> const socket, uint64 const timestamp)
{
	ET_ASSERT(m_PollDescriptors.size() == m_Clients.size() + 1u);
	m_PollDescriptors.emplace_back(socket, core::network::E_PollEvent::PE_In);
	m_Clients.emplace_back(socket, timestamp);

	ET_LOG_I(ET_CTX_TRACE, "New connection from '%s'", m_Clients.back().GetName().c_str());

	return m_Clients.back();
}

//---------------------------
// TraceServer::RemoveClient
//
void TraceServer::RemoveClient(core::network::I_Socket const* const socket)
{
	T_Clients::iterator const clientIt = GetClientIt(socket);
	ET_ASSERT(clientIt != m_Clients.cend());

	core::network::T_PollDescs::iterator const pollIt = m_PollDescriptors.begin() + ((clientIt - m_Clients.begin()) + 1u);
	ET_ASSERT(pollIt->m_Socket.Get() == clientIt->GetSocket());

	ET_LOG_I(ET_CTX_TRACE, "%s Disconnected", clientIt->GetName().c_str());

	core::RemoveSwap(m_Clients, clientIt);
	core::RemoveSwap(m_PollDescriptors, pollIt);
}

//------------------------
// TraceServer::GetClient
//
TraceClient& TraceServer::GetClient(core::network::I_Socket const* const socket)
{
	T_Clients::iterator const clientIt = GetClientIt(socket);
	ET_ASSERT(clientIt != m_Clients.cend());
	return *clientIt;
}

//--------------------------
// TraceServer::GetClientIt
//
TraceServer::T_Clients::iterator TraceServer::GetClientIt(core::network::I_Socket const* const socket)
{
	return std::find_if(m_Clients.begin(), m_Clients.end(), [socket](TraceClient const& client)
		{
			return (client.GetSocket() == socket);
		});
}


} // namespace trace
} // namespace et
