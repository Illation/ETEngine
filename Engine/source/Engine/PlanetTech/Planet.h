#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/SceneGraph/Entity.h>


class ShaderData;
class Frustum;
class Triangulator;
class Patch;
class Atmosphere;
class LightComponent;


class Planet : public Entity
{
public:
	Planet();
	virtual ~Planet();

	float GetRadius(){ return m_Radius; }
	float GetMaxHeight() { return m_MaxHeight; }
	int32 GetVertexCount();
	Triangulator* GetTriangulator() { return m_pTriangulator; }

	TextureData const* GetHeightMap() const { return m_pHeight.get(); }
	TextureData const* GetDiffuseMap() const { return m_pDiffuse.get(); }
	TextureData const* GetDetail1Map() const { return m_pDetail1.get(); }
	TextureData const* GetDetail2Map() const { return m_pDetail2.get(); }
	TextureData const* GetHeightDetailMap() const { return m_pHeightDetail.get(); }

	void SetAtmosphere(Atmosphere* pAtmosphere, float radius) 
	{ 
		m_pAtmopshere = pAtmosphere; 
		m_AtmRadius = radius;
	}
	float GetAtmosphereHeight() { return m_pAtmopshere ? m_AtmRadius : 0; }

	void SetSunlight(LightComponent* pLight);;

protected:
	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void DrawForward();

	virtual void LoadPlanet() = 0;

protected:

	//Planet parameters
	float m_Radius = 1737.1f;
	float m_MaxHeight = 10.7f;

	AssetPtr<TextureData> m_pDiffuse;
	AssetPtr<TextureData> m_pDetail1;
	AssetPtr<TextureData> m_pDetail2;
	AssetPtr<TextureData> m_pHeightDetail;
	AssetPtr<TextureData> m_pHeight;

private:
	bool m_Rotate = false;

	Atmosphere* m_pAtmopshere = nullptr;
	float m_AtmRadius;

	//Calculations
	Triangulator* m_pTriangulator = nullptr;
	Patch* m_pPatch = nullptr;
};
