#pragma once
#include <imgui/imgui.h>

#include "RmlDebuggerFwd.h"


// fwd
#if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
namespace Rml {
	class Context;
}
#endif


#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include <EtCore/UpdateCycle/Tickable.h>


namespace et {
namespace gui {


class RmlRenderer;
class FontEngine;
class SdfFont;


//---------------------------------
// RmlDebug
//
// Debugging options for Rml based UI
//
class RmlDebug final : public core::I_Tickable
{
	static vec2 const s_MaxTextureScale;

	// construct destruct
	//--------------------
public:
	RmlDebug(); // it doesn't matter when we tick
	void Init(Ptr<RmlRenderer const> const renderer, Ptr<FontEngine const> const fontEngine);

	// functionality
	//---------------
#if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
	void OnContextDestroyed(Rml::Context const* const context);
#endif

	// interface
	//-----------
protected:
	void OnTick() override;

	// utility
	//-----------
private:
	void ShowContextUI();
	void ShowRenderingUI();
	void ShowFontUI();

	void AddOpaqueImage(render::TextureData const* const texture, vec2 const maxScale);
	vec2 GetScale(vec2 const inScale, vec2 const maxScale) const;

	void AddFontAsset(AssetPtr<SdfFont> const& asset);
	void AddFontFace(size_t const faceIdx);


	// Data
	///////

	bool m_IsVisible = false;

	Ptr<RmlRenderer const> m_Renderer;
	Ptr<FontEngine const> m_FontEngine;

#if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
	int32 m_DebugContextIdx = -1;
	Ptr<Rml::Context> m_DebuggerContext;
#endif
};


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
