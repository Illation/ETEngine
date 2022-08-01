#pragma once
#include "Socket.h"


namespace et {
namespace core {

namespace network {


bool SendAllBytes(I_Socket const* const socket, std::vector<uint8> const& buffer);
bool ReceiveAllBytes(I_Socket const* const socket, std::vector<uint8>& inOutBuffer, size_t& outBytesReceived);


} // namespace network

} // namespace core
} // namespace et
