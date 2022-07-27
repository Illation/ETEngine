#pragma once


namespace et {
namespace core {

namespace base64 {


static inline bool IsBase64(unsigned char c) { return (isalnum(c) || (c == '+') || (c == '/')); }

std::string Encode(std::vector<uint8> const& data);
bool Decode(std::string const& encoded, std::vector<uint8>& decoded);


} // namespace base64

} // namespace core
} // namespace et
