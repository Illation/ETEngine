#pragma once
#include <EtCore/Content/AssetPointer.h>


class TextureData;
class NullMaterial;
class DirectionalShadowData;
class Camera;


//---------------------------------
// I_ShadowRenderer
//
// Interface for a class that can draw a shadow depth map
//
class I_ShadowRenderer
{
public:
	virtual ~I_ShadowRenderer() = default;

	virtual void DrawShadow(NullMaterial* const nullMaterial) = 0;
	virtual Camera const& GetCamera() const = 0;
};

//---------------------------------
// ShadowRenderer
//
// Class that can fill out shadow data for light sources
//
class ShadowRenderer final
{
	// construct destruct
	//---------------------
public:
	ShadowRenderer() = default;
	~ShadowRenderer();

	void Initialize();

	// functionality
	//---------------
	void MapDirectional(mat4 const& lightTransform, DirectionalShadowData& shadowData, I_ShadowRenderer* const shadowRenderer);

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