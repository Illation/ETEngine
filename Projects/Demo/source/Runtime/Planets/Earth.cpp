#include "stdafx.h"
#include "Earth.h"


Earth::Earth()
{
	m_Params.radius = 6371.1f;
	m_Params.height = 8.848f;
	m_Params.texDiffuseId = "Earth8k.jpg"_hash;
	m_Params.texHeightId = "EarthHeight8k.jpg"_hash;
}