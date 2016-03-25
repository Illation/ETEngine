#pragma once
#include "../Graphics/FrameBuffer.hpp"

class Gbuffer : public FrameBuffer
{
public:
	Gbuffer(bool demo = false);
	~Gbuffer();
	
private:
	void AccessShaderAttributes();
	void UploadDerivedVariables();

	//CameraPos
	GLint m_uCamPos;
};

