#pragma once
#include <EtRendering/GraphicsContext/GraphicsTypes.h>


namespace et {
namespace render {


//---------------------------------
// TextureParameters
//
// Variables controlling how a the GPU shows a texture
//
struct TextureParameters final
{
	TextureParameters(bool useMipMaps = true, bool depthTex = false);

	// Data
	///////

	// sample filtering
	E_TextureFilterMode minFilter = E_TextureFilterMode::Nearest;
	E_TextureFilterMode magFilter = E_TextureFilterMode::Nearest;
	E_TextureFilterMode mipFilter = E_TextureFilterMode::Linear;

	// pixel address mode
	E_TextureWrapMode wrapS = E_TextureWrapMode::Repeat;
	E_TextureWrapMode wrapT = E_TextureWrapMode::Repeat;
	E_TextureWrapMode wrapR = E_TextureWrapMode::Repeat;

	// default color for out fo range pixels
	vec4 borderColor;

	// downsampled pixels
	bool genMipMaps = true;

	// depth textures (e.g for shadow mapping)
	bool isDepthTex = false;
	E_TextureCompareMode compareMode = E_TextureCompareMode::None;

	RTTR_ENABLE()
};


} // namespace render
} // namespace et
