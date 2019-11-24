#include "stdafx.h"
#include "Moon.h"


Moon::Moon()
{
	m_Params.radius = 1737.1f;
	m_Params.height = 10.7f;
	m_Params.texDiffuseId = "moon8k.jpg"_hash;
	m_Params.texHeightId = "MoonHeight.jpg"_hash;

	m_AtmosphereAssetId = "atmo_earth.json"_hash;
	m_AtmosphereHeight = 100;
}