#pragma once

#include <Engine/SceneGraph/Entity.h>


class ShaderData;
class Frustum;
class TextureData;
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

	TextureData* GetHeightMap() { return m_pHeight; }
	TextureData* GetDiffuseMap() { return m_pDiffuse; }
	TextureData* GetDetail1Map() { return m_pDetail1; }
	TextureData* GetDetail2Map() { return m_pDetail2; }
	TextureData* GetHeightDetailMap() { return m_pHeightDetail; }

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

	TextureData* m_pDiffuse = nullptr;
	TextureData* m_pDetail1 = nullptr;
	TextureData* m_pDetail2 = nullptr;
	TextureData* m_pHeightDetail = nullptr;
	TextureData* m_pHeight = nullptr;

private:
	bool m_Rotate = false;

	Atmosphere* m_pAtmopshere = nullptr;
	float m_AtmRadius;

	//Calculations
	Triangulator* m_pTriangulator = nullptr;
	Patch* m_pPatch = nullptr;
};
