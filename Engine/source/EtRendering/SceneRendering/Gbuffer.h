#pragma once
#include <EtRendering/GraphicsTypes/FrameBuffer.h>


class Gbuffer : public FrameBuffer
{
public:
	Gbuffer(bool demo = false);
	virtual ~Gbuffer() = default;
	
private:
	void AccessShaderAttributes();
	void UploadDerivedVariables();
};

