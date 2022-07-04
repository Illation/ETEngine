#pragma once


namespace et {
namespace core {


//---------------------------------
// I_ClipboardController
//
// Interface class that can interact with the System clipboard
//
class I_ClipboardController
{
public:
	virtual ~I_ClipboardController() = default;

	virtual void SetClipboardText(std::string const& textUtf8) = 0;
	virtual void GetClipboardText(std::string& outTextUtf8) = 0;
};


} // namespace core
} // namespace et
