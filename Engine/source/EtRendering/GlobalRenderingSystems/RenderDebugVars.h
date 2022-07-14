#pragma once
#include <EtCore/Util/DebugUtilFwd.h>

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)

#include <EtRendering/SceneRendering/SceneRendererFwd.h>


namespace et {
namespace render {


//---------------------------------
// DebugVars
//
// Global debug parameters for various rendering systems
//
class DebugVars
{
	// construct destruct
	//--------------------
public:
	void Init();

	// accessors
	//-----------
	bool IsFrustumFrozen() const { return m_IsFrustumFrozen; }
	void OverrideMode(E_RenderMode& renderMode) const;
	bool AtmospheresHidden() const { return m_HideAtmospheres; }


	// Data
	///////

private:
	bool m_IsFrustumFrozen = false;

	E_RenderMode m_RenderMode = E_RenderMode::Shaded;
	bool m_OverrideRenderMode = false;

	bool m_HideAtmospheres = false;
};


} // namespace render
} // namespace et

#endif // ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
