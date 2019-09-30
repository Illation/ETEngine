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
#include <EtCore/stdafx.h>
#include <EtCore/Helper/InputManager.h>
#include <EtCore/Helper/PerformanceInfo.h>

#include <Engine/Components/TransformComponent.h>
#include <Engine/Base/Config.h>
#include <Engine/Base/SceneContext.h>
#include <Engine/GraphicsHelper/GraphicsApiContext.h>
#include <Engine/GraphicsHelper/Viewport.h>

//Working singleton Set
#define CAMERA static_cast<SceneContext*>(ContextManager::GetInstance()->GetActiveContext())->camera
#define SCENE static_cast<SceneContext*>(ContextManager::GetInstance()->GetActiveContext())->scene
#define INPUT InputManager::GetInstance()
#define TRANSFORM GetTransform()
#pragma endregion Macros