#pragma once
#include "../Graphics/FrameBuffer.hpp"

class ShaderData;
class TextureData;

static const int NUM_BLOOM_DOWNSAMPLES = 5;

class PostProcessingRenderer 
{
public:
	PostProcessingRenderer();
	~PostProcessingRenderer();

	void Initialize();
	void EnableInput();
	void Draw(GLuint FBO);

	void SetNumSamples(int numSamples) { m_NumSamples = numSamples; }
	GLuint GetTargetFBO() { return m_CollectFBO; }

	void SetExposure(float exp) { m_Exposure = exp; }
	void SetGamma(float gamma) { m_Gamma = gamma; }

	void SetBloomThreshold(float value) { m_Threshold = value; }
	void SetBloomMultiplier(float value) { m_BloomMult = value; }

private:

	int m_NumSamples = 1;

	ShaderData* m_pDownsampleShader;
	ShaderData* m_pGaussianShader;
	ShaderData* m_pPostProcShader;

	GLuint m_VAO;
	GLuint m_VBO;

	GLuint m_CollectFBO;
	GLuint m_CollectTex;
	GLuint m_CollectRBO;

	GLuint m_HDRoutFBO;
	GLuint m_ColorBuffers[2];

	GLuint m_PingPongFBO[2];
	GLuint m_PingPongTexture[2];

	GLuint m_DownSampleFBO[NUM_BLOOM_DOWNSAMPLES];
	GLuint m_DownSampleTexture[NUM_BLOOM_DOWNSAMPLES];
	GLuint m_DownPingPongFBO[NUM_BLOOM_DOWNSAMPLES];
	GLuint m_DownPingPongTexture[NUM_BLOOM_DOWNSAMPLES];

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
