#pragma once
#include <EtCore/Content/AssetPointer.h>


namespace et {
namespace render {


class rhi::ShaderData;


class ScreenSpaceReflections
{
public:
	ScreenSpaceReflections() = default;
	virtual ~ScreenSpaceReflections();
	void Initialize();

	void EnableInput();
	rhi::T_FbLoc GetTargetFBO() { return m_CollectFBO; }

	void Draw();
private:

	AssetPtr<rhi::ShaderData> m_pShader;

	rhi::T_FbLoc m_CollectFBO;
	rhi::TextureData* m_CollectTex = nullptr;
	rhi::T_RbLoc m_CollectRBO;
};


} // namespace render
} // namespace et
