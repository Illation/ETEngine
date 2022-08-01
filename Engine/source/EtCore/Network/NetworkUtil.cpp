#include "stdafx.h"
#include "NetworkUtil.h"


namespace et {
namespace core {

namespace network {


//------------------
// SendAllBytes
//
// Send data through a socket until all of it is sent (unless there is an error)
//
bool SendAllBytes(I_Socket const* const socket, std::vector<uint8> const& buffer)
{
	int32 bytesLeft = static_cast<int32>(buffer.size());

	while (bytesLeft > 0)
	{
		int32 const sent = socket->Send(reinterpret_cast<char const*>(&buffer[0]), bytesLeft);
		if (sent < 0)
		{
			ET_LOG_W(ET_CTX_CORE, "Error sending bytes!");
			break;
		}

		bytesLeft -= sent;
	}

	return (bytesLeft == 0);
}

//------------------
// ReceiveAllBytes
//
// Receive data on a socket until the buffer is filled or there is an error
//  - use formated buffer size for byte count
//
bool ReceiveAllBytes(I_Socket const* const socket, std::vector<uint8>& inOutBuffer, size_t& outBytesReceived)
{
	int32 bytesLeft = static_cast<int32>(inOutBuffer.size());

	while (bytesLeft > 0)
	{
		int32 const received = socket->Receive(reinterpret_cast<char*>(&inOutBuffer[0]), bytesLeft);
		if (received < 0)
		{
			ET_LOG_W(ET_CTX_CORE, "Error receiving bytes!");
			break;
		}
		else if (received == 0)
		{
			break;
		}

		bytesLeft -= received;
	}

	outBytesReceived = inOutBuffer.size() - static_cast<size_t>(bytesLeft);
	return (bytesLeft == 0);
}


} // namespace network

} // namespace core
} // namespace et
