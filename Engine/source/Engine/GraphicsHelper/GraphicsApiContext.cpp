#include "stdafx.h"

#include "GraphicsApiContext.h"

#include <glad/glad.h>

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/Uniform.h>


//===================
// API Context
//===================


//---------------------------------
// GraphicsApiContext::Initialize
//
// Initialize the state with default values
//
void GraphicsApiContext::Initialize()
{
	m_ViewportSize = Config::GetInstance()->GetWindow().Dimensions;

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_NumTextureUnits);
	for (int32 i = 0; i < m_NumTextureUnits; i++)
	{
		std::map<E_TextureType, T_TextureLoc> targets =
		{
			{ E_TextureType::Texture2D, 0 },
			{ E_TextureType::Texture3D, 0 },
			{ E_TextureType::CubeMap, 0 },
		};
		m_pTextureUnits.push_back(targets);
	}

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &m_MaxDrawBuffers);

	m_BufferTargets =
	{
		{ E_BufferType::Vertex, 0 },
		{ E_BufferType::Index, 0 },
		{ E_BufferType::Uniform, 0 }
	};


	// potentially hook up opengl to the logger
#if defined(ET_DEBUG)
#if defined(GRAPHICS_API_VERBOSE)

	auto glLogCallback = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		UNUSED(source);
		UNUSED(id);
		UNUSED(length);
		UNUSED(userParam);

		LogLevel level = LogLevel::Info;
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:
			level = LogLevel::Error;
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			level = LogLevel::Warning;
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			level = LogLevel::Warning;
			break;
		}

		if (severity == GL_DEBUG_SEVERITY_HIGH)
		{
			level = LogLevel::Error;
		}

		LOG(message, level);
		LOG("");
	}

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glLogCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

#endif
#endif
}

//---------------------------------
// GraphicsApiContext::EnOrDisAble
//
// Enable or disable a setting
//
void GraphicsApiContext::EnOrDisAble(bool &state, bool enabled, GLenum glState)
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
// GraphicsApiContext::EnOrDisAbleIndexed
//
// Enable or disable a setting at a specific index
// Note the index should be validated before calling this function
//
void GraphicsApiContext::EnOrDisAbleIndexed(std::vector<bool> &state, bool enabled, GLenum glState, uint32 index)
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

//---------------------------------
// GraphicsApiContext::GetTexTarget
//
uint32 GraphicsApiContext::GetTexTarget(E_TextureType const type) const
{
	switch (type)
	{
	case E_TextureType::Texture2D:	return GL_TEXTURE_2D;
	case E_TextureType::Texture3D:	return GL_TEXTURE_3D;
	case E_TextureType::CubeMap:	return GL_TEXTURE_CUBE_MAP;
	}

	ET_ASSERT(true, "Unhandled texture type!");
	return GL_NONE;
}

//---------------------------------
// GraphicsApiContext::GetTypeId
//
uint32 GraphicsApiContext::GetTypeId(E_DataType const type) const
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

	ET_ASSERT(true, "Unhandled data type!");
	return GL_NONE;
}

//---------------------------------
// GraphicsApiContext::ConvShaderType
//
GLenum GraphicsApiContext::ConvShaderType(E_ShaderType const type) const
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

	ET_ASSERT(true, "Unhandled shader type!");
	return GL_NONE;
}

//---------------------------------
// GraphicsApiContext::ConvDrawMode
//
GLenum GraphicsApiContext::ConvDrawMode(E_DrawMode const mode) const
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

	ET_ASSERT(true, "Unhandled draw mode!");
	return GL_NONE;
}

//---------------------------------
// GraphicsApiContext::ConvDrawMode
//
// Not all buffer types are currently listed, they will be added when support is needed
//
GLenum GraphicsApiContext::ConvBufferType(E_BufferType const type) const
{
	switch (type)
	{
	case E_BufferType::Vertex:	return GL_ARRAY_BUFFER;
	case E_BufferType::Index:	return GL_ELEMENT_ARRAY_BUFFER;
	case E_BufferType::Uniform:	return GL_UNIFORM_BUFFER;
	}

	ET_ASSERT(true, "Unhandled buffer type!");
	return GL_NONE;
}

//---------------------------------
// GraphicsApiContext::ConvUsageHint
//
GLenum GraphicsApiContext::ConvUsageHint(E_UsageHint const hint) const
{
	switch (hint)
	{
	case E_UsageHint::Static:	return GL_STATIC_DRAW;
	case E_UsageHint::Dynamic:	return GL_DYNAMIC_DRAW;
	}

	ET_ASSERT(true, "Unhandled usage hint!");
	return GL_NONE;
}

GLenum GraphicsApiContext::ConvAccessMode(E_AccessMode const mode) const
{
	switch (mode)
	{
	case E_AccessMode::Read:		return GL_READ_ONLY;
	case E_AccessMode::Write:		return GL_WRITE_ONLY;
	case E_AccessMode::ReadWrite:	return GL_READ_WRITE;
	}

	ET_ASSERT(true, "Unhandled access mode!");
	return GL_NONE;
}

//---------------------------------
// GraphicsApiContext::SetDepthEnabled
//
void GraphicsApiContext::SetDepthEnabled(bool enabled)
{
	EnOrDisAble(m_DepthTestEnabled, enabled, GL_DEPTH_TEST);
}

//---------------------------------
// GraphicsApiContext::SetBlendEnabled
//
// Set the buffers on which blending is enabled
//
void GraphicsApiContext::SetBlendEnabled(const std::vector<bool> &blendBuffers)
{
	for (uint32 i = 0; i < blendBuffers.size(); i++)
	{
		SetBlendEnabled(blendBuffers[i], i);
	}
}

//---------------------------------
// GraphicsApiContext::SetBlendEnabled
//
// Set whether blending is enabled on a specific buffer
//
void GraphicsApiContext::SetBlendEnabled(bool enabled, uint32 index)
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
// GraphicsApiContext::SetBlendEnabled
//
// Set whether blending pixels is enabled in the render pipeline
//
void GraphicsApiContext::SetBlendEnabled(bool enabled)
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
// GraphicsApiContext::SetStencilEnabled
//
void GraphicsApiContext::SetStencilEnabled(bool enabled)
{
	EnOrDisAble(m_StencilTestEnabled, enabled, GL_STENCIL_TEST);
}

//---------------------------------
// GraphicsApiContext::SetCullEnabled
//
void GraphicsApiContext::SetCullEnabled(bool enabled)
{
	EnOrDisAble(m_CullFaceEnabled, enabled, GL_CULL_FACE);
}

//---------------------------------
// GraphicsApiContext::SetSeamlessCubemapsEnabled
//
void GraphicsApiContext::SetSeamlessCubemapsEnabled(bool enabled)
{
	EnOrDisAble(m_SeamlessCubemapsEnabled, enabled, GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

//---------------------------------
// GraphicsApiContext::SetFaceCullingMode
//
// Set the culling mode (front back neither...)
//
void GraphicsApiContext::SetFaceCullingMode(GLenum cullMode)
{
	if (!(m_CullFaceMode == cullMode))
	{
		m_CullFaceMode = cullMode;
		glCullFace(m_CullFaceMode);
	}
}

//---------------------------------
// GraphicsApiContext::SetBlendEquation
//
// Set the equation we use to blend pixels
//
void GraphicsApiContext::SetBlendEquation(GLenum equation)
{
	if (!(m_BlendEquationRGB == equation && m_BlendEquationAlpha == equation))
	{
		m_BlendEquationRGB = equation;
		m_BlendEquationAlpha = equation;
		glBlendEquation(equation);
	}
}

//---------------------------------
// GraphicsApiContext::SetBlendFunction
//
// Set the function we use to blend pixels
//
void GraphicsApiContext::SetBlendFunction(GLenum sFactor, GLenum dFactor)
{
	if (!(m_BlendFuncSFactor == sFactor && m_BlendFuncDFactor == dFactor))
	{
		m_BlendFuncSFactor = sFactor;
		m_BlendFuncDFactor = dFactor;
		glBlendFunc(m_BlendFuncSFactor, m_BlendFuncDFactor);
	}
}

//---------------------------------
// GraphicsApiContext::SetViewport
//
// Set the dimensions of the current opengl viewport (not the engine viewport)
//
void GraphicsApiContext::SetViewport(ivec2 pos, ivec2 size)
{
	if (!(etm::nearEqualsV(m_ViewportPosition, pos) && etm::nearEqualsV(m_ViewportSize, size)))
	{
		m_ViewportPosition = pos;
		m_ViewportSize = size;
		glViewport(pos.x, pos.y, size.x, size.y);
	}
}

//---------------------------------
// GraphicsApiContext::GetViewport
//
// Get the dimensions of the current opengl viewport (not the engine viewport)
//
void GraphicsApiContext::GetViewport(ivec2 &pos, ivec2 &size)
{
	pos = m_ViewportPosition;
	size = m_ViewportSize;
}

//---------------------------------
// GraphicsApiContext::SetClearColor
//
// Set the colour that gets drawn when we clear the viewport
//
void GraphicsApiContext::SetClearColor(vec4 col)
{
	if (!(etm::nearEqualsV(m_ClearColor, col)))
	{
		m_ClearColor = col;
		glClearColor(col.r, col.g, col.b, col.a);
	}
}

//---------------------------------
// GraphicsApiContext::SetShader
//
// Set the shader we draw with
//
void GraphicsApiContext::SetShader(ShaderData const* pShader)
{
	if (!(m_pBoundShader == pShader))
	{
		// #todo: make shaders track uniforms automatically during precompilation and update those here too
		m_pBoundShader = pShader;
		glUseProgram(pShader->GetProgram());
	}
}

//---------------------------------
// GraphicsApiContext::BindFramebuffer
//
// Set the framebuffer we will draw to and read from
//
void GraphicsApiContext::BindFramebuffer(GLuint handle)
{
	if (!(m_ReadFramebuffer == handle && m_DrawFramebuffer == handle))
	{
		m_ReadFramebuffer = handle;
		m_DrawFramebuffer = handle;
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// GraphicsApiContext::BindReadFramebuffer
//
// Set the framebuffer we will read from
//
void GraphicsApiContext::BindReadFramebuffer(GLuint handle)
{
	if (!(m_ReadFramebuffer == handle))
	{
		m_ReadFramebuffer = handle;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// GraphicsApiContext::BindDrawFramebuffer
//
// Set the framebuffer we will draw to
//
void GraphicsApiContext::BindDrawFramebuffer(GLuint handle)
{
	if (!(m_DrawFramebuffer == handle))
	{
		m_DrawFramebuffer = handle;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// GraphicsApiContext::BindFramebuffer
//
// Set the active renderbuffer
//
void GraphicsApiContext::BindRenderbuffer(GLuint handle)
{
	if (handle != m_Renderbuffer)
	{
		m_Renderbuffer = handle;
		glBindRenderbuffer(GL_RENDERBUFFER, handle);
	}
}

//---------------------------------
// GraphicsApiContext::SetActiveTexture
//
// Set the currently active texture unit
//
void GraphicsApiContext::SetActiveTexture(uint32 unit)
{
	if (!(m_ActiveTexture == unit))
	{
		m_ActiveTexture = unit;
		glActiveTexture(GL_TEXTURE0 + unit);
	}
}

//---------------------------------
// GraphicsApiContext::BindTexture
//
// Bind a texture to a target
//
void GraphicsApiContext::BindTexture(E_TextureType const target, T_TextureLoc const handle)
{
	if (!(m_pTextureUnits[m_ActiveTexture][target] == handle))
	{
		m_pTextureUnits[m_ActiveTexture][target] = handle;
		glBindTexture(GetTexTarget(target), handle);
	}
}

//---------------------------------
// GraphicsApiContext::LazyBindTexture
//
// If the current texture isn't bound to its target, bind it
//
void GraphicsApiContext::LazyBindTexture(uint32 const unit, E_TextureType const target, T_TextureLoc const handle)
{
	if (!(m_pTextureUnits[unit][target] == handle))
	{
		SetActiveTexture(unit);
		m_pTextureUnits[m_ActiveTexture][target] = handle;
		glBindTexture(GetTexTarget(target), handle);
	}
}

//---------------------------------
// GraphicsApiContext::BindVertexArray
//
// Bind the current vertex array that the state operates on
//
void GraphicsApiContext::BindVertexArray(T_ArrayLoc const vertexArray)
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
// GraphicsApiContext::BindBuffer
//
// Bind the current buffer that the state operates on
//
void GraphicsApiContext::BindBuffer(E_BufferType const target, T_BufferLoc const buffer)
{
	if (m_BufferTargets[target] != buffer)
	{
		m_BufferTargets[target] = buffer;
		glBindBuffer(ConvBufferType(target), buffer);
	}
}

//---------------------------------
// GraphicsApiContext::SetLineWidth
//
// Set the width of lines that are drawn
//
void GraphicsApiContext::SetLineWidth(float const lineWidth)
{
	if (!etm::nearEquals(m_LineWidth, lineWidth))
	{
		m_LineWidth = lineWidth;
		glLineWidth(m_LineWidth);
	}
}

//---------------------------------
// GraphicsApiContext::DrawArrays
//
// Draw vertex data (without indices)
//
void GraphicsApiContext::DrawArrays(E_DrawMode const mode, uint32 const first, uint32 const count)
{
	glDrawArrays(ConvDrawMode(mode), first, count);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// GraphicsApiContext::DrawElements
//
// Draw vertex data with indices
//
void GraphicsApiContext::DrawElements(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices)
{
	glDrawElements(ConvDrawMode(mode), count, GetTypeId(type), indices);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// GraphicsApiContext::DrawElementsInstanced
//
// Draw instanced vertex data with indices
//
void GraphicsApiContext::DrawElementsInstanced(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices, uint32 const prims)
{
	glDrawElementsInstanced(ConvDrawMode(mode), count, GetTypeId(type), indices, prims);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// GraphicsApiContext::Flush
//
// Force OpenGL to execute all commands now
//
void GraphicsApiContext::Flush() const
{
	glFlush();
}

//---------------------------------
// GraphicsApiContext::Clear
//
// Clear the part of the currently set viewport that is mapped to the mask
//
void GraphicsApiContext::Clear(T_ClearFlags const mask) const
{
	GLbitfield field = 0;

	if (mask & E_ClearFlag::Color)
	{
		field |= GL_COLOR_BUFFER_BIT;
	}

	if (mask & E_ClearFlag::Depth)
	{
		field |= GL_DEPTH_BUFFER_BIT;
	}

	if (mask & E_ClearFlag::Stencil)
	{
		field |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(field);
}

//---------------------------------
// GraphicsApiContext::CreateVertexArray
//
T_ArrayLoc GraphicsApiContext::CreateVertexArray() const
{
	T_ArrayLoc ret;
	glGenVertexArrays(1, &ret);
	return ret;
}

//---------------------------------
// GraphicsApiContext::CreateBuffer
//
T_BufferLoc GraphicsApiContext::CreateBuffer() const
{
	T_BufferLoc ret;
	glGenBuffers(1, &ret);
	return ret;
}

//---------------------------------
// GraphicsApiContext::DeleteVertexArray
//
// Delete a vertex array
//
void GraphicsApiContext::DeleteVertexArray(T_ArrayLoc& loc) const
{
	glDeleteVertexArrays(1, &loc);
}

//---------------------------------
// GraphicsApiContext::DeleteBuffer
//
// Delete a buffer
//
void GraphicsApiContext::DeleteBuffer(T_BufferLoc& loc) const
{
	glDeleteBuffers(1, &loc);
}

//---------------------------------
// GraphicsApiContext::SetBufferData
//
// Fill the buffer at target with an array of data
//
void GraphicsApiContext::SetBufferData(E_BufferType const target, int64 const size, void const* const data, E_UsageHint const usage) const
{
	glBufferData(ConvBufferType(target), size, data, ConvUsageHint(usage));
}

//---------------------------------
// GraphicsApiContext::SetVertexAttributeArrayEnabled
//
// Enable or disable an attribute at an index in the current vertex array
//
void GraphicsApiContext::SetVertexAttributeArrayEnabled(uint32 const index, bool const enabled) const
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
// GraphicsApiContext::DefineVertexAttributePointer
//
// Define the type of data the attribute at a certain index in the current vertex array is mapped to
//
void GraphicsApiContext::DefineVertexAttributePointer(uint32 const index, 
	int32 const size, 
	E_DataType const type, 
	bool const norm, 
	int32 const stride, 
	size_t const offset) const
{
	glVertexAttribPointer(index, size, GetTypeId(type), (norm ? GL_TRUE : GL_FALSE), stride, static_cast<char const*>(0) + offset);
}

//---------------------------------
// GraphicsApiContext::DefineVertexAttribIPointer
//
// Same as above, but for non normalized integers
//
void GraphicsApiContext::DefineVertexAttribIPointer(uint32 const index, 
	int32 const size, 
	E_DataType const type, 
	int32 const stride, 
	size_t const offset) const
{
	glVertexAttribIPointer(index, size, GetTypeId(type), stride, static_cast<char const*>(0) + offset);
}

//---------------------------------
// GraphicsApiContext::DefineVertexAttribDivisor
//
// Additional vertex stride during instanced rendering
//
void GraphicsApiContext::DefineVertexAttribDivisor(uint32 const index, uint32 const divisor) const
{
	glVertexAttribDivisor(index, divisor);
}

//---------------------------------
// GraphicsApiContext::MapBuffer
//
// Map the data of a buffer to a pointer on the CPU so that it can be modified
//
void* GraphicsApiContext::MapBuffer(E_BufferType const target, E_AccessMode const access) const
{
	return glMapBuffer(ConvBufferType(target), ConvAccessMode(access));
}

//---------------------------------
// GraphicsApiContext::UnmapBuffer
//
// Unmap a buffer from the pointer it's mapped to on the CPU
//
void GraphicsApiContext::UnmapBuffer(E_BufferType const target) const
{
	glUnmapBuffer(ConvBufferType(target));
}

//---------------------------------
// GraphicsApiContext::UnmapBuffer
//
T_TextureLoc GraphicsApiContext::GenerateTexture() const
{
	T_TextureLoc ret;
	glGenTextures(1, &ret);
	return ret;
}

//---------------------------------
// GraphicsApiContext::DeleteTexture
//
void GraphicsApiContext::DeleteTexture(T_TextureLoc& handle) const
{
	glDeleteTextures(1, &handle);
}

//---------------------------------
// GraphicsApiContext::SetTextureData
//
// upload a textures bits to its GPU location
//
void GraphicsApiContext::SetTextureData(TextureData& texture, void* data) 
{
	uint32 const target = GetTexTarget(texture.GetTargetType());

	BindTexture(texture.GetTargetType(), texture.GetHandle());

	ivec2 const res = texture.GetResolution();
	int32 const intFmt = texture.GetInternalFormat();

	switch (texture.GetTargetType())
	{
	case E_TextureType::Texture2D:
		glTexImage2D(target, 0, intFmt, res.x, res.y, 0, texture.GetFormat(), texture.GetDataType(), data);
		break;

	case E_TextureType::Texture3D:
		glTexImage3D(target, 0, intFmt, res.x, res.y, texture.GetDepth(), 0, texture.GetFormat(), texture.GetDataType(), data);
		break;

	case E_TextureType::CubeMap:
		ET_ASSERT(res.x == res.y);
		ET_ASSERT(data == nullptr, "directly uploading data to a cubemap is not currently supported");

		for (uint8 face = 0u; face < TextureData::s_NumCubeFaces; ++face)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F, res.x, res.y, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		break;
	}
}

//---------------------------------
// GraphicsApiContext::SetTextureParams
//
// Update parameters on a texture
//
void GraphicsApiContext::SetTextureParams(TextureData const& texture, uint8& mipLevels, TextureParameters& prev, TextureParameters const& next, bool const force) 
{
	uint32 const target = GetTexTarget(texture.GetTargetType());
	BindTexture(texture.GetTargetType(), texture.GetHandle());

	// filter options
	//---------------
	// in the future it may make sense to create filter groups so that things such as anisotropy can be set globally
	if ((prev.minFilter != next.minFilter) || (prev.mipFilter != next.mipFilter) || (prev.genMipMaps != next.genMipMaps) || force)
	{
		int32 minFilter = GetMinFilter(next.minFilter, next.mipFilter, next.genMipMaps);
		ET_ASSERT(minFilter != 0);

		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
	}

	if ((prev.magFilter != next.magFilter) || force)
	{
		int32 filter = GetFilter(next.magFilter);
		ET_ASSERT(filter != 0);

		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
	}

	// address mode
	//-------------
	if ((prev.wrapS != next.wrapS) || force)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GetWrapMode(next.wrapS));
	}

	if ((prev.wrapT != next.wrapT) || force)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GetWrapMode(next.wrapT));
	}

	if ((texture.GetDepth() > 1) && ((prev.wrapR != next.wrapR) || force))
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GetWrapMode(next.wrapR));
	}

	// border color
	if (!etm::nearEqualsV(prev.borderColor, next.borderColor) || force)
	{
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, next.borderColor.data.data());
	}

	// other
	//-------
	if ((next.isDepthTex && (prev.compareMode != next.compareMode)) || (next.isDepthTex && force))
	{
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GetCompareMode(next.compareMode));//shadow map comp mode
	}

	// generate mip maps if we must
	//-----------------------------
	if ((!prev.genMipMaps && next.genMipMaps) || (next.genMipMaps && force) || (next.genMipMaps && (mipLevels == 0u)))
	{
		glGenerateMipmap(target);
		ivec2 const res = texture.GetResolution();
		float const largerRes = static_cast<float>(std::max(res.x, res.y));
		mipLevels = 1u + static_cast<uint8>(floor(log10(largerRes) / log10(2.f)));
	}

	prev = next;
}

//---------------------------------
// GraphicsApiContext::CreateShader
//
// Create a shader object and return its handle
//
T_ShaderLoc GraphicsApiContext::CreateShader(E_ShaderType const type) const
{
	return glCreateShader(ConvShaderType(type));
}

//---------------------------------
// GraphicsApiContext::CreateProgram
//
// Create a program object and return its handle
//
T_ShaderLoc GraphicsApiContext::CreateProgram() const
{
	return glCreateProgram();
}

//---------------------------------
// GraphicsApiContext::DeleteShader
//
// Delete a shader by its handle
//
void GraphicsApiContext::DeleteShader(T_ShaderLoc const shader)
{
	glDeleteShader(shader);
}

//---------------------------------
// GraphicsApiContext::DeleteProgram
//
// Delete a program by its handle
//
void GraphicsApiContext::DeleteProgram(T_ShaderLoc const program)
{
	glDeleteProgram(program);
}

//---------------------------------
// GraphicsApiContext::CompileShader
//
// Compile a shader
//
void GraphicsApiContext::CompileShader(T_ShaderLoc const shader, std::string const& source) const
{
	char const* sourcePtr = source.c_str();
	glShaderSource(shader, 1, &sourcePtr, nullptr);
	glCompileShader(shader);
}

//---------------------------------
// GraphicsApiContext::BindFragmentDataLocation
//
// Set the return member of a fragment shader
//
void GraphicsApiContext::BindFragmentDataLocation(T_ShaderLoc const program, uint32 const colorNumber, std::string const& name) const
{
	glBindFragDataLocation(program, colorNumber, name.c_str());
}

//---------------------------------
// GraphicsApiContext::AttachShader
//
// Attach a shader to a program before linking
//
void GraphicsApiContext::AttachShader(T_ShaderLoc const program, T_ShaderLoc const shader) const
{
	glAttachShader(program, shader);
}

//---------------------------------
// GraphicsApiContext::LinkProgram
//
// Link the shaders in a program
//
void GraphicsApiContext::LinkProgram(T_ShaderLoc const program) const
{
	glLinkProgram(program);
}

//---------------------------------
// GraphicsApiContext::IsShaderCompiled
//
bool GraphicsApiContext::IsShaderCompiled(T_ShaderLoc const shader) const
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	return (status == GL_TRUE);
}

//---------------------------------
// GraphicsApiContext::GetShaderInfo
//
// Get logged information about a shader
//
void GraphicsApiContext::GetShaderInfo(T_ShaderLoc const shader, std::string& info) const
{
	char buffer[512];
	glGetShaderInfoLog(shader, 512, NULL, buffer);
	info = std::string(buffer);
}

//---------------------------------
// GraphicsApiContext::GetAttributeCount
//
int32 GraphicsApiContext::GetAttributeCount(T_ShaderLoc const program) const
{
	GLint count;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
	return count;
}

//---------------------------------
// GraphicsApiContext::GetUniformCount
//
int32 GraphicsApiContext::GetUniformCount(T_ShaderLoc const program) const
{
	GLint count;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
	return count;
}

//---------------------------------
// GraphicsApiContext::GetActiveUniforms
//
// Get a list of initialized uniform objects for a particular uniform index
//
void GraphicsApiContext::GetActiveUniforms(T_ShaderLoc const program, uint32 const index, std::vector<I_Uniform*>& uniforms) const
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
		// generate the full name
		std::string fullName = uniName;
		if (arrayCount > 1)
		{
			fullName += "[" + std::to_string(arrayIdx) + "]" + endName;
		}

		I_Uniform* uni;
		switch (type)
		{
		case GL_BOOL:
			uni = new Uniform<bool>();
			break;
		case GL_INT:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_2D_SHADOW:
			uni = new Uniform<int32>();
			break;
		case GL_UNSIGNED_INT:
			uni = new Uniform<uint32>();
			break;
		case GL_FLOAT:
			uni = new Uniform<float>();
			break;
		case GL_FLOAT_VEC2:
			uni = new Uniform<vec2>();
			break;
		case GL_FLOAT_VEC3:
			uni = new Uniform<vec3>();
			break;
		case GL_FLOAT_VEC4:
			uni = new Uniform<vec4>();
			break;
		case GL_FLOAT_MAT3:
			uni = new Uniform<mat3>();
			break;
		case GL_FLOAT_MAT4:
			uni = new Uniform<mat4>();
			break;
		default:
			LOG(std::string("unknown uniform type ") + std::to_string(type), LogLevel::Warning);
			return;
		}

		uni->name = fullName;
		uni->location = glGetUniformLocation(program, uni->name.c_str());

		switch (type)
		{
		case GL_BOOL:
			glGetUniformiv(program, uni->location, reinterpret_cast<int32*>(&static_cast<Uniform<bool>*>(uni)->data));
			break;
		case GL_INT:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_2D_SHADOW:
			glGetUniformiv(program, uni->location, &static_cast<Uniform<int32>*>(uni)->data);
			break;
		case GL_UNSIGNED_INT:
			glGetUniformuiv(program, uni->location, &static_cast<Uniform<uint32>*>(uni)->data);
			break;
		case GL_FLOAT:
			glGetUniformfv(program, uni->location, &static_cast<Uniform<float>*>(uni)->data);
			break;
		case GL_FLOAT_VEC2:
			glGetUniformfv(program, uni->location, etm::valuePtr(static_cast<Uniform<vec2>*>(uni)->data));
			break;
		case GL_FLOAT_VEC3:
			glGetUniformfv(program, uni->location, etm::valuePtr(static_cast<Uniform<vec3>*>(uni)->data));
			break;
		case GL_FLOAT_VEC4:
			glGetUniformfv(program, uni->location, etm::valuePtr(static_cast<Uniform<vec4>*>(uni)->data));
			break;
		case GL_FLOAT_MAT3:
			glGetUniformfv(program, uni->location, etm::valuePtr(static_cast<Uniform<mat3>*>(uni)->data));
			break;
		case GL_FLOAT_MAT4:
			glGetUniformfv(program, uni->location, etm::valuePtr(static_cast<Uniform<mat4>*>(uni)->data));
			break;
		default:
			LOG(std::string("unknown uniform type ") + std::to_string(type), LogLevel::Warning);
			return;
		}

		uniforms.emplace_back(uni);
	}
}

//---------------------------------
// GraphicsApiContext::GetActiveAttribute
//
// Get information about an attribute in a program at a given index
//
void GraphicsApiContext::GetActiveAttribute(T_ShaderLoc const program, uint32 const index, AttributeDescriptor& info) const
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
		LOG(FS("unknown attribute type '%u'", type), LogLevel::Warning);
		return;
	}
}

//---------------------------------
// GraphicsApiContext::GetAttributeLocation
//
// ID for an attribute on this shader program
//
T_AttribLoc GraphicsApiContext::GetAttributeLocation(T_ShaderLoc const program, std::string const& name) const
{
	return glGetAttribLocation(program, name.c_str());
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload a boolean to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<bool> &uniform)
{
	glUniform1i(uniform.location, uniform.data);
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload a 4x4 Matrix to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<mat4> &uniform)
{
	glUniformMatrix4fv(uniform.location, 1, GL_FALSE, etm::valuePtr(uniform.data));
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload a 3x3 Matrix to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<mat3> &uniform)
{
	glUniformMatrix3fv(uniform.location, 1, GL_FALSE, etm::valuePtr(uniform.data));
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload a 4D Vector to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<vec4> &uniform)
{
	glUniform4f(uniform.location, uniform.data.x, uniform.data.y, uniform.data.z, uniform.data.w);
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload a 3D Vector to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<vec3> &uniform)
{
	glUniform3f(uniform.location, uniform.data.x, uniform.data.y, uniform.data.z);
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload a 2D Vector to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<vec2> &uniform)
{
	glUniform2f(uniform.location, uniform.data.x, uniform.data.y);
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload a scalar to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<float> &uniform)
{
	glUniform1f(uniform.location, uniform.data);
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload an integer to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<int32> &uniform)
{
	glUniform1i(uniform.location, uniform.data);
}

//---------------------------------
// GraphicsApiContext::UploadUniform
//
// Upload an unsigned integer to the GPU
//
void GraphicsApiContext::UploadUniform(const Uniform<uint32> &uniform)
{
	glUniform1ui(uniform.location, uniform.data);
}

//---------------------------------
// GraphicsApiContext::GenFramebuffers
//
// Create a number of framebuffer objects
//
void GraphicsApiContext::GenFramebuffers(GLsizei n, GLuint *ids) const
{
	glGenFramebuffers(n, ids);
}

//---------------------------------
// GraphicsApiContext::DeleteFramebuffers
//
// Frees the framebuffer GPU resources
//
void GraphicsApiContext::DeleteFramebuffers(GLsizei n, GLuint *ids) const
{
	glDeleteFramebuffers(n, ids);
}

//---------------------------------
// GraphicsApiContext::GenRenderBuffers
//
// Create a number of renderbuffer objects
//
void GraphicsApiContext::GenRenderBuffers(GLsizei n, GLuint *ids) const
{
	glGenRenderbuffers(n, ids);
}

//---------------------------------
// GraphicsApiContext::DeleteRenderBuffers
//
// Frees the renderbuffer GPU resources
//
void GraphicsApiContext::DeleteRenderBuffers(GLsizei n, GLuint *ids) const
{
	glDeleteRenderbuffers(n, ids);
}

//---------------------------------
// GraphicsApiContext::SetRenderbufferStorage
//
// Establish a renderbuffers dataformat and storage
//
void GraphicsApiContext::SetRenderbufferStorage(GLenum format, ivec2 const dimensions) const
{
	glRenderbufferStorage(GL_RENDERBUFFER, format, dimensions.x, dimensions.y);
}

//---------------------------------
// GraphicsApiContext::LinkTextureToFbo
//
// link to current draw FB with a color attachment
//
void GraphicsApiContext::LinkTextureToFbo(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<uint32>(attachment), texHandle, level);
}

//---------------------------------
// GraphicsApiContext::LinkTextureToFbo
//
// Same as above, but specifies a target
//
void GraphicsApiContext::LinkTextureToFbo2D(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<uint32>(attachment), GL_TEXTURE_2D, texHandle, level);
}

//---------------------------------
// GraphicsApiContext::LinkTextureToFbo
//
// Same as above, but specifies a target
//
void GraphicsApiContext::LinkCubeMapFaceToFbo2D(uint8 const face, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texHandle, level);
}

//---------------------------------
// GraphicsApiContext::LinkTextureToFbo
//
// Link a depth texture to an FBO
//
void GraphicsApiContext::LinkTextureToFboDepth(T_TextureLoc const texHandle) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texHandle, 0);
}

//---------------------------------
// GraphicsApiContext::LinkRenderbufferToFbo
//
void GraphicsApiContext::LinkRenderbufferToFbo(GLenum const attachment, uint32 const rboHandle) const
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboHandle);
}

//---------------------------------
// GraphicsApiContext::SetDrawBufferCount
//
// Setup the amount of color attachments on the current framebuffer
//
void GraphicsApiContext::SetDrawBufferCount(size_t count) const
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
// GraphicsApiContext::SetReadBufferEnabled
//
// For the current buffer, whether or not openGL will read a color value.
// Assumes double buffered rendering
//
void GraphicsApiContext::SetReadBufferEnabled(bool const val) const
{
	glReadBuffer(val ? GL_BACK : GL_NONE);
}

//-----------------------------------
// GraphicsApiContext::SetReadBufferEnabled
//
bool GraphicsApiContext::IsFramebufferComplete() const
{
	return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

//-----------------------------------
// GraphicsApiContext::CopyDepthReadToDrawFbo
//
void GraphicsApiContext::CopyDepthReadToDrawFbo(ivec2 const source, ivec2 const target) const
{
	glBlitFramebuffer( 0, 0, source.x, source.y, 0, 0, target.x, target.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

//-----------------------------------
// GraphicsApiContext::SetReadBufferEnabled
//
// Byte alignment requirements for pixel rows in memory
//
void GraphicsApiContext::SetPixelUnpackAlignment(int32 const val) const
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, val);
}
