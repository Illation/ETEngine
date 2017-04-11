#include "stdafx.hpp"

#include "ShadowRenderer.hpp"
#include <glm/gtx/transform.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "../Game/Materials/NullMaterial.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/TextureData.hpp"

ShadowRenderer::ShadowRenderer(){}
ShadowRenderer::~ShadowRenderer()
{
	SafeDelete(m_pMaterial);
}
void ShadowRenderer::Initialize()
{
	m_pMaterial = new NullMaterial();

	IsInitialized = true;
}

void ShadowRenderer::MapDirectional(TransformComponent *pTransform, DirectionalShadowData *pShadowData)
{
	//Set Framebuffer
	//Clear Framebuffer
	//Set viewport
	//Setup Depth stencil something
	//Calculate light camera matrix

	//Draw scene with light matrix and null material

	//reset viewport
}