#pragma once
#include "../StaticDependancies/glad/glad.h"

class ShaderData;
class TextureData;
class NullMaterial;
class MeshFilter;
class DirectionalShadowData;
class TransformComponent;

class ShadowRenderer : public Singleton<ShadowRenderer>
{
public:
	ShadowRenderer();
	virtual ~ShadowRenderer();

	void MapDirectional(TransformComponent *pTransform, DirectionalShadowData *pShadowData);
	mat4 GetLightVP() { return m_LightVP; }
	NullMaterial* GetNullMaterial() { return m_pMaterial; }

private:
	friend class AbstractFramework;
	friend class RenderPipeline; //should init and destroy singleton

	void Initialize();
	bool IsInitialized = false;


	NullMaterial* m_pMaterial;
	mat4 m_LightVP;
};

class DirectionalShadowData
{
	//later for cascaded shadow mapping we need a bunch of those
public:
	DirectionalShadowData(ivec2 Resolution);
	virtual ~DirectionalShadowData();

private:
	friend class ShadowRenderer;
	friend class DirectLightVolume;

	struct CascadeData
	{
		float distance;
		GLuint fbo;
		TextureData* pTexture;
		mat4 lightVP;
	};
	std::vector<CascadeData> m_Cascades;

	float m_Bias = 0;
};