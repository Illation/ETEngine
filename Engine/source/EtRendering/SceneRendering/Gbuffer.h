#pragma once
#include <EtRendering/GraphicsTypes/FrameBuffer.h>

namespace et {
namespace render {


class Gbuffer : public FrameBuffer
{
public:
	Gbuffer();
	virtual ~Gbuffer() = default;
	
private:
	void UploadDerivedVariables() override;
};


} // namespace render
} // namespace et
