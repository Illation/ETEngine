#include "stdafx.h"
#include "TextureParameters.h"


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
		.property("wrap	T",				&TextureParameters::wrapT)
		.property("wrap R",				&TextureParameters::wrapR)
		.property("border color",		&TextureParameters::borderColor)
		.property("generate mipmaps",	&TextureParameters::genMipMaps);
}


//==============================
// Texture Parameters
//==============================


//---------------------------------
// TextureParameters::c-tor
//
// default constructor with options about using mip maps or a depth texture
//
TextureParameters::TextureParameters(bool useMipMaps, bool depthTex) 
	: genMipMaps(useMipMaps)
	, isDepthTex(depthTex)
{ }


//---------------------------------
// GetFilter
//
// Convert E_TextureFilterMode to OpenGL enum
//
int32 GetFilter(E_TextureFilterMode const filter) 
{
	switch (filter)
	{
	case E_TextureFilterMode::Nearest:
		return GL_NEAREST;
	case E_TextureFilterMode::Linear:
		return GL_LINEAR;
	default:
		ET_ASSERT(true, "Unhandled texture filter mode!");
		return 0;
	}
}

//---------------------------------
// GetMinFilter
//
// Convert E_TextureFilterMode to OpenGL enum for min filters
//
int32 GetMinFilter(E_TextureFilterMode const minFilter, E_TextureFilterMode const mipFilter, bool const useMip) 
{
	if (useMip)
	{
		switch (minFilter)
		{
		case E_TextureFilterMode::Nearest:
			switch (mipFilter)
			{
			case E_TextureFilterMode::Nearest:
				return GL_NEAREST_MIPMAP_NEAREST;
			case E_TextureFilterMode::Linear:
				return GL_NEAREST_MIPMAP_LINEAR;
			default:
				ET_ASSERT(true, "Unhandled texture filter mode!");
				break;
			}
			break;

		case E_TextureFilterMode::Linear:
			switch (mipFilter)
			{
			case E_TextureFilterMode::Nearest:
				return GL_LINEAR_MIPMAP_NEAREST;
			case E_TextureFilterMode::Linear:
				return GL_LINEAR_MIPMAP_LINEAR;
			default:
				ET_ASSERT(true, "Unhandled texture filter mode!");
				break;
			}
			break;

		default:
			ET_ASSERT(true, "Unhandled texture filter mode!");
			break;
		}
		return 0;
	}

	return GetFilter(minFilter);
}

//---------------------------------
// GetWrapMode
//
// Convert E_TextureWrapMode to OpenGL enum
//
int32 GetWrapMode(E_TextureWrapMode const wrap) 
{
	switch (wrap)
	{
	case E_TextureWrapMode::ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	case E_TextureWrapMode::ClampToBorder:
		return GL_CLAMP_TO_BORDER;
	case E_TextureWrapMode::MirrorClampToEdge:
		return  GL_MIRROR_CLAMP_TO_EDGE;
	case E_TextureWrapMode::MirroredRepeat:
		return  GL_MIRRORED_REPEAT;
	case E_TextureWrapMode::Repeat:
		return  GL_REPEAT;
	default:
		ET_ASSERT(true, "Unhandled texture wrap mode!");
		return 0;
	}
}

//---------------------------------
// GetCompareMode
//
// Convert E_TextureCompareMode to OpenGL enum
//
int32 GetCompareMode(E_TextureCompareMode const comp) 
{
	switch (comp)
	{
	case E_TextureCompareMode::CompareRToTexture:
		return GL_COMPARE_REF_TO_TEXTURE;
	case E_TextureCompareMode::None:
		return GL_NONE;
	default:
		ET_ASSERT(true, "Unhandled texture compare mode!");
		return 0;
	}
}
