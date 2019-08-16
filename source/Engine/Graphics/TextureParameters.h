#pragma once


//---------------------------------
// E_TextureFilterMode
//
// How to sample a (sub) pixel in the texture
//
enum class E_TextureFilterMode
{
	Nearest,
	Linear
	// Anisotropic
};

//---------------------------------
// E_TextureWrapMode
//
// How to sample a pixel outside of normalized texture coordinates
//
enum class E_TextureWrapMode
{
	ClampToEdge,
	ClampToBorder,
	MirrorClampToEdge,
	MirroredRepeat,
	Repeat
};

//---------------------------------
// E_TextureCompareMode
//
// How to compare a texture to the depth texture
//
enum class E_TextureCompareMode
{
	CompareRToTexture,
	None
};


//---------------------------------
// TextureParameters
//
// Variables controlling how a the GPU shows a texture
//
struct TextureParameters final
{
	TextureParameters(bool useMipMaps = false, bool depthTex = false);

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
	bool genMipMaps = false;

	// depth textures (e.g for shadow mapping)
	bool isDepthTex = false;
	E_TextureCompareMode compareMode = E_TextureCompareMode::None;

	RTTR_ENABLE()
};

int32 GetFilter(E_TextureFilterMode const filter);
int32 GetMinFilter(E_TextureFilterMode const minFilter, E_TextureFilterMode const mipFilter, bool const useMip);
int32 GetWrapMode(E_TextureWrapMode const wrap);
int32 GetCompareMode(E_TextureCompareMode const comp);