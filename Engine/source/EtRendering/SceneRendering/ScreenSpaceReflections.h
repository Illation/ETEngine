#pragma once
#include <EtCore/Content/AssetPointer.h>


// fwd
namespace et { namespace rhi {
	class ShaderData;
} namespace render {
	class Gbuffer;
} }


namespace et {
namespace render {


class ScreenSpaceReflections
{
public:
	ScreenSpaceReflections() = default;
	virtual ~ScreenSpaceReflections();
	void Initialize();

	void EnableInput();
	rhi::T_FbLoc GetTargetFBO() { return m_CollectFBO; }

	void Draw(Gbuffer const& gbuffer);
private:

	AssetPtr<rhi::ShaderData> m_Shader;

	rhi::T_FbLoc m_CollectFBO;
	rhi::TextureData* m_CollectTex = nullptr;
	rhi::T_RbLoc m_CollectRBO;
};


} // namespace render
} // namespace et
