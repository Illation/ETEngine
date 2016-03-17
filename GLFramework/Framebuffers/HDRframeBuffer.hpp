#pragma once
#include "../Graphics/FrameBuffer.hpp"

class HDRframeBuffer : public FrameBuffer
{
public:
	HDRframeBuffer();
	~HDRframeBuffer();

	void SetExposure(float exp) { m_Exposure = exp; }

private:
	void AccessShaderAttributes();
	void UploadDerivedVariables();

	GLuint m_uExposure;
	float m_Exposure = 1.f;
};
