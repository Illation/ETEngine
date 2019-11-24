#pragma once

typedef int32 T_AttribLoc;
typedef int32 T_UniformLoc;
typedef uint32 T_ShaderLoc;

typedef uint32 T_TextureLoc;

typedef uint32 T_BufferLoc;
typedef uint32 T_ArrayLoc;

typedef uint32 T_FbLoc;
typedef uint32 T_RbLoc;


//---------------------------------
// E_DataType
//
// Per attribute data type
//
enum class E_DataType
{
	Byte,
	UByte,
	Short,
	UShort,
	Int,
	UInt,

	Half,
	Float,
	Double,

	Invalid
};

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
// E_TextureType
//
// Denotes the type a texture can be
//
enum class E_TextureType : uint8
{
	Texture2D,
	Texture3D,
	CubeMap
};

//---------------------------------
// E_ShaderType
//
// Shaders for each type in the programmable render pipeline
//
enum class E_ShaderType : uint8
{
	Compute,
	Vertex,
	TesselationControl,
	TesselationEvaluation,
	Geometry,
	Fragment
};

//---------------------------------
// E_DrawMode
//
// How vertex data should be drawn
//
enum class E_DrawMode : uint8
{
	Points,
	LineStrip,
	LineLoop,
	Lines,
	TriangleStrip,
	TriangleFan,
	Triangles,
	Patches
};

//---------------------------------
// E_ClearFlags
//
// Which buffers should be reset
//
typedef uint8 T_ClearFlags;
enum E_ClearFlag : T_ClearFlags
{
	Color = 1 << 0,
	Depth = 1 << 1,
	Stencil = 1 << 2
};

//---------------------------------
// E_BufferType
//
// Specifies the purpose / target of buffer data and operations
//
enum class E_BufferType : uint8
{
	Vertex,
	Index,
	Uniform
};

//---------------------------------
// E_UsageHint
//
// Whether or not a buffer will change often
//
enum class E_UsageHint : uint8
{
	Static,
	Dynamic
};

//---------------------------------
// E_AccessMode
//
enum class E_AccessMode : uint8
{
	Read,
	Write,
	ReadWrite
};

//---------------------------------
// E_RenderBufferFormat
//
enum class E_RenderBufferFormat : uint8
{
	Depth24,
	Depth24_Stencil8
};

//---------------------------------
// E_FaceCullMode
//
enum class E_FaceCullMode : uint8
{
	Front,
	Back,
	FrontBack
};

//---------------------------------
// E_BlendEquation
//
enum class E_BlendEquation : uint8
{
	Add,
	Subtract,
	RevSub,
	Min,
	Max
};

//---------------------------------
// E_BlendEquation
//
enum class E_BlendFactor : uint8
{
	Zero,
	One,
	SourceCol,
	OneMinusSourceCol,
	DestCol,
	OneMinusDestCol,
	SourceAlpha,
	OneMinusSourceAlpha,
	DestAlpha,
	OneMinusDestAlpha,
	ConstCol,
	OneMinusConstCol,
	ConstAlpha,
	OneMinusConstAlpha
};

//---------------------------------
// E_BlendEquation
//
// this list is expected to grow as more color formats are needed
//
enum class E_ColorFormat : uint8
{
	Depth,
	DepthStencil,

	Red,
	RG,
	RGB,
	RGBA,

	// only for internal formats
	Depth24,

	R8,
	RG16f,
	RGB16f,
	RGBA8,
	RGBA16f,
	RGBA32f,
	SRGB
};

//---------------------------------
// E_DepthFunc
//
// How to compare depth values
//
enum class E_DepthFunc : uint8
{
	Never, 
	Less,
	Equal,
	LEqual,
	Greater,
	NotEqual,
	GEqual,
	Always
};
