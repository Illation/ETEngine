#include "stdafx.h"
#include "Earth.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>


Earth::Earth():Planet()
{
	m_Radius = 6371.1f;
	m_MaxHeight = 8.848f;
}
Earth::~Earth()
{
}
void Earth::LoadPlanet()
{
	m_pDiffuse = ResourceManager::GetInstance()->GetAssetData<TextureData>("Earth8k.jpg"_hash);
	m_pHeight = ResourceManager::GetInstance()->GetAssetData<TextureData>("EarthHeight8k.jpg"_hash);
}