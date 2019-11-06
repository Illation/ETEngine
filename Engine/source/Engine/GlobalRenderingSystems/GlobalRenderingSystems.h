#pragma once

#include "PrimitiveRenderer.h"
#include "PbrPrefilter.h"
#include "AtmospherePrecompute.h"
#include "CIE.h"
#include "LightVolume.h"
#include "ScreenshotCapture.h"


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
	AtmospherePrecompute& GetAtmospherPrecompute() { return m_AtmospherePrecompute; }
	ScreenshotCapture& GetScreenshotCapture() { return m_ScreenshotCapture; }

	// Data
	///////
private:

	PrimitiveRenderer m_PrimitiveRenderer;

	PbrPrefilter m_PbrPrefilter;

	CIE m_Cie;

	DirectLightVolume m_DirectLightVolume;
	PointLightVolume m_PointLightVolume;

	AtmospherePrecompute m_AtmospherePrecompute;

	ScreenshotCapture m_ScreenshotCapture;
};