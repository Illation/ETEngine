#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsTypes/FrameBuffer.h>
#include <EtRendering/GraphicsTypes/PostProcessingSettings.h>


namespace et {
namespace render {


class rhi::ShaderData;
class rhi::TextureData;
class I_OverlayRenderer;


static const int32 NUM_BLOOM_DOWNSAMPLES = 5;

class PostProcessingRenderer 
{
public:
	PostProcessingRenderer();
	~PostProcessingRenderer();

	void Initialize();

	void EnableInput();
	void Draw(rhi::T_FbLoc const FBO, PostProcessingSettings const& settings, std::function<void(rhi::T_FbLoc const)>& onDrawOverlaysFn);

	rhi::T_FbLoc GetTargetFBO() { return m_CollectFBO; }
	rhi::TextureData* GetTargetTexture() { return m_CollectTex; }

private:

	void DeleteFramebuffers();
	void GenerateFramebuffers();
	void ResizeFBTextures();

	AssetPtr<rhi::ShaderData> m_pDownsampleShader;
	AssetPtr<rhi::ShaderData> m_pGaussianShader;
	AssetPtr<rhi::ShaderData> m_pPostProcShader;
	AssetPtr<rhi::ShaderData> m_pFXAAShader;

	rhi::T_FbLoc m_CollectFBO;
	rhi::TextureData* m_CollectTex = nullptr;
	rhi::T_RbLoc m_CollectRBO;

	rhi::T_FbLoc m_HDRoutFBO;
	rhi::TextureData* m_ColorBuffers[2];

	rhi::T_FbLoc m_PingPongFBO[2];
	rhi::TextureData* m_PingPongTexture[2];

	rhi::T_FbLoc m_DownSampleFBO[NUM_BLOOM_DOWNSAMPLES];
	rhi::TextureData* m_DownSampleTexture[NUM_BLOOM_DOWNSAMPLES];
	rhi::T_FbLoc m_DownPingPongFBO[NUM_BLOOM_DOWNSAMPLES];
	rhi::TextureData* m_DownPingPongTexture[NUM_BLOOM_DOWNSAMPLES];
};


} // namespace render
} // namespace et
