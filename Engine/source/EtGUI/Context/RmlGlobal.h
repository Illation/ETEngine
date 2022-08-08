#pragma once
#include "RmlSystemInterface.h"
#include "RmlFileInterface.h"
#include "DataModelFactory.h"
#include "RmlDebug.h"

#include <EtGUI/Fonts/FontEngine.h>
#include <EtGUI/Fonts/FontEffects.h>
#include <EtGUI/Rendering/RmlRenderer.h>


namespace et {
namespace gui {


//---------------------------------
// RmlGlobal
//
// Singleton that manages global access into the RmlUi library
//
class RmlGlobal final
{
	// definitions
	//-------------
private:
	friend class RefPtr<RmlGlobal>;
	friend class Create<RmlGlobal>;

	static RefPtr<RmlGlobal> s_Instance;
	static DataModelFactory s_DataModelFactory;

	// singleton access
	//------------------
public:
	static RefPtr<RmlGlobal> GetInstance();
	static void Destroy();
	static bool IsInitialized();

	static DataModelFactory& GetDataModelFactory() { return s_DataModelFactory; }

	// construct destruct
	//--------------------
private:
	RmlGlobal();
	~RmlGlobal();

	// functionality
	//---------------
public:
	void SetCursorShapeManager(Ptr<core::I_CursorShapeManager> const cursorMan);
	void SetClipboardController(Ptr<core::I_ClipboardController> const clipboardController);

	void SetGraphicsContext(Ptr<rhi::I_GraphicsContextApi> const graphicsContext);
	void SetRIShader(AssetPtr<rhi::ShaderData> const shader, AssetPtr<rhi::ShaderData> const textShader);
	void SetRIView(ivec2 const dim, mat4 const& viewProj);


#if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
	void OnContextDestroyed(Rml::Context const* const context);
#endif

	// Data
	///////
	
private:
	RmlSystemInterface m_SystemInterface;
	RmlFileInterface m_FileInterface;
	FontEngine m_FontEngine;
	RmlRenderer m_Renderer;

	FontEffectGlow::Instancer m_GlowInstancer;
	FontEffectOutline::Instancer m_OutlineInstancer;
	FontEffectShadow::Instancer m_ShadowInstancer;
	FontEffectBlur::Instancer m_BlurInstancer;

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)
	RmlDebug m_Debug;
#endif
};


} // namespace gui
} // namespace et
