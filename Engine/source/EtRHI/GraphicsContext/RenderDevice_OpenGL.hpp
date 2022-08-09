
#include <EtCore/UpdateCycle/PerformanceInfo.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/GraphicsTypes/TextureFormat.h>

#include <iomanip>


namespace et {

	ET_REGISTER_TRACE_CTX(ET_CTX_OPENGL);

namespace rhi {


//======================
// Conversion functions
//======================


namespace GL_DEVICE_NS {


//---------------------------------
// ConvTextureType
//
GLenum ConvTextureType(E_TextureType const type)
{
	switch (type)
	{
	case E_TextureType::Texture2D:	return GL_TEXTURE_2D;
	case E_TextureType::Texture3D:	return GL_TEXTURE_3D;
	case E_TextureType::CubeMap:	return GL_TEXTURE_CUBE_MAP;
	}

	ET_ERROR("Unhandled texture type!");
	return GL_NONE;
}

//---------------------------------
// ConvDataType
//
GLenum ConvDataType(E_DataType const type)
{
	switch (type)
	{
	case E_DataType::Byte: return GL_BYTE;
	case E_DataType::UByte: return GL_UNSIGNED_BYTE;
	case E_DataType::Short: return GL_SHORT;
	case E_DataType::UShort: return GL_UNSIGNED_SHORT;
	case E_DataType::Int: return GL_INT;
	case E_DataType::UInt: return GL_UNSIGNED_INT;
	case E_DataType::Half: return GL_HALF_FLOAT;
	case E_DataType::Float: return GL_FLOAT;
	case E_DataType::Double: return GL_DOUBLE;
	}

	ET_ERROR("Unhandled data type!");
	return GL_NONE;
}

//---------------------------------
// ConvShaderType
//
GLenum ConvShaderType(E_ShaderType const type)
{
	switch (type)
	{
	case E_ShaderType::Compute:					return GL_COMPUTE_SHADER;
	case E_ShaderType::Vertex:					return GL_VERTEX_SHADER;
	case E_ShaderType::TesselationControl:		return GL_TESS_CONTROL_SHADER;
	case E_ShaderType::TesselationEvaluation:	return GL_TESS_EVALUATION_SHADER;
	case E_ShaderType::Geometry:				return GL_GEOMETRY_SHADER;
	case E_ShaderType::Fragment:				return GL_FRAGMENT_SHADER;
	}

	ET_ERROR("Unhandled shader type!");
	return GL_NONE;
}

//---------------------------------
// ConvDrawMode
//
GLenum ConvDrawMode(E_DrawMode const mode)
{
	switch (mode)
	{
	case E_DrawMode::Points:		return GL_POINTS;
	case E_DrawMode::LineStrip:		return GL_LINE_STRIP;
	case E_DrawMode::LineLoop:		return GL_LINE_LOOP;
	case E_DrawMode::Lines:			return GL_LINES;
	case E_DrawMode::TriangleStrip:	return GL_TRIANGLE_STRIP;
	case E_DrawMode::TriangleFan:	return GL_TRIANGLE_FAN;
	case E_DrawMode::Triangles:		return GL_TRIANGLES;
	case E_DrawMode::Patches:		return GL_PATCHES;
	}

	ET_ERROR("Unhandled draw mode!");
	return GL_NONE;
}

//---------------------------------
// ConvDrawMode
//
// Not all buffer types are currently listed, they will be added when support is needed
//
GLenum ConvBufferType(E_BufferType const type)
{
	switch (type)
	{
	case E_BufferType::Vertex:	return GL_ARRAY_BUFFER;
	case E_BufferType::Index:	return GL_ELEMENT_ARRAY_BUFFER;
	case E_BufferType::Uniform:	return GL_UNIFORM_BUFFER;
	}

	ET_ERROR("Unhandled buffer type!");
	return GL_NONE;
}

//---------------------------------
// ConvUsageHint
//
GLenum ConvUsageHint(E_UsageHint const hint)
{
	switch (hint)
	{
	case E_UsageHint::Static:	return GL_STATIC_DRAW;
	case E_UsageHint::Dynamic:	return GL_DYNAMIC_DRAW;
	case E_UsageHint::Stream:	return GL_STREAM_DRAW;
	}

	ET_ERROR("Unhandled usage hint!");
	return GL_NONE;
}

//---------------------------------
// ConvAccessMode
//
GLenum ConvAccessMode(E_AccessMode const mode)
{
	switch (mode)
	{
	case E_AccessMode::Read:		return GL_READ_ONLY;
	case E_AccessMode::Write:		return GL_WRITE_ONLY;
	case E_AccessMode::ReadWrite:	return GL_READ_WRITE;
	}

	ET_ERROR("Unhandled access mode!");
	return GL_NONE;
}

//---------------------------------
// ConvFaceCullMode
//
GLenum ConvFaceCullMode(E_FaceCullMode const mode)
{
	switch (mode)
	{
	case E_FaceCullMode::Front:		return GL_FRONT;
	case E_FaceCullMode::Back:		return GL_BACK;
	case E_FaceCullMode::FrontBack:	return GL_FRONT_AND_BACK;
	}

	ET_ERROR("Unhandled cull mode!");
	return GL_NONE;
}

//---------------------------------
// ConvFaceCullMode
//
GLenum ConvPolygonMode(E_PolygonMode const mode)
{
	switch (mode)
	{
	case E_PolygonMode::Point:		return GL_POINT;
	case E_PolygonMode::Line:		return GL_LINE;
	case E_PolygonMode::Fill:		return GL_FILL;
	}

	ET_ERROR("Unhandled polygon mode!");
	return GL_NONE;
}


//---------------------------------
// ConvBlendEquation
//
GLenum ConvBlendEquation(E_BlendEquation const equ)
{
	switch (equ)
	{
	case E_BlendEquation::Add:		return GL_FUNC_ADD;
	case E_BlendEquation::Subtract:	return GL_FUNC_SUBTRACT;
	case E_BlendEquation::RevSub:	return GL_FUNC_REVERSE_SUBTRACT;
	case E_BlendEquation::Min:		return GL_MIN;
	case E_BlendEquation::Max:		return GL_MAX;
	}

	ET_ERROR("Unhandled blend equation!");
	return GL_NONE;
}

//---------------------------------
// ConvBlendFactor
//
GLenum ConvBlendFactor(E_BlendFactor const fac)
{
	switch (fac)
	{
	case E_BlendFactor::Zero:					return GL_ZERO;
	case E_BlendFactor::One:					return GL_ONE;
	case E_BlendFactor::SourceCol:				return GL_SRC_COLOR;
	case E_BlendFactor::OneMinusSourceCol:		return GL_ONE_MINUS_SRC_COLOR;
	case E_BlendFactor::DestCol:				return GL_DST_COLOR;
	case E_BlendFactor::OneMinusDestCol:		return GL_ONE_MINUS_DST_COLOR;
	case E_BlendFactor::SourceAlpha:			return GL_SRC_ALPHA;
	case E_BlendFactor::OneMinusSourceAlpha:	return GL_ONE_MINUS_SRC_ALPHA;
	case E_BlendFactor::DestAlpha:				return GL_DST_ALPHA;
	case E_BlendFactor::OneMinusDestAlpha:		return GL_ONE_MINUS_DST_ALPHA;
	case E_BlendFactor::ConstCol:				return GL_CONSTANT_COLOR;
	case E_BlendFactor::OneMinusConstCol:		return GL_ONE_MINUS_CONSTANT_COLOR;
	case E_BlendFactor::ConstAlpha:				return GL_CONSTANT_ALPHA;
	case E_BlendFactor::OneMinusConstAlpha:		return GL_ONE_MINUS_CONSTANT_ALPHA;
	}

	ET_ERROR("Unhandled blend factor!");
	return GL_NONE;
}

//---------------------------------
// ConvColorFormat
//
// must grow along with E_ColorFormat - reinterpret to GLint for internal format
//
GLenum ConvColorFormat(E_ColorFormat const fmt)
{
	switch (fmt)
	{
	case E_ColorFormat::Depth:				return GL_DEPTH_COMPONENT;
	case E_ColorFormat::DepthStencil:		return GL_DEPTH_STENCIL;

	case E_ColorFormat::Red:				return GL_RED;
	case E_ColorFormat::RG:					return GL_RG;
	case E_ColorFormat::RGB:				return GL_RGB;
	case E_ColorFormat::RGBA:				return GL_RGBA;

	case E_ColorFormat::BGR:				return GL_BGR;
	case E_ColorFormat::BGRA:				return GL_BGRA;

	case E_ColorFormat::Depth24:			return GL_DEPTH_COMPONENT24;

	case E_ColorFormat::R8:					return GL_R8;
	case E_ColorFormat::RG8:				return GL_RG8;
	case E_ColorFormat::RG16f:				return GL_RG16F;
	case E_ColorFormat::RGB8:				return GL_RGB8;
	case E_ColorFormat::RGB16f:				return GL_RGB16F;
	case E_ColorFormat::RGBA8:				return GL_RGBA8;
	case E_ColorFormat::RGBA16f:			return GL_RGBA16F;
	case E_ColorFormat::RGBA32f:			return GL_RGBA32F;
	case E_ColorFormat::SRGB8:				return GL_SRGB8;
	case E_ColorFormat::SRGBA8:				return GL_SRGB8_ALPHA8;

	case E_ColorFormat::BC1_RGB:			return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case E_ColorFormat::BC1_RGBA:			return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	case E_ColorFormat::BC1_SRGB:			return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
	case E_ColorFormat::BC1_SRGBA:			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
	case E_ColorFormat::BC3_RGBA:			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case E_ColorFormat::BC3_SRGBA:			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
	case E_ColorFormat::BC4_Red:			return GL_COMPRESSED_RED_RGTC1;
	case E_ColorFormat::BC4_Red_Signed:		return GL_COMPRESSED_SIGNED_RED_RGTC1;
	case E_ColorFormat::BC5_RG:				return GL_COMPRESSED_RG_RGTC2;
	case E_ColorFormat::BC5_RG_Signed:		return GL_COMPRESSED_SIGNED_RG_RGTC2;
	case E_ColorFormat::BC6H_RGB:			return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
	case E_ColorFormat::BC6H_RGB_Signed:	return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
	case E_ColorFormat::BC7_RGBA:			return GL_COMPRESSED_RGBA_BPTC_UNORM;
	case E_ColorFormat::BC7_SRGBA:			return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
	}

	ET_ERROR("Unhandled color format!");
	return GL_NONE;
}

//---------------------
// DeriveLayoutAndType
//
// get upload enums based on storage format for uploading null data
//
void DeriveLayoutAndType(E_ColorFormat const storageFmt, GLenum& colorLayout, GLenum& dataType)
{
	switch (storageFmt)
	{
	case E_ColorFormat::DepthStencil:
		colorLayout = GL_DEPTH_STENCIL;
		dataType = GL_UNSIGNED_BYTE;
		return;

	case E_ColorFormat::Depth:
	case E_ColorFormat::Depth24:
		colorLayout = GL_DEPTH_COMPONENT;
		dataType = GL_UNSIGNED_BYTE;
		return;

	case E_ColorFormat::Red:
	case E_ColorFormat::R8:	
		colorLayout = GL_RED;
		dataType = GL_UNSIGNED_BYTE;
		return;

	case E_ColorFormat::RG8:
	case E_ColorFormat::RG:	
		colorLayout = GL_RG;
		dataType = GL_UNSIGNED_BYTE;
		return;

	case E_ColorFormat::RG16f:
		colorLayout = GL_RG;
		dataType = GL_HALF_FLOAT;
		return;

	case E_ColorFormat::RGB:
	case E_ColorFormat::RGB8:
	case E_ColorFormat::SRGB8:
		colorLayout = GL_RGB;
		dataType = GL_UNSIGNED_BYTE;
		return;

	case E_ColorFormat::RGB16f:
		colorLayout = GL_RGB;
		dataType = GL_HALF_FLOAT;
		return;

	case E_ColorFormat::RGBA:
	case E_ColorFormat::RGBA8:
	case E_ColorFormat::SRGBA8:
		colorLayout = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		return;

	case E_ColorFormat::RGBA16f:
		colorLayout = GL_RGBA;
		dataType = GL_HALF_FLOAT;
		return;

	case E_ColorFormat::RGBA32f:
		colorLayout = GL_RGBA;
		dataType = GL_FLOAT;
		return;
	}

	ET_ERROR("Unhandled color format!");
}

//---------------------------------
// GetFilter
//
// Convert E_TextureFilterMode to OpenGL enum
//
GLenum ConvFilter(E_TextureFilterMode const filter)
{
	switch (filter)
	{
	case E_TextureFilterMode::Nearest: return GL_NEAREST;
	case E_TextureFilterMode::Linear:  return GL_LINEAR;
	}

	ET_ERROR("Unhandled texture filter mode!");
	return GL_NONE;
}

//---------------------------------
// GetMinFilter
//
// Convert E_TextureFilterMode to OpenGL enum for min filters
//
GLenum ConvMinFilter(E_TextureFilterMode const minFilter, E_TextureFilterMode const mipFilter, bool const useMip)
{
	if (useMip)
	{
		switch (minFilter)
		{
		case E_TextureFilterMode::Nearest:
			switch (mipFilter)
			{
			case E_TextureFilterMode::Nearest: return GL_NEAREST_MIPMAP_NEAREST;
			case E_TextureFilterMode::Linear:  return GL_NEAREST_MIPMAP_LINEAR;

			default:
				ET_ERROR("Unhandled texture filter mode!");
				break;
			}
			break;

		case E_TextureFilterMode::Linear:
			switch (mipFilter)
			{
			case E_TextureFilterMode::Nearest: return GL_LINEAR_MIPMAP_NEAREST;
			case E_TextureFilterMode::Linear: return GL_LINEAR_MIPMAP_LINEAR;

			default:
				ET_ERROR("Unhandled texture filter mode!");
				break;
			}
			break;

		default:
			ET_ERROR("Unhandled texture filter mode!");
			break;
		}
		return 0;
	}

	return ConvFilter(minFilter);
}

//---------------------------------
// ConvWrapMode
//
// Convert E_TextureWrapMode to OpenGL enum
//
GLenum ConvWrapMode(E_TextureWrapMode const wrap)
{
	switch (wrap)
	{
	case E_TextureWrapMode::ClampToEdge:		return GL_CLAMP_TO_EDGE;
	case E_TextureWrapMode::ClampToBorder:		return GL_CLAMP_TO_BORDER;
	case E_TextureWrapMode::MirrorClampToEdge:	return GL_MIRROR_CLAMP_TO_EDGE;
	case E_TextureWrapMode::MirroredRepeat:		return GL_MIRRORED_REPEAT;
	case E_TextureWrapMode::Repeat:				return GL_REPEAT;
	}

	ET_ERROR("Unhandled texture wrap mode!");
	return 0;
}

//---------------------------------
// ConvCompareMode
//
// Convert E_TextureCompareMode to OpenGL enum
//
GLenum ConvCompareMode(E_TextureCompareMode const comp) 
{
	switch (comp)
	{
	case E_TextureCompareMode::CompareRToTexture:	return GL_COMPARE_REF_TO_TEXTURE;
	case E_TextureCompareMode::None:				return GL_NONE;
	}

	ET_ERROR("Unhandled texture compare mode!");
	return 0;
}

//---------------------------------
// ConvDepthStencilFunction
//
// Also valid for T_StencilFunc
//
GLenum ConvDepthStencilFunction(E_DepthFunc const func)
{
	switch (func)
	{
	case E_DepthFunc::Never:	return GL_NEVER;
	case E_DepthFunc::Less:		return GL_LESS;
	case E_DepthFunc::Equal:	return GL_EQUAL;
	case E_DepthFunc::LEqual:	return GL_LEQUAL;
	case E_DepthFunc::Greater:	return GL_GREATER;
	case E_DepthFunc::NotEqual:	return GL_NOTEQUAL;
	case E_DepthFunc::GEqual:	return GL_GEQUAL;
	case E_DepthFunc::Always:	return GL_ALWAYS;
	}

	ET_ERROR("Unhandled depth/stencil function!");
	return 0;
}

//---------------------------------
// ConvDepthStencilFunction
//
GLenum ConvStencilOp(E_StencilOp const op)
{
	switch (op)
	{
	case E_StencilOp::Keep:				return GL_KEEP;
	case E_StencilOp::Zero:				return GL_ZERO;
	case E_StencilOp::Replace:			return GL_REPLACE;
	case E_StencilOp::Increment:		return GL_INCR;
	case E_StencilOp::IncrementWrap:	return GL_INCR_WRAP;
	case E_StencilOp::Decrement:		return GL_DECR;
	case E_StencilOp::DecrementWrap:	return GL_DECR_WRAP;
	case E_StencilOp::Invert:			return GL_INVERT;
	}

	ET_ERROR("Unhandled depth function!");
	return 0;
}

//---------------------------------
// ParseParamType
//
E_ParamType ParseParamType(GLenum const param) 
{
	switch (param)
	{
	case GL_SAMPLER_2D:			return E_ParamType::Texture2D;
	case GL_SAMPLER_3D:			return E_ParamType::Texture3D;
	case GL_SAMPLER_CUBE:		return E_ParamType::TextureCube;
	case GL_SAMPLER_2D_SHADOW:	return E_ParamType::TextureShadow;
	case GL_FLOAT_MAT4:			return E_ParamType::Matrix4x4;
	case GL_FLOAT_MAT3:			return E_ParamType::Matrix3x3;
	case GL_FLOAT_VEC4:			return E_ParamType::Vector4;
	case GL_FLOAT_VEC3:			return E_ParamType::Vector3;
	case GL_FLOAT_VEC2:			return E_ParamType::Vector2;
	case GL_UNSIGNED_INT:		return E_ParamType::UInt;
	case GL_INT:				return E_ParamType::Int;
	case GL_FLOAT:				return E_ParamType::Float;
	case GL_BOOL:				return E_ParamType::Boolean;
	}

	ET_ERROR("Unhandled GLenum parameter type!");
	return E_ParamType::Invalid;
}


} // namespace GL_DEVICE_NS


//====================
// Texture Unit Cache
//====================


//------------------------------------
// GlContext::TextureUnitCache::c-tor
//
GL_DEVICE_CLASSNAME::TextureUnitCache::TextureUnitCache(size_t const size) 
	: m_MaxUnits(size) 
{
	for (T_TextureUnit unit = 0u; unit < m_MaxUnits; ++unit)
	{
		m_List.emplace_front(unit);
	}
}

//-----------------------------------
// GlContext::TextureUnitCache::Bind
//
// Ensure a texture is bound, and return the unit it is bound to.
// If the texture isn't bound, assign it to the least recently used unit
// If the texture is bound and ensureActive is false, it is not guaranteed that the active unit is set to the current texture
//
T_TextureUnit GL_DEVICE_CLASSNAME::TextureUnitCache::Bind(E_TextureType const type, T_TextureLoc const tex, bool const ensureActive)
{
	// we don't want to be using this function to unbind textures
	ET_ASSERT(tex != 0u);

	auto mapIt = m_Map.find(tex); // check if the texture is currently bound

	if (mapIt != m_Map.cend()) // the texture is already bound
	{
		// this is now the most recently used unit
		m_List.push_front(*(mapIt->second));
		m_List.erase(mapIt->second);
		mapIt->second = m_List.begin();

		Unit const& binding = *(mapIt->second);

		// for texture modification purposes, this is now ensured to be the active unit
		if (ensureActive)
		{
			EnsureActive(binding.unit);
		}

		return binding.unit;
	}
	else // the texture isn't bound
	{
		// find the least recently used unit
		Unit currentBinding = m_List.back();

		// remove mapping of LRU unit, unless no texture is bound to it yet
		if (currentBinding.texture != 0u)
		{
			auto oldMapIt = m_Map.find(currentBinding.texture);
			ET_ASSERT(oldMapIt != m_Map.cend());
			m_Map.erase(oldMapIt);
		}

		ET_ASSERT(currentBinding.texture != tex);
		currentBinding.texture = tex;

		// this unit is now most recently used
		m_List.push_front(currentBinding);
		m_List.pop_back();
		m_Map.emplace(tex, m_List.begin());

		// we need to ensure that this is the active unit so we can bind a new texture to it
		EnsureActive(currentBinding.unit);

		// do the binding
		glBindTexture(GL_DEVICE_NS::ConvTextureType(type), tex);

		return currentBinding.unit;
	}
}

//----------------------------------------------
// GlContext::TextureUnitCache::Unbind
//
void GL_DEVICE_CLASSNAME::TextureUnitCache::Unbind(E_TextureType const type, T_TextureLoc const tex)
{
	auto mapIt = m_Map.find(tex);

	// the texture is currently bound
	if (mapIt != m_Map.cend())
	{
		Unit const& binding = *(mapIt->second);

		// ensure the current active texture is the one we unbind
		EnsureActive(binding.unit);

		// unbind that texture
		glBindTexture(GL_DEVICE_NS::ConvTextureType(type), 0u);

		// LRU unit is unit with texture, texture is unset
		m_List.emplace_back(mapIt->second->unit, 0u);
		m_List.erase(mapIt->second);

		// unreference texture from map
		m_Map.erase(mapIt);
	}
}

//----------------------------------------------
// GlContext::TextureUnitCache::EnsureActive
//
// set the current active texture to the targetUnit, if not already
//
void GL_DEVICE_CLASSNAME::TextureUnitCache::EnsureActive(T_TextureUnit const targetUnit)
{
	if (m_ActiveUnit != targetUnit)
	{
		m_ActiveUnit = targetUnit;
		glActiveTexture(GL_TEXTURE0 + m_ActiveUnit);
	}
}

//----------------------------------------------
// GlContext::TextureUnitCache::OnTextureDelete
//
// When a texture gets deleted, texture units referring to it get reset
//
void GL_DEVICE_CLASSNAME::TextureUnitCache::OnTextureDelete(T_TextureLoc const tex)
{
	auto mapIt = m_Map.find(tex);

	// the texture is currently bound
	if (mapIt != m_Map.cend()) 
	{
		// LRU unit is unit with texture, texture is unset
		m_List.emplace_back(mapIt->second->unit, 0u);
		m_List.erase(mapIt->second);

		// unreference texture from map
		m_Map.erase(mapIt);
	}

	// opengl will have freed the unit in its call upon glDeleteTextures
}


//===================
// Open GL Context
//===================


//---------------------------------
// GlContext::d-tor
//
GL_DEVICE_CLASSNAME::~GL_DEVICE_CLASSNAME()
{
	ET_LOG_I(ET_CTX_RENDER, "OpenGL unloaded");
}

//---------------------------------
// GlContext::Initialize
//
// Initialize the state with default values
//
void GL_DEVICE_CLASSNAME::Initialize(ivec2 const dimensions)
{
	m_ViewportSize = dimensions;
	m_ScissorSize = dimensions;

	if (m_IsInitialized)
	{
		return;
	}

	// Info
	//******

	ET_LOG_I(ET_CTX_RENDER, "OpenGL loaded");
	ET_LOG_I(ET_CTX_RENDER, "");
	ET_LOG_I(ET_CTX_RENDER, "Vendor:   %s", glGetString(GL_VENDOR));
	ET_LOG_I(ET_CTX_RENDER, "Renderer: %s", glGetString(GL_RENDERER));
	ET_LOG_I(ET_CTX_RENDER, "Version:  %s", glGetString(GL_VERSION));
	ET_LOG_I(ET_CTX_RENDER, "");

	// Check extensions
	//******************

	std::vector<std::string> requiredExtensions = { 
		"GL_ARB_bindless_texture", 
		"GL_EXT_texture_sRGB", 
		"GL_EXT_texture_compression_s3tc",
		"GL_ARB_texture_compression_rgtc",
		"GL_ARB_texture_compression_bptc" };

	std::vector<core::HashString> requiredExtHashes;
	for (std::string const& required : requiredExtensions)
	{
		requiredExtHashes.emplace_back(core::HashString(required.c_str()));
	}

	std::vector<core::HashString> foundExtensions;

	int32 numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for (int32 i = 0; i < numExtensions; ++i)
	{
		GLubyte const* foundExt = glGetStringi(GL_EXTENSIONS, i);
		foundExtensions.emplace_back(core::HashString(reinterpret_cast<char const*>(foundExt)));
	}

	bool allFound = true;

	ET_LOG_I(ET_CTX_RENDER, "Required extensions:");
	for (size_t reqIdx = 0u; reqIdx < requiredExtensions.size(); ++ reqIdx)
	{
		core::HashString const required = requiredExtHashes[reqIdx];

		if (std::find(foundExtensions.cbegin(), foundExtensions.cend(), required) != foundExtensions.cend())
		{
			ET_LOG_I(ET_CTX_RENDER, "\t%s - found", requiredExtensions[reqIdx].c_str());

			if (required == "GL_ARB_bindless_texture"_hash)
			{
				m_BindlessTexturesAvailable = true;
			}
		}
		else
		{
			ET_LOG_W(ET_CTX_RENDER, "\t%s - not found", requiredExtensions[reqIdx].c_str());
			allFound = false;
		}
	}

	ET_LOG_I(ET_CTX_RENDER, "");

	if (!allFound)
	{
		ET_LOG_E(ET_CTX_RENDER, "Not all required OpenGL extensions are available on this device! Try updating your graphics card drivers!");
	}

	// texture units
	//***************

	int32 maxTexUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTexUnits);
	m_TextureUnits = TextureUnitCache(static_cast<size_t>(maxTexUnits));

	// draw buffers
	//***************

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &m_MaxDrawBuffers);

	m_BufferTargets =
	{
		{ E_BufferType::Vertex, 0 },
		{ E_BufferType::Index, 0 },
		{ E_BufferType::Uniform, 0 }
	};

	// polygon modes
	//***************
	m_PolygonMode.resize(static_cast<size_t>(E_FaceCullMode::COUNT), E_PolygonMode::Fill);

	// cubemaps
	//**********

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// debugging
	//***********

	// potentially hook up opengl to the logger
#if ET_CT_IS_ENABLED(ET_CT_RHI_VERBOSE)

	auto glLogCallback = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
		{
			ET_UNUSED(source);
			ET_UNUSED(id);
			ET_UNUSED(length);
			ET_UNUSED(userParam);

			core::E_TraceLevel level = core::E_TraceLevel::TL_Info;
			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR:
				level = core::E_TraceLevel::TL_Error;
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				level = core::E_TraceLevel::TL_Warning;
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				level = core::E_TraceLevel::TL_Warning;
				break;
			}

			if (severity == GL_DEBUG_SEVERITY_HIGH)
			{
				level = core::E_TraceLevel::TL_Fatal;
			}

			ET_TRACE(ET_CTX_OPENGL, level, false, message);
		};

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glLogCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

#endif

	m_IsInitialized = true;
}

//---------------------------------
// GlContext::SetDepthEnabled
//
void GL_DEVICE_CLASSNAME::SetDepthEnabled(bool const enabled)
{
	EnOrDisAble(m_DepthTestEnabled, enabled, GL_DEPTH_TEST);
}

//---------------------------------
// GlContext::SetBlendEnabled
//
// Set the buffers on which blending is enabled
//
void GL_DEVICE_CLASSNAME::SetBlendEnabled(std::vector<bool> const& blendBuffers)
{
	for (uint32 i = 0; i < blendBuffers.size(); i++)
	{
		SetBlendEnabled(blendBuffers[i], i);
	}
}

//---------------------------------
// GlContext::SetBlendEnabled
//
// Set whether blending is enabled on a specific buffer
//
void GL_DEVICE_CLASSNAME::SetBlendEnabled(bool const enabled, uint32 const index)
{
	ET_ASSERT(static_cast<int32>(index) < m_MaxDrawBuffers);

	m_IndividualBlend = true;
	if (index >= m_BlendEnabledIndexed.size())
	{
		m_BlendEnabledIndexed.push_back(m_BlendEnabled);
	}

	EnOrDisAbleIndexed(m_BlendEnabledIndexed, enabled, GL_BLEND, index);
}

//---------------------------------
// GlContext::SetBlendEnabled
//
// Set whether blending pixels is enabled in the rhi pipeline
//
void GL_DEVICE_CLASSNAME::SetBlendEnabled(bool const enabled)
{
	// if we previously blended per buffer index, reset those
	if (m_IndividualBlend)
	{
		m_BlendEnabled = !enabled;
		for (uint32 i = 0; i<m_BlendEnabledIndexed.size(); ++i)
		{
			m_BlendEnabledIndexed[i] = enabled;
		}

		m_IndividualBlend = false;
	}

	EnOrDisAble(m_BlendEnabled, enabled, GL_BLEND);
}

//---------------------------------
// GlContext::SetStencilEnabled
//
void GL_DEVICE_CLASSNAME::SetStencilEnabled(bool const enabled)
{
	EnOrDisAble(m_StencilTestEnabled, enabled, GL_STENCIL_TEST);
}

//---------------------------------
// GlContext::SetCullEnabled
//
void GL_DEVICE_CLASSNAME::SetCullEnabled(bool const enabled)
{
	EnOrDisAble(m_CullFaceEnabled, enabled, GL_CULL_FACE);
}

//---------------------------------
// GlContext::SetCullEnabled
//
void GL_DEVICE_CLASSNAME::SetScissorEnabled(bool const enabled)
{
	EnOrDisAble(m_ScissorEnabled, enabled, GL_SCISSOR_TEST);
}

//---------------------------------
// GlContext::SetColorMask
//
void GL_DEVICE_CLASSNAME::SetColorMask(T_ColorFlags const flags)
{
	if (flags != m_ColorMask)
	{
		m_ColorMask = flags;

		bool const r = flags & E_ColorFlag::CF_Red;
		bool const g = flags & E_ColorFlag::CF_Green;
		bool const b = flags & E_ColorFlag::CF_Blue;
		bool const a = flags & E_ColorFlag::CF_Alpha;
		glColorMask(r, g, b, a);
	}
}

//---------------------------------
// GlContext::SetDepthMask
//
void GL_DEVICE_CLASSNAME::SetDepthMask(bool const flag)
{
	if (flag != m_DepthMask)
	{
		m_DepthMask = flag;
		glDepthMask(flag);
	}
}

//---------------------------------
// GlContext::SetStencilMask
//
void GL_DEVICE_CLASSNAME::SetStencilMask(uint32 const mask)
{
	if (mask != m_StencilMask)
	{
		m_StencilMask = mask;
		glStencilMask(mask);
	}
}

//---------------------------------
// GlContext::SetFaceCullingMode
//
// Set the culling mode (front back neither...)
//
void GL_DEVICE_CLASSNAME::SetFaceCullingMode(E_FaceCullMode const cullMode)
{
	if (!(m_CullFaceMode == cullMode))
	{
		m_CullFaceMode = cullMode;
		glCullFace(GL_DEVICE_NS::ConvFaceCullMode(m_CullFaceMode));
	}
}

//---------------------------
// GlContext::SetPolygonMode
//
// Set how to fill triangles
//
void GL_DEVICE_CLASSNAME::SetPolygonMode(E_FaceCullMode const cullMode, E_PolygonMode const mode)
{
	size_t const modeIdx(static_cast<size_t>(cullMode));
	if (m_PolygonMode[modeIdx] != mode)
	{
		m_PolygonMode[modeIdx] = mode;
		glPolygonMode(GL_DEVICE_NS::ConvFaceCullMode(cullMode), GL_DEVICE_NS::ConvPolygonMode(mode));
	}
}

//---------------------------------
// GlContext::SetBlendEquation
//
// Set the equation we use to blend pixels
//
void GL_DEVICE_CLASSNAME::SetBlendEquation(E_BlendEquation const equation)
{
	if (!(m_BlendEquationRGB == equation && m_BlendEquationAlpha == equation))
	{
		m_BlendEquationRGB = equation;
		m_BlendEquationAlpha = equation;
		glBlendEquation(GL_DEVICE_NS::ConvBlendEquation(equation));
	}
}

//---------------------------------
// GlContext::SetBlendFunction
//
// Set the function we use to blend pixels
//
void GL_DEVICE_CLASSNAME::SetBlendFunction(E_BlendFactor const sFactor, E_BlendFactor const dFactor)
{
	if (!(m_BlendFuncSFactor == sFactor && m_BlendFuncSFactorAlpha == sFactor &&
		m_BlendFuncDFactor == dFactor && m_BlendFuncDFactorAlpha == dFactor))
	{
		m_BlendFuncSFactor = sFactor;
		m_BlendFuncSFactorAlpha = sFactor;
		m_BlendFuncDFactor = dFactor;
		m_BlendFuncDFactorAlpha = dFactor;
		glBlendFunc(GL_DEVICE_NS::ConvBlendFactor(sFactor), GL_DEVICE_NS::ConvBlendFactor(dFactor));
	}
}

//-------------------------------------
// GlContext::SetBlendFunctionSeparate
//
// Set the function we use to blend pixels - with separate functions for alpha and RGB
//
void GL_DEVICE_CLASSNAME::SetBlendFunctionSeparate(E_BlendFactor const sRGB, E_BlendFactor const sAlpha, 
	E_BlendFactor const dRGB, E_BlendFactor const dAlpha)
{
	if (!(m_BlendFuncSFactor == sRGB && m_BlendFuncSFactorAlpha == sAlpha &&
		m_BlendFuncDFactor == dRGB && m_BlendFuncDFactorAlpha == dAlpha))
	{
		m_BlendFuncSFactor = sRGB;
		m_BlendFuncSFactorAlpha = sAlpha;
		m_BlendFuncDFactor = dRGB;
		m_BlendFuncDFactorAlpha = dAlpha;
		glBlendFuncSeparate(GL_DEVICE_NS::ConvBlendFactor(sRGB), GL_DEVICE_NS::ConvBlendFactor(dRGB), 
			GL_DEVICE_NS::ConvBlendFactor(sAlpha), GL_DEVICE_NS::ConvBlendFactor(dAlpha));
	}
}

//---------------------------------
// GlContext::SetDepthFunction
//
// How to compare depth values
//
void GL_DEVICE_CLASSNAME::SetDepthFunction(E_DepthFunc const func) 
{
	if (m_DepthFunc != func)
	{
		m_DepthFunc = func;
		glDepthFunc(GL_DEVICE_NS::ConvDepthStencilFunction(func));
	}
}

//---------------------------------
// GlContext::SetStencilFunction
//
// How to compare stencil values
//
void GL_DEVICE_CLASSNAME::SetStencilFunction(T_StencilFunc const func, int32 const reference, uint32 const mask)
{
	if (!((func == m_StencilFunc) && (reference == m_StencilRef) && (mask == m_StencilFuncMask)))
	{
		m_StencilFunc = func;
		m_StencilRef = reference;
		m_StencilFuncMask = mask;
		glStencilFunc(GL_DEVICE_NS::ConvDepthStencilFunction(func), reference, mask);
	}
}

//---------------------------------
// GlContext::SetStencilOperation
//
// What to do when the stencil test passes or fails
//
void GL_DEVICE_CLASSNAME::SetStencilOperation(E_StencilOp const sFail, E_StencilOp const dFail, E_StencilOp const dsPass)
{
	if (!((sFail == m_StencilSFail) && (dFail == m_StencilDFail) && (dsPass == m_StencilDSPass)))
	{
		m_StencilSFail = sFail;
		m_StencilDFail = dFail;
		m_StencilDSPass = dsPass;
		glStencilOp(GL_DEVICE_NS::ConvStencilOp(sFail), GL_DEVICE_NS::ConvStencilOp(dFail), GL_DEVICE_NS::ConvStencilOp(dsPass));
	}
}

//---------------------------------
// GlContext::SetScissor
//
// Set the dimensions of the scissor rectangle
//
void GL_DEVICE_CLASSNAME::SetScissor(ivec2 const pos, ivec2 const size)
{
	if (!(math::nearEqualsV(m_ScissorPosition, pos) && math::nearEqualsV(m_ScissorSize, size)))
	{
		m_ScissorPosition = pos;
		m_ScissorSize = size;
		glScissor(pos.x, pos.y, size.x, size.y);
	}
}

//---------------------------------
// GlContext::SetViewport
//
// Set the dimensions of the current opengl viewport (not the engine viewport)
//
void GL_DEVICE_CLASSNAME::SetViewport(ivec2 const pos, ivec2 const size)
{
	if (!(math::nearEqualsV(m_ViewportPosition, pos) && math::nearEqualsV(m_ViewportSize, size)))
	{
		m_ViewportPosition = pos;
		m_ViewportSize = size;
		glViewport(pos.x, pos.y, size.x, size.y);
	}
}

//---------------------------------
// GlContext::GetViewport
//
// Get the dimensions of the current opengl viewport (not the engine viewport)
//
void GL_DEVICE_CLASSNAME::GetViewport(ivec2& pos, ivec2& size)
{
	pos = m_ViewportPosition;
	size = m_ViewportSize;
}

//---------------------------------
// GlContext::SetClearColor
//
// Set the colour that gets drawn when we clear the viewport
//
void GL_DEVICE_CLASSNAME::SetClearColor(vec4 const& col)
{
	if (!(math::nearEqualsV(m_ClearColor, col)))
	{
		m_ClearColor = col;
		glClearColor(col.r, col.g, col.b, col.a);
	}
}

//---------------------------------
// GlContext::SetShader
//
// Set the shader we draw with
//
void GL_DEVICE_CLASSNAME::SetShader(ShaderData const* pShader)
{
	if (!(m_pBoundShader == pShader))
	{
		// #todo: make shaders track uniforms automatically during precompilation and update those here too
		m_pBoundShader = pShader;
		glUseProgram(pShader->GetProgram());
	}
}

//---------------------------------
// GlContext::BindFramebuffer
//
// Set the framebuffer we will draw to and read from
//
void GL_DEVICE_CLASSNAME::BindFramebuffer(T_FbLoc const handle)
{
	if (!(m_ReadFramebuffer == handle && m_DrawFramebuffer == handle))
	{
		m_ReadFramebuffer = handle;
		m_DrawFramebuffer = handle;
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// GlContext::BindReadFramebuffer
//
// Set the framebuffer we will read from
//
void GL_DEVICE_CLASSNAME::BindReadFramebuffer(T_FbLoc const handle)
{
	if (!(m_ReadFramebuffer == handle))
	{
		m_ReadFramebuffer = handle;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// GlContext::BindDrawFramebuffer
//
// Set the framebuffer we will draw to
//
void GL_DEVICE_CLASSNAME::BindDrawFramebuffer(T_FbLoc const handle)
{
	if (!(m_DrawFramebuffer == handle))
	{
		m_DrawFramebuffer = handle;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// GlContext::BindFramebuffer
//
// Set the active renderbuffer
//
void GL_DEVICE_CLASSNAME::BindRenderbuffer(T_RbLoc const handle)
{
	if (handle != m_Renderbuffer)
	{
		m_Renderbuffer = handle;
		glBindRenderbuffer(GL_RENDERBUFFER, handle);
	}
}

//---------------------------------
// GlContext::BindTexture
//
// Bind a texture to a target
//
T_TextureUnit GL_DEVICE_CLASSNAME::BindTexture(E_TextureType const target, T_TextureLoc const texLoc, bool const ensureActive)
{
	return m_TextureUnits.Bind(target, texLoc, ensureActive);
}

//---------------------------------
// GlContext::BindTexture
//
// Unbind a texture from whatever target it is bound to
//
void GL_DEVICE_CLASSNAME::UnbindTexture(E_TextureType const target, T_TextureLoc const texLoc)
{
	m_TextureUnits.Unbind(target, texLoc);
}

//---------------------------------
// GlContext::BindVertexArray
//
// Bind the current vertex array that the state operates on
//
void GL_DEVICE_CLASSNAME::BindVertexArray(T_ArrayLoc const vertexArray)
{
	if (!(m_VertexArray == vertexArray))
	{
		m_VertexArray = vertexArray;
		glBindVertexArray(vertexArray);

		//Aparrently binding a new vertex array unbinds the old element array buffer
		//Coincedentially it doesn't seem to happen with array buffers
		m_BufferTargets[E_BufferType::Index] = 0;
	}
}

//---------------------------------
// GlContext::BindBuffer
//
// Bind the current buffer that the state operates on
//
void GL_DEVICE_CLASSNAME::BindBuffer(E_BufferType const target, T_BufferLoc const buffer)
{
	if (m_BufferTargets[target] != buffer)
	{
		m_BufferTargets[target] = buffer;
		glBindBuffer(GL_DEVICE_NS::ConvBufferType(target), buffer);
	}
}

//---------------------------------
// GlContext::SetLineWidth
//
// Set the width of lines that are drawn
//
void GL_DEVICE_CLASSNAME::SetLineWidth(float const lineWidth)
{
	if (!math::nearEquals(m_LineWidth, lineWidth))
	{
		m_LineWidth = lineWidth;
		glLineWidth(m_LineWidth);
	}
}

//---------------------------------
// GlContext::GetActiveFramebuffer
//
// Rerieves the currently active framebuffer, and updates the state to reflect that
//
T_FbLoc GL_DEVICE_CLASSNAME::GetActiveFramebuffer()
{
	int32 result = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &result);

	T_FbLoc ret = static_cast<T_FbLoc>(result);

	m_ReadFramebuffer = ret;
	m_DrawFramebuffer = ret;

	return ret;
}

//---------------------------------
// GlContext::DrawArrays
//
// Draw vertex data (without indices)
//
void GL_DEVICE_CLASSNAME::DrawArrays(E_DrawMode const mode, uint32 const first, uint32 const count)
{
	glDrawArrays(GL_DEVICE_NS::ConvDrawMode(mode), first, count);

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	core::PerformanceInfo::GetInstance()->m_DrawCalls++;
#endif
}

//---------------------------------
// GlContext::DrawElements
//
// Draw vertex data with indices
//
void GL_DEVICE_CLASSNAME::DrawElements(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices)
{
	glDrawElements(GL_DEVICE_NS::ConvDrawMode(mode), count, GL_DEVICE_NS::ConvDataType(type), indices);

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	core::PerformanceInfo::GetInstance()->m_DrawCalls++;
#endif
}

//---------------------------------
// GlContext::DrawElementsInstanced
//
// Draw instanced vertex data with indices
//
void GL_DEVICE_CLASSNAME::DrawElementsInstanced(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices, uint32 const prims)
{
	glDrawElementsInstanced(GL_DEVICE_NS::ConvDrawMode(mode), count, GL_DEVICE_NS::ConvDataType(type), indices, prims);

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	core::PerformanceInfo::GetInstance()->m_DrawCalls++;
#endif
}

//---------------------------------
// GlContext::Flush
//
// Force OpenGL to execute all commands now
//
void GL_DEVICE_CLASSNAME::Flush() const
{
	glFlush();
}

//---------------------------------
// GlContext::Finish
//
// Force OpenGL to execute all commands now and don't return until its done
//
void GL_DEVICE_CLASSNAME::Finish() const
{
	glFinish();
}

//---------------------------------
// GlContext::Clear
//
// Clear the part of the currently set viewport that is mapped to the mask
//
void GL_DEVICE_CLASSNAME::Clear(T_ClearFlags const mask) const
{
	GLbitfield field = 0;

	if (mask & E_ClearFlag::CF_Color)
	{
		field |= GL_COLOR_BUFFER_BIT;
	}

	if (mask & E_ClearFlag::CF_Depth)
	{
		field |= GL_DEPTH_BUFFER_BIT;
	}

	if (mask & E_ClearFlag::CF_Stencil)
	{
		field |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(field);
}

//---------------------------------
// GlContext::CreateVertexArray
//
T_ArrayLoc GL_DEVICE_CLASSNAME::CreateVertexArray() const
{
	T_ArrayLoc ret;
	glGenVertexArrays(1, &ret);
	return ret;
}

//---------------------------------
// GlContext::CreateBuffer
//
T_BufferLoc GL_DEVICE_CLASSNAME::CreateBuffer() const
{
	T_BufferLoc ret;
	glGenBuffers(1, &ret);
	return ret;
}

//---------------------------------
// GlContext::DeleteVertexArray
//
// Delete a vertex array
//
void GL_DEVICE_CLASSNAME::DeleteVertexArray(T_ArrayLoc& loc) const
{
	glDeleteVertexArrays(1, &loc);
}

//---------------------------------
// GlContext::DeleteBuffer
//
// Delete a buffer
//
void GL_DEVICE_CLASSNAME::DeleteBuffer(T_BufferLoc& loc) const
{
	glDeleteBuffers(1, &loc);
}

//---------------------------------
// GlContext::SetBufferData
//
// Fill the buffer at target with an array of data
//
void GL_DEVICE_CLASSNAME::SetBufferData(E_BufferType const target, int64 const size, void const* const data, E_UsageHint const usage) const
{
	glBufferData(GL_DEVICE_NS::ConvBufferType(target), size, data, GL_DEVICE_NS::ConvUsageHint(usage));
}

//---------------------------------
// GlContext::SetVertexAttributeArrayEnabled
//
// Enable or disable an attribute at an index in the current vertex array
//
void GL_DEVICE_CLASSNAME::SetVertexAttributeArrayEnabled(uint32 const index, bool const enabled) const
{
	if (enabled)
	{
		glEnableVertexAttribArray(index);
	}
	else
	{
		glDisableVertexAttribArray(index);
	}
}

//---------------------------------
// GlContext::DefineVertexAttributePointer
//
// Define the type of data the attribute at a certain index in the current vertex array is mapped to
//
void GL_DEVICE_CLASSNAME::DefineVertexAttributePointer(uint32 const index,
	int32 const size, 
	E_DataType const type, 
	bool const norm, 
	int32 const stride, 
	size_t const offset) const
{
	glVertexAttribPointer(index, size, GL_DEVICE_NS::ConvDataType(type), (norm ? GL_TRUE : GL_FALSE), stride, static_cast<char const*>(0) + offset);
}

//---------------------------------
// GlContext::DefineVertexAttribIPointer
//
// Same as above, but for non normalized integers
//
void GL_DEVICE_CLASSNAME::DefineVertexAttribIPointer(uint32 const index,
	int32 const size, 
	E_DataType const type, 
	int32 const stride, 
	size_t const offset) const
{
	glVertexAttribIPointer(index, size, GL_DEVICE_NS::ConvDataType(type), stride, static_cast<char const*>(0) + offset);
}

//---------------------------------
// GlContext::DefineVertexAttribDivisor
//
// Additional vertex stride during instanced rendering
//
void GL_DEVICE_CLASSNAME::DefineVertexAttribDivisor(uint32 const index, uint32 const divisor) const
{
	glVertexAttribDivisor(index, divisor);
}

//---------------------------------
// GlContext::MapBuffer
//
// Map the data of a buffer to a pointer on the CPU so that it can be modified
//
void* GL_DEVICE_CLASSNAME::MapBuffer(E_BufferType const target, E_AccessMode const access) const
{
	return glMapBuffer(GL_DEVICE_NS::ConvBufferType(target), GL_DEVICE_NS::ConvAccessMode(access));
}

//---------------------------------
// GlContext::UnmapBuffer
//
// Unmap a buffer from the pointer it's mapped to on the CPU
//
void GL_DEVICE_CLASSNAME::UnmapBuffer(E_BufferType const target) const
{
	glUnmapBuffer(GL_DEVICE_NS::ConvBufferType(target));
}

//---------------------------------
// GlContext::BindBufferRange
//
// Bind a buffer to a shared location (index)
//
void GL_DEVICE_CLASSNAME::BindBufferRange(E_BufferType const target, 
	uint32 const index, 
	T_BufferLoc const buffer, 
	size_t const offset, 
	size_t const size) const
{
	glBindBufferRange(GL_DEVICE_NS::ConvBufferType(target), index, buffer, offset, size);
}

//---------------------------------
// GlContext::UnmapBuffer
//
T_TextureLoc GL_DEVICE_CLASSNAME::GenerateTexture() const
{
	T_TextureLoc ret;
	glGenTextures(1, &ret);
	return ret;
}

//---------------------------------
// GlContext::DeleteTexture
//
void GL_DEVICE_CLASSNAME::DeleteTexture(T_TextureLoc& texLoc)
{
	// reset bound textures
	m_TextureUnits.OnTextureDelete(texLoc);
	
	glDeleteTextures(1, &texLoc);

	texLoc = 0u;
}

//---------------------------------
// GlContext::UploadTextureData
//
// upload a textures bits to its GPU location
//
void GL_DEVICE_CLASSNAME::UploadTextureData(TextureData& texture, 
	void const* const data, 
	E_ColorFormat const layout, 
	E_DataType const dataType, 
	int32 const mipLevel)
{
	uint32 const target = GL_DEVICE_NS::ConvTextureType(texture.GetTargetType());
	ivec2 res = texture.GetResolution() / (1 << mipLevel);
	GLint const intFmt = static_cast<GLint>(GL_DEVICE_NS::ConvColorFormat(texture.GetStorageFormat()));
	ET_ASSERT(layout <= E_ColorFormat::BGRA, "Texture layout can't specify storage format!"); // possibly the enum should be split

	BindTexture(texture.GetTargetType(), texture.GetLocation(), true);

	switch (texture.GetTargetType())
	{
	case E_TextureType::Texture2D:
		glTexImage2D(target, mipLevel, intFmt, res.x, res.y, 0, GL_DEVICE_NS::ConvColorFormat(layout), GL_DEVICE_NS::ConvDataType(dataType), data);
		break;

	case E_TextureType::Texture3D:
		glTexImage3D(target, 
			mipLevel,
			intFmt, 
			res.x, 
			res.y, 
			texture.GetDepth(), 
			0, 
			GL_DEVICE_NS::ConvColorFormat(layout),
			GL_DEVICE_NS::ConvDataType(dataType),
			data);
		break;

	case E_TextureType::CubeMap:
	{
		size_t const faceSize = static_cast<size_t>(DataTypeInfo::GetTypeSize(dataType)) *
			static_cast<size_t>(TextureFormat::GetChannelCount(layout)) *
			static_cast<size_t>(res.x) *
			static_cast<size_t>(res.y);
		for (uint8 face = 0u; face < TextureData::s_NumCubeFaces; ++face)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
				mipLevel, 
				intFmt, 
				res.x, 
				res.y, 
				0, 
				GL_DEVICE_NS::ConvColorFormat(layout), 
				GL_DEVICE_NS::ConvDataType(dataType), 
				reinterpret_cast<void const*>(reinterpret_cast<uint8 const*>(data) + (faceSize * static_cast<size_t>(face))));
		}

		break;
	}

	default:
		ET_ERROR("Unsupported texture type for uploading texture data");
		break;
	}
}


//----------------------------------------
// GlContext::UploadCompressedTextureData
//
// upload a textures bits to its GPU location
//
void GL_DEVICE_CLASSNAME::UploadCompressedTextureData(TextureData& texture, void const* const data, size_t const size, int32 const mipLevel)
{
	uint32 const target = GL_DEVICE_NS::ConvTextureType(texture.GetTargetType());
	ivec2 res = texture.GetResolution() / (1 << mipLevel);
	GLint const intFmt = static_cast<GLint>(GL_DEVICE_NS::ConvColorFormat(texture.GetStorageFormat()));

	BindTexture(texture.GetTargetType(), texture.GetLocation(), true);

	switch (texture.GetTargetType())
	{
	case E_TextureType::Texture2D:
		glCompressedTexImage2D(target, mipLevel, intFmt, res.x, res.y, 0, static_cast<int32>(size), data);
		break;

	case E_TextureType::CubeMap:
	{
		size_t const faceSize = size / TextureData::s_NumCubeFaces;
		for (uint8 face = 0u; face < TextureData::s_NumCubeFaces; ++face)
		{
			void const* const faceData = reinterpret_cast<void const*>(reinterpret_cast<uint8 const*>(data) + (faceSize * static_cast<size_t>(face)));
			glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mipLevel, intFmt, res.x, res.y, 0, faceSize, faceData);
		}

		break;
	}

	default:
		ET_ERROR("Unsupported texture type for uploading compressed texture data");
		break;
	}
}

//---------------------------------
// GlContext::AllocateTextureStorage
//
// upload a textures bits to its GPU location
//
void GL_DEVICE_CLASSNAME::AllocateTextureStorage(TextureData& texture)
{
	uint32 const target = GL_DEVICE_NS::ConvTextureType(texture.GetTargetType());

	BindTexture(texture.GetTargetType(), texture.GetLocation(), true);

	ivec2 const res = texture.GetResolution();

	E_ColorFormat const storageFmt = texture.GetStorageFormat();
	GLint const intFmt = static_cast<GLint>(GL_DEVICE_NS::ConvColorFormat(storageFmt));

	GLenum colorLayout, dataType;
	GL_DEVICE_NS::DeriveLayoutAndType(storageFmt, colorLayout, dataType);

	switch (texture.GetTargetType())
	{
	case E_TextureType::Texture2D:
		glTexImage2D(target, 0, intFmt, res.x, res.y, 0, colorLayout, dataType, nullptr);
		break;

	case E_TextureType::Texture3D:
		glTexImage3D(target, 0, intFmt, res.x, res.y, texture.GetDepth(), 0, colorLayout, dataType, nullptr);
		break;

	case E_TextureType::CubeMap:
		ET_ASSERT(res.x == res.y);
		for (uint8 face = 0u; face < TextureData::s_NumCubeFaces; ++face)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, intFmt, res.x, res.y, 0, colorLayout, dataType, nullptr);
		}

		break;
	}
}

//---------------------------------
// GlContext::SetTextureParams
//
// Update parameters on a texture
//
void GL_DEVICE_CLASSNAME::SetTextureParams(TextureData const& texture, TextureParameters& prev, TextureParameters const& next, bool const force)
{
	uint32 const target = GL_DEVICE_NS::ConvTextureType(texture.GetTargetType());
	BindTexture(texture.GetTargetType(), texture.GetLocation(), true);

	// filter options
	//---------------
	// in the future it may make sense to create filter groups so that things such as anisotropy can be set globally
	if ((prev.minFilter != next.minFilter) || (prev.mipFilter != next.mipFilter) || (prev.genMipMaps != next.genMipMaps) || force)
	{
		int32 minFilter = GL_DEVICE_NS::ConvMinFilter(next.minFilter, next.mipFilter, next.genMipMaps);
		ET_ASSERT(minFilter != 0);

		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
	}

	if ((prev.magFilter != next.magFilter) || force)
	{
		int32 filter = GL_DEVICE_NS::ConvFilter(next.magFilter);
		ET_ASSERT(filter != 0);

		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
	}

	// address mode
	//-------------
	if ((prev.wrapS != next.wrapS) || force)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_DEVICE_NS::ConvWrapMode(next.wrapS));
	}

	if ((prev.wrapT != next.wrapT) || force)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_DEVICE_NS::ConvWrapMode(next.wrapT));
	}

	if ((texture.GetDepth() > 1) && ((prev.wrapR != next.wrapR) || force))
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_DEVICE_NS::ConvWrapMode(next.wrapR));
	}

	// border color
	if (!math::nearEqualsV(prev.borderColor, next.borderColor) || force)
	{
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, next.borderColor.data.data());
	}

	// other
	//-------
	if ((next.isDepthTex && (prev.compareMode != next.compareMode)) || (next.isDepthTex && force))
	{
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_DEVICE_NS::ConvCompareMode(next.compareMode));//shadow map comp mode
	}

	// max mip level incase mips where manually uploaded
	//---------------------------------------------------
	if (next.genMipMaps && (next.mipFilter == E_TextureFilterMode::Linear) && (texture.GetNumMipLevels() > 0u))
	{
		glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<int32>(texture.GetNumMipLevels()));
	}

	prev = next;
}

//---------------------------------
// GlContext::GenerateMipMaps
//
void GL_DEVICE_CLASSNAME::GenerateMipMaps(TextureData const& texture, uint8& mipLevels)
{
	uint32 const target = GL_DEVICE_NS::ConvTextureType(texture.GetTargetType());
	BindTexture(texture.GetTargetType(), texture.GetLocation(), true);

	glGenerateMipmap(target);

	ivec2 const res = texture.GetResolution();
	float const largerRes = static_cast<float>(std::max(res.x, res.y));
	mipLevels = 1u + static_cast<uint8>(floor(log10(largerRes) / log10(2.f)));
}

//---------------------------------
// GlContext::GetTextureHandle
//
// Create a handle for bindless access of a texture, at the cost of not being able to change parameters any longer
//
T_TextureHandle GL_DEVICE_CLASSNAME::GetTextureHandle(T_TextureLoc const texLoc) const
{
	if (!m_BindlessTexturesAvailable)
	{
		return 0u;
	}

	return glGetTextureHandleARB(texLoc);
}

//--------------------------------------
// GlContext::SetTextureHandleResidency
//
// 'bind' a texture handle so it can be used by shaders
//
void GL_DEVICE_CLASSNAME::SetTextureHandleResidency(T_TextureHandle const handle, bool const isResident) const
{
	if (!m_BindlessTexturesAvailable)
	{
		return;
	}

	if (isResident)
	{
		glMakeTextureHandleResidentARB(handle);
	}
	else
	{
		glMakeTextureHandleNonResidentARB(handle);
	}
}

//--------------------------------------
// GlContext::GetTextureData
//
// Get the pixels from a texture
//  - for cubemaps the pixels are simply stored sequential
//
void GL_DEVICE_CLASSNAME::GetTextureData(TextureData const& texture,
	uint8 const mipLevel, 
	E_ColorFormat const format, 
	E_DataType const dataType, 
	void* const data)
{
	uint32 const target = GL_DEVICE_NS::ConvTextureType(texture.GetTargetType());
	BindTexture(texture.GetTargetType(), texture.GetLocation(), true);

	GLenum const glLayout = GL_DEVICE_NS::ConvColorFormat(format);
	GLenum const glDataType = GL_DEVICE_NS::ConvDataType(dataType);

	if (texture.GetTargetType() == E_TextureType::CubeMap)
	{
		ivec2 res = texture.GetResolution();
		res = res / (1 << static_cast<int32>(mipLevel));
		size_t const offset = static_cast<size_t>(DataTypeInfo::GetTypeSize(dataType)) * 
			static_cast<size_t>(TextureFormat::GetChannelCount(format)) *
			static_cast<size_t>(res.x) *
			static_cast<size_t>(res.y);

		for (uint8 face = 0u; face < TextureData::s_NumCubeFaces; ++face)
		{
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
				static_cast<int32>(mipLevel), 
				glLayout, 
				glDataType, 
				reinterpret_cast<void*>(reinterpret_cast<uint8*>(data) + (offset * static_cast<size_t>(face))));
		}
	}
	else
	{
		glGetTexImage(target, static_cast<int32>(mipLevel), glLayout, glDataType, data);
	}
}

//---------------------------------
// GlContext::CreateShader
//
// Create a shader object and return its handle
//
T_ShaderLoc GL_DEVICE_CLASSNAME::CreateShader(E_ShaderType const type) const
{
	return glCreateShader(GL_DEVICE_NS::ConvShaderType(type));
}

//---------------------------------
// GlContext::CreateProgram
//
// Create a program object and return its handle
//
T_ShaderLoc GL_DEVICE_CLASSNAME::CreateProgram() const
{
	return glCreateProgram();
}

//---------------------------------
// GlContext::DeleteShader
//
// Delete a shader by its handle
//
void GL_DEVICE_CLASSNAME::DeleteShader(T_ShaderLoc const shader)
{
	glDeleteShader(shader);
}

//---------------------------------
// GlContext::DeleteProgram
//
// Delete a program by its handle
//
void GL_DEVICE_CLASSNAME::DeleteProgram(T_ShaderLoc const program)
{
	glDeleteProgram(program);
}

//---------------------------------
// GlContext::CompileShader
//
// Compile a shader
//
void GL_DEVICE_CLASSNAME::CompileShader(T_ShaderLoc const shader, std::string const& source) const
{
	char const* sourcePtr = source.c_str();
	glShaderSource(shader, 1, &sourcePtr, nullptr);
	glCompileShader(shader);
}

//---------------------------------
// GlContext::BindFragmentDataLocation
//
// Set the return member of a fragment shader
//
void GL_DEVICE_CLASSNAME::BindFragmentDataLocation(T_ShaderLoc const program, uint32 const colorNumber, std::string const& name) const
{
	glBindFragDataLocation(program, colorNumber, name.c_str());
}

//---------------------------------
// GlContext::AttachShader
//
// Attach a shader to a program before linking
//
void GL_DEVICE_CLASSNAME::AttachShader(T_ShaderLoc const program, T_ShaderLoc const shader) const
{
	glAttachShader(program, shader);
}

//---------------------------------
// GlContext::LinkProgram
//
// Link the shaders in a program
//
void GL_DEVICE_CLASSNAME::LinkProgram(T_ShaderLoc const program) const
{
	glLinkProgram(program);
}

//---------------------------------
// GlContext::IsShaderCompiled
//
bool GL_DEVICE_CLASSNAME::IsShaderCompiled(T_ShaderLoc const shader) const
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	return (status == GL_TRUE);
}

//---------------------------------
// GlContext::GetShaderInfo
//
// Get logged information about a shader
//
void GL_DEVICE_CLASSNAME::GetShaderInfo(T_ShaderLoc const shader, std::string& info) const
{
	char buffer[512];
	glGetShaderInfoLog(shader, 512, NULL, buffer);
	info = std::string(buffer);
}

//---------------------------------
// GlContext::GetUniformBlockIndex
//
// Get the index of a uniform block within a shader
//
T_BlockIndex GL_DEVICE_CLASSNAME::GetUniformBlockIndex(T_ShaderLoc const program, std::string const& blockName) const
{
	return glGetUniformBlockIndex(program, blockName.c_str());
}

//---------------------------------
// GlContext::IsBlockIndexValid
//
bool GL_DEVICE_CLASSNAME::IsBlockIndexValid(T_BlockIndex const index) const
{
	return (index != GL_INVALID_INDEX);
}

//---------------------------------
// GlContext::GetUniformBlockNames
//
std::vector<std::string> GL_DEVICE_CLASSNAME::GetUniformBlockNames(T_ShaderLoc const program) const
{
	// amount of uniform blocks in the shader
	GLint blockCount = 0;
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);

	// length of the longest blocks name for our string buffer
	GLint maxNameLength = 0;
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxNameLength);

	std::vector<std::string> ret;

	GLchar* name = new GLchar[maxNameLength]; // no need to keep reallocating

	// get all the names
	for (GLint i = 0; i < blockCount; ++i)
	{
		GLsizei length;
		glGetActiveUniformBlockName(program, static_cast<GLuint>(i), maxNameLength, &length, name);
		ret.emplace_back(name, length);
	}

	delete[] name;

	return ret; // should be moved automatically
}

//--------------------------------------
// GlContext::GetUniformIndicesForBlock
//
std::vector<int32> GL_DEVICE_CLASSNAME::GetUniformIndicesForBlock(T_ShaderLoc const program, T_BlockIndex const blockIndex) const
{
	std::vector<int32> ret;

	GLint indexCount = 0;
	glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &indexCount);

	if (indexCount <= 0)
	{
		return ret;
	}

	GLint* indices = new GLint[indexCount];
	glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices);

	for (GLint i = 0; i < indexCount; ++i)
	{
		ret.emplace_back(indices[i]);
	}

	delete[] indices;

	return ret;
}

//-----------------------------------
// GlContext::SetUniformBlockBinding
//
void GL_DEVICE_CLASSNAME::SetUniformBlockBinding(T_ShaderLoc const program, T_BlockIndex const blockIndex, uint32 const bindingIndex) const
{
	glUniformBlockBinding(program, blockIndex, bindingIndex);
}

//---------------------------------
// GlContext::GetAttributeCount
//
int32 GL_DEVICE_CLASSNAME::GetAttributeCount(T_ShaderLoc const program) const
{
	GLint count;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
	return count;
}

//---------------------------------
// GlContext::GetUniformCount
//
int32 GL_DEVICE_CLASSNAME::GetUniformCount(T_ShaderLoc const program) const
{
	GLint count;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
	return count;
}

//---------------------------------
// GlContext::GetActiveUniforms
//
// Get a list of initialized uniform objects for a particular uniform index
//
void GL_DEVICE_CLASSNAME::GetActiveUniforms(T_ShaderLoc const program, uint32 const index, std::vector<UniformDescriptor>& uniforms) const
{
	GLint arrayCount; // support 1D arrays of structs
	GLenum type;

	const GLsizei bufSize = 256;
	GLchar name[bufSize];
	GLsizei length;

	glGetActiveUniform(program, index, bufSize, &length, &arrayCount, &type, name);
	std::string uniName = std::string(name, length);
	std::string endName;

	// if we have an array of structs, separate out the beginning and end bit so we can create our name with the index
	if (arrayCount > 1)
	{
		size_t const found = uniName.find(']');

		if (found < uniName.size() - 1)
		{
			endName = uniName.substr(uniName.find(']') + 1);
		}

		uniName = uniName.substr(0, uniName.find('['));
	}

	// for each array uniform (or the single uniform
	for (GLint arrayIdx = 0; arrayIdx < arrayCount; ++arrayIdx)
	{		
		uniforms.push_back(UniformDescriptor());
		UniformDescriptor& uni = uniforms[uniforms.size() - 1];

		// generate the full name
		uni.name = uniName;
		if (arrayCount > 1)
		{
			uni.name += "[" + std::to_string(arrayIdx) + "]" + endName;
		}

		uni.type = GL_DEVICE_NS::ParseParamType(type);

		uni.location = glGetUniformLocation(program, uni.name.c_str());
	}
}

//---------------------------------
// GlContext::GetActiveAttribute
//
// Get information about an attribute in a program at a given index
//
void GL_DEVICE_CLASSNAME::GetActiveAttribute(T_ShaderLoc const program, uint32 const index, AttributeDescriptor& info) const
{
	GLint size = 0;
	GLenum type = 0;

	const GLsizei bufSize = 256;
	GLchar name[bufSize];
	GLsizei length = 0;

	glGetActiveAttrib(program, index, bufSize, &length, &size, &type, name);

	info.name = std::string(name, length);

	switch (type)
	{
	case GL_FLOAT:
		info.dataType = E_DataType::Float;
		info.dataCount = 1u;
		break;
	case GL_FLOAT_VEC2:
		info.dataType = E_DataType::Float;
		info.dataCount = 2u;
		break;
	case GL_FLOAT_VEC3:
		info.dataType = E_DataType::Float;
		info.dataCount = 3u;
		break;
	case GL_FLOAT_VEC4:
		info.dataType = E_DataType::Float;
		info.dataCount = 4u;
		break;
	case GL_FLOAT_MAT2:
		info.dataType = E_DataType::Float;
		info.dataCount = 4u;
		break;
	case GL_FLOAT_MAT3:
		info.dataType = E_DataType::Float;
		info.dataCount = 9u;
		break;
	case GL_FLOAT_MAT4:
		info.dataType = E_DataType::Float;
		info.dataCount = 16u;
		break;
	case GL_FLOAT_MAT2x3:
		info.dataType = E_DataType::Float;
		info.dataCount = 6u;
		break;
	case GL_FLOAT_MAT2x4:
		info.dataType = E_DataType::Float;
		info.dataCount = 8u;
		break;
	case GL_FLOAT_MAT3x2:
		info.dataType = E_DataType::Float;
		info.dataCount = 6u;
		break;
	case GL_FLOAT_MAT3x4:
		info.dataType = E_DataType::Float;
		info.dataCount = 12u;
		break;
	case GL_FLOAT_MAT4x2:
		info.dataType = E_DataType::Float;
		info.dataCount = 8u;
		break;
	case GL_FLOAT_MAT4x3:
		info.dataType = E_DataType::Float;
		info.dataCount = 12u;
		break;
	case GL_INT:
		info.dataType = E_DataType::Int;
		info.dataCount = 1u;
		break;
	case GL_INT_VEC2:
		info.dataType = E_DataType::Int;
		info.dataCount = 2u;
		break;
	case GL_INT_VEC3:
		info.dataType = E_DataType::Int;
		info.dataCount = 3u;
		break;
	case GL_INT_VEC4:
		info.dataType = E_DataType::Int;
		info.dataCount = 4u;
		break;
	case GL_UNSIGNED_INT:
		info.dataType = E_DataType::UInt;
		info.dataCount = 1u;
		break;
	case GL_UNSIGNED_INT_VEC2:
		info.dataType = E_DataType::UInt;
		info.dataCount = 2u;
		break;
	case GL_UNSIGNED_INT_VEC3:
		info.dataType = E_DataType::UInt;
		info.dataCount = 3u;
		break;
	case GL_UNSIGNED_INT_VEC4:
		info.dataType = E_DataType::UInt;
		info.dataCount = 4u;
		break;
	case GL_DOUBLE:
		info.dataType = E_DataType::Double;
		info.dataCount = 1u;
		break;
	case GL_DOUBLE_VEC2:
		info.dataType = E_DataType::Double;
		info.dataCount = 2u;
		break;
	case GL_DOUBLE_VEC3:
		info.dataType = E_DataType::Double;
		info.dataCount = 3u;
		break;
	case GL_DOUBLE_VEC4:
		info.dataType = E_DataType::Double;
		info.dataCount = 4u;
		break;
	case GL_DOUBLE_MAT2:
		info.dataType = E_DataType::Double;
		info.dataCount = 4u;
		break;
	case GL_DOUBLE_MAT3:
		info.dataType = E_DataType::Double;
		info.dataCount = 9u;
		break;
	case GL_DOUBLE_MAT4:
		info.dataType = E_DataType::Double;
		info.dataCount = 16u;
		break;
	case GL_DOUBLE_MAT2x3:
		info.dataType = E_DataType::Double;
		info.dataCount = 6u;
		break;
	case GL_DOUBLE_MAT2x4:
		info.dataType = E_DataType::Double;
		info.dataCount = 8u;
		break;
	case GL_DOUBLE_MAT3x2:
		info.dataType = E_DataType::Double;
		info.dataCount = 6u;
		break;
	case GL_DOUBLE_MAT3x4:
		info.dataType = E_DataType::Double;
		info.dataCount = 12u;
		break;
	case GL_DOUBLE_MAT4x2:
		info.dataType = E_DataType::Double;
		info.dataCount = 8u;
		break;
	case GL_DOUBLE_MAT4x3:
		info.dataType = E_DataType::Double;
		info.dataCount = 12u;
		break;

	default:
		ET_TRACE_W(ET_CTX_RENDER, "unknown attribute type '%u'", type);
		return;
	}
}

//---------------------------------
// GlContext::GetAttributeLocation
//
// ID for an attribute on this shader program
//
T_AttribLoc GL_DEVICE_CLASSNAME::GetAttributeLocation(T_ShaderLoc const program, std::string const& name) const
{
	return glGetAttribLocation(program, name.c_str());
}

//---------------------------------
// GlContext::PopulateUniform
//
// Fill the data with the shaders current parameter value
//
void GL_DEVICE_CLASSNAME::PopulateUniform(T_ShaderLoc const program, T_UniformLoc const location, E_ParamType const type, void* data) const
{
	switch (type)
	{
	case E_ParamType::Texture2D:
	case E_ParamType::Texture3D:
	case E_ParamType::TextureCube:
	case E_ParamType::TextureShadow:
		*static_cast<TextureData const**>(data) = nullptr;
		return;
	case E_ParamType::Matrix4x4:
		glGetUniformfv(program, location, math::valuePtr(*static_cast<mat4*>(data)));
		return;
	case E_ParamType::Matrix3x3:
		glGetUniformfv(program, location, math::valuePtr(*static_cast<mat3*>(data)));
		return;
	case E_ParamType::Vector4:
		glGetUniformfv(program, location, math::valuePtr(*static_cast<vec4*>(data)));
		return;
	case E_ParamType::Vector3:
		glGetUniformfv(program, location, math::valuePtr(*static_cast<vec3*>(data)));
		return;
	case E_ParamType::Vector2:
		glGetUniformfv(program, location, math::valuePtr(*static_cast<vec2*>(data)));
		return;
	case E_ParamType::UInt:
		glGetUniformuiv(program, location, static_cast<uint32*>(data));
		return;
	case E_ParamType::Int:
		glGetUniformiv(program, location, static_cast<int32*>(data));
		return;
	case E_ParamType::Float:
		glGetUniformfv(program, location, static_cast<float*>(data));
		return;
	case E_ParamType::Boolean:
		int32 temp;
		glGetUniformiv(program, location, &temp);
		*static_cast<bool*>(data) = static_cast<bool>(temp);
		return;
	}

	ET_ASSERT(true, "Unhandled parameter type!");
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a boolean to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, bool const data) const
{
	glUniform1i(location, data);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload an integer to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, int32 const data) const
{
	glUniform1i(location, data);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload an unsigned integer to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, uint32 const data) const
{
	glUniform1ui(location, data);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a scalar to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, float const data) const
{
	glUniform1f(location, data);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 2D Vector to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, vec2 const data) const
{
	glUniform2f(location, data.x, data.y);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 3D Vector to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, vec3 const& data) const
{
	glUniform3f(location, data.x, data.y, data.z);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 4D Vector to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, vec4 const& data) const
{
	glUniform4f(location, data.x, data.y, data.z, data.w);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 3x3 Matrix to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, mat3 const& data) const
{
	glUniformMatrix3fv(location, 1, GL_FALSE, math::valuePtr(data));
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 4x4 Matrix to the GPU
//
void GL_DEVICE_CLASSNAME::UploadUniform(T_UniformLoc const location, mat4 const& data) const
{
	glUniformMatrix4fv(location, 1, GL_FALSE, math::valuePtr(data));
}

//---------------------------------
// GlContext::GenFramebuffers
//
// Create a number of framebuffer objects
//
void GL_DEVICE_CLASSNAME::GenFramebuffers(int32 const n, T_FbLoc *ids) const
{
	glGenFramebuffers(n, ids);
}

//---------------------------------
// GlContext::DeleteFramebuffers
//
// Frees the framebuffer GPU resources
//
void GL_DEVICE_CLASSNAME::DeleteFramebuffers(int32 const n, T_FbLoc *ids) 
{
	for (int32 idx = 0; idx < n; ++idx)
	{
		if (ids[idx] == m_ReadFramebuffer)
		{
			m_ReadFramebuffer = 0u;
			break;
		}

		if (ids[idx] == m_DrawFramebuffer)
		{
			m_DrawFramebuffer = 0u;
			break;
		}
	}

	glDeleteFramebuffers(n, ids);
}

//---------------------------------
// GlContext::GenRenderBuffers
//
// Create a number of renderbuffer objects
//
void GL_DEVICE_CLASSNAME::GenRenderBuffers(int32 const n, T_RbLoc *ids) const
{
	glGenRenderbuffers(n, ids);
}

//---------------------------------
// GlContext::DeleteRenderBuffers
//
// Frees the renderbuffer GPU resources
//
void GL_DEVICE_CLASSNAME::DeleteRenderBuffers(int32 const n, T_RbLoc *ids) 
{
	for (int32 idx = 0; idx < n; ++idx)
	{
		if (ids[idx] == m_Renderbuffer)
		{
			m_Renderbuffer = 0u;
			break;
		}
	}

	glDeleteRenderbuffers(n, ids);
}

//---------------------------------
// GlContext::SetRenderbufferStorage
//
// Establish a renderbuffers dataformat and storage
//
void GL_DEVICE_CLASSNAME::SetRenderbufferStorage(E_RenderBufferFormat const format, ivec2 const dimensions) const
{
	GLenum glFmt = GL_NONE;
	switch (format)
	{
	case E_RenderBufferFormat::Depth24:
		glFmt = GL_DEPTH_COMPONENT24;
		break;

	case E_RenderBufferFormat::Depth24_Stencil8:
		glFmt = GL_DEPTH24_STENCIL8;
		break;
	}

	ET_ASSERT(glFmt != GL_NONE, "Unhandled rhi buffer format!");

	glRenderbufferStorage(GL_RENDERBUFFER, glFmt, dimensions.x, dimensions.y);
}

//---------------------------------
// GlContext::LinkTextureToFbo
//
// link to current draw FB with a color attachment
//
void GL_DEVICE_CLASSNAME::LinkTextureToFbo(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<uint32>(attachment), texHandle, level);
}

//---------------------------------
// GlContext::LinkTextureToFbo
//
// Same as above, but specifies a target
//
void GL_DEVICE_CLASSNAME::LinkTextureToFbo2D(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<uint32>(attachment), GL_TEXTURE_2D, texHandle, level);
}

//---------------------------------
// GlContext::LinkTextureToFbo
//
// Same as above, but specifies a target
//
void GL_DEVICE_CLASSNAME::LinkCubeMapFaceToFbo2D(uint8 const face, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texHandle, level);
}

//---------------------------------
// GlContext::LinkTextureToFbo
//
// Link a depth texture to an FBO
//
void GL_DEVICE_CLASSNAME::LinkTextureToFboDepth(T_TextureLoc const texHandle) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texHandle, 0);
}

//---------------------------------
// GlContext::LinkRenderbufferToFbo
//
void GL_DEVICE_CLASSNAME::LinkRenderbufferToFbo(E_RenderBufferFormat const attachment, uint32 const rboHandle) const
{
	GLenum glFmt = GL_NONE;
	switch (attachment)
	{
	case E_RenderBufferFormat::Depth24:
		glFmt = GL_DEPTH_ATTACHMENT;
		break;

	case E_RenderBufferFormat::Depth24_Stencil8:
		glFmt = GL_DEPTH_STENCIL_ATTACHMENT;
		break;
	}

	ET_ASSERT(glFmt != GL_NONE, "Unhandled rhi buffer format!");

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, glFmt, GL_RENDERBUFFER, rboHandle);
}

//---------------------------------
// GlContext::SetDrawBufferCount
//
// Setup the amount of color attachments on the current framebuffer
//
void GL_DEVICE_CLASSNAME::SetDrawBufferCount(size_t const count) const
{
	// we may also disable drawing color
	if (count == 0)
	{
		glDrawBuffer(GL_NONE);
		return;
	}

	// setup attachment list
	std::vector<GLenum> attachments;
	attachments.reserve(count);

	for (size_t i = 0u; i < count; ++i)
	{
		attachments.emplace_back(GL_COLOR_ATTACHMENT0 + static_cast<uint32>(i));
	}

	// set the buffers
	glDrawBuffers(static_cast<GLsizei>(count), attachments.data());
}

//-----------------------------------
// GlContext::SetReadBufferEnabled
//
// For the current buffer, whether or not openGL will read a color value.
// Assumes double buffered rendering
//
void GL_DEVICE_CLASSNAME::SetReadBufferEnabled(bool const val) const
{
	glReadBuffer(val ? GL_BACK : GL_NONE);
}

//-----------------------------------
// GlContext::SetReadBufferEnabled
//
bool GL_DEVICE_CLASSNAME::IsFramebufferComplete() const
{
	return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

//-----------------------------------
// GlContext::CopyDepthReadToDrawFbo
//
void GL_DEVICE_CLASSNAME::CopyDepthReadToDrawFbo(ivec2 const source, ivec2 const target) const
{
	glBlitFramebuffer( 0, 0, source.x, source.y, 0, 0, target.x, target.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

//-----------------------------------
// GlContext::SetReadBufferEnabled
//
// Byte alignment requirements for pixel rows in memory
//
void GL_DEVICE_CLASSNAME::SetPixelUnpackAlignment(int32 const val) const
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, val);
}

//---------------------------------
// GlContext::ReadPixels
//
// Read pixels into an array from the current framebuffer
//
void GL_DEVICE_CLASSNAME::ReadPixels(ivec2 const pos, ivec2 const size, E_ColorFormat const format, E_DataType const type, void* data) const
{
	glReadPixels(pos.x, pos.y, size.x, size.y, GL_DEVICE_NS::ConvColorFormat(format), GL_DEVICE_NS::ConvDataType(type), data);
}

//---------------------------------
// GlContext::DebugPushGroup
//
// Push a debugging scope onto the stack e.g for renderdoc
//
void GL_DEVICE_CLASSNAME::DebugPushGroup(std::string const& message, bool const isThirdParty) const
{
	GLenum const source = isThirdParty ? GL_DEBUG_SOURCE_THIRD_PARTY : GL_DEBUG_SOURCE_APPLICATION;
	glPushDebugGroup(source, GetHash(message), static_cast<GLsizei>(message.length()), message.c_str());
}

//---------------------------------
// GlContext::DebugPopGroup
//
// Pop the debugging scope. <see above>
//
void GL_DEVICE_CLASSNAME::DebugPopGroup() const
{
	glPopDebugGroup();
}


// Utility
///////////


//---------------------------------
// GlContext::EnOrDisAble
//
// Enable or disable a setting
//
void GL_DEVICE_CLASSNAME::EnOrDisAble(bool &state, bool enabled, GLenum glState)
{
	if (!(state == enabled))
	{
		state = enabled;
		if (state)
		{
			glEnable(glState);
		}
		else
		{
			glDisable(glState);
		}
	}
}

//---------------------------------
// GlContext::EnOrDisAbleIndexed
//
// Enable or disable a setting at a specific index
// Note the index should be validated before calling this function
//
void GL_DEVICE_CLASSNAME::EnOrDisAbleIndexed(std::vector<bool> &state, bool enabled, GLenum glState, uint32 index)
{
	if (!(state[index] == enabled))
	{
		state[index] = enabled;
		if (enabled)
		{
			glEnablei(glState, index);
		}
		else
		{
			glDisablei(glState, index);
		}
	}
}


} // namespace rhi
} // namespace et
