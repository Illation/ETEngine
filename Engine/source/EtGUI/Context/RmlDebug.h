#pragma once
#include <imgui/imgui.h>

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include <EtCore/UpdateCycle/Tickable.h>


namespace et {
namespace gui {


class RmlRenderer;
class FontEngine;


//---------------------------------
// RmlDebug
//
// Debugging options for Rml based UI
//
class RmlDebug final : public core::I_Tickable
{
public:
	RmlDebug(); // it doesn't matter when we tick
	void Init(Ptr<RmlRenderer const> const renderer, Ptr<FontEngine const> const fontEngine);

	// interface
	//-----------
protected:
	void OnTick() override;

	// utility
	//-----------
private:
	void ShowContextUI();
	void ShowRenderingUI();

	void AddOpaqueImage(render::TextureData const* const texture, vec2 const maxScale);
	vec2 GetScale(vec2 const inScale, vec2 const maxScale) const;


	// Data
	///////

	bool m_IsVisible = false;

	Ptr<RmlRenderer const> m_Renderer;
	Ptr<FontEngine const> m_FontEngine;
};


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
