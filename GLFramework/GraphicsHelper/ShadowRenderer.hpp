#pragma once
#include "../StaticDependancies/glad/glad.h"

class ShaderData;
class TextureData;
class NullMaterial;
class MeshFilter;
class DirectionalShadowData;
class TransformComponent;

class ShadowRenderer : public Singleton<DirectLightVolume>
{
public:
	ShadowRenderer();
	virtual ~ShadowRenderer();

	void MapDirectional(TransformComponent *pTransform, DirectionalShadowData *pShadowData);
	glm::mat4 GetLightWVP() { return m_LightWVP; }
	NullMaterial* GetNullMaterial() { return m_pMaterial; }

private:
	friend class AbstractFramework; //should init and destroy singleton

	void Initialize();
	bool IsInitialized = false;


	NullMaterial* m_pMaterial;
	glm::mat4 m_LightWVP;
};

class DirectionalShadowData
{
	//later for cascaded shadow mapping we need a bunch of those
public:
	DirectionalShadowData(glm::ivec2 Resolution, glm::ivec2 Dimensions, float ncp, float fcp);
	virtual ~DirectionalShadowData();

private:
	GLuint m_FBO;
	GLuint m_RBO;
	TextureData* m_pTexture;

	glm::ivec2 m_Dimensions;
	float m_NCP;
	float m_FCP;
};