#include "stdafx.h"
#include "ContentManager.h"

#include "MeshFilterLoader.h"
#include "FontLoader.h"


std::vector<AbstractLoader*> ContentManager::m_Loaders = std::vector<AbstractLoader*>();
bool ContentManager::m_IsInitialized = false;

ContentManager::ContentManager()
{
}


ContentManager::~ContentManager(void)
{
}

void ContentManager::Release()
{
	for(AbstractLoader *ldr:m_Loaders)
	{	
		ldr->Unload();
		if (!(ldr == nullptr))
		{
			delete ldr;
			ldr = nullptr;
		}
	}

	m_Loaders.clear();
}

void ContentManager::Initialize()
{
	if(!m_IsInitialized)
	{
		m_IsInitialized = true;
		AddLoader(new MeshFilterLoader());
		AddLoader(new FontLoader());
	}
}

void ContentManager::AddLoader(AbstractLoader* loader)
{ 
	for(AbstractLoader *ldr:m_Loaders)
	{	
		if(ldr->GetType()==loader->GetType())
		{
			if (!(loader == nullptr))
			{
				delete loader;
				loader = nullptr;
			}
			break;
		}
	}

	m_Loaders.push_back(loader);
}