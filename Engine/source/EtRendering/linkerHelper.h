#pragma once

#include <EtCore/Content/AssetStub.h>
#include <EtCore/Reflection/registerMath.h>
#include <EtRendering/Graphics/Shader.h>
#include <EtRendering/Graphics/Mesh.h>
#include <EtRendering/Graphics/EnvironmentMap.h>
#include <EtRendering/Graphics/SpriteFont.h>
#include <EtRendering/Graphics/TextureData.h>


namespace render {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	FORCE_LINKING(StubAsset)
	FORCE_LINKING(ShaderAsset)
	FORCE_LINKING(MeshAsset)
	FORCE_LINKING(EnvironmentMapAsset)
	FORCE_LINKING(FontAsset)
	FORCE_LINKING(TextureAsset)
	FORCE_LINKING(MathRegistrationLinkEnforcer)
}


}


