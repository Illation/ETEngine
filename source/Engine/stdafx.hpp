#pragma once

#pragma region
//C RunTime Header Files
#include <wchar.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

using namespace std;
#pragma endregion stl

#pragma region
//SDL and opengl Header files
#include "staticDependancies/glad/glad.h"
#include <SDL.h>
#include <SDL_opengl.h>
#pragma endregion sdl-opengl

#pragma region
#ifndef GLM_FORCE_LEFT_HANDED
	#define GLM_FORCE_LEFT_HANDED
#endif
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
using namespace glm;
#pragma endregion glm

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

template<class T>
inline void SafeDelete(T &pObjectToDelete)
{
	if (pObjectToDelete != 0)
	{
		delete(pObjectToDelete);
		pObjectToDelete = 0;
	}
}

template<typename T>
inline void Clamp(T& value, T hi, T lo)
{
	if (value > hi)
		value = hi;

	if (value < lo)
		value = lo;
}
#pragma endregion Templates

#pragma region
#include "Helper/AtomicTypes.hpp"
#include "Components/TransformComponent.hpp"
#include "Content/ContentManager.hpp"

#include "Base\Context.hpp"
#include "Base\Settings.hpp"
#include "Base\InputManager.hpp"
#include "Helper/Logger.hpp"
#include "Helper/MathHelper.hpp"
#include "Helper/PerformanceInfo.hpp"
#include "GraphicsHelper/RenderPipeline.hpp"
#include "GraphicsHelper/RenderState.hpp"
//Working singleton Set
#define TIME Context::GetInstance()->pTime
#define CAMERA Context::GetInstance()->pCamera
#define SCENE Context::GetInstance()->pScene
#define SETTINGS Settings::GetInstance()
#define INPUT InputManager::GetInstance()
#define LOGGER Logger
#define CONTENT ContentManager
#define TRANSFORM GetTransform()
#define WINDOW Settings::GetInstance()->Window
#define GRAPHICS Settings::GetInstance()->Graphics
#define PERFORMANCE PerformanceInfo::GetInstance()
#define PIPELINE RenderPipeline::GetInstance()
#define STATE RenderPipeline::GetInstance()->GetState()
#pragma endregion Macros