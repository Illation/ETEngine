#include <Engine/stdafx.h>
#include "GlobalRenderingSystems.h"


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
	m_Cie.LoadData();
	m_PbrPrefilter.Precompute(Config::GetInstance()->GetGraphics().PbrBrdfLutSize);

	// set up screenshot manager
	std::string const& screenshotDir = Config::GetInstance()->GetScreenshotDir();
	if (!screenshotDir.empty())
	{
		m_ScreenshotCapture.Initialize(Config::GetInstance()->GetUserDirPath() + screenshotDir);
	}
	else
	{
		m_ScreenshotCapture.Initialize(Config::GetInstance()->GetUserDirPath() + std::string("./"));
	}
}
