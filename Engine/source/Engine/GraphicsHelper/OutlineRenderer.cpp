#include "stdafx.h"
#include "OutlineRenderer.h"

#include "SceneRenderer.h"
#include "PrimitiveRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/FrameBuffer.h>
#include <Engine/Framebuffers/GBuffer.h>
#include <Engine/Materials/ColorMaterial.h>
#include <Engine/SceneGraph/Entity.h>


//====================
// Outline Renderer
//====================


//---------------------------------
// OutlineRenderer::d-tor
//
OutlineRenderer::~OutlineRenderer()
{
	SafeDelete(m_Material);

	DestroyRenderTarget();
}

//---------------------------------
// OutlineRenderer::Initialize
//
void OutlineRenderer::Initialize()
{
	m_SobelShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostSobel.glsl"_hash);
	m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdColorShader.glsl"_hash);

	m_Material = new ColorMaterial();
	m_Material->Initialize();

	CreateRenderTarget();

	Config::GetInstance()->GetWindow().WindowResizeEvent.AddListener( std::bind( &OutlineRenderer::OnWindowResize, this ) );
}

//---------------------------------
// OutlineRenderer::GenerateRenderTarget
//
void OutlineRenderer::CreateRenderTarget()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_DrawTarget);
	api->BindFramebuffer(m_DrawTarget);
	m_DrawTex = new TextureData(windowSettings.Dimensions, E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
	m_DrawTex->Build();
	m_DrawTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, m_DrawTex->GetHandle(), 0);

	api->GenRenderBuffers(1, &m_DrawDepth);
	api->BindRenderbuffer(m_DrawDepth);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24, windowSettings.Dimensions);
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24, m_DrawDepth);

	api->BindFramebuffer(0);
}

//---------------------------------
// OutlineRenderer::DestroyRenderTarget
//
void OutlineRenderer::DestroyRenderTarget()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteRenderBuffers(1, &m_DrawDepth);
	SafeDelete(m_DrawTex);
	api->DeleteFramebuffers(1, &m_DrawTarget);
}

//---------------------------------
// OutlineRenderer::AddEntity
//
// Adds an entity to the current color list
//
void OutlineRenderer::AddEntity(Entity* const entity)
{
	auto listIt = AccessEntityListIt(m_Color);
	ET_ASSERT(listIt != m_Lists.cend());

	listIt->entities.emplace_back(entity);
}

//---------------------------------
// OutlineRenderer::AddEntities
//
// Add a bunch of entities to the current color list
//
void OutlineRenderer::AddEntities(std::vector<Entity*> const& entities)
{
	auto listIt = AccessEntityListIt(m_Color);
	ET_ASSERT(listIt != m_Lists.cend());

	listIt->entities.insert(listIt->entities.end(), entities.begin(), entities.end());
}

//---------------------------------
// OutlineRenderer::Draw
//
void OutlineRenderer::Draw(T_FbLoc const targetFb)
{
	if (m_Lists.empty())
	{
		return;
	}

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetViewport(ivec2(0), Config::GetInstance()->GetWindow().Dimensions);

	// draw the shapes as colors to the intermediate rendertarget
	//------------------------------------------------------------
	api->BindFramebuffer(m_DrawTarget);

	api->SetClearColor(vec4(vec3(0.f), 1.f));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

	api->SetShader(m_Shader.get());
	m_Shader->Upload("worldViewProj"_hash, CAMERA->GetViewProj());

	// bind the gbuffers depth texture
	//m_Shader->Upload("texGBufferA"_hash, 0);
	//api->LazyBindTexture(0, E_TextureType::Texture2D, SceneRenderer::GetInstance()->GetGBuffer()->GetTextures()[0]->GetHandle());

	//api->SetDepthEnabled(true);
	//api->SetDepthFunction(E_DepthFunc::GEqual);

	for (EntityList& list : m_Lists)
	{
		m_Shader->Upload("uColor"_hash, m_Color);

		for (Entity* const entity : list.entities)
		{
			entity->RootDrawMaterial(static_cast<Material*>(m_Material));
		}
	}

	m_Lists.clear();

	//return;

	// apply a sobel shader to the colored shapes and render it to the target framebuffer
	//------------------------------------------------------------------------------------

	api->BindFramebuffer(targetFb);

	api->SetDepthEnabled(false);
	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);

	api->SetShader(m_SobelShader.get());

	m_SobelShader->Upload("inColorTex"_hash, 1);
	api->LazyBindTexture(1, E_TextureType::Texture2D, m_DrawTex->GetHandle());

	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	api->SetBlendEnabled(false);
}

//---------------------------------
// OutlineRenderer::AccessEntityListIt
//
// Find or create an entity list matching our color
//
OutlineRenderer::T_EntityLists::iterator OutlineRenderer::AccessEntityListIt(vec4 const& col)
{
	auto listIt = std::find_if(m_Lists.begin(), m_Lists.end(), [&col](EntityList const& list)
		{
			return etm::nearEqualsV(list.color, col);
		});

	// create a new one if none was found
	if (listIt == m_Lists.cend())
	{
		m_Lists.emplace_back(EntityList());
		listIt = std::prev(m_Lists.end());
		listIt->color = col;
	}

	return listIt;
}

//---------------------------------
// OutlineRenderer::OnWindowResize
//
void OutlineRenderer::OnWindowResize()
{
	DestroyRenderTarget();
	CreateRenderTarget();
}
