#pragma once
#include <EtCore/Content/AssetPointer.h>

class TextureData;
class NullMaterial;
class DirectionalShadowData;
class TransformComponent;

class ShadowRenderer final
{
	// definitions
	//-------------
private:
	friend class SceneRenderer; 

	// construct destruct
	//---------------------
	ShadowRenderer() = default;
	~ShadowRenderer();

	void Initialize();

	// functionality
	//---------------
public:
	void MapDirectional(TransformComponent *pTransform, DirectionalShadowData *pShadowData);

	// accessors
	//---------------
	mat4 GetLightVP() { return m_LightVP; }
	NullMaterial* GetNullMaterial() { return m_pMaterial; }

	// Data
	///////
private:

	bool IsInitialized = false;

	AssetPtr<ShaderData> m_Shader;
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
		T_FbLoc fbo;
		TextureData* pTexture;
		mat4 lightVP;
	};
	std::vector<CascadeData> m_Cascades;

	float m_Bias = 0;
};