#pragma once
#include <RmlUi/Core/SystemInterface.h>


namespace et {
namespace gui {


//---------------------------------
// RmlSystemInterface
//
// Implementation of RmlUi's system interface
//
class RmlSystemInterface final : public Rml::SystemInterface
{
public:
	// construct destruct
	//--------------------
	RmlSystemInterface() : Rml::SystemInterface() {}
	~RmlSystemInterface() = default;

	// interface implementation
	//--------------------------
	double GetElapsedTime() override;
	bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
};


} // namespace gui
} // namespace et

