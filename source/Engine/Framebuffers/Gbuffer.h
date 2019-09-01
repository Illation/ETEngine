#pragma once
#include <Engine/Graphics/FrameBuffer.h>


class Gbuffer : public FrameBuffer
{
public:
	Gbuffer(bool demo = false);
	virtual ~Gbuffer() = default;
	
private:
	void AccessShaderAttributes();
	void UploadDerivedVariables();
};

