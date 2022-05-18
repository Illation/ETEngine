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
	RmlSystemInterface() : Rml::SystemInterface() {}
	~RmlSystemInterface() = default;

	double GetElapsedTime() override;
	bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
};


} // namespace gui
} // namespace et

