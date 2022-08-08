#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsTypes/Camera.h>


namespace et {
namespace render {


class StarField final
{
public:
	StarField(core::HashString const assetId);
	virtual ~StarField();

	void Draw(Camera const& cam) const;

	void SetRadius(float radius) { m_Radius = radius; }
	void SetMaxStars(uint32 maxStars) { m_MaxStars = maxStars; }
	void SetDrawnStars(uint32 drawnStars) { m_DrawnStars = drawnStars; }
	void SetBaseFlux(float mult) { m_BaseFlux = mult; }
	void SetBaseMag(float mag) { m_BaseMag = mag; }

private:
	std::vector<vec4> m_Stars;

	AssetPtr<rhi::ShaderData> m_pShader;
	AssetPtr<rhi::TextureData> m_pSprite;

	rhi::T_ArrayLoc m_VAO = 0;
	rhi::T_BufferLoc m_VBO = 0;

	uint32 m_MaxStars = 0;
	uint32 m_DrawnStars = 32000;

	float m_Radius = 0.001f;

	float m_BaseFlux = 0.000001f;
	float m_BaseMag = 11.f;
};


} // namespace render
} // namespace et
