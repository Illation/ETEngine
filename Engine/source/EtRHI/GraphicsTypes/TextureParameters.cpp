#include "stdafx.h"
#include "TextureParameters.h"


namespace et {
namespace rhi {


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

    registration::enumeration<E_TextureFilterMode>("E_TextureFilterMode") (
		value("Nearest", E_TextureFilterMode::Nearest),
		value("Linear", E_TextureFilterMode::Linear) );

    registration::enumeration<E_TextureWrapMode>("E_TextureWrapMode") (
		value("ClampToEdge", E_TextureWrapMode::ClampToEdge),
		value("ClampToBorder", E_TextureWrapMode::ClampToBorder),
		value("MirrorClampToEdge", E_TextureWrapMode::MirrorClampToEdge),
		value("MirroredRepeat", E_TextureWrapMode::MirroredRepeat),
		value("Repeat", E_TextureWrapMode::Repeat) );
	
    registration::enumeration<E_TextureCompareMode>("E_TextureCompareMode") (
		value("CompareRToTexture", E_TextureCompareMode::CompareRToTexture),
		value("None", E_TextureCompareMode::None) );

	registration::class_<TextureParameters>("texture parameters")
		.property("min filter",			&TextureParameters::minFilter)
		.property("mag filter",			&TextureParameters::magFilter)
		.property("mipmap filter",		&TextureParameters::mipFilter)
		.property("wrap S",				&TextureParameters::wrapS)
		.property("wrap T",				&TextureParameters::wrapT)
		.property("border color",		&TextureParameters::borderColor)
		.property("generate mipmaps",	&TextureParameters::genMipMaps);
}


//====================
// Texture Parameters
//====================


//---------------------------------
// TextureParameters::c-tor
//
// default constructor with options about using mip maps or a depth texture
//
TextureParameters::TextureParameters(bool useMipMaps, bool depthTex) 
	: genMipMaps(useMipMaps)
	, isDepthTex(depthTex)
{ }


} // namespace rhi
} // namespace et
