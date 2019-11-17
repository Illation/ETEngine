#pragma once

#pragma region
#include <EtRendering/stdafx.h>
#include <EtCore/Helper/InputManager.h>

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/Config/Config.h>
#include <EtFramework/Config/SceneContext.h>

//Working singleton Set
#define CAMERA static_cast<SceneContext*>(ContextManager::GetInstance()->GetActiveContext())->camera
#define SCENE static_cast<SceneContext*>(ContextManager::GetInstance()->GetActiveContext())->scene
#define INPUT InputManager::GetInstance()
#define TRANSFORM GetTransform()
#pragma endregion Macros