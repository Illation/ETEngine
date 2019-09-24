#include <map>

#include "GraphicsApiContext.h"

#ifndef GL_CONTEXT_CLASSNAME
#error you must declare the name of the class with #define GL_CONTEXT_CLASSNAME YourGlImplementationClassName
#endif GL_CONTEXT_CLASSNAME

//---------------------------------
// GlContext
//
// Wrapper for all graphics API calls, avoids resubmitting api calls by caching some of the state CPU side
//
class GL_CONTEXT_CLASSNAME final : public I_GraphicsApiContext
{
public:

	// init deinit
	//--------------
	GL_CONTEXT_CLASSNAME() : I_GraphicsApiContext() {}
	~GL_CONTEXT_CLASSNAME() = default;

	//===============================
	// Interface implementation
	//===============================

	void Initialize() override;

	// State changes
	//--------------
	void SetDepthEnabled(bool const enabled) override;
	void SetBlendEnabled(bool const enabled) override;
	void SetBlendEnabled(bool const enabled, uint32 const index) override;
	void SetBlendEnabled(std::vector<bool> const& blendBuffers) override;
	void SetStencilEnabled(bool const enabled) override;
	void SetCullEnabled(bool const enabled) override;

	void SetSeamlessCubemapsEnabled(bool const enabled) override;

	void SetFaceCullingMode(E_FaceCullMode const cullMode) override;
	void SetBlendEquation(E_BlendEquation const equation) override;
	void SetBlendFunction(E_BlendFactor const sFactor, E_BlendFactor const dFactor) override;

	void SetViewport(ivec2 const pos, ivec2 const size) override;
	void GetViewport(ivec2& pos, ivec2& size) override;

	void SetClearColor(vec4 const& col) override;

	void SetShader(ShaderData const* pShader) override;

	void BindFramebuffer(T_FbLoc const handle) override;
	void BindReadFramebuffer(T_FbLoc const handle) override;
	void BindDrawFramebuffer(T_FbLoc const handle) override;

	void BindRenderbuffer(T_RbLoc const handle) override;

	void SetActiveTexture(uint32 const unit) override;
	void BindTexture(E_TextureType const target, T_TextureLoc const handle) override;

	//Makes sure that a texture is bound to a units target for shading, 
	//only changes active texture unit if the texture was not bound yet
	void LazyBindTexture(uint32 const unit, E_TextureType const target, T_TextureLoc const handle) override;

	void BindVertexArray(T_ArrayLoc const vertexArray) override;
	void BindBuffer(E_BufferType const target, T_BufferLoc const buffer) override;

	void SetLineWidth(float const lineWidth) override;

	T_FbLoc GetActiveFramebuffer() override;

	//Draw Calls
	//--------------
	void DrawArrays(E_DrawMode const mode, uint32 const first, uint32 const count) override;
	void DrawElements(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices) override;
	void DrawElementsInstanced(E_DrawMode const mode, 
		uint32 const count, 
		E_DataType const type, 
		const void * indices, 
		uint32 const primcount) override;

	// other commands
	//--------------
	void Flush() const;
	void Clear(T_ClearFlags const mask) const override;

	T_ArrayLoc CreateVertexArray() const override;
	T_BufferLoc CreateBuffer() const override;

	void DeleteVertexArray(T_ArrayLoc& loc) const override;
	void DeleteBuffer(T_BufferLoc& loc) const override;

	void SetBufferData(E_BufferType const target, 
		int64 const size, 
		void const* const data, 
		E_UsageHint const usage) const override;
	void SetVertexAttributeArrayEnabled(uint32 const index, bool const enabled) const override; 

	void* MapBuffer(E_BufferType const target, E_AccessMode const access) const override;
	void UnmapBuffer(E_BufferType const target) const override;

	T_TextureLoc GenerateTexture() const override;
	void DeleteTexture(T_TextureLoc& handle) const override;
	void SetTextureData(TextureData& texture, void* data) override;
	void SetTextureParams(TextureData const& texture, 
		uint8& mipLevels, 
		TextureParameters& prev, 
		TextureParameters const& next, 
		bool const force) override;

	T_ShaderLoc CreateShader(E_ShaderType const type) const override;
	T_ShaderLoc CreateProgram() const override;
	void DeleteShader(T_ShaderLoc const shader) override;
	void DeleteProgram(T_ShaderLoc const program) override;

	void CompileShader(T_ShaderLoc const shader, std::string const& source) const override;
	void BindFragmentDataLocation(T_ShaderLoc const program, uint32 const colorNumber, std::string const& name) const override;
	void AttachShader(T_ShaderLoc const program, T_ShaderLoc const shader) const override;
	void LinkProgram(T_ShaderLoc const program) const override;

	bool IsShaderCompiled(T_ShaderLoc const shader) const override;
	void GetShaderInfo(T_ShaderLoc const shader, std::string& info) const override;

	int32 GetAttributeCount(T_ShaderLoc const program) const override;
	int32 GetUniformCount(T_ShaderLoc const program) const override;
	void GetActiveUniforms(T_ShaderLoc const program, uint32 const index, std::vector<I_Uniform*>& uniforms) const override;
	void GetActiveAttribute(T_ShaderLoc const program, uint32 const index, AttributeDescriptor& info) const override;
	T_AttribLoc GetAttributeLocation(T_ShaderLoc const program, std::string const& name) const override;

	void UploadUniform(const Uniform<bool> &uniform) override;
	void UploadUniform(const Uniform<int32> &uniform) override;
	void UploadUniform(const Uniform<uint32> &uniform) override;
	void UploadUniform(const Uniform<float> &uniform) override;
	void UploadUniform(const Uniform<vec2> &uniform) override;
	void UploadUniform(const Uniform<vec3> &uniform) override;
	void UploadUniform(const Uniform<vec4> &uniform) override;
	void UploadUniform(const Uniform<mat3> &uniform) override;
	void UploadUniform(const Uniform<mat4> &uniform) override;

	void DefineVertexAttributePointer(uint32 const index, 
		int32 const size, 
		E_DataType const type, 
		bool const norm, 
		int32 const stride, 
		size_t const offset) const override;
	void DefineVertexAttribIPointer(uint32 const index, 
		int32 const size, 
		E_DataType const type, 
		int32 const stride, 
		size_t const offset) const override;
	void DefineVertexAttribDivisor(uint32 const index, uint32 const divisor) const override;

	void GenFramebuffers(int32 const n, T_FbLoc *ids) const override;
	void DeleteFramebuffers(int32 const n, T_FbLoc *ids) const override;

	void GenRenderBuffers(int32 const n, T_RbLoc *ids) const override;
	void DeleteRenderBuffers(int32 const n, T_RbLoc *ids) const override;

	void SetRenderbufferStorage(E_RenderBufferFormat const format, ivec2 const dimensions) const override;

	void LinkTextureToFbo(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const override;
	void LinkTextureToFbo2D(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const override; 
	void LinkCubeMapFaceToFbo2D(uint8 const face, T_TextureLoc const texHandle, int32 const level) const override;
	void LinkTextureToFboDepth(T_TextureLoc const texHandle) const override;

	void LinkRenderbufferToFbo(E_RenderBufferFormat const attachment, uint32 const rboHandle) const override;

	void SetDrawBufferCount(size_t const count) const override;
	void SetReadBufferEnabled(bool const val) const override;

	bool IsFramebufferComplete() const override;

	void CopyDepthReadToDrawFbo(ivec2 const source, ivec2 const target) const override;

	void SetPixelUnpackAlignment(int32 const val) const override;

	void SetDepthFunction(E_DepthFunc const func) const override;

	void ReadPixels(ivec2 const pos, ivec2 const size, E_ColorFormat const format, E_DataType const type, void* data) const override;

private:

	//===============================
	// Open GL Conversions
	//===============================

	void EnOrDisAble(bool &state, bool enabled, GLenum glState);

	//The index should be validated before calling this function, only blend and scissor test can be larger than 0
	//the index for blending must be smaller than max draw buffers too
	void EnOrDisAbleIndexed(std::vector<bool> &state, bool enabled, GLenum glState, uint32 index);

	GLenum ConvTextureType(E_TextureType const type) const;
	GLenum ConvDataType(E_DataType const type) const;
	GLenum ConvShaderType(E_ShaderType const type) const;
	GLenum ConvDrawMode(E_DrawMode const mode) const;
	GLenum ConvBufferType(E_BufferType const type) const;
	GLenum ConvUsageHint(E_UsageHint const hint) const;
	GLenum ConvAccessMode(E_AccessMode const mode) const;
	GLenum ConvFaceCullMode(E_FaceCullMode const mode) const;
	GLenum ConvBlendEquation(E_BlendEquation const equ) const;
	GLenum ConvBlendFactor(E_BlendFactor const fac) const;
	GLenum ConvColorFormat(E_ColorFormat const fmt) const;
	GLenum ConvFilter(E_TextureFilterMode const filter) const;
	GLenum ConvMinFilter(E_TextureFilterMode const minFilter, E_TextureFilterMode const mipFilter, bool const useMip) const;
	GLenum ConvWrapMode(E_TextureWrapMode const wrap) const;
	GLenum ConvCompareMode(E_TextureCompareMode const comp) const;
	GLenum ConvDepthFunction(E_DepthFunc const func) const;

	//================
	// Current State
	//================

	// Data
	///////

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
	E_BlendEquation m_BlendEquationRGB = E_BlendEquation::Add;
	E_BlendEquation m_BlendEquationAlpha = E_BlendEquation::Add;
	E_BlendFactor m_BlendFuncSFactor = E_BlendFactor::One;
	E_BlendFactor m_BlendFuncDFactor = E_BlendFactor::Zero;

	ivec2 m_ViewportPosition = ivec2(0);
	ivec2 m_ViewportSize; //initialize with values used during context creation

	vec4 m_ClearColor = vec4(0);

	ShaderData const* m_pBoundShader = nullptr;

	uint32 m_ActiveTexture = 0;
	int32 m_NumTextureUnits; //depends on gpu and drivers
	// #todo: in the future, abstract texture data here to support all types of textures
	std::vector<std::map<E_TextureType, T_TextureLoc> > m_pTextureUnits; 

	T_ArrayLoc m_VertexArray = 0;
	std::map<E_BufferType, T_BufferLoc> m_BufferTargets;

	float m_LineWidth = 1.f;
};

