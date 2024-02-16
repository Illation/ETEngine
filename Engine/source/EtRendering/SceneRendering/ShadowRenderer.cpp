#include "stdafx.h"
#include "ShadowRenderer.h"

#include <limits>

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>

#include <EtRendering/GraphicsTypes/Camera.h>
#include <EtRendering/GraphicsTypes/DirectionalShadowData.h>
#include <EtRendering/GraphicsTypes/Frustum.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace render {


//=================
// Shadow Renderer
//=================


//---------------------------------
// ShadowRenderer::Initialize
//
void ShadowRenderer::Initialize()
{
	m_Shader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("FwdNullShader.glsl"));
}

//---------------------------------
// ShadowRenderer::MapDirectional
//
// Fill out shadow data for a directional lights perspective relative to a camera
//
void ShadowRenderer::MapDirectional(mat4 const& lightTransform, DirectionalShadowData& shadowData, I_ShadowRenderer* const shadowRenderer)
{
	GraphicsSettings const& graphicsSettings = RenderingSystems::Instance()->GetGraphicsSettings();

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	//Calculate light camera matrix
	//*****************************
	//view
	vec3 worldPos = math::decomposePosition(lightTransform);
	vec3 lookAt = worldPos - (lightTransform * vec4(vec3::FORWARD, 1.f)).xyz;
	vec3 upVec = (lightTransform * vec4(vec3::UP, 1.f)).xyz;
	mat4 lightView = math::lookAt(worldPos, lookAt, upVec);

	//transform frustum into light space
	FrustumCorners corners = shadowRenderer->GetCamera().GetFrustum().GetCorners();
	corners.Transform(lightView);

	std::vector<DirectionalShadowData::CascadeData>& cascades = shadowData.AccessCascades();
	for (int32 i = 0; i < cascades.size(); i++)
	{
		//calculate orthographic projection matrix based on cascade
		float cascadeStart = (i == 0) ? 0 : cascades[i - 1].distance / graphicsSettings.CSMDrawDistance;
		float cascadeEnd = cascades[i].distance / graphicsSettings.CSMDrawDistance;
		std::vector<vec3> cascade;
		cascade.push_back(corners.na + (corners.fa - corners.na)*cascadeStart);
		cascade.push_back(corners.nb + (corners.fb - corners.nb)*cascadeStart);
		cascade.push_back(corners.nc + (corners.fc - corners.nc)*cascadeStart);
		cascade.push_back(corners.nd + (corners.fd - corners.nd)*cascadeStart);
		cascade.push_back(corners.fa + (corners.fa - corners.na)*cascadeEnd);
		cascade.push_back(corners.fb + (corners.fb - corners.nb)*cascadeEnd);
		cascade.push_back(corners.fc + (corners.fc - corners.nc)*cascadeEnd);
		cascade.push_back(corners.fd + (corners.fd - corners.nd)*cascadeEnd);

		float left = std::numeric_limits<float>::max();
		float right = std::numeric_limits<float>::lowest();
		float bottom = std::numeric_limits<float>::max();
		float top = std::numeric_limits<float>::lowest();
		float zFar = std::numeric_limits<float>::lowest();

		float zNear = -graphicsSettings.CSMDrawDistance;//temp, should be calculated differently

		for (size_t j = 0; j < cascade.size(); j++)
		{
			if (cascade[j].x < left) left = cascade[j].x;
			if (cascade[j].x > right) right = cascade[j].x;
			if (cascade[j].y < bottom) bottom = cascade[j].y;
			if (cascade[j].y > top) top = cascade[j].y;
			if (cascade[j].z > zFar) zFar = cascade[j].z;
		}

		float mult = 0.25f;
		mat4 lightProjection = math::orthographic(left*mult, right*mult, bottom*mult, top*mult, zNear, zFar*mult);

		//view projection
		mat4 lightVP = lightView * lightProjection;
		cascades[i].lightVP = lightVP;

		//Set viewport
		ivec2 res = cascades[i].texture->GetResolution();
		device->SetViewport(ivec2(0), res);
		//Set Framebuffer
		device->BindFramebuffer(cascades[i].fbo);
		//Clear Framebuffer
		device->Clear(rhi::E_ClearFlag::CF_Color | rhi::E_ClearFlag::CF_Depth);

		device->SetShader(m_Shader.get());
		m_Shader->Upload("worldViewProj"_hash, lightVP);

		//Draw scene with light matrix and null material
		shadowRenderer->DrawShadow(RenderingSystems::Instance()->GetNullMaterial());
	}
}


} // namespace render
} // namespace et
