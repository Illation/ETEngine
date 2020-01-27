#include <EtRendering/stdafx.h>
#include "GlobalRenderingSystems.h"

#include <rttr/registration>

#include <EtCore/Content/ResourceManager.h>


namespace et {
namespace render {


//===================
// Graphics Settings
//===================


// reflection - done here because graphics settings don't have a source file with linked code
RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<GraphicsSettings>("graphics settings")
		.constructor<>()
		.property("use FXAA", &GraphicsSettings::UseFXAA)
		.property("CSM cascade count", &GraphicsSettings::NumCascades)
		.property("CSM draw distance", &GraphicsSettings::CSMDrawDistance)
		.property("PCF sample count", &GraphicsSettings::NumPCFSamples)
		.property("BRDF LUT size", &GraphicsSettings::PbrBrdfLutSize)
		.property("texture scale factor", &GraphicsSettings::TextureScaleFactor)
		.property("bloom blur passes", &GraphicsSettings::NumBlurPasses)
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
void RenderingSystems::AddReference(GraphicsSettings const& settings)
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

	m_NullMaterial = core::ResourceManager::Instance()->GetAssetData<Material>(core::HashString("M_Null.json"));
	m_ColorMaterial = core::ResourceManager::Instance()->GetAssetData<Material>(core::HashString("M_Color.json"));

	m_Patch.Init(4);
}


} // namespace render
} // namespace et
