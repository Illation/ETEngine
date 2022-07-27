#include "stdafx.h"
#include "TraceServer.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/Reflection/TypeInfoRegistry.h>


namespace et {
namespace trace {


//==================
// E T Trace Server
//==================


// static
std::string const TraceServer::s_Port("3491");


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
	core::TraceService::Instance()->StartFileLogging("cooker.log");

	core::TypeInfoRegistry::Instance().Initialize(); 

	ET_LOG_I(ET_CTX_TRACE, "E.T.Trace");
	ET_LOG_I(ET_CTX_TRACE, "/////////");
	ET_LOG_I(ET_CTX_TRACE, "");
	ET_LOG_I(ET_CTX_TRACE, " - version: %s", build::Version::s_Name.c_str());
	ET_LOG_I(ET_CTX_TRACE, "");

	// Setup Socket
	//--------------

	core::network::I_Socket::IncrementUseCount(); // Init network library

	std::vector<core::network::AddressInfo> addresses = core::network::I_Socket::GetAddressInfo(nullptr,
		s_Port.c_str(),
		core::network::E_AddressFamily::Unspecified,
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

	m_PollDescriptors.push_back(core::network::PollDesc());
	m_PollDescriptors.back().m_Socket = m_ListenerSocket;
	m_PollDescriptors.back().m_InFlags = core::network::E_PollEvent::PE_In;

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
		int32 const pollCount = core::network::I_Socket::Poll(m_PollDescriptors, -1);
		if (pollCount == -1)
		{
			ET_FATAL("Polling failed!");
		}

		std::vector<core::network::PollDesc> pollDescriptorsCopy = m_PollDescriptors;
		for (core::network::PollDesc const& pollDesc : pollDescriptorsCopy)
		{
			if (pollDesc.m_Events & core::network::E_PollEvent::PE_Disconnected)
			{
				ET_ASSERT(pollDesc.m_Socket != m_ListenerSocket);

				core::network::Endpoint remoteEp;
				pollDesc.m_Socket->GetPeerName(remoteEp);
				ET_LOG_I(ET_CTX_TRACE, "%s:%i Disconnected",
					core::network::I_Socket::GetAddressString(remoteEp.m_Address).c_str(),
					core::network::I_Socket::PortNtoH(remoteEp.m_Port));

				m_PollDescriptors[&pollDesc - &pollDescriptorsCopy.front()] = m_PollDescriptors.back();
				m_PollDescriptors.pop_back();

				continue;
			}

			if (!(pollDesc.m_Events & core::network::E_PollEvent::PE_In))
			{
				continue;
			}

			if (pollDesc.m_Socket == m_ListenerSocket)
			{
				core::network::Endpoint remoteEp;
				bool wouldBlock;
				RefPtr<core::network::I_Socket> newSocket = m_ListenerSocket->Accept(remoteEp, wouldBlock);
				if (newSocket == nullptr)
				{
					ET_ERROR("Failed to accept new connection");
				}
				else
				{
					m_PollDescriptors.push_back(core::network::PollDesc());
					m_PollDescriptors.back().m_Socket = newSocket;
					m_PollDescriptors.back().m_InFlags = core::network::E_PollEvent::PE_In;

					ET_LOG_I(ET_CTX_TRACE, "New connection from '%s:%i'", 
						core::network::I_Socket::GetAddressString(remoteEp.m_Address).c_str(),
						core::network::I_Socket::PortNtoH(remoteEp.m_Port));
				}
			}
			else
			{
				char buf[256];
				int32 const numBytes = pollDesc.m_Socket->Receive(buf, sizeof(buf));

				if (numBytes < 0)
				{
					ET_ERROR("Error receiving data");
				}
				else if (numBytes == 0)
				{
					core::network::Endpoint remoteEp;
					pollDesc.m_Socket->GetPeerName(remoteEp);
					ET_LOG_I(ET_CTX_TRACE, "%s:%i Disconnected",
						core::network::I_Socket::GetAddressString(remoteEp.m_Address).c_str(),
						core::network::I_Socket::PortNtoH(remoteEp.m_Port));

					m_PollDescriptors[&pollDesc - &pollDescriptorsCopy.front()] = m_PollDescriptors.back();
					m_PollDescriptors.pop_back();
				}
				else
				{
					core::network::Endpoint remoteEp;
					pollDesc.m_Socket->GetPeerName(remoteEp);
					std::string const message(FS("%s:%i: %s", 
						core::network::I_Socket::GetAddressString(remoteEp.m_Address).c_str(),
						core::network::I_Socket::PortNtoH(remoteEp.m_Port), 
						std::string(buf, numBytes).c_str()));

					ET_LOG_I(ET_CTX_TRACE, "Received message: %s", message.c_str());

					for (core::network::PollDesc const& innerDesc : m_PollDescriptors)
					{
						if ((innerDesc.m_Socket != m_ListenerSocket) && (innerDesc.m_Socket != pollDesc.m_Socket))
						{
							if (!innerDesc.m_Socket->Send(message.c_str(), static_cast<int32>(message.size())))
							{
								ET_ERROR("Error sending data");
							}
						}
					}
				}
			}
		}
	}
}


} // namespace cooker
} // namespace et
