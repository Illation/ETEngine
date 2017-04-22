#pragma once
#include "../staticDependancies/glad/glad.h"
#include "ContentLoader.hpp"
#include "..\Graphics\TextureData.hpp"
#include <string>

class HdrLoader : public ContentLoader<HDRMap>
{
public:
	HdrLoader();
	~HdrLoader();

	void UseSrgb(bool use) { m_UseSrgb = use; }
	void SetCubemapRes(int res) { m_CubemapRes = res; }

protected:
	virtual HDRMap* LoadContent(const std::string& assetFile);
	virtual void Destroy(HDRMap* objToDestroy);

	bool m_UseSrgb = false;

private:
	int m_CubemapRes = 1024;
	int m_IrradianceRes = 32;
	int m_RadianceRes = 1024;
	int m_BrdfLutRes = 512;

	//All this stuff should move to global graphics helpers
	void RenderCube();
	void RenderQuad();
	GLuint m_CubeVAO = 0;
	GLuint m_CubeVBO = 0;
	GLuint m_QuadVAO = 0;
	GLuint m_QuadVBO = 0;
};

