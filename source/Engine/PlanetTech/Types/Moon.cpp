#include "stdafx.h"
#include "Moon.h"

#include "../../Texture.h"

Moon::Moon():Planet()
{
	m_pDiffuse = new Texture("./Textures/moon8k.jpg");
	m_pHeight = new Texture("./Textures/MoonHeight.jpg");

	m_Radius = 1737.1f;
	m_MaxHeight = 10.7f;
}
Moon::~Moon()
{
}