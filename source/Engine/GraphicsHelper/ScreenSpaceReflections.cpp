#include "stdafx.h"
#include "ScreenSpaceReflections.h"

#include "RenderPipeline.h"
#include "PrimitiveRenderer.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Helper/Commands.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Framebuffers/PostProcessingRenderer.h>


ScreenSpaceReflections::ScreenSpaceReflections()
{

}

ScreenSpaceReflections::~ScreenSpaceReflections()
{
	glDeleteRenderbuffers(1, &m_CollectRBO);
	delete m_CollectTex; m_CollectTex = nullptr;
	glDeleteFramebuffers(1, &m_CollectFBO);
}

void ScreenSpaceReflections::Initialize()
{
	m_pShader = ResourceManager::GetInstance()->GetAssetData<ShaderData>("PostScreenSpaceReflections.glsl"_hash);
	GetUniforms();

	int32 width = WINDOW.Width, height = WINDOW.Height;

	TextureParameters params = TextureParameters();
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	glGenFramebuffers(1, &m_CollectFBO);
	STATE->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new TextureData(width, height, GL_RGB16F, GL_RGB, GL_FLOAT);
	m_CollectTex->Build();
	m_CollectTex->SetParameters(params);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_CollectTex->GetHandle(), 0);
	//Render Buffer for depth and stencil
	glGenRenderbuffers(1, &m_CollectRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_CollectRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_CollectRBO);
}

void ScreenSpaceReflections::EnableInput()
{
	STATE->BindFramebuffer(m_CollectFBO);
}

void ScreenSpaceReflections::Draw()
{
	//Hotreload shader
	//if (INPUT->GetKeyState(static_cast<uint32>(SDLK_LALT)) == E_KeyState::Down && 
	//	INPUT->GetKeyState(static_cast<uint32>(SDLK_r)) == E_KeyState::Pressed)
	//{
	//	//if there is a debugger attached copy over the resource files 
	//	DebugCopyResourceFiles();
	//	//reload the shader
	//	m_pShader = CONTENT::Reload<ShaderData>("Shaders/PostScreenSpaceReflections.glsl");
	//	GetUniforms();
	//}
	STATE->SetShader(m_pShader.get());

	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferA"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferB"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferC"), 2);
	auto gbufferTex = RenderPipeline::GetInstance()->GetGBuffer()->GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		STATE->LazyBindTexture(i, GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uFinalImage"), 3);
	STATE->LazyBindTexture(3, GL_TEXTURE_2D, m_CollectTex->GetHandle());
	//for position reconstruction
	glUniform1f(glGetUniformLocation(m_pShader->GetProgram(), "K"), sin(TIME->GetTime())*20+25);
	glUniform1f(m_uProjA, CAMERA->GetDepthProjA());
	glUniform1f(m_uProjB, CAMERA->GetDepthProjB());
	glUniformMatrix4fv(m_uViewProjInv, 1, GL_FALSE, etm::valuePtr(CAMERA->GetStatViewProjInv()));
	glUniformMatrix4fv(m_uProjection, 1, GL_FALSE, etm::valuePtr(CAMERA->GetViewProj()));
	glUniformMatrix4fv(m_uViewInv, 1, GL_FALSE, etm::valuePtr(CAMERA->GetViewInv()));
	glUniform3fv(m_uCamPos, 1, etm::valuePtr(CAMERA->GetTransform()->GetPosition()));

	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}

void ScreenSpaceReflections::GetUniforms()
{
	m_uCamPos = glGetUniformLocation(m_pShader->GetProgram(), "camPos");
	m_uProjA = glGetUniformLocation(m_pShader->GetProgram(), "projectionA");
	m_uProjB = glGetUniformLocation(m_pShader->GetProgram(), "projectionB");
	m_uViewProjInv = glGetUniformLocation(m_pShader->GetProgram(), "viewProjInv");
	m_uProjection = glGetUniformLocation(m_pShader->GetProgram(), "projection");
	m_uViewInv = glGetUniformLocation(m_pShader->GetProgram(), "viewInv");
}
