#include "stdafx.h"
#include "ShadedSceneRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneStructure/RenderScene.h>
#include <EtRendering/GraphicsTypes/EnvironmentMap.h>
#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/PlanetTech/StarField.h>


namespace render {


//=======================
// Shaded Scene Renderer
//=======================


//---------------------------------
// ShadedSceneRenderer::GetCurrent
//
// Utility function to retrieve the scene renderer for the currently active viewport
//
ShadedSceneRenderer* ShadedSceneRenderer::GetCurrent()
{
	Viewport* const viewport = Viewport::GetCurrentViewport();
	if (viewport == nullptr)
	{
		return nullptr;
	}

	I_ViewportRenderer* const viewRenderer = viewport->GetViewportRenderer();
	if (viewRenderer == nullptr)
	{
		return nullptr;
	}

	ET_ASSERT(viewRenderer->GetType() == typeid(ShadedSceneRenderer));

	return static_cast<ShadedSceneRenderer*>(viewRenderer);
}

//--------------------------------------------------------------------------


//---------------------------------
// ShadedSceneRenderer::c-tor
//
ShadedSceneRenderer::ShadedSceneRenderer(render::Scene* const renderScene)
	: I_ViewportRenderer()
	, m_RenderScene(renderScene)
{ }

//---------------------------------
// ShadedSceneRenderer::d-tor
//
// make sure all the singletons this system requires are uninitialized
//
ShadedSceneRenderer::~ShadedSceneRenderer()
{
	RenderingSystems::RemoveReference();
}

//-------------------------------------------
// ShadedSceneRenderer::InitRenderingSystems
//
// Create required buffers and subrendering systems etc
//
void ShadedSceneRenderer::InitRenderingSystems()
{
	RenderingSystems::AddReference();

	m_TextRenderer.Initialize();
	m_SpriteRenderer.Initialize();

	m_ShadowRenderer.Initialize();
	m_PostProcessing.Initialize();

	m_GBuffer.Initialize();
	m_GBuffer.Enable(true);

	m_SSR.Initialize();

	m_ClearColor = vec3(200.f / 255.f, 114.f / 255.f, 200.f / 255.f)*0.0f;

	m_SkyboxShader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdSkyboxShader.glsl"_hash);

	m_IsInitialized = true;
}

//---------------------------------
// ShadedSceneRenderer::OnResize
//
void ShadedSceneRenderer::OnResize(ivec2 const dim)
{
	m_Dimensions = dim;

	if (!m_IsInitialized)
	{
		return;
	}

	m_PostProcessing.~PostProcessingRenderer();
	new(&m_PostProcessing) PostProcessingRenderer();
	m_PostProcessing.Initialize();

	m_SSR.~ScreenSpaceReflections();
	new(&m_SSR) ScreenSpaceReflections();
	m_SSR.Initialize();
}

//---------------------------------
// ShadedSceneRenderer::OnRender
//
// Main scene drawing function
//
void ShadedSceneRenderer::OnRender(T_FbLoc const targetFb)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	// Global variables for all rendering systems
	//********************************************
	RenderingSystems::Instance()->GetSharedVarController().UpdataData(m_Camera, m_GBuffer);

	//Shadow Mapping
	//**************
	api->DebugPushGroup("shadow map generation");

	api->SetDepthEnabled(true);
	api->SetCullEnabled(true);

	auto lightIt = m_RenderScene->GetDirectionalLightsShaded().begin();
	auto shadowIt = m_RenderScene->GetDirectionalShadowData().begin();
	while ((lightIt != m_RenderScene->GetDirectionalLightsShaded().end()) && (shadowIt != m_RenderScene->GetDirectionalShadowData().end()))
	{
		mat4 const& transform = m_RenderScene->GetNodes()[lightIt->m_NodeId];
		m_ShadowRenderer.MapDirectional(transform, *shadowIt, this);

		lightIt++;
		shadowIt++;
	}

	api->DebugPopGroup();

	//Deferred Rendering
	//******************
	api->DebugPushGroup("deferred render pass");
	//Step one: Draw the data onto gBuffer
	m_GBuffer.Enable();

	//reset viewport
	api->SetViewport(ivec2(0), m_Dimensions);

	api->DebugPushGroup("clear previous pass");
	api->SetClearColor(vec4(m_ClearColor, 1.f));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);
	api->DebugPopGroup();

	// draw terrains
	api->DebugPushGroup("terrains");
	api->SetCullEnabled(false);
	Patch& patch = RenderingSystems::Instance()->GetPatch();
	for (Planet& planet : m_RenderScene->GetTerrains())
	{
		if (planet.GetTriangulator().Update(m_RenderScene->GetNodes()[planet.GetNodeId()], m_Camera))
		{
			planet.GetTriangulator().GenerateGeometry();
		}

		//Bind patch instances
		patch.BindInstances(planet.GetTriangulator().GetPositions());
		patch.UploadDistanceLUT(planet.GetTriangulator().GetDistanceLUT());
		patch.Draw(planet, m_RenderScene->GetNodes()[planet.GetNodeId()]);
	}
	api->DebugPopGroup();

	// render opaque objects to GBuffer
	api->DebugPushGroup("opaque objects");
	api->SetCullEnabled(true);
	DrawMaterialCollectionGroup(m_RenderScene->GetOpaqueRenderables());
	api->DebugPopGroup();

	api->DebugPushGroup("extensions");
	m_Events.Notify(E_RenderEvent::RenderDeferred, new RenderEventData(this, m_GBuffer.Get()));
	api->DebugPopGroup();

	api->DebugPopGroup();

	api->DebugPushGroup("lighting pass");
	// render ambient IBL
	api->DebugPushGroup("image based lighting");
	api->SetFaceCullingMode(E_FaceCullMode::Back);
	api->SetCullEnabled(false);

	m_SSR.EnableInput();
	m_GBuffer.Draw();
	api->DebugPopGroup();

	//copy Z-Buffer from gBuffer
	api->DebugPushGroup("blit");
	api->BindReadFramebuffer(m_GBuffer.Get());
	api->BindDrawFramebuffer(m_SSR.GetTargetFBO());
	api->CopyDepthReadToDrawFbo(m_Dimensions, m_Dimensions);
	api->DebugPopGroup();

	// Render Light Volumes
	api->DebugPushGroup("light volumes");
	api->SetDepthEnabled(false);
	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);

	api->SetCullEnabled(true);
	api->SetFaceCullingMode(E_FaceCullMode::Front);

	// pointlights
	api->DebugPushGroup("point lights");
	for (Light const& pointLight : m_RenderScene->GetPointLights())
	{
		mat4 const& transform = m_RenderScene->GetNodes()[pointLight.m_NodeId];
		float const scale = etm::length(etm::decomposeScale(transform));
		vec3 const pos = etm::decomposePosition(transform);

		RenderingSystems::Instance()->GetPointLightVolume().Draw(pos, scale, pointLight.m_Color);
	}
	api->DebugPopGroup();
	
	// direct
	api->DebugPushGroup("directional lights");
	for (Light const& dirLight : m_RenderScene->GetDirectionalLights())
	{
		mat4 const& transform = m_RenderScene->GetNodes()[dirLight.m_NodeId];
		vec3 const dir = (transform * vec4(vec3::FORWARD, 1.f)).xyz;
		RenderingSystems::Instance()->GetDirectLightVolume().Draw(dir, dirLight.m_Color);
	}
	api->DebugPopGroup();

	// direct with shadow
	api->DebugPushGroup("directional lights shadowed");
	lightIt = m_RenderScene->GetDirectionalLightsShaded().begin();
	shadowIt = m_RenderScene->GetDirectionalShadowData().begin();
	while ((lightIt != m_RenderScene->GetDirectionalLightsShaded().end()) && (shadowIt != m_RenderScene->GetDirectionalShadowData().end()))
	{
		Light const& dirLight = *lightIt;
		DirectionalShadowData const& shadow = *shadowIt;

		mat4 const& transform = m_RenderScene->GetNodes()[dirLight.m_NodeId];
		vec3 const dir = (transform * vec4(vec3::FORWARD, 1.f)).xyz;

		RenderingSystems::Instance()->GetDirectLightVolume().DrawShadowed(dir, dirLight.m_Color, shadow);

		lightIt++;
		shadowIt++;
	}
	api->DebugPopGroup();

	api->SetFaceCullingMode(E_FaceCullMode::Back);
	api->SetBlendEnabled(false);

	api->SetCullEnabled(false);
	api->DebugPopGroup(); // light volumes

	api->DebugPushGroup("extensions");
	m_Events.Notify(E_RenderEvent::RenderLights, new RenderEventData(this, m_SSR.GetTargetFBO()));
	api->DebugPopGroup(); 

	// draw SSR
	api->DebugPushGroup("reflections");
	m_PostProcessing.EnableInput();
	m_SSR.Draw();
	api->DebugPopGroup();
	// copy depth again
	api->DebugPushGroup("blit");
	api->BindReadFramebuffer(m_SSR.GetTargetFBO());
	api->BindDrawFramebuffer(m_PostProcessing.GetTargetFBO());
	api->CopyDepthReadToDrawFbo(m_Dimensions, m_Dimensions);
	api->DebugPopGroup();

	api->DebugPopGroup(); // lighting

	//Forward Rendering
	//******************
	api->DebugPushGroup("forward render pass");
	api->SetDepthEnabled(true);

	// draw skybox
	api->DebugPushGroup("skybox");
	Skybox const& skybox = m_RenderScene->GetSkybox();
	if (skybox.m_EnvironmentMap != nullptr)
	{
		api->SetShader(m_SkyboxShader.get());
		m_SkyboxShader->Upload("skybox"_hash, skybox.m_EnvironmentMap->GetRadiance());

		m_SkyboxShader->Upload("numMipMaps"_hash, skybox.m_EnvironmentMap->GetNumMipMaps());
		m_SkyboxShader->Upload("roughness"_hash, skybox.m_Roughness);

		api->SetDepthFunction(E_DepthFunc::LEqual);
		RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Cube>();
	}
	api->DebugPopGroup();

	// draw stars
	api->DebugPushGroup("stars");
	StarField const* const starfield = m_RenderScene->GetStarfield();
	if (starfield != nullptr)
	{
		starfield->Draw(m_Camera);
	}
	api->DebugPopGroup();

	// forward rendering
	api->DebugPushGroup("forward renderables");
	api->SetCullEnabled(true);
	DrawMaterialCollectionGroup(m_RenderScene->GetForwardRenderables());
	api->DebugPopGroup();

	api->DebugPushGroup("extensions");
	m_Events.Notify(E_RenderEvent::RenderForward, new RenderEventData(this, m_PostProcessing.GetTargetFBO()));
	api->DebugPopGroup();
	
	// draw atmospheres
	api->DebugPushGroup("atmospheres");
	if (m_RenderScene->GetAtmosphereInstances().size() > 0u)
	{
		api->SetFaceCullingMode(E_FaceCullMode::Front);
		api->SetDepthEnabled(false);
		api->SetBlendEnabled(true);
		api->SetBlendEquation(E_BlendEquation::Add);
		api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);

		for (AtmosphereInstance const& atmoInst : m_RenderScene->GetAtmosphereInstances())
		{
			vec3 const pos = etm::decomposePosition(m_RenderScene->GetNodes()[atmoInst.nodeId]);

			ET_ASSERT(atmoInst.lightId != core::INVALID_SLOT_ID);
			Light const& sun = m_RenderScene->GetLight(atmoInst.lightId);
			vec3 const sunDir = etm::normalize((m_RenderScene->GetNodes()[sun.m_NodeId] * vec4(vec3::FORWARD, 1.f)).xyz);

			m_RenderScene->GetAtmosphere(atmoInst.atmosphereId).Draw(pos, atmoInst.height, atmoInst.groundRadius, sunDir);
		}

		api->SetFaceCullingMode(E_FaceCullMode::Back);
		api->SetBlendEnabled(false);
		api->SetDepthEnabled(true);
	}
	api->DebugPopGroup();

	api->DebugPopGroup(); // forward render pass

	// add scene sprites before the overlay pass
	api->DebugPushGroup("post processing pass");
	SpriteRenderer::E_ScalingMode const scalingMode = SpriteRenderer::E_ScalingMode::Texture;
	for (Sprite const& sprite : m_RenderScene->GetSprites())
	{
		mat4 const& transform = m_RenderScene->GetNodes()[sprite.node];

		vec3 pos, scale;
		quat rot;
		etm::decomposeTRS(transform, pos, rot, scale);

		m_SpriteRenderer.Draw(sprite.texture.get(), pos.xy, sprite.color, sprite.pivot, scale.xy, rot.Roll(), pos.z, scalingMode);
	}

	// post processing
	api->SetCullEnabled(false);
	m_PostProcessing.Draw(targetFb, m_RenderScene->GetPostProcessingSettings(), this);
	api->DebugPopGroup(); // post processing pass
}

//---------------------------------
// ShadedSceneRenderer::DrawShadow
//
// Render the scene to the depth buffer of the current framebuffer
//
void ShadedSceneRenderer::DrawShadow(I_Material const* const nullMaterial)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	// No need to set shaders or upload material parameters as that is the calling functions responsibility
	for (MaterialCollection::Mesh const& mesh : m_RenderScene->GetShadowCasters().m_Meshes)
	{
		api->BindVertexArray(mesh.m_VAO);
		for (T_NodeId const node : mesh.m_Instances)
		{
			// #todo: collect a list of transforms and draw this instanced
			mat4 const& transform = m_RenderScene->GetNodes()[node];
			Sphere instSphere = Sphere((transform * vec4(mesh.m_BoundingVolume.pos, 1.f)).xyz,
				etm::length(etm::decomposeScale(transform)) * mesh.m_BoundingVolume.radius);

			if (true) // #todo: light frustum check
			{
				nullMaterial->GetBaseMaterial()->GetShader()->Upload("model"_hash, transform);
				api->DrawElements(E_DrawMode::Triangles, mesh.m_IndexCount, mesh.m_IndexDataType, 0);
			}
		}
	}
}

//--------------------------------------------------
// ShadedSceneRenderer::DrawMaterialCollectionGroup
//
// Draws all meshes in a list of shaders
//
void ShadedSceneRenderer::DrawMaterialCollectionGroup(core::slot_map<MaterialCollection> const& collectionGroup)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	for (MaterialCollection const& collection : collectionGroup)
	{
		api->SetShader(collection.m_Shader.get());
		for (MaterialCollection::MaterialInstance const& material : collection.m_Materials)
		{
			ET_ASSERT(collection.m_Shader.get() == material.m_Material->GetBaseMaterial()->GetShader());

			collection.m_Shader->UploadParameterBlock(material.m_Material->GetParameters());
			for (MaterialCollection::Mesh const& mesh : material.m_Meshes)
			{
				api->BindVertexArray(mesh.m_VAO);
				for (T_NodeId const node : mesh.m_Instances)
				{
					// #todo: collect a list of transforms and draw this instanced
					mat4 const& transform = m_RenderScene->GetNodes()[node];
					Sphere instSphere = Sphere((transform * vec4(mesh.m_BoundingVolume.pos, 1.f)).xyz, 
						etm::length(etm::decomposeScale(transform)) * mesh.m_BoundingVolume.radius);

					if (m_Camera.GetFrustum().ContainsSphere(instSphere) != VolumeCheck::OUTSIDE)
					{
						collection.m_Shader->Upload("model"_hash, transform);
						api->DrawElements(E_DrawMode::Triangles, mesh.m_IndexCount, mesh.m_IndexDataType, 0);
					}
				}
			}
		}
	}
}

//-----------------------------------
// ShadedSceneRenderer::DrawOverlays
//
// Post scene things which should be drawn to the viewport before anti aliasing
//
void ShadedSceneRenderer::DrawOverlays(T_FbLoc const targetFb)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DebugPushGroup("draw overlays");

	m_SpriteRenderer.Draw();
	m_TextRenderer.Draw();

	api->DebugPushGroup("extensions");
	m_Events.Notify(E_RenderEvent::RenderOutlines, new RenderEventData(this, targetFb));
	api->DebugPopGroup(); // extensions

	api->DebugPopGroup(); // draw overlays
}


} // namespace render
