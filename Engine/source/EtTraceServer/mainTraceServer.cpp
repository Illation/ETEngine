#include "stdafx.h"

#include <EtCore/linkerHelper.h>

#include <EtTraceServer/TraceServer.h>



//---------------------------------
// main
//
// Main function for the trace server
//
int main(int argc, char *argv[])
{
	et::core::ForceLinking();

	et::trace::TraceServer traceServer(argc, argv);
	if (traceServer.GetReturnCode() == et::trace::TraceServer::E_ReturnCode::Success)
	{
		traceServer.Run();
	}

	return static_cast<int>(traceServer.GetReturnCode());
}