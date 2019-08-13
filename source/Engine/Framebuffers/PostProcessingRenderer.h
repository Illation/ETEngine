#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/Graphics/FrameBuffer.h>
#include <Engine/Graphics/PostProcessingSettings.h>


class ShaderData;
class TextureData;

static const int32 NUM_BLOOM_DOWNSAMPLES = 5;

class PostProcessingRenderer 
{
public:
	PostProcessingRenderer();
	~PostProcessingRenderer();

	void Initialize();

	void EnableInput();
	void Draw(GLuint FBO, const PostProcessingSettings &settings);

	GLuint GetTargetFBO() { return m_CollectFBO; }
	TextureData* GetTargetTexture() { return m_CollectTex; }

private:

	void DeleteFramebuffers();
	void GenerateFramebuffers();
	void ResizeFBTextures();

	AssetPtr<ShaderData> m_pDownsampleShader;
	AssetPtr<ShaderData> m_pGaussianShader;
	AssetPtr<ShaderData> m_pPostProcShader;
	AssetPtr<ShaderData> m_pFXAAShader;

	GLuint m_CollectFBO;
	TextureData* m_CollectTex = nullptr;
	GLuint m_CollectRBO;

	GLuint m_HDRoutFBO;
	TextureData* m_ColorBuffers[2];

	GLuint m_PingPongFBO[2];
	TextureData* m_PingPongTexture[2];

	GLuint m_DownSampleFBO[NUM_BLOOM_DOWNSAMPLES];
	TextureData* m_DownSampleTexture[NUM_BLOOM_DOWNSAMPLES];
	GLuint m_DownPingPongFBO[NUM_BLOOM_DOWNSAMPLES];
	TextureData* m_DownPingPongTexture[NUM_BLOOM_DOWNSAMPLES];

	GLuint m_uHorizontal;

	GLuint m_uExposure;
	GLuint m_uGamma;
	GLuint m_uBloomMult;
	GLint m_uThreshold;

	GLint m_uInverseScreen;
};
