#include "stdafx.h"
#include "EntityIdRenderer.h"

#include "IdMaterial.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/FrameBuffer.h>
#include <Engine/SceneRendering/SceneRenderer.h>
#include <Engine/SceneRendering/GBuffer.h>
#include <Engine/SceneGraph/Entity.h>


//====================
// Entity ID Renderer
//====================


//---------------------------------
// EntityIdRenderer::d-tor
//
EntityIdRenderer::~EntityIdRenderer()
{
	SafeDelete(m_Material);

	DestroyRenderTarget();
}

//---------------------------------
// EntityIdRenderer::Initialize
//
void EntityIdRenderer::Initialize()
{
	m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdIdShader.glsl"_hash);

	m_Material = new IdMaterial();
	m_Material->Initialize();

	CreateRenderTarget();
}

//---------------------------------
// EntityIdRenderer::GenerateRenderTarget
//
void EntityIdRenderer::CreateRenderTarget()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Nearest;
	params.magFilter = E_TextureFilterMode::Nearest;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_DrawTarget);
	api->BindFramebuffer(m_DrawTarget);
	m_DrawTex = new TextureData(dim, E_ColorFormat::RGBA8, E_ColorFormat::RGBA, E_DataType::UByte);
	m_DrawTex->Build();
	m_DrawTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, m_DrawTex->GetHandle(), 0);

	api->GenRenderBuffers(1, &m_DrawDepth);
	api->BindRenderbuffer(m_DrawDepth);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24, dim);
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24, m_DrawDepth);

	api->BindFramebuffer(0);
}

//---------------------------------
// EntityIdRenderer::DestroyRenderTarget
//
void EntityIdRenderer::DestroyRenderTarget()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteRenderBuffers(1, &m_DrawDepth);
	SafeDelete(m_DrawTex);
	api->DeleteFramebuffers(1, &m_DrawTarget);
}

//---------------------------------
// EntityIdRenderer::Pick
//
// Picks an entity by drawing each entity visible to the scene renderer with a color calculated from its ID, 
//  - then converting the color under the specified pixel back to the IT and finding the appropriate entity
//
void EntityIdRenderer::Pick(ivec2 const pixel, Viewport* const viewport, std::function<void(Entity* const)>& onEntityPicked)
{
	if (m_ViewportToPickFrom == nullptr)
	{
		m_PixelToPick = pixel;
		m_ViewportToPickFrom = viewport;
		m_OnEntityPicked = onEntityPicked;

		m_ViewportToPickFrom->RegisterListener(this);
	}
	else
	{
		LOG("EntityIdRenderer::Pick > some system is already picking for a viewport this frame!", LogLevel::Warning);
	}
}

//---------------------------------
// EntityIdRenderer::OnViewportPreRender
//
// Runs before the viewports rendererer does its thing
//
void EntityIdRenderer::OnViewportPreRender(T_FbLoc const targetFb)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	// if the view size changed since the last pick we need to recreate our rendertarget with the correct size
	if (!(etm::nearEqualsV(m_LastViewSize, dim)))
	{
		DestroyRenderTarget();
		CreateRenderTarget();

		m_LastViewSize = dim;
	}

	api->SetViewport(ivec2(0), dim);

	// draw the shapes as colors to the intermediate rendertarget
	//------------------------------------------------------------
	api->BindFramebuffer(m_DrawTarget);

	api->SetClearColor(vec4(0.f));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

	api->SetShader(m_Shader.get());
	m_Shader->Upload("worldViewProj"_hash, SceneRenderer::GetCurrent()->GetCamera().GetViewProj());

	api->SetDepthEnabled(true);

	for (AbstractScene* const scene : SceneRenderer::GetCurrent()->GetRenderScenes())
	{
		for (Entity* const entity : scene->GetEntities())
		{
			// convert the entities ID to a color
			T_Hash const entityId = entity->GetId();

			uint32 r = (entityId & 0x000000FF) >> 0;
			uint32 g = (entityId & 0x0000FF00) >> 8;
			uint32 b = (entityId & 0x00FF0000) >> 16;
			uint32 a = (entityId & 0xFF000000) >> 24;

			vec4 const color(static_cast<float>(r) / 255.f,
				static_cast<float>(g) / 255.f,
				static_cast<float>(b) / 255.f,
				static_cast<float>(a) / 255.f);

			// upload the color to the material and adraw the entity with it
			m_Shader->Upload("uColor"_hash, color);
			entity->RootDrawMaterial(static_cast<Material*>(m_Material));
		}
	}

	api->BindFramebuffer(targetFb);
}

//---------------------------------
// EntityIdRenderer::OnViewportPostFlush
//
// Runs after everything else has finished rendering
//
void EntityIdRenderer::OnViewportPostFlush(T_FbLoc const targetFb)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->BindFramebuffer(m_DrawTarget);

	api->Finish();

	// openGL flips along the y axis
	ivec2 texCoord = ivec2(m_PixelToPick.x, m_DrawTex->GetResolution().y - m_PixelToPick.y);

	// read the pixels and set the previous renderbuffer again
	uint8 pixels[4];
	api->ReadPixels(texCoord, ivec2(1), E_ColorFormat::RGBA, E_DataType::UByte, pixels);
	api->BindFramebuffer(targetFb);

	api->Flush(); // ensure this is not the active framebuffer before swapping

	// convert the read pixels back into the entity ID
	T_Hash pickedID =
		static_cast<uint32>(pixels[0]) +
		static_cast<uint32>(pixels[1]) * 256 +
		static_cast<uint32>(pixels[2]) * 256 * 256 +
		static_cast<uint32>(pixels[3]) * 256 * 256 * 256;

	Entity* foundEntity = nullptr;

	for (AbstractScene* const scene : SceneRenderer::GetCurrent()->GetRenderScenes())
	{
		Entity* const entity = scene->GetEntity(pickedID);

		if (entity != nullptr)
		{
			foundEntity = entity;
		}
	}

	m_OnEntityPicked(foundEntity);

	m_ViewportToPickFrom->UnregisterListener(this);
	m_ViewportToPickFrom = nullptr;
}

