#pragma once
#include <EtRendering/GraphicsTypes/FrameBuffer.h>

namespace et {
namespace render {


class Gbuffer : public FrameBuffer
{
public:
	Gbuffer(bool demo = false);
	virtual ~Gbuffer() = default;
	
private:
	void AccessShaderAttributes();
	void UploadDerivedVariables();
};


} // namespace render
} // namespace et
