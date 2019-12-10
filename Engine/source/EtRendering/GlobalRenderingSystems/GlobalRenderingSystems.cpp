#include <EtRendering/stdafx.h>
#include "GlobalRenderingSystems.h"

#include <rttr/registration>

#include <EtCore/Content/ResourceManager.h>


//===================
// Graphics Settings
//===================


// reflection - done here because graphics settings don't have a source file with linked code
RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<render::GraphicsSettings>("graphics settings")
		.constructor<>()
		.property("use FXAA", &render::GraphicsSettings::UseFXAA)
		.property("CSM cascade count", &render::GraphicsSettings::NumCascades)
		.property("CSM draw distance", &render::GraphicsSettings::CSMDrawDistance)
		.property("PCF sample count", &render::GraphicsSettings::NumPCFSamples)
		.property("BRDF LUT size", &render::GraphicsSettings::PbrBrdfLutSize)
		.property("texture scale factor", &render::GraphicsSettings::TextureScaleFactor)
		.property("bloom blur passes", &render::GraphicsSettings::NumBlurPasses)
		;
}


//===========================
// Global Rendering Systems
//===========================


// static
RenderingSystems* RenderingSystems::s_Instance = nullptr;
size_t RenderingSystems::s_RefCount = 0u;


//---------------------------------
// RenderingSystems::Instance
//
// Retrieve the singleton
//
RenderingSystems* RenderingSystems::Instance()
{
	ET_ASSERT(s_Instance != nullptr);

	return s_Instance;
}

//---------------------------------
// RenderingSystems::AddReference
//
// This version of add reference initializes with the specified graphics settings
//
void RenderingSystems::AddReference(render::GraphicsSettings const& settings)
{
	s_RefCount++;

	if (s_RefCount == 1u)
	{
		s_Instance = new RenderingSystems();
		s_Instance->SetGraphicsSettings(settings);
		s_Instance->Initialize();
	}
	else
	{
		s_Instance->SetGraphicsSettings(settings);
	}
}

//---------------------------------
// RenderingSystems::AddReference
//
// Should be called upon creation of a system that requires global rendering systems, such as the scene renderer, to ensure singleton existance
//
void RenderingSystems::AddReference()
{
	s_RefCount++;

	if (s_RefCount == 1u)
	{
		s_Instance = new RenderingSystems();
		s_Instance->Initialize();
	}
}

//---------------------------------
// RenderingSystems::RemoveReference
//
// Should be called upon destruction of a depending system which called AddReference()
//
void RenderingSystems::RemoveReference()
{
	ET_ASSERT(s_RefCount > 0u);
	s_RefCount--;

	if (s_RefCount == 0u)
	{
		delete s_Instance;
		s_Instance = nullptr;
	}
}

//---------------------------------
// RenderingSystems::Initialize
//
// Called upon first reference creation, and initializes all rendering systems
//
void RenderingSystems::Initialize()
{
	m_SharedVarController.Init();

	m_AtmospherePrecompute.Init();

	m_Cie.LoadData();
	m_PbrPrefilter.Precompute(m_GraphicsSettings.PbrBrdfLutSize);

	m_NullMaterial = ResourceManager::Instance()->GetAssetData<render::Material>("M_Null.json"_hash);
	m_ColorMaterial = ResourceManager::Instance()->GetAssetData<render::Material>("M_Color.json"_hash);

	m_Patch.Init(4);
}
