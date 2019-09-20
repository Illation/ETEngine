#include "stdafx.h"
#include "ScreenSpaceReflections.h"

#include "RenderPipeline.h"
#include "PrimitiveRenderer.h"

#include <glad/glad.h>

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
	STATE->DeleteRenderBuffers(1, &m_CollectRBO);
	delete m_CollectTex; m_CollectTex = nullptr;
	STATE->DeleteFramebuffers(1, &m_CollectFBO);
}

void ScreenSpaceReflections::Initialize()
{
	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostScreenSpaceReflections.glsl"_hash);

	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	STATE->GenFramebuffers(1, &m_CollectFBO);
	STATE->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new TextureData(windowSettings.Width, windowSettings.Height, GL_RGB16F, GL_RGB, GL_FLOAT);
	m_CollectTex->Build();
	m_CollectTex->SetParameters(params);
	STATE->LinkTextureToFbo2D(0, GL_TEXTURE_2D, m_CollectTex->GetHandle(), 0);
	//Render Buffer for depth and stencil
	STATE->GenRenderBuffers(1, &m_CollectRBO);
	STATE->BindRenderbuffer(m_CollectRBO);
	STATE->SetRenderbufferStorage(GL_DEPTH24_STENCIL8, windowSettings.Dimensions);
	STATE->LinkRenderbufferToFbo(GL_DEPTH_STENCIL_ATTACHMENT, m_CollectRBO);
}

void ScreenSpaceReflections::EnableInput()
{
	STATE->BindFramebuffer(m_CollectFBO);
}

void ScreenSpaceReflections::Draw()
{
	//Hotreload shader
	//if (INPUT->GetKeyState(E_KbdKey::LeftAlt) == E_KeyState::Down && 
	//	INPUT->GetKeyState(E_KbdKey::R) == E_KeyState::Pressed)
	//{
	//	//if there is a debugger attached copy over the resource files 
	//	DebugCopyResourceFiles();
	//	//reload the shader
	//	m_pShader = CONTENT::Reload<ShaderData>("Shaders/PostScreenSpaceReflections.glsl");
	//	GetUniforms();
	//}
	STATE->SetShader(m_pShader.get());

	m_pShader->Upload("texGBufferA"_hash, 0);
	m_pShader->Upload("texGBufferB"_hash, 1);
	m_pShader->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = RenderPipeline::GetInstance()->GetGBuffer()->GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		STATE->LazyBindTexture(i, GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}
	m_pShader->Upload("uFinalImage"_hash, 3);
	STATE->LazyBindTexture(3, GL_TEXTURE_2D, m_CollectTex->GetHandle());
	//for position reconstruction
	m_pShader->Upload("K"_hash, sinf(TIME->GetTime()) * 20 + 25);
	m_pShader->Upload("projectionA"_hash, CAMERA->GetDepthProjA());
	m_pShader->Upload("projectionB"_hash, CAMERA->GetDepthProjB());
	m_pShader->Upload("viewProjInv"_hash, CAMERA->GetStatViewProjInv());
	m_pShader->Upload("projection"_hash, CAMERA->GetViewProj());
	//m_pShader->Upload("viewInv"_hash, CAMERA->GetViewInv());
	m_pShader->Upload("camPos"_hash, CAMERA->GetTransform()->GetPosition());

	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}