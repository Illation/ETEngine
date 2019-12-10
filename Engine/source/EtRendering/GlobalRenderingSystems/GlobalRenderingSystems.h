#pragma once

#include "GraphicsSettings.h"
#include "PrimitiveRenderer.h"
#include "PbrPrefilter.h"
#include "AtmospherePrecompute.h"
#include "CIE.h"
#include "LightVolume.h"
#include "SharedVarController.h"

#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/PlanetTech/Patch.h>


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

	static void AddReference(render::GraphicsSettings const& settings);
	static void AddReference();
	static void RemoveReference();

	// construct destruct
	//--------------------
private:
	RenderingSystems() = default;
	~RenderingSystems() = default;

	void Initialize();

	// functionality
	//---------------
public:
	void SetGraphicsSettings(render::GraphicsSettings const& settings) { m_GraphicsSettings = settings; }

	// accessors
	//-----------
	render::GraphicsSettings const& GetGraphicsSettings() const { return m_GraphicsSettings; }

	PrimitiveRenderer& GetPrimitiveRenderer() { return m_PrimitiveRenderer; }
	PbrPrefilter& GetPbrPrefilter() { return m_PbrPrefilter; }
	render::SharedVarController& GetSharedVarController() { return m_SharedVarController; }
	CIE& GetCie() { return m_Cie; }
	DirectLightVolume& GetDirectLightVolume() { return m_DirectLightVolume; }
	PointLightVolume& GetPointLightVolume() { return m_PointLightVolume; }
	render::AtmospherePrecompute& GetAtmospherPrecompute() { return m_AtmospherePrecompute; }
	render::Patch& GetPatch() { return m_Patch; }
	render::Material const* GetNullMaterial() const { return m_NullMaterial.get(); }
	render::Material const* GetColorMaterial() const { return m_ColorMaterial.get(); }

	// Data
	///////
private:

	render::GraphicsSettings m_GraphicsSettings;

	PrimitiveRenderer m_PrimitiveRenderer;

	PbrPrefilter m_PbrPrefilter;

	render::SharedVarController m_SharedVarController;

	CIE m_Cie;

	DirectLightVolume m_DirectLightVolume;
	PointLightVolume m_PointLightVolume;

	render::AtmospherePrecompute m_AtmospherePrecompute;
	render::Patch m_Patch;

	AssetPtr<render::Material> m_NullMaterial;
	AssetPtr<render::Material> m_ColorMaterial;
};