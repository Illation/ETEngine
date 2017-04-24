#pragma once
#include "../SceneGraph/Entity.hpp"

class ShaderData;
class Frustum;
class TextureData;
class Triangulator;
class Patch;

class Planet : public Entity
{
public:
	Planet();
	virtual ~Planet();

	float GetRadius(){ return m_Radius; }
	float GetMaxHeight() { return m_MaxHeight; }
	int GetVertexCount();
	Triangulator* GetTriangulator() { return m_pTriangulator; }

	TextureData* GetHeightMap() { return m_pHeight; }
	TextureData* GetDiffuseMap() { return m_pDiffuse; }
	TextureData* GetDetail1Map() { return m_pDetail1; }
	TextureData* GetDetail2Map() { return m_pDetail2; }
	TextureData* GetHeightDetailMap() { return m_pHeightDetail; }

protected:
	virtual void Initialize();
	virtual void Update();
	virtual void Draw();

	virtual void LoadPlanet() = 0;
	
	//dunno if I should keep this
	void DrawWire();

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

	//Calculations
	Triangulator* m_pTriangulator = nullptr;
	Patch* m_pPatch = nullptr;
};
