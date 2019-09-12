#include "stdafx.h"

#include "RenderState.h"

#include <glad/glad.h>

#include <Engine/Graphics/Shader.h>


//===================
// Render State
//===================


//---------------------------------
// RenderState::Initialize
//
// Initialize the state with default values
//
void RenderState::Initialize()
{
	m_ViewportSize = WINDOW.Dimensions;

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_NumTextureUnits);
	for (int32 i = 0; i < m_NumTextureUnits; i++)
	{
		std::map<GLenum, GLuint> targets = 
		{
			{ GL_TEXTURE_1D, 0 },
			{ GL_TEXTURE_2D, 0 },
			{ GL_TEXTURE_3D, 0 },
			{ GL_TEXTURE_1D_ARRAY, 0 },
			{ GL_TEXTURE_2D_ARRAY, 0 },
			{ GL_TEXTURE_RECTANGLE, 0 },
			{ GL_TEXTURE_CUBE_MAP, 0 },
			{ GL_TEXTURE_2D_MULTISAMPLE, 0 },
			{ GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 0 },
			{ GL_TEXTURE_BUFFER, 0 },
			{ GL_TEXTURE_CUBE_MAP_ARRAY, 0 }
		};
		m_pTextureUnits.push_back(targets);
	}

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &m_MaxDrawBuffers);

	m_BufferTargets =
	{
		{ GL_ARRAY_BUFFER, 0 },
		{ GL_ATOMIC_COUNTER_BUFFER, 0 },
		{ GL_COPY_READ_BUFFER, 0 },
		{ GL_COPY_WRITE_BUFFER, 0 },
		{ GL_DISPATCH_INDIRECT_BUFFER, 0 },
		{ GL_DRAW_INDIRECT_BUFFER, 0 },
		{ GL_ELEMENT_ARRAY_BUFFER, 0 },
		{ GL_PIXEL_PACK_BUFFER, 0 },
		{ GL_PIXEL_UNPACK_BUFFER, 0 },
		{ GL_QUERY_BUFFER, 0 },
		{ GL_SHADER_STORAGE_BUFFER, 0 },
		{ GL_TEXTURE_BUFFER, 0 },
		{ GL_TRANSFORM_FEEDBACK_BUFFER, 0 },
		{ GL_UNIFORM_BUFFER, 0 }
	};
}

//---------------------------------
// RenderState::EnOrDisAble
//
// Enable or disable a setting
//
void RenderState::EnOrDisAble(bool &state, bool enabled, GLenum glState)
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
// RenderState::EnOrDisAbleIndexed
//
// Enable or disable a setting at a specific index
// Note the index should be validated before calling this function
//
void RenderState::EnOrDisAbleIndexed(std::vector<bool> &state, bool enabled, GLenum glState, uint32 index)
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
// RenderState::SetBlendEnabled
//
// Set the buffers on which blending is enabled
//
void RenderState::SetBlendEnabled(const std::vector<bool> &blendBuffers)
{
	for (uint32 i = 0; i < blendBuffers.size(); i++)
	{
		SetBlendEnabled(blendBuffers[i], i);
	}
}

//---------------------------------
// RenderState::SetBlendEnabled
//
// Set whether blending is enabled on a specific buffer
//
void RenderState::SetBlendEnabled(bool enabled, uint32 index)
{
	assert((int32)index < m_MaxDrawBuffers);
	m_IndividualBlend = true;
	if (index >= m_BlendEnabledIndexed.size())
		m_BlendEnabledIndexed.push_back(m_BlendEnabled);
	EnOrDisAbleIndexed(m_BlendEnabledIndexed, enabled, GL_BLEND, index);
}

//---------------------------------
// RenderState::SetBlendEnabled
//
// Set whether blending pixels is enabled in the render pipeline
//
void RenderState::SetBlendEnabled(bool enabled)
{
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
// RenderState::SetFaceCullingMode
//
// Set the culling mode (front back neither...)
//
void RenderState::SetFaceCullingMode(GLenum cullMode)
{
	if (!(m_CullFaceMode == cullMode))
	{
		m_CullFaceMode = cullMode;
		glCullFace(m_CullFaceMode);
	}
}

//---------------------------------
// RenderState::SetBlendEquation
//
// Set the equation we use to blend pixels
//
void RenderState::SetBlendEquation(GLenum equation)
{
	if (!(m_BlendEquationRGB == equation && m_BlendEquationAlpha == equation))
	{
		m_BlendEquationRGB = equation;
		m_BlendEquationAlpha = equation;
		glBlendEquation(equation);
	}
}

//---------------------------------
// RenderState::SetBlendFunction
//
// Set the function we use to blend pixels
//
void RenderState::SetBlendFunction(GLenum sFactor, GLenum dFactor)
{
	if (!(m_BlendFuncSFactor == sFactor && m_BlendFuncDFactor == dFactor))
	{
		m_BlendFuncSFactor = sFactor;
		m_BlendFuncDFactor = dFactor;
		glBlendFunc(m_BlendFuncSFactor, m_BlendFuncDFactor);
	}
}

//---------------------------------
// RenderState::SetViewport
//
// Set the dimensions of the current opengl viewport (not the engine viewport)
//
void RenderState::SetViewport(ivec2 pos, ivec2 size)
{
	if (!(etm::nearEqualsV(m_ViewportPosition, pos) && etm::nearEqualsV(m_ViewportSize, size)))
	{
		m_ViewportPosition = pos;
		m_ViewportSize = size;
		glViewport(pos.x, pos.y, size.x, size.y);
	}
}

//---------------------------------
// RenderState::GetViewport
//
// Get the dimensions of the current opengl viewport (not the engine viewport)
//
void RenderState::GetViewport(ivec2 &pos, ivec2 &size)
{
	pos = m_ViewportPosition;
	size = m_ViewportSize;
}

//---------------------------------
// RenderState::SetClearColor
//
// Set the colour that gets drawn when we clear the viewport
//
void RenderState::SetClearColor(vec4 col)
{
	if (!(etm::nearEqualsV(m_ClearColor, col)))
	{
		m_ClearColor = col;
		glClearColor(col.r, col.g, col.b, col.a);
	}
}

//---------------------------------
// RenderState::SetShader
//
// Set the shader we draw with
//
void RenderState::SetShader(ShaderData const* pShader)
{
	if (!(m_pBoundShader == pShader))
	{
		// #todo: make shaders track uniforms automatically during precompilation and update those here too
		m_pBoundShader = pShader;
		glUseProgram(pShader->GetProgram());
	}
}

//---------------------------------
// RenderState::BindFramebuffer
//
// Set the framebuffer we will draw to and read from
//
void RenderState::BindFramebuffer(GLuint handle)
{
	if (!(m_ReadFramebuffer == handle && m_DrawFramebuffer == handle))
	{
		m_ReadFramebuffer = handle;
		m_DrawFramebuffer = handle;
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// RenderState::BindReadFramebuffer
//
// Set the framebuffer we will read from
//
void RenderState::BindReadFramebuffer(GLuint handle)
{
	if (!(m_ReadFramebuffer == handle))
	{
		m_ReadFramebuffer = handle;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// RenderState::BindDrawFramebuffer
//
// Set the framebuffer we will draw to
//
void RenderState::BindDrawFramebuffer(GLuint handle)
{
	if (!(m_DrawFramebuffer == handle))
	{
		m_DrawFramebuffer = handle;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	}
}

//---------------------------------
// RenderState::SetActiveTexture
//
// Set the currently active texture unit
//
void RenderState::SetActiveTexture(uint32 unit)
{
	if (!(m_ActiveTexture == unit))
	{
		m_ActiveTexture = unit;
		glActiveTexture(GL_TEXTURE0 + unit);
	}
}

//---------------------------------
// RenderState::BindTexture
//
// Bind a texture to a target
//
void RenderState::BindTexture(GLenum target, GLuint handle)
{
	if (!(m_pTextureUnits[m_ActiveTexture][target] == handle))
	{
		m_pTextureUnits[m_ActiveTexture][target] = handle;
		glBindTexture(target, handle);
	}
}

//---------------------------------
// RenderState::LazyBindTexture
//
// If the current texture isn't bound to its target, bind it
//
void RenderState::LazyBindTexture(uint32 unit, GLenum target, GLuint handle)
{
	if (!(m_pTextureUnits[unit][target] == handle))
	{
		SetActiveTexture(unit);
		m_pTextureUnits[m_ActiveTexture][target] = handle;
		glBindTexture(target, handle);
	}
}

//---------------------------------
// RenderState::BindVertexArray
//
// Bind the current vertex array that the state operates on
//
void RenderState::BindVertexArray(GLuint vertexArray)
{
	if (!(m_VertexArray == vertexArray))
	{
		m_VertexArray = vertexArray;
		glBindVertexArray(vertexArray);

		//Aparrently binding a new vertex array unbinds the old element array buffer
		//Coincedentially it doesn't seem to happen with array buffers
		m_BufferTargets[GL_ELEMENT_ARRAY_BUFFER] = 0;
	}
}

//---------------------------------
// RenderState::BindBuffer
//
// Bind the current buffer that the state operates on
//
void RenderState::BindBuffer(GLenum target, GLuint buffer)
{
	if (!(m_BufferTargets[target] == buffer))
	{
		m_BufferTargets[target] = buffer;
		glBindBuffer(target, buffer);
	}
}

//---------------------------------
// RenderState::SetLineWidth
//
// Set the width of lines that are drawn
//
void RenderState::SetLineWidth(float const lineWidth)
{
	if (!etm::nearEquals(m_LineWidth, lineWidth))
	{
		m_LineWidth = lineWidth;
		glLineWidth(m_LineWidth);
	}
}

//---------------------------------
// RenderState::DrawArrays
//
// Draw vertex data (without indices)
//
void RenderState::DrawArrays(GLenum mode, uint32 first, uint32 count) 
{
	glDrawArrays(mode, first, count);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// RenderState::DrawElements
//
// Draw vertex data with indices
//
void RenderState::DrawElements(GLenum mode, uint32 count, GLenum type, const void * indices)
{
	glDrawElements(mode, count, type, indices);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// RenderState::DrawElementsInstanced
//
// Draw instanced vertex data with indices
//
void RenderState::DrawElementsInstanced(GLenum mode, uint32 count, GLenum type, const void * indices, uint32 primcount)
{
	glDrawElementsInstanced(mode, count, type, indices, primcount);
	PERFORMANCE->m_DrawCalls++;
}

//---------------------------------
// RenderState::Flush
//
// Force OpenGL to execute all commands now
//
void RenderState::Flush() const
{
	glFlush();
}

//---------------------------------
// RenderState::Clear
//
// Clear the part of the currently set viewport that is mapped to the mask
//
void RenderState::Clear(GLbitfield mask) const
{
	glClear(mask);
}

//---------------------------------
// RenderState::GenerateVertexArrays
//
// Generate a vertex array
//
void RenderState::GenerateVertexArrays(GLsizei n, GLuint *arrays) const
{
	glGenVertexArrays(n, arrays);
}

//---------------------------------
// RenderState::GenerateBuffers
//
// Generate a buffer
//
void RenderState::GenerateBuffers(GLsizei n, GLuint *buffers) const
{
	glGenBuffers(n, buffers);
}

//---------------------------------
// RenderState::DeleteVertexArrays
//
// Delete a vertex array
//
void RenderState::DeleteVertexArrays(GLsizei n, GLuint *arrays) const
{
	glDeleteVertexArrays(n, arrays);
}

//---------------------------------
// RenderState::DeleteBuffers
//
// Delete a buffer
//
void RenderState::DeleteBuffers(GLsizei n, GLuint *buffers) const
{
	glDeleteBuffers(n, buffers);
}

//---------------------------------
// RenderState::SetBufferData
//
// Fill the buffer at target with an array of data
//
void RenderState::SetBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) const
{
	glBufferData(target, size, data, usage);
}

//---------------------------------
// RenderState::SetVertexAttributeArrayEnabled
//
// Enable or disable an attribute at an index in the current vertex array
//
void RenderState::SetVertexAttributeArrayEnabled(GLuint index, bool enabled) const
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
// RenderState::DefineVertexAttributePointer
//
// Define the type of data the attribute at a certain index in the current vertex array is mapped to
//
void RenderState::DefineVertexAttributePointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid const* pointer) 
	const
{
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

//---------------------------------
// RenderState::MapBuffer
//
// Map the data of a buffer to a pointer on the CPU so that it can be modified
//
void* RenderState::MapBuffer(GLenum target, GLenum access) const
{
	return glMapBuffer(target, access);
}

//---------------------------------
// RenderState::UnmapBuffer
//
// Unmap a buffer from the pointer it's mapped to on the CPU
//
void RenderState::UnmapBuffer(GLenum target) const
{
	glUnmapBuffer(target);
}

//---------------------------------
// RenderState::CreateShader
//
// Create a shader object and return its handle
//
GLuint RenderState::CreateShader(GLenum shaderType) const
{
	return glCreateShader(shaderType);
}

//---------------------------------
// RenderState::CreateProgram
//
// Create a program object and return its handle
//
GLuint RenderState::CreateProgram() const
{
	return glCreateProgram();
}

//---------------------------------
// RenderState::DeleteShader
//
// Delete a shader by its handle
//
void RenderState::DeleteShader(GLuint shader)
{
	glDeleteShader(shader);
}

//---------------------------------
// RenderState::DeleteProgram
//
// Delete a program by its handle
//
void RenderState::DeleteProgram(GLuint program)
{
	glDeleteProgram(program);
}

//---------------------------------
// RenderState::SetShaderSource
//
// Set the source code of a shader
//
void RenderState::SetShaderSource(GLuint shader, GLsizei count, GLchar const **string, int32* length) const
{
	glShaderSource(shader, count, string, length);
}

//---------------------------------
// RenderState::CompileShader
//
// Compile a shader
//
void RenderState::CompileShader(GLuint shader) const
{
	glCompileShader(shader);
}

//---------------------------------
// RenderState::BindFragmentDataLocation
//
// Set the return member of a fragment shader
//
void RenderState::BindFragmentDataLocation(GLuint program, GLuint colorNumber, std::string const& name) const
{
	glBindFragDataLocation(program, colorNumber, name.c_str());
}

//---------------------------------
// RenderState::AttachShader
//
// Attach a shader to a program before linking
//
void RenderState::AttachShader(GLuint program, GLuint shader) const
{
	glAttachShader(program, shader);
}

//---------------------------------
// RenderState::LinkProgram
//
// Link the shaders in a program
//
void RenderState::LinkProgram(GLuint program) const
{
	glLinkProgram(program);
}

//---------------------------------
// RenderState::GetShaderIV
//
// Get an integer value from a shader
//
void RenderState::GetShaderIV(GLuint shader, GLenum pname, GLint *params) const
{
	glGetShaderiv(shader, pname, params);
}

//---------------------------------
// RenderState::GetShaderInfoLog
//
// Get logged information about a shader
//
void RenderState::GetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) const
{
	glGetShaderInfoLog(shader, maxLength, length, infoLog);
}

//---------------------------------
// RenderState::GetProgramIV
//
// Get an integer value from a program
//
void RenderState::GetProgramIV(GLuint program, GLenum pname, GLint *params) const
{
	glGetProgramiv(program, pname, params);
}

//---------------------------------
// RenderState::GetProgramIV
//
// Get information about a uniform in a program at a given index
//
void RenderState::GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) const
{
	glGetActiveUniform(program, index, bufSize, length, size, type, name);
}

//---------------------------------
// RenderState::GetActiveAttribute
//
// Get information about an attribute in a program at a given index
//
void RenderState::GetActiveAttribute(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) const
{
	glGetActiveAttrib(program, index, bufSize, length, size, type, name);
}
