#pragma once

#include "PrimitiveRenderer.h"
#include "PbrPrefilter.h"
#include "AtmospherePrecompute.h"
#include "CIE.h"
#include "LightVolume.h"
#include "ScreenshotCapture.h"

#include <Engine/Materials/NullMaterial.h>
#include <Engine/PlanetTech/Patch.h>


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

	static void AddReference();
	static void RemoveReference();

	// construct destruct
	//--------------------
private:
	RenderingSystems() = default;
	~RenderingSystems() = default;

	void Initialize();

	// accessors
	//-----------
public:
	PrimitiveRenderer& GetPrimitiveRenderer() { return m_PrimitiveRenderer; }
	PbrPrefilter& GetPbrPrefilter() { return m_PbrPrefilter; }
	CIE& GetCie() { return m_Cie; }
	DirectLightVolume& GetDirectLightVolume() { return m_DirectLightVolume; }
	PointLightVolume& GetPointLightVolume() { return m_PointLightVolume; }
	render::AtmospherePrecompute& GetAtmospherPrecompute() { return m_AtmospherePrecompute; }
	render::Patch& GetPatch() { return m_Patch; }
	ScreenshotCapture& GetScreenshotCapture() { return m_ScreenshotCapture; }
	NullMaterial* GetNullMaterial() { return &m_NullMaterial; }

	// Data
	///////
private:

	PrimitiveRenderer m_PrimitiveRenderer;

	PbrPrefilter m_PbrPrefilter;

	CIE m_Cie;

	DirectLightVolume m_DirectLightVolume;
	PointLightVolume m_PointLightVolume;

	render::AtmospherePrecompute m_AtmospherePrecompute;
	render::Patch m_Patch;

	ScreenshotCapture m_ScreenshotCapture;

	NullMaterial m_NullMaterial;
};