#pragma once

#define NOMINMAX

//#define GRAPHICS_API_DEBUG
//#define GRAPHICS_API_VERBOSE

#pragma region
//C RunTime Header Files
#include <wchar.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

#pragma endregion stl

#pragma region
//*****************************************************************************
//Declare templates for releasing interfaces and deleting objects 
//*****************************************************************************
template<class Interface>
inline void SafeRelease(Interface &pInterfaceToRelease)
{
	if (pInterfaceToRelease != 0)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = 0;
	}
}
#pragma endregion Templates

#pragma region
#include <EtCore/stdafx.h>
#include <EtCore/Helper/InputManager.h>
#include <EtCore/Helper/PerformanceInfo.h>

#include <Engine/Components/TransformComponent.h>
#include <Engine/Base/Settings.h>
#include <Engine/Base/SceneContext.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/GraphicsHelper/RenderState.h>
#include <Engine/GraphicsHelper/Viewport.h>

//Working singleton Set
#define CAMERA static_cast<SceneContext*>(ContextManager::GetInstance()->GetActiveContext())->camera
#define SCENE static_cast<SceneContext*>(ContextManager::GetInstance()->GetActiveContext())->scene
#define SETTINGS Settings::GetInstance()
#define INPUT InputManager::GetInstance()
#define TRANSFORM GetTransform()
#define WINDOW Settings::GetInstance()->Window
#define GRAPHICS Settings::GetInstance()->Graphics
#define PIPELINE RenderPipeline::GetInstance()
#define STATE Viewport::GetGlobalRenderState()
#pragma endregion Macros