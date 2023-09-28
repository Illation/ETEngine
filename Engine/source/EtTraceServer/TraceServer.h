#pragma once
#include "TraceClient.h"

#include <EtGUI/Context/DataModel.h>

#include <EtApplication/GuiApplication.h>
#include <EtApplication/GuiWindow.h>


namespace et { 
namespace trace {


//-------------
// TraceServer
//
// Main class for the trace server program. Essentially all the work can be done by calling Run
//
class TraceServer final : public app::GuiApplication
{
	// definitions
	//-------------
	typedef std::vector<TraceClient> T_Clients;

	//----------
	// GuiData
	//
	// Data model for the hello world screen - this is to test the UI and will be replaced
	//
	struct GuiData : public gui::I_DataModel
	{
		bool m_ShowPanel = false;
	};

public:
	enum class E_ReturnCode
	{
		Success = 0,

		TraceServerAlreadyRunning,
		ErrorListening,
		ErrorDuringExecution,

		Error
	};

	// construct destruct
	//--------------------
	TraceServer(int32 const argc, char* const argv[]);
	~TraceServer();

	// functionality
	//---------------
	void Run();

	// accessors
	//-----------
	E_ReturnCode GetReturnCode() const { return m_ReturnCode; }

	// utility
	//---------
private:
	void ReceiveData(TraceClient& client, uint64 const timestamp);

	TraceClient& AddClient(RefPtr<core::network::I_Socket> const socket, uint64 const timestamp);
	void RemoveClient(core::network::I_Socket const* const socket);
	TraceClient& GetClient(core::network::I_Socket const* const socket);

	T_Clients::iterator GetClientIt(core::network::I_Socket const* const socket);

	void HandleNetworking();


	// Data
	///////

	// Server
	RefPtr<core::network::I_Socket> m_ListenerSocket; 

	// Clients
	core::network::T_PollDescs m_PollDescriptors;
	T_Clients m_Clients;

	// UI
	Ptr<app::GuiWindow> m_MainWindow;

	// General
	core::Time m_Time;
	E_ReturnCode m_ReturnCode = E_ReturnCode::Success;
};


} // namespace trace
} // namespace et
