#pragma once
#include "../Graphics/FrameBuffer.hpp"

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
	void Draw(GLuint FBO);

	GLuint GetTargetFBO() { return m_CollectFBO; }

	void SetExposure(float exp) { m_Exposure = exp; }
	float GetExposure() {return m_Exposure; }
	void SetGamma(float gamma) { m_Gamma = gamma; }

	void SetBloomThreshold(float value) { m_Threshold = value; }
	void SetBloomMultiplier(float value) { m_BloomMult = value; }

private:

	void DeleteFramebuffers();
	void GenerateFramebuffers();
	void ResizeFBTextures();

	ShaderData* m_pDownsampleShader;
	ShaderData* m_pGaussianShader;
	ShaderData* m_pPostProcShader;

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
	float m_Exposure = 1.f;
	GLuint m_uGamma;
	float m_Gamma = 2.2f;

	GLuint m_uBloomMult;
	float m_BloomMult = 0.5f;
	GLint m_uThreshold;
	float m_Threshold = 1.5f;
};
