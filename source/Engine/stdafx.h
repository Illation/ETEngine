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

#include <rttr/registration>

#pragma endregion stl

#pragma region
//SDL and opengl Header files
#include <ThirdParty/glad/glad.h>
#include <SDL.h>
#include <SDL_opengl.h>
#pragma endregion sdl-opengl

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

//platform independant utility to suppress unused parameter warnings from https://stackoverflow.com/questions/4851075
// use expression as sub-expression,
// then make type of full expression int, discard result
#define UNUSED(x) (void)(x)

#pragma warning(disable : 4201) //nameless struct union - used in math library

#pragma region
#include <EtCore/stdafx.h>
#include <EtCore/Helper/InputManager.h>
#include <EtCore/Helper/PerformanceInfo.h>

#include <Engine/Components/TransformComponent.h>
#include <Engine/Content/ContentManager.h>
#include <Engine/Base/Settings.h>
#include <Engine/Base/SceneContext.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/GraphicsHelper/RenderState.h>

//Working singleton Set
#define CAMERA static_cast<SceneContext*>(ContextManager::GetInstance()->GetActiveContext())->camera
#define SCENE static_cast<SceneContext*>(ContextManager::GetInstance()->GetActiveContext())->scene
#define SETTINGS Settings::GetInstance()
#define INPUT InputManager::GetInstance()
#define CONTENT ContentManager
#define TRANSFORM GetTransform()
#define WINDOW Settings::GetInstance()->Window
#define GRAPHICS Settings::GetInstance()->Graphics
#define PIPELINE RenderPipeline::GetInstance()
#define STATE RenderPipeline::GetInstance()->GetState()
#pragma endregion Macros