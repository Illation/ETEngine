#include "stdafx.h"
#include "MainFramework.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/SpriteFont.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtFramework/SceneGraph/SceneManager.h>
#include <EtFramework/Audio/AudioManager.h>

#include <Runtime/Scenes/EditorScene.h>
#include <Runtime/Scenes/PlanetTestScene.h>
#include <Runtime/Scenes/PhysicsTestScene.h>


//================
// Main Framework
//================


//--------------------------
// MainFramework::AddScenes
//
// Init function
//
void MainFramework::AddScenes()
{
	// Fonts
	m_DebugFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Ubuntu-Regular.ttf"_hash);

	// scenes
	SceneManager* const sceneMan = SceneManager::GetInstance();

	sceneMan->AddScene(new EditorScene());
	sceneMan->AddScene(new PlanetTestScene());
	sceneMan->AddScene(new PhysicsTestScene());
	
	sceneMan->GetEventDispatcher().Register(E_SceneEvent::Activated,
		T_SceneEventCallback([this](T_SceneEventFlags const flags, SceneEventData const* const evnt)
		{
			UNUSED(flags);
			UNUSED(evnt);

			CameraComponent* const cam = CAMERA;
			ET_ASSERT(evnt->scene->GetActiveCamera() == cam);

			m_CameraController.SetCameraComponent(cam);
			m_CameraController.Reset();
		}));

	// audio
	AudioManager::GetInstance()->SetDistanceModel(AL_INVERSE_DISTANCE);
}

//--------------------------
// MainFramework::OnTick
//
// Demo specific updates indepenant of scene
//
void MainFramework::OnTick()
{
	SceneManager* const sceneMan = SceneManager::GetInstance();
	InputManager* const input = InputManager::GetInstance();

	// Scene switching
	//-----------------
	if(input->GetKeyState(E_KbdKey::F3) == E_KeyState::Pressed)
	{
		sceneMan->PreviousScene();
	}

	if(input->GetKeyState(E_KbdKey::F4) == E_KeyState::Pressed)
	{
		sceneMan->NextScene();
	}

	// Screenshots
	//-------------
	if (input->GetKeyState(E_KbdKey::F12) == E_KeyState::Pressed)
	{
		m_ScreenshotCapture.Take(Viewport::GetCurrentViewport());
	}

	// view
	//------
	m_CameraController.Update();

	bool const up = (input->GetKeyState(E_KbdKey::Up) == E_KeyState::Down);
	if (up || (input->GetKeyState(E_KbdKey::Down) == E_KeyState::Down))
	{
		render::Scene& renderScene = sceneMan->GetRenderScene();
		PostProcessingSettings ppSettings = renderScene.GetPostProcessingSettings();

		float const newExp = ppSettings.exposure * 4.f;
		ppSettings.exposure += (newExp - ppSettings.exposure) * TIME->DeltaTime() * (up ? 1.f : -1.f);

		LOG(FS("Exposure: %f", ppSettings.exposure));

		renderScene.SetPostProcessingSettings(ppSettings);
	}

	// debug info
	//------------
	if (input->GetKeyState(E_KbdKey::H) == E_KeyState::Pressed)
	{
		m_DrawDebugInfo = !m_DrawDebugInfo;
	}

	if (input->GetKeyState(E_KbdKey::J) == E_KeyState::Pressed)
	{
		m_DrawFontAtlas = !m_DrawFontAtlas;
	}

	I_ViewportRenderer* const viewRenderer = Viewport::GetCurrentViewport()->GetViewportRenderer();
	if (viewRenderer != nullptr && viewRenderer->GetType() == typeid(render::ShadedSceneRenderer))
	{
		render::ShadedSceneRenderer* const sceneRenderer = static_cast<render::ShadedSceneRenderer*>(viewRenderer);

		if (m_DrawDebugInfo)
		{
			TextRenderer& textRenderer = sceneRenderer->GetTextRenderer();

			textRenderer.SetFont(m_DebugFont.get());
			textRenderer.SetColor(vec4(1, 0.3f, 0.3f, 1));
			std::string outString = "FPS: " + std::to_string(PERFORMANCE->GetRegularFPS());
			textRenderer.DrawText(outString, vec2(20, 20 + (m_DebugFont->GetFontSize()*1.1f) * 1));
			textRenderer.SetColor(vec4(1, 1, 1, 1));
			outString = "Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS());
			textRenderer.DrawText(outString, vec2(20, 20 + (m_DebugFont->GetFontSize()*1.1f) * 2));
			outString = "Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls);
			textRenderer.DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*1.1f) * 3), 128);
		}

		if (m_DrawFontAtlas)
		{
			SpriteRenderer::E_ScalingMode const scalingMode = SpriteRenderer::E_ScalingMode::TextureAbs;
			sceneRenderer->GetSpriteRenderer().Draw(m_DebugFont->GetAtlas(), vec2(1000, 0), vec4(1), vec2(0), vec2(1), 0, 0, scalingMode);
		}
	}
}