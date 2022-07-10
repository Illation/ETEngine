#pragma once
#include <imgui/imgui.h>

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include <EtCore/UpdateCycle/Tickable.h>


namespace et {
namespace gui {


//---------------------------------
// RmlDebug
//
// Debugging options for Rml based UI
//
class RmlDebug final : public core::I_Tickable
{
public:
	RmlDebug(); // it doesn't matter when we tick

	// interface
	//-----------
protected:
	void OnTick() override;


	// Data
	///////
private:

	bool m_IsVisible = false;
};


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
