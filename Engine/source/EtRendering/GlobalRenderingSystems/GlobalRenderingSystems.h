#pragma once

#include "GraphicsSettings.h"
#include "PbrPrefilter.h"
#include "AtmospherePrecompute.h"
#include "CIE.h"
#include "LightVolume.h"
#include "RenderDebugVars.h"
#include "SharedVarController.h"

#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/PlanetTech/Patch.h>


namespace et {
namespace render {


//---------------------------------
// RenderingSystems
//
// Container for all rendering systems that only need to exist once
//
class RenderingSystems final
{
	// definitions
	//-------------
private:
	static RenderingSystems* s_Instance;
	static size_t s_RefCount;

	friend class RenderingSystems;

	// singleton
	//------------
public:
	static RenderingSystems* Instance();

	static void AddReference(GraphicsSettings const& settings);
	static void AddReference();
	static void RemoveReference();

	// construct destruct
	//--------------------
private:
	RenderingSystems() = default;
	~RenderingSystems();

	void Initialize();

	// functionality
	//---------------
public:
	void SetGraphicsSettings(GraphicsSettings const& settings) { m_GraphicsSettings = settings; }

	// accessors
	//-----------
	GraphicsSettings const& GetGraphicsSettings() const { return m_GraphicsSettings; }

	PbrPrefilter& GetPbrPrefilter() { return m_PbrPrefilter; }
	SharedVarController& GetSharedVarController() { return m_SharedVarController; }
	CIE& GetCie() { return m_Cie; }
	DirectLightVolume& GetDirectLightVolume() { return m_DirectLightVolume; }
	PointLightVolume& GetPointLightVolume() { return m_PointLightVolume; }
	AtmospherePrecompute& GetAtmospherPrecompute() { return m_AtmospherePrecompute; }
	Patch& GetPatch() { return m_Patch; }
	Material const* GetNullMaterial() const { return m_NullMaterial.get(); }
	Material const* GetColorMaterial() const { return m_ColorMaterial.get(); }

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	DebugVars const& GetDebugVars() const { return m_DebugVars; }
#endif

	// Data
	///////
private:

	GraphicsSettings m_GraphicsSettings;

	PbrPrefilter m_PbrPrefilter;

	SharedVarController m_SharedVarController;

	CIE m_Cie;

	DirectLightVolume m_DirectLightVolume;
	PointLightVolume m_PointLightVolume;

	AtmospherePrecompute m_AtmospherePrecompute;
	Patch m_Patch;

	AssetPtr<Material> m_NullMaterial;
	AssetPtr<Material> m_ColorMaterial;

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	DebugVars m_DebugVars;
#endif
};


} // namespace render
} // namespace et
