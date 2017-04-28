#include "stdafx.hpp"

#include "RenderState.hpp"

RenderState::RenderState()
{
}
RenderState::~RenderState()
{
}

void RenderState::Initialize()
{
	m_ViewportSize = glm::ivec2(WINDOW.Width, WINDOW.Height);
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

void RenderState::SetViewport(glm::ivec2 pos, glm::ivec2 size)
{
	if (!(m_ViewportPosition == pos && m_ViewportSize == size))
	{
		m_ViewportPosition = pos;
		m_ViewportSize = size;
		glViewport(pos.x, pos.y, size.x, size.y);
	}
}

void RenderState::SetClearColor(glm::vec4 col)
{
	if (!(m_ClearColor == col))
	{
		m_ClearColor = col;
		glClearColor(col.r, col.g, col.b, col.a);
	}
}

//void RenderState::BindFramebuffer(GLuint handle)
//{
//	if (!(m_BoundFramebuffer == handle))
//	{
//		m_BoundFramebuffer = handle;
//		glBindFramebuffer(handle);
//	}
//}