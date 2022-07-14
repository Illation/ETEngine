#pragma once


namespace et {
namespace render {


typedef int32 T_AttribLoc;
typedef int32 T_UniformLoc;
typedef uint32 T_ShaderLoc;
typedef int32 T_BlockIndex;

typedef uint32 T_TextureUnit;
typedef uint32 T_TextureLoc;
typedef uint64 T_TextureHandle;

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
	CubeMap,

	COUNT
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
	CF_Color = 1 << 0,
	CF_Depth = 1 << 1,
	CF_Stencil = 1 << 2
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
	Dynamic,
	Stream
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
	FrontBack,

	COUNT
};

//---------------------------------
// E_PolygonMode
//
// How polygons should be filled
//
enum class E_PolygonMode : uint8
{
	Point,
	Line,
	Fill
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
	// base formats / layouts
	Depth,
	DepthStencil,

	Red,
	RG,
	RGB,
	RGBA,

	// swizzled layouts
	BGR,
	BGRA,

	// only for storage formats
	Depth24,

	R8,
	RG8,
	RG16f,
	RGB8,
	RGB16f,
	RGBA8,
	RGBA16f,
	RGBA32f,
	SRGB8,
	SRGBA8,

	// compressed
	BC1_RGB, // aka DXT 1
	BC1_RGBA, // aka DXT 1 + Alpha Mask
	BC1_SRGB, // aka DXT 1 sRGB
	BC1_SRGBA, // aka DXT 1 sRGB + Alpha Mask

	BC3_RGBA, // aka DXT 5
	BC3_SRGBA, // aka DXT 5 sRGB

	BC4_Red, // height maps / masks
	BC4_Red_Signed, // height maps / masks

	BC5_RG, // normal maps
	BC5_RG_Signed, // normal maps

	BC6H_RGB, // HDR 
	BC6H_RGB_Signed, // HDR 

	BC7_RGBA, // only when targeting modern GPUs
	BC7_SRGBA, // ^ but sRGB

	//-----------
	Invalid
};

//---------------------------------
// E_ClearFlags
//
// Which buffers should be reset
//
typedef uint8 T_ColorFlags;
enum E_ColorFlag : T_ColorFlags
{
	CF_None = 0,

	CF_Red		= 1 << 0,
	CF_Green	= 1 << 1,
	CF_Blue	= 1 << 2,
	CF_Alpha	= 1 << 3,

	CF_All = 0xFF
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
	Always,

	Invalid
};

typedef E_DepthFunc T_StencilFunc;

//---------------------------------
// E_StencilOp
//
// What to do depending on the stencil test
//
enum class E_StencilOp : uint8
{
	Keep,
	Zero,
	Replace,
	Increment,
	IncrementWrap,
	Decrement,
	DecrementWrap,
	Invert,

	Invalid
};

//---------------------------------
// E_ParamType
//
// Parameter types for shaders or materials
//
enum class E_ParamType : uint8
{
	Texture2D,
	Texture3D,
	TextureCube,
	TextureShadow,
	Matrix4x4,
	Matrix3x3,
	Vector4,
	Vector3,
	Vector2,
	UInt,
	Int,
	Float,
	Boolean,

	Invalid
};

//---------------------------------
// UniformDescriptor
//
// access information for a uniform parameter within a parameterBlock
//
struct UniformDescriptor
{
	T_UniformLoc location;
	E_ParamType type;
	std::string name;
};


} // namespace render
} // namespace et
