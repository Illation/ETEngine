#include "stdafx.h"
#include "EntityIdRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/FrameBuffer.h>
#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>

#include <EtFramework/Components/ModelComponent.h>
#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace edit {


//====================
// Entity ID Renderer
//====================


//---------------------------------
// EntityIdRenderer::d-tor
//
EntityIdRenderer::~EntityIdRenderer()
{
	DestroyRenderTarget();
}

//---------------------------------
// EntityIdRenderer::Initialize
//
void EntityIdRenderer::Initialize()
{
	m_Shader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("editor_only/Shaders/FwdIdShader.glsl"));
	m_Material = core::ResourceManager::Instance()->GetAssetData<render::Material>(core::HashString("editor_only/Materials/M_Id.json"));

	CreateRenderTarget();
}

//---------------------------------
// EntityIdRenderer::GenerateRenderTarget
//
void EntityIdRenderer::CreateRenderTarget()
{
	using namespace render;

	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	ivec2 dim;
	render::Viewport* const vp = Viewport::GetCurrentViewport();
	if (vp != nullptr)
	{
		dim = vp->GetDimensions();
	}
	else
	{
		api->GetViewport(ivec2(), dim);
	}

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Nearest;
	params.magFilter = E_TextureFilterMode::Nearest;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_DrawTarget);
	api->BindFramebuffer(m_DrawTarget);
	m_DrawTex = new TextureData(E_ColorFormat::RGBA8, dim);
	m_DrawTex->AllocateStorage();
	m_DrawTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, m_DrawTex->GetLocation(), 0);

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
	using namespace render;

	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

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
void EntityIdRenderer::Pick(ivec2 const pixel, render::Viewport* const viewport, std::function<void(fw::T_EntityId const)>& onEntityPicked)
{
	if (m_ViewportToPickFrom == nullptr)
	{
		m_PixelToPick = pixel;
		m_ViewportToPickFrom = viewport;
		m_OnEntityPicked = onEntityPicked;

		m_VPCallbackId = m_ViewportToPickFrom->GetEventDispatcher().Register(render::E_ViewportEvent::VP_PreRender | render::E_ViewportEvent::VP_PostFlush,
			render::T_ViewportEventCallback([this](render::T_ViewportEventFlags const flags, render::ViewportEventData const* const data) -> void
			{
				if (flags & render::E_ViewportEvent::VP_PreRender)
				{
					OnViewportPreRender(data->targetFb);
				}
				else
				{
					OnViewportPostFlush(data->targetFb);
				}
			}));
	}
	else
	{
		LOG("EntityIdRenderer::Pick > some system is already picking for a viewport this frame!", core::LogLevel::Warning);
	}
}

//---------------------------------
// EntityIdRenderer::OnViewportPreRender
//
// Runs before the viewports rendererer does its thing
//
void EntityIdRenderer::OnViewportPreRender(render::T_FbLoc const targetFb)
{
	using namespace render;

	I_GraphicsContextApi* const api = m_ViewportToPickFrom->GetApiContext();

	// extract the camera from the viewport
	Camera const* camera = nullptr;

	I_ViewportRenderer* const viewRenderer = m_ViewportToPickFrom->GetViewportRenderer();
	ET_ASSERT(viewRenderer != nullptr);
	if (viewRenderer->GetType() == rttr::type::get<render::ShadedSceneRenderer>())
	{
		render::ShadedSceneRenderer* const shadedSceneRenderer = static_cast<render::ShadedSceneRenderer*>(viewRenderer);
		camera = &(shadedSceneRenderer->GetCamera());

		// we need to manually update the camera info a bit earlier during this draw call
		RenderingSystems::Instance()->GetSharedVarController().UpdataData(*camera, shadedSceneRenderer->GetGBuffer());
	}
	else
	{
		ET_ASSERT(true, "Can't find camera foro current view renderer");
		return;
	}

	ivec2 const dim = m_ViewportToPickFrom->GetDimensions();

	// if the view size changed since the last pick we need to recreate our rendertarget with the correct size
	if (!(math::nearEqualsV(m_LastViewSize, dim)))
	{
		DestroyRenderTarget();
		CreateRenderTarget();

		m_LastViewSize = dim;
	}

	api->SetViewport(ivec2(0), dim);

	// draw the shapes as colors to the intermediate rendertarget
	//------------------------------------------------------------
	api->BindFramebuffer(m_DrawTarget);

	vec4 invalidCol;
	GetIdColor(fw::INVALID_ENTITY_ID, invalidCol);
	api->SetClearColor(invalidCol);
	api->Clear(E_ClearFlag::CF_Color | E_ClearFlag::CF_Depth);

	api->SetShader(m_Shader.get());

	api->SetDepthEnabled(true);

	ET_ASSERT(fw::UnifiedScene::Instance().GetSceneId() != 0u);
	std::vector<fw::T_EntityId> const& entities = fw::UnifiedScene::Instance().GetEcs().GetEntities();
	for (fw::T_EntityId const entity : entities)
	{
		DrawEntity(entity, *camera);
	}

	api->BindFramebuffer(targetFb);
}

//---------------------------------
// EntityIdRenderer::OnViewportPostFlush
//
// Runs after everything else has finished rendering
//
void EntityIdRenderer::OnViewportPostFlush(render::T_FbLoc const targetFb)
{
	using namespace render;

	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

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
	fw::T_EntityId pickedID =
		static_cast<uint32>(pixels[0]) +
		static_cast<uint32>(pixels[1]) * 256 +
		static_cast<uint32>(pixels[2]) * 256 * 256 +
		static_cast<uint32>(pixels[3]) * 256 * 256 * 256;

	m_OnEntityPicked(pickedID);

	m_ViewportToPickFrom->GetEventDispatcher().Unregister(m_VPCallbackId);
	m_ViewportToPickFrom = nullptr;
}

//------------------------------
// EntityIdRenderer::DrawEntity
//
// Render the components of an entity, if possible
//
void EntityIdRenderer::DrawEntity(fw::T_EntityId const entity, render::Camera const& camera) const
{
	using namespace render;

	fw::EcsController const& ecs = fw::UnifiedScene::Instance().GetEcs();

	// For now we only pick models
	if (!(ecs.HasComponent<fw::ModelComponent>(entity) && ecs.HasComponent<fw::TransformComponent>(entity)))
	{
		return;
	}

	fw::ModelComponent const& modelComp = ecs.GetComponent<fw::ModelComponent>(entity);
	fw::TransformComponent const& transfComp = ecs.GetComponent<fw::TransformComponent>(entity);

	// check the cameras frustum
	MeshData const* const mesh = modelComp.GetMesh().get();

	math::Sphere const& bv = mesh->GetBoundingSphere();
	math::Sphere const instSphere((transfComp.GetWorld() * vec4(bv.pos, 1.f)).xyz, math::length(transfComp.GetScale()) * bv.radius);

	if (camera.GetFrustum().ContainsSphere(instSphere) != VolumeCheck::OUTSIDE)
	{
		vec4 color;
		GetIdColor(entity, color);

		// upload the color to the material and adraw the entity with it
		m_Shader->Upload("uColor"_hash, color);

		I_GraphicsContextApi* const api = m_ViewportToPickFrom->GetApiContext();;

		MeshSurface const* surface = modelComp.GetMesh()->GetSurface(m_Material.get());

		api->BindVertexArray(surface->GetVertexArray());
		m_Shader->Upload("model"_hash, transfComp.GetWorld());

		api->DrawElements(E_DrawMode::Triangles, static_cast<uint32>(mesh->GetIndexCount()), mesh->GetIndexDataType(), 0);
	}
}

//------------------------------
// EntityIdRenderer::GetIdColor
//
// convert an entities ID to a color
//
void EntityIdRenderer::GetIdColor(fw::T_EntityId const id, vec4& col) const
{
	uint32 r = (id & 0x000000FF) >> 0;
	uint32 g = (id & 0x0000FF00) >> 8;
	uint32 b = (id & 0x00FF0000) >> 16;
	uint32 a = (id & 0xFF000000) >> 24;

	col = vec4(static_cast<float>(r) / 255.f,
		static_cast<float>(g) / 255.f,
		static_cast<float>(b) / 255.f,
		static_cast<float>(a) / 255.f);
}


} // namespace edit
} // namespace et
