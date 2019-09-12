#pragma once

#include <EtCore/Content/AssetStub.h>
#include <EtCore/Reflection/registerMath.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/Mesh.h>
#include <Engine/Audio/AudioData.h>
#include <Engine/Graphics/EnvironmentMap.h>
#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Graphics/TextureData.h>

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
	FORCE_LINKING(AudioAsset)
	FORCE_LINKING(EnvironmentMapAsset)
	FORCE_LINKING(FontAsset)
	FORCE_LINKING(TextureAsset)
	FORCE_LINKING(MathRegistrationLinkEnforcer)
}


