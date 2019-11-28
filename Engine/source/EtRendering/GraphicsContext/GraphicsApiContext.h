#pragma once

#include <EtRendering/GraphicsTypes/VertexInfo.h>
#include <EtRendering/GraphicsTypes/TextureParameters.h>

#include "GraphicsTypes.h"


// forward declarations
class ShaderData;
class TextureData;

class I_Uniform;
template<typename T>
class Uniform;


//---------------------------------
// I_GraphicsApiContext
//
// Interface for any graphics API wrapper
//
class I_GraphicsApiContext
{
public:

	// init deinit
	//--------------
	I_GraphicsApiContext() = default;
	virtual ~I_GraphicsApiContext() = default;

	virtual void Initialize(ivec2 const dimensions) = 0;

	// State changes
	//--------------
	virtual void SetDepthEnabled(bool const enabled) = 0;
	virtual void SetBlendEnabled(bool const enabled) = 0;
	virtual void SetBlendEnabled(bool const enabled, uint32 const index) = 0;
	virtual void SetBlendEnabled(std::vector<bool> const& blendBuffers) = 0;
	virtual void SetStencilEnabled(bool const enabled) = 0;
	virtual void SetCullEnabled(bool const enabled) = 0;

	virtual void SetSeamlessCubemapsEnabled(bool const enabled) = 0;

	virtual void SetFaceCullingMode(E_FaceCullMode const cullMode) = 0;
	virtual void SetBlendEquation(E_BlendEquation const equation) = 0;
	virtual void SetBlendFunction(E_BlendFactor const sFactor, E_BlendFactor const dFactor) = 0;

	virtual void SetViewport(ivec2 const pos, ivec2 const size) = 0;
	virtual void GetViewport(ivec2& pos, ivec2& size) = 0;

	virtual void SetClearColor(vec4 const& col) = 0;

	virtual void SetShader(ShaderData const* pShader) = 0;

	virtual void BindFramebuffer(T_FbLoc const handle) = 0;
	virtual void BindReadFramebuffer(T_FbLoc const handle) = 0;
	virtual void BindDrawFramebuffer(T_FbLoc const handle) = 0;

	virtual void BindRenderbuffer(T_RbLoc const handle) = 0;

	virtual void SetActiveTexture(uint32 const unit) = 0;
	virtual void BindTexture(E_TextureType const target, T_TextureLoc const handle) = 0;

	//Makes sure that a texture is bound to a units target for shading, 
	//only changes active texture unit if the texture was not bound yet
	virtual void LazyBindTexture(uint32 const unit, E_TextureType const target, T_TextureLoc const handle) = 0;

	virtual void BindVertexArray(T_ArrayLoc const vertexArray) = 0;
	virtual void BindBuffer(E_BufferType const target, T_BufferLoc const buffer) = 0;

	virtual void SetLineWidth(float const lineWidth) = 0;

	virtual T_FbLoc GetActiveFramebuffer() = 0;

	//Draw Calls
	//--------------
	virtual void DrawArrays(E_DrawMode const mode, uint32 const first, uint32 const count) = 0;
	virtual void DrawElements(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices) = 0;
	virtual void DrawElementsInstanced(E_DrawMode const mode, 
		uint32 const count, 
		E_DataType const type, 
		const void * indices, 
		uint32 const primcount) = 0;

	// other commands
	//--------------
	virtual void Flush() const = 0;
	virtual void Finish() const = 0;
	virtual void Clear(T_ClearFlags const mask) const = 0;

	virtual T_ArrayLoc CreateVertexArray() const = 0;
	virtual T_BufferLoc CreateBuffer() const = 0;

	virtual void DeleteVertexArray(T_ArrayLoc& loc) const = 0;
	virtual void DeleteBuffer(T_BufferLoc& loc) const = 0;

	virtual void SetBufferData(E_BufferType const target, int64 const size, void const* const data, E_UsageHint const usage) const = 0;
	// could at some point be a member on VertexArray data object
	virtual void SetVertexAttributeArrayEnabled(uint32 const index, bool const enabled) const = 0; 

	virtual void* MapBuffer(E_BufferType const target, E_AccessMode const access) const = 0;
	virtual void UnmapBuffer(E_BufferType const target) const = 0;

	virtual void BindBufferRange(E_BufferType const target, 
		uint32 const index, 
		T_BufferLoc const buffer, 
		size_t const offset, 
		size_t const size) const = 0;

	virtual T_TextureLoc GenerateTexture() const = 0;
	virtual void DeleteTexture(T_TextureLoc& handle) = 0;
	virtual void SetTextureData(TextureData& texture, void* data) = 0;
	virtual void SetTextureParams(TextureData const& texture, 
		uint8& mipLevels, 
		TextureParameters& prev, 
		TextureParameters const& next, 
		bool const force) = 0;

	virtual T_ShaderLoc CreateShader(E_ShaderType const type) const = 0;
	virtual T_ShaderLoc CreateProgram() const = 0;
	virtual void DeleteShader(T_ShaderLoc const shader) = 0;
	virtual void DeleteProgram(T_ShaderLoc const program) = 0;

	virtual void CompileShader(T_ShaderLoc const shader, std::string const& source) const = 0;
	virtual void BindFragmentDataLocation(T_ShaderLoc const program, uint32 const colorNumber, std::string const& name) const = 0;
	virtual void AttachShader(T_ShaderLoc const program, T_ShaderLoc const shader) const = 0;
	virtual void LinkProgram(T_ShaderLoc const program) const = 0;

	virtual bool IsShaderCompiled(T_ShaderLoc const shader) const = 0;
	virtual void GetShaderInfo(T_ShaderLoc const shader, std::string& info) const = 0;

	virtual T_BlockIndex GetUniformBlockIndex(T_ShaderLoc const program, std::string const& blockName) const = 0;
	virtual bool IsBlockIndexValid(T_BlockIndex const index) const = 0;

	virtual void SetUniformBlockBinding(T_ShaderLoc const program, T_BlockIndex const blockIndex, uint32 const bindingIndex) const = 0;

	virtual int32 GetAttributeCount(T_ShaderLoc const program) const = 0;
	virtual int32 GetUniformCount(T_ShaderLoc const program) const = 0;
	virtual void GetActiveUniforms(T_ShaderLoc const program, uint32 const index, std::vector<I_Uniform*>& uniforms) const = 0;
	virtual void GetActiveAttribute(T_ShaderLoc const program, uint32 const index, AttributeDescriptor& info) const = 0;
	virtual T_AttribLoc GetAttributeLocation(T_ShaderLoc const program, std::string const& name) const = 0;

	virtual void UploadUniform(const Uniform<bool> &uniform) = 0;
	virtual void UploadUniform(const Uniform<int32> &uniform) = 0;
	virtual void UploadUniform(const Uniform<uint32> &uniform) = 0;
	virtual void UploadUniform(const Uniform<float> &uniform) = 0;
	virtual void UploadUniform(const Uniform<vec2> &uniform) = 0;
	virtual void UploadUniform(const Uniform<vec3> &uniform) = 0;
	virtual void UploadUniform(const Uniform<vec4> &uniform) = 0;
	virtual void UploadUniform(const Uniform<mat3> &uniform) = 0;
	virtual void UploadUniform(const Uniform<mat4> &uniform) = 0;

	virtual void DefineVertexAttributePointer(uint32 const index, 
		int32 const size, 
		E_DataType const type, 
		bool const norm, 
		int32 const stride, 
		size_t const offset) const = 0;
	virtual void DefineVertexAttribIPointer(uint32 const index, 
		int32 const size, 
		E_DataType const type, 
		int32 const stride, 
		size_t const offset) const = 0;
	virtual void DefineVertexAttribDivisor(uint32 const index, uint32 const divisor) const = 0;

	virtual void GenFramebuffers(int32 const n, T_FbLoc *ids) const = 0;
	virtual void DeleteFramebuffers(int32 const n, T_FbLoc *ids) = 0;

	virtual void GenRenderBuffers(int32 const n, T_RbLoc *ids) const = 0;
	virtual void DeleteRenderBuffers(int32 const n, T_RbLoc *ids) = 0;

	virtual void SetRenderbufferStorage(E_RenderBufferFormat const format, ivec2 const dimensions) const = 0;

	virtual void LinkTextureToFbo(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const = 0;
	// link to current draw FB with a color attachment
	virtual void LinkTextureToFbo2D(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const = 0; 
	virtual void LinkCubeMapFaceToFbo2D(uint8 const face, T_TextureLoc const texHandle, int32 const level) const = 0;
	virtual void LinkTextureToFboDepth(T_TextureLoc const texHandle) const = 0;

	virtual void LinkRenderbufferToFbo(E_RenderBufferFormat const attachment, uint32 const rboHandle) const = 0;

	virtual void SetDrawBufferCount(size_t const count) const = 0;
	virtual void SetReadBufferEnabled(bool const val) const = 0;

	virtual bool IsFramebufferComplete() const = 0;

	virtual void CopyDepthReadToDrawFbo(ivec2 const source, ivec2 const target) const = 0;

	virtual void SetPixelUnpackAlignment(int32 const val) const = 0;

	virtual void SetDepthFunction(E_DepthFunc const func) const = 0;

	virtual void ReadPixels(ivec2 const pos, ivec2 const size, E_ColorFormat const format, E_DataType const type, void* data) const = 0;

	virtual void DebugPushGroup(std::string const& message, bool const isThirdParty = false) const = 0;
	virtual void DebugPopGroup() const = 0;
};