#pragma once
#include <EtCore/Network/Socket.h>


namespace et { 
namespace trace {


//-------------
// TraceServer
//
// Main class for the trace server program. Essentially all the work can be done by calling Run
//
class TraceServer final
{
	// definitions
	//-------------
public:
	enum class E_ReturnCode
	{
		Success = 0,

		Error
	};

	static std::string const s_Port;


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


	// Data
	///////

private:
	RefPtr<core::network::I_Socket> m_ListenerSocket;
	std::vector<core::network::PollDesc> m_PollDescriptors;

	E_ReturnCode m_ReturnCode = E_ReturnCode::Success;
};


} // namespace trace
} // namespace et
