#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/Uniform.h>


//===================
// Open GL Context
//===================


//---------------------------------
// GlContext::Initialize
//
// Initialize the state with default values
//
void GL_CONTEXT_CLASSNAME::Initialize()
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
// GlContext::SetDepthEnabled
//
void GL_CONTEXT_CLASSNAME::SetDepthEnabled(bool const enabled)
{
	EnOrDisAble(m_DepthTestEnabled, enabled, GL_DEPTH_TEST);
}

//---------------------------------
// GlContext::SetBlendEnabled
//
// Set the buffers on which blending is enabled
//
void GL_CONTEXT_CLASSNAME::SetBlendEnabled(std::vector<bool> const& blendBuffers)
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
void GL_CONTEXT_CLASSNAME::SetBlendEnabled(bool const enabled, uint32 const index)
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
// Set whether blending pixels is enabled in the render pipeline
//
void GL_CONTEXT_CLASSNAME::SetBlendEnabled(bool const enabled)
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
void GL_CONTEXT_CLASSNAME::SetStencilEnabled(bool const enabled)
{
	EnOrDisAble(m_StencilTestEnabled, enabled, GL_STENCIL_TEST);
}

//---------------------------------
// GlContext::SetCullEnabled
//
void GL_CONTEXT_CLASSNAME::SetCullEnabled(bool const enabled)
{
	EnOrDisAble(m_CullFaceEnabled, enabled, GL_CULL_FACE);
}

//---------------------------------
// GlContext::SetSeamlessCubemapsEnabled
//
void GL_CONTEXT_CLASSNAME::SetSeamlessCubemapsEnabled(bool const enabled)
{
	EnOrDisAble(m_SeamlessCubemapsEnabled, enabled, GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

//---------------------------------
// GlContext::SetFaceCullingMode
//
// Set the culling mode (front back neither...)
//
void GL_CONTEXT_CLASSNAME::SetFaceCullingMode(E_FaceCullMode const cullMode)
{
	if (!(m_CullFaceMode == cullMode))
	{
		m_CullFaceMode = cullMode;
		glCullFace(ConvFaceCullMode(m_CullFaceMode));
	}
}

//---------------------------------
// GlContext::SetBlendEquation
//
// Set the equation we use to blend pixels
//
void GL_CONTEXT_CLASSNAME::SetBlendEquation(E_BlendEquation const equation)
{
	if (!(m_BlendEquationRGB == equation && m_BlendEquationAlpha == equation))
	{
		m_BlendEquationRGB = equation;
		m_BlendEquationAlpha = equation;
		glBlendEquation(ConvBlendEquation(equation));
	}
}

//---------------------------------
// GlContext::SetBlendFunction
//
// Set the function we use to blend pixels
//
void GL_CONTEXT_CLASSNAME::SetBlendFunction(E_BlendFactor const sFactor, E_BlendFactor const dFactor)
{
	if (!(m_BlendFuncSFactor == sFactor && m_BlendFuncDFactor == dFactor))
	{
		m_BlendFuncSFactor = sFactor;
		m_BlendFuncDFactor = dFactor;
		glBlendFunc(ConvBlendFactor(sFactor), ConvBlendFactor(dFactor));
	}
}

//---------------------------------
// GlContext::SetViewport
//
// Set the dimensions of the current opengl viewport (not the engine viewport)
//
void GL_CONTEXT_CLASSNAME::SetViewport(ivec2 const pos, ivec2 const size)
{
	if (!(etm::nearEqualsV(m_ViewportPosition, pos) && etm::nearEqualsV(m_ViewportSize, size)))
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
void GL_CONTEXT_CLASSNAME::GetViewport(ivec2& pos, ivec2& size)
{
	pos = m_ViewportPosition;
	size = m_ViewportSize;
}

//---------------------------------
// GlContext::SetClearColor
//
// Set the colour that gets drawn when we clear the viewport
//
void GL_CONTEXT_CLASSNAME::SetClearColor(vec4 const& col)
{
	if (!(etm::nearEqualsV(m_ClearColor, col)))
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
void GL_CONTEXT_CLASSNAME::SetShader(ShaderData const* pShader)
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
void GL_CONTEXT_CLASSNAME::BindFramebuffer(T_FbLoc const handle)
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
void GL_CONTEXT_CLASSNAME::BindReadFramebuffer(T_FbLoc const handle)
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
void GL_CONTEXT_CLASSNAME::BindDrawFramebuffer(T_FbLoc const handle)
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
void GL_CONTEXT_CLASSNAME::BindRenderbuffer(T_RbLoc const handle)
{
	if (handle != m_Renderbuffer)
	{
		m_Renderbuffer = handle;
		glBindRenderbuffer(GL_RENDERBUFFER, handle);
	}
}

//---------------------------------
// GlContext::SetActiveTexture
//
// Set the currently active texture unit
//
void GL_CONTEXT_CLASSNAME::SetActiveTexture(uint32 const unit)
{
	if (!(m_ActiveTexture == unit))
	{
		m_ActiveTexture = unit;
		glActiveTexture(GL_TEXTURE0 + unit);
	}
}

//---------------------------------
// GlContext::BindTexture
//
// Bind a texture to a target
//
void GL_CONTEXT_CLASSNAME::BindTexture(E_TextureType const target, T_TextureLoc const handle)
{
	if (!(m_pTextureUnits[m_ActiveTexture][target] == handle))
	{
		m_pTextureUnits[m_ActiveTexture][target] = handle;
		glBindTexture(ConvTextureType(target), handle);
	}
}

//---------------------------------
// GlContext::LazyBindTexture
//
// If the current texture isn't bound to its target, bind it
//
void GL_CONTEXT_CLASSNAME::LazyBindTexture(uint32 const unit, E_TextureType const target, T_TextureLoc const handle)
{
	if (!(m_pTextureUnits[unit][target] == handle))
	{
		SetActiveTexture(unit);
		m_pTextureUnits[m_ActiveTexture][target] = handle;
		glBindTexture(ConvTextureType(target), handle);
	}
}

//---------------------------------
// GlContext::BindVertexArray
//
// Bind the current vertex array that the state operates on
//
void GL_CONTEXT_CLASSNAME::BindVertexArray(T_ArrayLoc const vertexArray)
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
void GL_CONTEXT_CLASSNAME::BindBuffer(E_BufferType const target, T_BufferLoc const buffer)
{
	if (m_BufferTargets[target] != buffer)
	{
		m_BufferTargets[target] = buffer;
		glBindBuffer(ConvBufferType(target), buffer);
	}
}

//---------------------------------
// GlContext::SetLineWidth
//
// Set the width of lines that are drawn
//
void GL_CONTEXT_CLASSNAME::SetLineWidth(float const lineWidth)
{
	if (!etm::nearEquals(m_LineWidth, lineWidth))
	{
		m_LineWidth = lineWidth;
		glLineWidth(m_LineWidth);
	}
}

//---------------------------------
// GlContext::DrawArrays
//
// Draw vertex data (without indices)
//
void GL_CONTEXT_CLASSNAME::DrawArrays(E_DrawMode const mode, uint32 const first, uint32 const count)
{
	glDrawArrays(ConvDrawMode(mode), first, count);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// GlContext::DrawElements
//
// Draw vertex data with indices
//
void GL_CONTEXT_CLASSNAME::DrawElements(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices)
{
	glDrawElements(ConvDrawMode(mode), count, ConvDataType(type), indices);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// GlContext::DrawElementsInstanced
//
// Draw instanced vertex data with indices
//
void GL_CONTEXT_CLASSNAME::DrawElementsInstanced(E_DrawMode const mode, uint32 const count, E_DataType const type, const void * indices, uint32 const prims)
{
	glDrawElementsInstanced(ConvDrawMode(mode), count, ConvDataType(type), indices, prims);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// GlContext::Flush
//
// Force OpenGL to execute all commands now
//
void GL_CONTEXT_CLASSNAME::Flush() const
{
	glFlush();
}

//---------------------------------
// GlContext::Clear
//
// Clear the part of the currently set viewport that is mapped to the mask
//
void GL_CONTEXT_CLASSNAME::Clear(T_ClearFlags const mask) const
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
// GlContext::CreateVertexArray
//
T_ArrayLoc GL_CONTEXT_CLASSNAME::CreateVertexArray() const
{
	T_ArrayLoc ret;
	glGenVertexArrays(1, &ret);
	return ret;
}

//---------------------------------
// GlContext::CreateBuffer
//
T_BufferLoc GL_CONTEXT_CLASSNAME::CreateBuffer() const
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
void GL_CONTEXT_CLASSNAME::DeleteVertexArray(T_ArrayLoc& loc) const
{
	glDeleteVertexArrays(1, &loc);
}

//---------------------------------
// GlContext::DeleteBuffer
//
// Delete a buffer
//
void GL_CONTEXT_CLASSNAME::DeleteBuffer(T_BufferLoc& loc) const
{
	glDeleteBuffers(1, &loc);
}

//---------------------------------
// GlContext::SetBufferData
//
// Fill the buffer at target with an array of data
//
void GL_CONTEXT_CLASSNAME::SetBufferData(E_BufferType const target, int64 const size, void const* const data, E_UsageHint const usage) const
{
	glBufferData(ConvBufferType(target), size, data, ConvUsageHint(usage));
}

//---------------------------------
// GlContext::SetVertexAttributeArrayEnabled
//
// Enable or disable an attribute at an index in the current vertex array
//
void GL_CONTEXT_CLASSNAME::SetVertexAttributeArrayEnabled(uint32 const index, bool const enabled) const
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
void GL_CONTEXT_CLASSNAME::DefineVertexAttributePointer(uint32 const index,
	int32 const size, 
	E_DataType const type, 
	bool const norm, 
	int32 const stride, 
	size_t const offset) const
{
	glVertexAttribPointer(index, size, ConvDataType(type), (norm ? GL_TRUE : GL_FALSE), stride, static_cast<char const*>(0) + offset);
}

//---------------------------------
// GlContext::DefineVertexAttribIPointer
//
// Same as above, but for non normalized integers
//
void GL_CONTEXT_CLASSNAME::DefineVertexAttribIPointer(uint32 const index,
	int32 const size, 
	E_DataType const type, 
	int32 const stride, 
	size_t const offset) const
{
	glVertexAttribIPointer(index, size, ConvDataType(type), stride, static_cast<char const*>(0) + offset);
}

//---------------------------------
// GlContext::DefineVertexAttribDivisor
//
// Additional vertex stride during instanced rendering
//
void GL_CONTEXT_CLASSNAME::DefineVertexAttribDivisor(uint32 const index, uint32 const divisor) const
{
	glVertexAttribDivisor(index, divisor);
}

//---------------------------------
// GlContext::MapBuffer
//
// Map the data of a buffer to a pointer on the CPU so that it can be modified
//
void* GL_CONTEXT_CLASSNAME::MapBuffer(E_BufferType const target, E_AccessMode const access) const
{
	return glMapBuffer(ConvBufferType(target), ConvAccessMode(access));
}

//---------------------------------
// GlContext::UnmapBuffer
//
// Unmap a buffer from the pointer it's mapped to on the CPU
//
void GL_CONTEXT_CLASSNAME::UnmapBuffer(E_BufferType const target) const
{
	glUnmapBuffer(ConvBufferType(target));
}

//---------------------------------
// GlContext::UnmapBuffer
//
T_TextureLoc GL_CONTEXT_CLASSNAME::GenerateTexture() const
{
	T_TextureLoc ret;
	glGenTextures(1, &ret);
	return ret;
}

//---------------------------------
// GlContext::DeleteTexture
//
void GL_CONTEXT_CLASSNAME::DeleteTexture(T_TextureLoc& handle) const
{
	glDeleteTextures(1, &handle);
}

//---------------------------------
// GlContext::SetTextureData
//
// upload a textures bits to its GPU location
//
void GL_CONTEXT_CLASSNAME::SetTextureData(TextureData& texture, void* data)
{
	uint32 const target = ConvTextureType(texture.GetTargetType());

	BindTexture(texture.GetTargetType(), texture.GetHandle());

	ivec2 const res = texture.GetResolution();

	GLint const intFmt = static_cast<GLint>(ConvColorFormat(texture.GetInternalFormat()));
	E_ColorFormat format = texture.GetFormat();
	ET_ASSERT(format <= E_ColorFormat::RGBA, "Texture format can't specify data sizes!");

	switch (texture.GetTargetType())
	{
	case E_TextureType::Texture2D:
		glTexImage2D(target, 0, intFmt, res.x, res.y, 0, ConvColorFormat(format), ConvDataType(texture.GetDataType()), data);
		break;

	case E_TextureType::Texture3D:
		glTexImage3D(target, 0, intFmt, res.x, res.y, texture.GetDepth(), 0, ConvColorFormat(format), ConvDataType(texture.GetDataType()), data);
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
// GlContext::SetTextureParams
//
// Update parameters on a texture
//
void GL_CONTEXT_CLASSNAME::SetTextureParams(TextureData const& texture, uint8& mipLevels, TextureParameters& prev, TextureParameters const& next, bool const force)
{
	uint32 const target = ConvTextureType(texture.GetTargetType());
	BindTexture(texture.GetTargetType(), texture.GetHandle());

	// filter options
	//---------------
	// in the future it may make sense to create filter groups so that things such as anisotropy can be set globally
	if ((prev.minFilter != next.minFilter) || (prev.mipFilter != next.mipFilter) || (prev.genMipMaps != next.genMipMaps) || force)
	{
		int32 minFilter = ConvMinFilter(next.minFilter, next.mipFilter, next.genMipMaps);
		ET_ASSERT(minFilter != 0);

		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
	}

	if ((prev.magFilter != next.magFilter) || force)
	{
		int32 filter = ConvFilter(next.magFilter);
		ET_ASSERT(filter != 0);

		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
	}

	// address mode
	//-------------
	if ((prev.wrapS != next.wrapS) || force)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, ConvWrapMode(next.wrapS));
	}

	if ((prev.wrapT != next.wrapT) || force)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_T, ConvWrapMode(next.wrapT));
	}

	if ((texture.GetDepth() > 1) && ((prev.wrapR != next.wrapR) || force))
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_R, ConvWrapMode(next.wrapR));
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
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, ConvCompareMode(next.compareMode));//shadow map comp mode
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
// GlContext::CreateShader
//
// Create a shader object and return its handle
//
T_ShaderLoc GL_CONTEXT_CLASSNAME::CreateShader(E_ShaderType const type) const
{
	return glCreateShader(ConvShaderType(type));
}

//---------------------------------
// GlContext::CreateProgram
//
// Create a program object and return its handle
//
T_ShaderLoc GL_CONTEXT_CLASSNAME::CreateProgram() const
{
	return glCreateProgram();
}

//---------------------------------
// GlContext::DeleteShader
//
// Delete a shader by its handle
//
void GL_CONTEXT_CLASSNAME::DeleteShader(T_ShaderLoc const shader)
{
	glDeleteShader(shader);
}

//---------------------------------
// GlContext::DeleteProgram
//
// Delete a program by its handle
//
void GL_CONTEXT_CLASSNAME::DeleteProgram(T_ShaderLoc const program)
{
	glDeleteProgram(program);
}

//---------------------------------
// GlContext::CompileShader
//
// Compile a shader
//
void GL_CONTEXT_CLASSNAME::CompileShader(T_ShaderLoc const shader, std::string const& source) const
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
void GL_CONTEXT_CLASSNAME::BindFragmentDataLocation(T_ShaderLoc const program, uint32 const colorNumber, std::string const& name) const
{
	glBindFragDataLocation(program, colorNumber, name.c_str());
}

//---------------------------------
// GlContext::AttachShader
//
// Attach a shader to a program before linking
//
void GL_CONTEXT_CLASSNAME::AttachShader(T_ShaderLoc const program, T_ShaderLoc const shader) const
{
	glAttachShader(program, shader);
}

//---------------------------------
// GlContext::LinkProgram
//
// Link the shaders in a program
//
void GL_CONTEXT_CLASSNAME::LinkProgram(T_ShaderLoc const program) const
{
	glLinkProgram(program);
}

//---------------------------------
// GlContext::IsShaderCompiled
//
bool GL_CONTEXT_CLASSNAME::IsShaderCompiled(T_ShaderLoc const shader) const
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
void GL_CONTEXT_CLASSNAME::GetShaderInfo(T_ShaderLoc const shader, std::string& info) const
{
	char buffer[512];
	glGetShaderInfoLog(shader, 512, NULL, buffer);
	info = std::string(buffer);
}

//---------------------------------
// GlContext::GetAttributeCount
//
int32 GL_CONTEXT_CLASSNAME::GetAttributeCount(T_ShaderLoc const program) const
{
	GLint count;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
	return count;
}

//---------------------------------
// GlContext::GetUniformCount
//
int32 GL_CONTEXT_CLASSNAME::GetUniformCount(T_ShaderLoc const program) const
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
void GL_CONTEXT_CLASSNAME::GetActiveUniforms(T_ShaderLoc const program, uint32 const index, std::vector<I_Uniform*>& uniforms) const
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
// GlContext::GetActiveAttribute
//
// Get information about an attribute in a program at a given index
//
void GL_CONTEXT_CLASSNAME::GetActiveAttribute(T_ShaderLoc const program, uint32 const index, AttributeDescriptor& info) const
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
// GlContext::GetAttributeLocation
//
// ID for an attribute on this shader program
//
T_AttribLoc GL_CONTEXT_CLASSNAME::GetAttributeLocation(T_ShaderLoc const program, std::string const& name) const
{
	return glGetAttribLocation(program, name.c_str());
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a boolean to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<bool> &uniform)
{
	glUniform1i(uniform.location, uniform.data);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 4x4 Matrix to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<mat4> &uniform)
{
	glUniformMatrix4fv(uniform.location, 1, GL_FALSE, etm::valuePtr(uniform.data));
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 3x3 Matrix to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<mat3> &uniform)
{
	glUniformMatrix3fv(uniform.location, 1, GL_FALSE, etm::valuePtr(uniform.data));
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 4D Vector to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<vec4> &uniform)
{
	glUniform4f(uniform.location, uniform.data.x, uniform.data.y, uniform.data.z, uniform.data.w);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 3D Vector to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<vec3> &uniform)
{
	glUniform3f(uniform.location, uniform.data.x, uniform.data.y, uniform.data.z);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a 2D Vector to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<vec2> &uniform)
{
	glUniform2f(uniform.location, uniform.data.x, uniform.data.y);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload a scalar to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<float> &uniform)
{
	glUniform1f(uniform.location, uniform.data);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload an integer to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<int32> &uniform)
{
	glUniform1i(uniform.location, uniform.data);
}

//---------------------------------
// GlContext::UploadUniform
//
// Upload an unsigned integer to the GPU
//
void GL_CONTEXT_CLASSNAME::UploadUniform(const Uniform<uint32> &uniform)
{
	glUniform1ui(uniform.location, uniform.data);
}

//---------------------------------
// GlContext::GenFramebuffers
//
// Create a number of framebuffer objects
//
void GL_CONTEXT_CLASSNAME::GenFramebuffers(int32 const n, T_FbLoc *ids) const
{
	glGenFramebuffers(n, ids);
}

//---------------------------------
// GlContext::DeleteFramebuffers
//
// Frees the framebuffer GPU resources
//
void GL_CONTEXT_CLASSNAME::DeleteFramebuffers(int32 const n, T_FbLoc *ids) const
{
	glDeleteFramebuffers(n, ids);
}

//---------------------------------
// GlContext::GenRenderBuffers
//
// Create a number of renderbuffer objects
//
void GL_CONTEXT_CLASSNAME::GenRenderBuffers(int32 const n, T_RbLoc *ids) const
{
	glGenRenderbuffers(n, ids);
}

//---------------------------------
// GlContext::DeleteRenderBuffers
//
// Frees the renderbuffer GPU resources
//
void GL_CONTEXT_CLASSNAME::DeleteRenderBuffers(int32 const n, T_RbLoc *ids) const
{
	glDeleteRenderbuffers(n, ids);
}

//---------------------------------
// GlContext::SetRenderbufferStorage
//
// Establish a renderbuffers dataformat and storage
//
void GL_CONTEXT_CLASSNAME::SetRenderbufferStorage(E_RenderBufferFormat const format, ivec2 const dimensions) const
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

	ET_ASSERT(glFmt != GL_NONE, "Unhandled render buffer format!");

	glRenderbufferStorage(GL_RENDERBUFFER, glFmt, dimensions.x, dimensions.y);
}

//---------------------------------
// GlContext::LinkTextureToFbo
//
// link to current draw FB with a color attachment
//
void GL_CONTEXT_CLASSNAME::LinkTextureToFbo(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<uint32>(attachment), texHandle, level);
}

//---------------------------------
// GlContext::LinkTextureToFbo
//
// Same as above, but specifies a target
//
void GL_CONTEXT_CLASSNAME::LinkTextureToFbo2D(uint8 const attachment, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<uint32>(attachment), GL_TEXTURE_2D, texHandle, level);
}

//---------------------------------
// GlContext::LinkTextureToFbo
//
// Same as above, but specifies a target
//
void GL_CONTEXT_CLASSNAME::LinkCubeMapFaceToFbo2D(uint8 const face, T_TextureLoc const texHandle, int32 const level) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texHandle, level);
}

//---------------------------------
// GlContext::LinkTextureToFbo
//
// Link a depth texture to an FBO
//
void GL_CONTEXT_CLASSNAME::LinkTextureToFboDepth(T_TextureLoc const texHandle) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texHandle, 0);
}

//---------------------------------
// GlContext::LinkRenderbufferToFbo
//
void GL_CONTEXT_CLASSNAME::LinkRenderbufferToFbo(E_RenderBufferFormat const attachment, uint32 const rboHandle) const
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

	ET_ASSERT(glFmt != GL_NONE, "Unhandled render buffer format!");

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, glFmt, GL_RENDERBUFFER, rboHandle);
}

//---------------------------------
// GlContext::SetDrawBufferCount
//
// Setup the amount of color attachments on the current framebuffer
//
void GL_CONTEXT_CLASSNAME::SetDrawBufferCount(size_t const count) const
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
void GL_CONTEXT_CLASSNAME::SetReadBufferEnabled(bool const val) const
{
	glReadBuffer(val ? GL_BACK : GL_NONE);
}

//-----------------------------------
// GlContext::SetReadBufferEnabled
//
bool GL_CONTEXT_CLASSNAME::IsFramebufferComplete() const
{
	return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

//-----------------------------------
// GlContext::CopyDepthReadToDrawFbo
//
void GL_CONTEXT_CLASSNAME::CopyDepthReadToDrawFbo(ivec2 const source, ivec2 const target) const
{
	glBlitFramebuffer( 0, 0, source.x, source.y, 0, 0, target.x, target.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

//-----------------------------------
// GlContext::SetReadBufferEnabled
//
// Byte alignment requirements for pixel rows in memory
//
void GL_CONTEXT_CLASSNAME::SetPixelUnpackAlignment(int32 const val) const
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, val);
}

//---------------------------------
// GlContext::SetDepthFunction
//
// How to compare depth values
//
void GL_CONTEXT_CLASSNAME::SetDepthFunction(E_DepthFunc const func) const
{
	glDepthFunc(ConvDepthFunction(func));
}

//---------------------------------
// GlContext::ReadPixels
//
// Read pixels into an array from the current framebuffer
//
void GL_CONTEXT_CLASSNAME::ReadPixels(ivec2 const pos, ivec2 const size, E_ColorFormat const format, E_DataType const type, void* data) const
{
	glReadPixels(pos.x, pos.y, size.x, size.y, ConvColorFormat(format), ConvDataType(type), data);
}


// Open GL Conversions
////////////////////////


//---------------------------------
// GlContext::EnOrDisAble
//
// Enable or disable a setting
//
void GL_CONTEXT_CLASSNAME::EnOrDisAble(bool &state, bool enabled, GLenum glState)
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
void GL_CONTEXT_CLASSNAME::EnOrDisAbleIndexed(std::vector<bool> &state, bool enabled, GLenum glState, uint32 index)
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
// GlContext::ConvTextureType
//
GLenum GL_CONTEXT_CLASSNAME::ConvTextureType(E_TextureType const type) const
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
// GlContext::ConvDataType
//
GLenum GL_CONTEXT_CLASSNAME::ConvDataType(E_DataType const type) const
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
// GlContext::ConvShaderType
//
GLenum GL_CONTEXT_CLASSNAME::ConvShaderType(E_ShaderType const type) const
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
// GlContext::ConvDrawMode
//
GLenum GL_CONTEXT_CLASSNAME::ConvDrawMode(E_DrawMode const mode) const
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
// GlContext::ConvDrawMode
//
// Not all buffer types are currently listed, they will be added when support is needed
//
GLenum GL_CONTEXT_CLASSNAME::ConvBufferType(E_BufferType const type) const
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
// GlContext::ConvUsageHint
//
GLenum GL_CONTEXT_CLASSNAME::ConvUsageHint(E_UsageHint const hint) const
{
	switch (hint)
	{
	case E_UsageHint::Static:	return GL_STATIC_DRAW;
	case E_UsageHint::Dynamic:	return GL_DYNAMIC_DRAW;
	}

	ET_ASSERT(true, "Unhandled usage hint!");
	return GL_NONE;
}

//---------------------------------
// GlContext::ConvAccessMode
//
GLenum GL_CONTEXT_CLASSNAME::ConvAccessMode(E_AccessMode const mode) const
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
// GlContext::ConvFaceCullMode
//
GLenum GL_CONTEXT_CLASSNAME::ConvFaceCullMode(E_FaceCullMode const mode) const
{
	switch (mode)
	{
	case E_FaceCullMode::Front:		return GL_FRONT;
	case E_FaceCullMode::Back:		return GL_BACK;
	case E_FaceCullMode::FrontBack:	return GL_FRONT_AND_BACK;
	}

	ET_ASSERT(true, "Unhandled access mode!");
	return GL_NONE;
}

//---------------------------------
// GlContext::ConvBlendEquation
//
GLenum GL_CONTEXT_CLASSNAME::ConvBlendEquation(E_BlendEquation const equ) const
{
	switch (equ)
	{
	case E_BlendEquation::Add:		return GL_FUNC_ADD;
	case E_BlendEquation::Subtract:	return GL_FUNC_SUBTRACT;
	case E_BlendEquation::RevSub:	return GL_FUNC_REVERSE_SUBTRACT;
	case E_BlendEquation::Min:		return GL_MIN;
	case E_BlendEquation::Max:		return GL_MAX;
	}

	ET_ASSERT(true, "Unhandled blend equation!");
	return GL_NONE;
}

//---------------------------------
// GlContext::ConvBlendFactor
//
GLenum GL_CONTEXT_CLASSNAME::ConvBlendFactor(E_BlendFactor const fac) const
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

	ET_ASSERT(true, "Unhandled blend factor!");
	return GL_NONE;
}

//---------------------------------
// GlContext::ConvColorFormat
//
// must grow along with E_ColorFormat - reinterpret to GLint for internal format
//
GLenum GL_CONTEXT_CLASSNAME::ConvColorFormat(E_ColorFormat const fmt) const
{
	switch (fmt)
	{
	case E_ColorFormat::Depth:			return GL_DEPTH_COMPONENT;
	case E_ColorFormat::DepthStencil:	return GL_DEPTH_STENCIL;

	case E_ColorFormat::Red:			return GL_RED;
	case E_ColorFormat::RG:				return GL_RG;
	case E_ColorFormat::RGB:			return GL_RGB;
	case E_ColorFormat::RGBA:			return GL_RGBA;

	case E_ColorFormat::Depth24:		return GL_DEPTH_COMPONENT24;

	case E_ColorFormat::RG16f:			return GL_RG16F;
	case E_ColorFormat::RGB16f:			return GL_RGB16F;
	case E_ColorFormat::RGBA16f:		return GL_RGBA16F;
	case E_ColorFormat::RGBA32f:		return GL_RGBA32F;
	case E_ColorFormat::SRGB:			return GL_SRGB;
	}

	ET_ASSERT(true, "Unhandled color format!");
	return GL_NONE;
}

//---------------------------------
// GlContext::GetFilter
//
// Convert E_TextureFilterMode to OpenGL enum
//
GLenum GL_CONTEXT_CLASSNAME::ConvFilter(E_TextureFilterMode const filter) const
{
	switch (filter)
	{
	case E_TextureFilterMode::Nearest: return GL_NEAREST;
	case E_TextureFilterMode::Linear:  return GL_LINEAR;
	}

	ET_ASSERT(true, "Unhandled texture filter mode!");
	return GL_NONE;
}

//---------------------------------
// GlContext::GetMinFilter
//
// Convert E_TextureFilterMode to OpenGL enum for min filters
//
GLenum GL_CONTEXT_CLASSNAME::ConvMinFilter(E_TextureFilterMode const minFilter, E_TextureFilterMode const mipFilter, bool const useMip) const
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
				ET_ASSERT(true, "Unhandled texture filter mode!");
				break;
			}
			break;

		case E_TextureFilterMode::Linear:
			switch (mipFilter)
			{
			case E_TextureFilterMode::Nearest: return GL_LINEAR_MIPMAP_NEAREST;
			case E_TextureFilterMode::Linear: return GL_LINEAR_MIPMAP_LINEAR;

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

	return ConvFilter(minFilter);
}

//---------------------------------
// GlContext::ConvWrapMode
//
// Convert E_TextureWrapMode to OpenGL enum
//
GLenum GL_CONTEXT_CLASSNAME::ConvWrapMode(E_TextureWrapMode const wrap) const
{
	switch (wrap)
	{
	case E_TextureWrapMode::ClampToEdge:		return GL_CLAMP_TO_EDGE;
	case E_TextureWrapMode::ClampToBorder:		return GL_CLAMP_TO_BORDER;
	case E_TextureWrapMode::MirrorClampToEdge:	return GL_MIRROR_CLAMP_TO_EDGE;
	case E_TextureWrapMode::MirroredRepeat:		return GL_MIRRORED_REPEAT;
	case E_TextureWrapMode::Repeat:				return GL_REPEAT;
	}

	ET_ASSERT(true, "Unhandled texture wrap mode!");
	return 0;
}

//---------------------------------
// GlContext::ConvCompareMode
//
// Convert E_TextureCompareMode to OpenGL enum
//
GLenum GL_CONTEXT_CLASSNAME::ConvCompareMode(E_TextureCompareMode const comp) const
{
	switch (comp)
	{
	case E_TextureCompareMode::CompareRToTexture:	return GL_COMPARE_REF_TO_TEXTURE;
	case E_TextureCompareMode::None:				return GL_NONE;
	}

	ET_ASSERT(true, "Unhandled texture compare mode!");
	return 0;
}

//---------------------------------
// GlContext::ConvDepthFunction
//
GLenum GL_CONTEXT_CLASSNAME::ConvDepthFunction(E_DepthFunc const func) const
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

	ET_ASSERT(true, "Unhandled depth function!");
	return 0;
}
