#include "stdafx.h"
#include "ResourceManager.h"


//===================
// Resource Manager
//===================


// static
ResourceManager* ResourceManager::s_Instance = nullptr;


//----------------------------------
// ResourceManager::SetInstance
//
// Initializes the singleton with a derived implementation
//
void ResourceManager::SetInstance(ResourceManager* const instance)
{
	s_Instance = instance;
	s_Instance->Init();
}

//----------------------------------
// ResourceManager::DestroyInstance
//
// Deinitializes the singleton
//
void ResourceManager::DestroyInstance()
{
	s_Instance->Deinit();
	SafeDelete(s_Instance);
}
