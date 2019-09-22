#pragma once

#include <map>
#include <glad/glad.h>

#include <Engine/Graphics/VertexInfo.h>


// forward declarations
class ShaderData;
class TextureData;

class I_Uniform;
template<typename T>
class Uniform;


typedef int32 T_AttribLoc;
typedef int32 T_UniformLoc;
typedef uint32 T_ShaderLoc;

typedef uint32 T_TextureLoc;

typedef uint32 T_BufferLoc;
typedef uint32 T_ArrayLoc;

typedef uint32 T_FbLoc;
typedef uint32 T_RbLoc;


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
	Color	= 1 << 0,
	Depth	= 1 << 1,
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
// GraphicsApiContext
//
// Wrapper for all graphics API calls, avoids resubmitting api calls by caching some of the state CPU side
//
class GraphicsApiContext
{
public:

	// init deinit
	//--------------
	GraphicsApiContext() = default;

	void Initialize();

	// State changes
	//--------------
	void SetDepthEnabled(bool const enabled);
	void SetBlendEnabled(bool const enabled);
	void SetBlendEnabled(bool const enabled, uint32 const index);
	void SetBlendEnabled(std::vector<bool> const& blendBuffers);
	void SetStencilEnabled(bool const enabled);
	void SetCullEnabled(bool const enabled);

	void SetSeamlessCubemapsEnabled(bool const enabled);

	void SetFaceCullingMode(E_FaceCullMode const cullMode);
	void SetBlendEquation(GLenum equation);
	void SetBlendFunction(GLenum sFactor, GLenum dFactor);

	void SetViewport(ivec2 const pos, ivec2 const size);
	void GetViewport(ivec2& pos, ivec2& size);

	void SetClearColor(vec4 const& col);

	void SetShader(ShaderData const* pShader);

	void BindFramebuffer(T_FbLoc const handle);
	void BindReadFramebuffer(T_FbLoc const handle);
	void BindDrawFramebuffer(T_FbLoc const handle);

	void BindRenderbuffer(T_RbLoc const handle);

	void SetActiveTexture(uint32 const unit);
	void BindTexture(E_TextureType const target, T_TextureLoc const handle);

	//Makes sure that a texture is bound to a units target for shading, 
	//only changes active texture unit if the texture was not bound yet
	void LazyBindTexture(uint32 const unit, E_TextureType const target, T_TextureLoc const handle);

	void BindVertexArray(T_ArrayLoc const vertexArray);
	void BindBuffer(E_BufferType const target, T_BufferLoc const buffer);

	void SetLineWidth(float const lineWidth);

	//Draw Calls
	//--------------
	void DrawArrays(E_DrawMode const mode, uint32 const first, uint32 const count);
	void DrawElements(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices);
	void DrawElementsInstanced(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices, uint32 const primcount);

	// other commands
	//--------------
	void Flush() const;
	void Clear(T_ClearFlags const mask) const;

	T_ArrayLoc CreateVertexArray() const;
	T_BufferLoc CreateBuffer() const;

	void DeleteVertexArray(T_ArrayLoc& loc) const;
	void DeleteBuffer(T_BufferLoc& loc) const;

	void SetBufferData(E_BufferType const target, int64 const size, void const* const data, E_UsageHint const usage) const;
	void SetVertexAttributeArrayEnabled(uint32 const index, bool const enabled) const; // could at some point be a member on VertexArray data object

	void* MapBuffer(E_BufferType const target, E_AccessMode const access) const;
	void UnmapBuffer(E_BufferType const target) const;

	T_TextureLoc GenerateTexture() const;
	void DeleteTexture(T_TextureLoc& handle) const;
	void SetTextureData(TextureData& texture, void* data);
	void SetTextureParams(TextureData const& texture, uint8& mipLevels, TextureParameters& prev, TextureParameters const& next, bool const force);

	T_ShaderLoc CreateShader(E_ShaderType const type) const;
	T_ShaderLoc CreateProgram() const;
	void DeleteShader(T_ShaderLoc const shader);
	void DeleteProgram(T_ShaderLoc const program);

	void CompileShader(T_ShaderLoc const shader, std::string const& source) const;
	void BindFragmentDataLocation(T_ShaderLoc const program, uint32 const colorNumber, std::string const& name) const;
	void AttachShader(T_ShaderLoc const program, T_ShaderLoc const shader) const;
	void LinkProgram(T_ShaderLoc const program) const;

	bool IsShaderCompiled(T_ShaderLoc const shader) const;
	void GetShaderInfo(T_ShaderLoc const shader, std::string& info) const;

	int32 GetAttributeCount(T_ShaderLoc const program) const;
	int32 GetUniformCount(T_ShaderLoc const program) const;
	void GetActiveUniforms(T_ShaderLoc const program, uint32 const index, std::vector<I_Uniform*>& uniforms) const;
	void GetActiveAttribute(T_ShaderLoc const program, uint32 const index, AttributeDescriptor& info) const;
	T_AttribLoc GetAttributeLocation(T_ShaderLoc const program, std::string const& name) const;

	void UploadUniform(const Uniform<bool> &uniform);
	void UploadUniform(const Uniform<int32> &uniform);
	void UploadUniform(const Uniform<uint32> &uniform);
	void UploadUniform(const Uniform<float> &uniform);
	void UploadUniform(const Uniform<vec2> &uniform);
	void UploadUniform(const Uniform<vec3> &uniform);
	void UploadUniform(const Uniform<vec4> &uniform);
	void UploadUniform(const Uniform<mat3> &uniform);
	void UploadUniform(const Uniform<mat4> &uniform);

	void DefineVertexAttributePointer(uint32 const index, int32 const size, E_DataType const type, bool const norm, int32 const stride, size_t const offset) const;
	void DefineVertexAttribIPointer(uint32 const index, int32 const size, E_DataType const type, int32 const stride, size_t const offset) const;
	void DefineVertexAttribDivisor(uint32 const index, uint32 const divisor) const;

	void GenFramebuffers(int32 const n, T_FbLoc *ids) const;
	void DeleteFramebuffers(int32 const n, T_FbLoc *ids) const;

	void GenRenderBuffers(int32 const n, T_RbLoc *ids) const;
	void DeleteRenderBuffers(int32 const n, T_RbLoc *ids) const;

	void SetRenderbufferStorage(E_RenderBufferFormat const format, ivec2 const dimensions) const;

	void LinkTextureToFbo(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const;
	void LinkTextureToFbo2D(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const; // link to current draw FB with a color attachment
	void LinkCubeMapFaceToFbo2D(uint8 const face, T_TextureLoc const texHandle, int32 const level) const;
	void LinkTextureToFboDepth(T_TextureLoc const texHandle) const;

	void LinkRenderbufferToFbo(E_RenderBufferFormat const attachment, uint32 const rboHandle) const;

	void SetDrawBufferCount(size_t const count) const;
	void SetReadBufferEnabled(bool const val) const;

	bool IsFramebufferComplete() const;

	void CopyDepthReadToDrawFbo(ivec2 const source, ivec2 const target) const;

	void SetPixelUnpackAlignment(int32 const val) const;

private:

	void EnOrDisAble(bool &state, bool enabled, GLenum glState);

	//The index should be validated before calling this function, only blend and scissor test can be larger than 0
	//the index for blending must be smaller than max draw buffers too
	void EnOrDisAbleIndexed(std::vector<bool> &state, bool enabled, GLenum glState, uint32 index);

	uint32 GetTexTarget(E_TextureType const type) const;
	uint32 GetTypeId(E_DataType const type) const;

	GLenum ConvShaderType(E_ShaderType const type) const;
	GLenum ConvDrawMode(E_DrawMode const mode) const;
	GLenum ConvBufferType(E_BufferType const type) const;
	GLenum ConvUsageHint(E_UsageHint const hint) const;
	GLenum ConvAccessMode(E_AccessMode const mode) const;
	GLenum ConvFaceCullMode(E_FaceCullMode const mode) const;

	T_FbLoc m_ReadFramebuffer = 0;
	T_FbLoc m_DrawFramebuffer = 0;

	T_RbLoc m_Renderbuffer = 0u;

	int32 m_MaxDrawBuffers; //Depends on gpu and drivers

	bool m_DepthTestEnabled = false;

	bool m_CullFaceEnabled = false;
	E_FaceCullMode m_CullFaceMode = E_FaceCullMode::Back;
	
	bool m_StencilTestEnabled = false;

	bool m_SeamlessCubemapsEnabled = false;

	bool m_BlendEnabled = false;
	bool m_IndividualBlend = false;
	std::vector<bool> m_BlendEnabledIndexed;
	GLenum m_BlendEquationRGB = GL_FUNC_ADD;
	GLenum m_BlendEquationAlpha = GL_FUNC_ADD;
	GLenum m_BlendFuncSFactor = GL_ONE;
	GLenum m_BlendFuncDFactor = GL_ZERO;

	ivec2 m_ViewportPosition = ivec2(0);
	ivec2 m_ViewportSize; //initialize with values used during context creation

	vec4 m_ClearColor = vec4(0);

	ShaderData const* m_pBoundShader = nullptr;

	uint32 m_ActiveTexture = 0;
	int32 m_NumTextureUnits; //depends on gpu and drivers
	std::vector<std::map<E_TextureType, T_TextureLoc> > m_pTextureUnits; // #todo: in the future, abstract texture data here to support all types of textures

	T_ArrayLoc m_VertexArray = 0;
	std::map<E_BufferType, T_BufferLoc> m_BufferTargets;

	float m_LineWidth = 1.f;
};