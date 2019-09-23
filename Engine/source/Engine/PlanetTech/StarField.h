#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/SceneGraph/Entity.h>


class StarField : public Entity
{
public:
	StarField(T_Hash const assetId);
	virtual ~StarField();

	void SetRadius(float radius) { m_Radius = radius; }
	void SetMaxStars(uint32 maxStars) { m_MaxStars = maxStars; }
	void SetDrawnStars(uint32 drawnStars) { m_DrawnStars = drawnStars; }
	void SetBaseFlux(float mult) { m_BaseFlux = mult; }
	void SetBaseMag(float mag) { m_BaseMag = mag; }

protected:
	virtual void Initialize();
	virtual void Update() {}
	virtual void Draw() {}
	virtual void DrawForward();

private:
	std::vector<vec4> m_Stars;
	T_Hash m_AssetId;

	AssetPtr<ShaderData> m_pShader;
	AssetPtr<TextureData> m_pSprite;

	T_ArrayLoc m_VAO = 0;
	T_BufferLoc m_VBO = 0;

	uint32 m_MaxStars = 0;
	uint32 m_DrawnStars = 32000;

	float m_Radius = 0.001f;

	float m_BaseFlux = 0.000001f;
	float m_BaseMag = 11.f;
};
