#include <map>

#include "GraphicsContextApi.h"

#ifndef GL_CONTEXT_CLASSNAME
#error you must declare the name of the class with #define GL_CONTEXT_CLASSNAME YourGlImplementationClassName
#endif GL_CONTEXT_CLASSNAME

#ifndef GL_CONTEXT_NS
#	define GL_CONTEXT_NS GL_CONTEXT_CLASSNAME ## _ns
#endif GL_CONTEXT_NS


namespace et {
namespace render {


//---------------------------------
// GL_CONTEXT_NS
//
// conversion functions
//
namespace GL_CONTEXT_NS {

	GLenum ConvTextureType(E_TextureType const type);
	GLenum ConvDataType(E_DataType const type);
	GLenum ConvShaderType(E_ShaderType const type);
	GLenum ConvDrawMode(E_DrawMode const mode);
	GLenum ConvBufferType(E_BufferType const type);
	GLenum ConvUsageHint(E_UsageHint const hint);
	GLenum ConvAccessMode(E_AccessMode const mode);
	GLenum ConvFaceCullMode(E_FaceCullMode const mode);
	GLenum ConvPolygonMode(E_PolygonMode const mode);
	GLenum ConvBlendEquation(E_BlendEquation const equ);
	GLenum ConvBlendFactor(E_BlendFactor const fac);
	GLenum ConvColorFormat(E_ColorFormat const fmt);
	void DeriveLayoutAndType(E_ColorFormat const storageFmt, GLenum& colorLayout, GLenum& dataType);
	GLenum ConvFilter(E_TextureFilterMode const filter);
	GLenum ConvMinFilter(E_TextureFilterMode const minFilter, E_TextureFilterMode const mipFilter, bool const useMip);
	GLenum ConvWrapMode(E_TextureWrapMode const wrap);
	GLenum ConvCompareMode(E_TextureCompareMode const comp);
	GLenum ConvDepthStencilFunction(E_DepthFunc const func);
	GLenum ConvStencilOp(E_StencilOp const op);

	E_ParamType ParseParamType(GLenum const param);

} // namespace GL_CONTEXT_NS


//---------------------------------
// GlContext
//
// Wrapper for all graphics API calls, avoids resubmitting api calls by caching some of the state CPU side
//
class GL_CONTEXT_CLASSNAME final : public I_GraphicsContextApi
{
	// definitions
	//-------------

	//----------------------------
	// TextureUnitCache
	//
	// Manages assignment of bindings for textures
	//
	class TextureUnitCache
	{
		// definitions
		//-------------
		struct Unit
		{
			Unit(T_TextureUnit const u = 0u, T_TextureLoc const t = 0u) : unit(u), texture(t) {}

			T_TextureUnit unit;
			T_TextureLoc texture;
		};

		typedef std::list<Unit> T_LruList;

		// construct destruct
		//--------------------
	public:
		TextureUnitCache(size_t const size = 0u);

		// functionality
		//---------------
		T_TextureUnit Bind(E_TextureType const type, T_TextureLoc const tex, bool const ensureActive);
		void Unbind(E_TextureType const type, T_TextureLoc const tex);
		void OnTextureDelete(T_TextureLoc const tex);

		// utility
		//---------
	private:
		void EnsureActive(T_TextureUnit const targetUnit);

		// Data
		///////

		T_TextureUnit m_ActiveUnit = 0u;
		size_t m_MaxUnits;

		T_LruList m_List;
		std::unordered_map<T_TextureLoc, T_LruList::iterator> m_Map;
	};

public:

	// init deinit
	//--------------
	GL_CONTEXT_CLASSNAME() : I_GraphicsContextApi() {}
	~GL_CONTEXT_CLASSNAME();

	//===============================
	// Interface implementation
	//===============================

	void Initialize(ivec2 const dimensions) override;

	// State changes
	//--------------
	void SetDepthEnabled(bool const enabled) override;
	void SetBlendEnabled(bool const enabled) override;
	void SetBlendEnabled(bool const enabled, uint32 const index) override;
	void SetBlendEnabled(std::vector<bool> const& blendBuffers) override;
	void SetStencilEnabled(bool const enabled) override;
	void SetCullEnabled(bool const enabled) override;
	void SetScissorEnabled(bool const enabled) override;

	void SetColorMask(T_ColorFlags const flags) override;
	void SetDepthMask(bool const flag) override;
	void SetStencilMask(uint32 const mask) override;

	void SetFaceCullingMode(E_FaceCullMode const cullMode) override;
	void SetPolygonMode(E_FaceCullMode const cullMode, E_PolygonMode const mode) override;

	void SetBlendEquation(E_BlendEquation const equation) override;
	void SetBlendFunction(E_BlendFactor const sFactor, E_BlendFactor const dFactor) override;
	void SetBlendFunctionSeparate(E_BlendFactor const sRGB, E_BlendFactor const sAlpha, E_BlendFactor const dRGB, E_BlendFactor const dAlpha) override;

	void SetDepthFunction(E_DepthFunc const func) override;

	void SetStencilFunction(T_StencilFunc const func, int32 const reference, uint32 const mask) override;
	void SetStencilOperation(E_StencilOp const sFail, E_StencilOp const dFail, E_StencilOp const dsPass) override;

	void SetScissor(ivec2 const pos, ivec2 const size) override;

	void SetViewport(ivec2 const pos, ivec2 const size) override;
	void GetViewport(ivec2& pos, ivec2& size) override;

	void SetClearColor(vec4 const& col) override;

	void SetShader(ShaderData const* pShader) override;

	void BindFramebuffer(T_FbLoc const handle) override;
	void BindReadFramebuffer(T_FbLoc const handle) override;
	void BindDrawFramebuffer(T_FbLoc const handle) override;

	void BindRenderbuffer(T_RbLoc const handle) override;

	T_TextureUnit BindTexture(E_TextureType const target, T_TextureLoc const texLoc, bool const ensureActive) override;
	void UnbindTexture(E_TextureType const target, T_TextureLoc const texLoc) override;

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
	void Flush() const override;
	void Finish() const override;
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

	void BindBufferRange(E_BufferType const target,
		uint32 const index,
		T_BufferLoc const buffer,
		size_t const offset,
		size_t const size) const override;

	T_TextureLoc GenerateTexture() const override;
	void DeleteTexture(T_TextureLoc& texLoc) override;
	void UploadTextureData(TextureData& texture, 
		void const* const data, 
		E_ColorFormat const layout, 
		E_DataType const dataType, 
		int32 const mipLevel) override;
	void UploadCompressedTextureData(TextureData& texture, void const* const data, size_t const size, int32 const mipLevel) override;
	void AllocateTextureStorage(TextureData& texture) override;
	void SetTextureParams(TextureData const& texture, 
		TextureParameters& prev, 
		TextureParameters const& next, 
		bool const force) override;
	void GenerateMipMaps(TextureData const& texture, uint8& mipLevels) override;
	T_TextureHandle GetTextureHandle(T_TextureLoc const texLoc) const override;
	void SetTextureHandleResidency(T_TextureHandle const handle, bool const isResident) const override;
	void GetTextureData(TextureData const& texture, 
		uint8 const mipLevel, 
		E_ColorFormat const format, 
		E_DataType const dataType, 
		void* const data) override;

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

	T_BlockIndex GetUniformBlockIndex(T_ShaderLoc const program, std::string const& blockName) const override;
	bool IsBlockIndexValid(T_BlockIndex const index) const override;
	std::vector<std::string> GetUniformBlockNames(T_ShaderLoc const program) const override;
	std::vector<int32> GetUniformIndicesForBlock(T_ShaderLoc const program, T_BlockIndex const blockIndex) const override;

	void SetUniformBlockBinding(T_ShaderLoc const program, T_BlockIndex const blockIndex, uint32 const bindingIndex) const override;

	int32 GetAttributeCount(T_ShaderLoc const program) const override;
	int32 GetUniformCount(T_ShaderLoc const program) const override;
	void GetActiveUniforms(T_ShaderLoc const program, uint32 const index, std::vector<UniformDescriptor>& uniforms) const override;
	void GetActiveAttribute(T_ShaderLoc const program, uint32 const index, AttributeDescriptor& info) const override;
	T_AttribLoc GetAttributeLocation(T_ShaderLoc const program, std::string const& name) const override;

	void PopulateUniform(T_ShaderLoc const program, T_UniformLoc const location, E_ParamType const type, void* data) const override;

	void UploadUniform(T_UniformLoc const location, bool const data) const override;
	void UploadUniform(T_UniformLoc const location, int32 const data) const override;
	void UploadUniform(T_UniformLoc const location, uint32 const data) const override;
	void UploadUniform(T_UniformLoc const location, float const data) const override;
	void UploadUniform(T_UniformLoc const location, vec2 const data) const override;
	void UploadUniform(T_UniformLoc const location, vec3 const& data) const override;
	void UploadUniform(T_UniformLoc const location, vec4 const& data) const override;
	void UploadUniform(T_UniformLoc const location, mat3 const& data) const override;
	void UploadUniform(T_UniformLoc const location, mat4 const& data) const override;

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
	void DeleteFramebuffers(int32 const n, T_FbLoc *ids) override;

	void GenRenderBuffers(int32 const n, T_RbLoc *ids) const override;
	void DeleteRenderBuffers(int32 const n, T_RbLoc *ids) override;

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

	void ReadPixels(ivec2 const pos, ivec2 const size, E_ColorFormat const format, E_DataType const type, void* data) const override;

	void DebugPushGroup(std::string const& message, bool const isThirdParty = false) const override;
	void DebugPopGroup() const override;

private:

	//=========
	// Utility
	//=========

	void EnOrDisAble(bool &state, bool enabled, GLenum glState);

	//The index should be validated before calling this function, only blend and scissor test can be larger than 0
	//the index for blending must be smaller than max draw buffers too
	void EnOrDisAbleIndexed(std::vector<bool> &state, bool enabled, GLenum glState, uint32 index);


	//================
	// Current State
	//================

	// Data
	///////

	bool m_IsInitialized = false;


	T_FbLoc m_ReadFramebuffer = 0;
	T_FbLoc m_DrawFramebuffer = 0;

	T_RbLoc m_Renderbuffer = 0u;

	int32 m_MaxDrawBuffers; //Depends on gpu and drivers

	bool m_DepthTestEnabled = false;

	bool m_CullFaceEnabled = false;
	E_FaceCullMode m_CullFaceMode = E_FaceCullMode::Back;
	std::vector<E_PolygonMode> m_PolygonMode; // one for each face cull mode
	
	bool m_StencilTestEnabled = false;

	bool m_BlendEnabled = false;
	bool m_IndividualBlend = false;
	std::vector<bool> m_BlendEnabledIndexed;

	T_ColorFlags m_ColorMask = E_ColorFlag::CF_All;
	bool m_DepthMask = true;
	uint32 m_StencilMask = 0xFFFFFFFFu;

	E_BlendEquation m_BlendEquationRGB = E_BlendEquation::Add;
	E_BlendEquation m_BlendEquationAlpha = E_BlendEquation::Add;
	E_BlendFactor m_BlendFuncSFactor = E_BlendFactor::One;
	E_BlendFactor m_BlendFuncSFactorAlpha = E_BlendFactor::One;
	E_BlendFactor m_BlendFuncDFactor = E_BlendFactor::Zero;
	E_BlendFactor m_BlendFuncDFactorAlpha = E_BlendFactor::Zero;

	E_DepthFunc m_DepthFunc = E_DepthFunc::Less;

	T_StencilFunc m_StencilFunc = T_StencilFunc::Always;
	int32 m_StencilRef = 0;
	uint32 m_StencilFuncMask = 0xFFFFFFFFu;
	E_StencilOp m_StencilSFail = E_StencilOp::Keep;
	E_StencilOp m_StencilDFail = E_StencilOp::Keep;
	E_StencilOp m_StencilDSPass = E_StencilOp::Keep;

	bool m_ScissorEnabled = false;
	ivec2 m_ScissorPosition = ivec2(0);
	ivec2 m_ScissorSize; // initialize with values used during context creation

	ivec2 m_ViewportPosition = ivec2(0);
	ivec2 m_ViewportSize; // initialize with values used during context creation

	vec4 m_ClearColor = vec4(0);

	ShaderData const* m_pBoundShader = nullptr;

	TextureUnitCache m_TextureUnits;
	bool m_BindlessTexturesAvailable = false;

	T_ArrayLoc m_VertexArray = 0;
	std::map<E_BufferType, T_BufferLoc> m_BufferTargets;

	float m_LineWidth = 1.f;
};


} // namespace render
} // namespace et
