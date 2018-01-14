#include "stdafx.hpp"
#include "Earth.hpp"
#include "..\Engine\Graphics\TextureData.hpp"

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
	m_pDiffuse = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/Earth8k.jpg");
	m_pHeight = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/EarthHeight8k.jpg");
}