#pragma once
#include <EtCore/linkerHelper.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/GraphicsTypes/TextureFormat.h>


namespace et {
namespace rhi {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	core::ForceLinking();

	FORCE_LINKING(ShaderAsset)
	FORCE_LINKING(TextureAsset)
	FORCE_LINKING(TextureFormat)
}


} // namespace rhi
} // namespace et
