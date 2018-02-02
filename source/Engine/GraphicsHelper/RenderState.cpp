#include "stdafx.hpp"

#include "RenderState.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/TextureData.hpp"

RenderState::RenderState()
{
}
RenderState::~RenderState()
{
}

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
//Note the index should be validated before calling this function
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

void RenderState::SetBlendEnabled(const std::vector<bool> &blendBuffers)
{
	for (uint32 i = 0; i < blendBuffers.size(); i++)
	{
		SetBlendEnabled(blendBuffers[i], i);
	}
}
void RenderState::SetBlendEnabled(bool enabled, uint32 index)
{
	assert((int32)index < m_MaxDrawBuffers);
	m_IndividualBlend = true;
	if (index >= m_BlendEnabledIndexed.size())
		m_BlendEnabledIndexed.push_back(m_BlendEnabled);
	EnOrDisAbleIndexed(m_BlendEnabledIndexed, enabled, GL_BLEND, index);
}
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

void RenderState::SetFaceCullingMode(GLenum cullMode)
{
	if (!(m_CullFaceMode == cullMode))
	{
		m_CullFaceMode = cullMode;
		glCullFace(m_CullFaceMode);
	}
}

void RenderState::SetBlendEquation(GLenum equation)
{
	if (!(m_BlendEquationRGB == equation && m_BlendEquationAlpha == equation))
	{
		m_BlendEquationRGB = equation;
		m_BlendEquationAlpha = equation;
		glBlendEquation(equation);
	}
}
void RenderState::SetBlendFunction(GLenum sFactor, GLenum dFactor)
{
	if (!(m_BlendFuncSFactor == sFactor && m_BlendFuncDFactor == dFactor))
	{
		m_BlendFuncSFactor = sFactor;
		m_BlendFuncDFactor = dFactor;
		glBlendFunc(m_BlendFuncSFactor, m_BlendFuncDFactor);
	}
}

void RenderState::SetViewport(ivec2 pos, ivec2 size)
{
	if (!(etm::nearEqualsV(m_ViewportPosition, pos) && etm::nearEqualsV(m_ViewportSize, size)))
	{
		m_ViewportPosition = pos;
		m_ViewportSize = size;
		glViewport(pos.x, pos.y, size.x, size.y);
	}
}

void RenderState::GetViewport(ivec2 &pos, ivec2 &size)
{
	pos = m_ViewportPosition;
	size = m_ViewportSize;
}

void RenderState::SetClearColor(vec4 col)
{
	if (!(etm::nearEqualsV(m_ClearColor, col)))
	{
		m_ClearColor = col;
		glClearColor(col.r, col.g, col.b, col.a);
	}
}

void RenderState::SetShader(ShaderData* pShader)
{
	if (!(m_pBoundShader == pShader))
	{
		// #todo: make shaders track uniforms automatically during precompilation and update those here too
		m_pBoundShader = pShader;
		glUseProgram(pShader->GetProgram());
	}
}

void RenderState::BindFramebuffer(GLuint handle)
{
	if (!(m_ReadFramebuffer == handle && m_DrawFramebuffer == handle))
	{
		m_ReadFramebuffer = handle;
		m_DrawFramebuffer = handle;
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
	}
}
void RenderState::BindReadFramebuffer(GLuint handle)
{
	if (!(m_ReadFramebuffer == handle))
	{
		m_ReadFramebuffer = handle;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	}
}
void RenderState::BindDrawFramebuffer(GLuint handle)
{
	if (!(m_DrawFramebuffer == handle))
	{
		m_DrawFramebuffer = handle;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	}
}

void RenderState::SetActiveTexture(uint32 unit)
{
	if (!(m_ActiveTexture == unit))
	{
		m_ActiveTexture = unit;
		glActiveTexture(GL_TEXTURE0 + unit);
	}
}
void RenderState::BindTexture(GLenum target, GLuint handle)
{
	if (!(m_pTextureUnits[m_ActiveTexture][target] == handle))
	{
		m_pTextureUnits[m_ActiveTexture][target] = handle;
		glBindTexture(target, handle);
	}
}
void RenderState::LazyBindTexture(uint32 unit, GLenum target, GLuint handle)
{
	if (!(m_pTextureUnits[unit][target] == handle))
	{
		SetActiveTexture(unit);
		m_pTextureUnits[m_ActiveTexture][target] = handle;
		glBindTexture(target, handle);
	}
}

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
void RenderState::BindBuffer(GLenum target, GLuint buffer)
{
	if (!(m_BufferTargets[target] == buffer))
	{
		m_BufferTargets[target] = buffer;
		glBindBuffer(target, buffer);
	}
}