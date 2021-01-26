#include "stdafx.h"
#include "EditableEnvironmentMap.h"

#include <stb/stb_image.h>

#include <EtCore/FileSystem/FileUtil.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtPipeline/Import/CompressedCube.h>


namespace et {
namespace pl {


//================================
// Editable Environment Map Asset
//================================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableEnvironmentMapAsset, "editable environment map asset")
		.property("is compressed", &EditableEnvironmentMapAsset::m_IsCompressed)
		.property("compression quality", &EditableEnvironmentMapAsset::m_CompressionQuality)
	END_REGISTER_CLASS_POLYMORPHIC(EditableEnvironmentMapAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableEnvironmentMapAsset) // force the asset class to be linked as it is only used in reflection


//---------------------------------------------
// EditableEnvironmentMapAsset::LoadFromMemory
//
// Loads an equirectangular texture, converts it to a cubemap, and prefilters irradiance and radiance cubemaps for IBL
//
bool EditableEnvironmentMapAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	render::EnvironmentMapAsset const* const envMapAsset = static_cast<render::EnvironmentMapAsset const*>(m_Asset);

	//load equirectangular texture
	//****************************
	std::string const extension = core::FileUtil::ExtractExtension(envMapAsset->GetName());
	ET_ASSERT(extension == "hdr", "Expected HDR file format!");

	stbi_set_flip_vertically_on_load(true);
	int32 width = 0;
	int32 height = 0;
	int32 channels = 0;
	float* hdrFloats = stbi_loadf_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &channels, 0);

	if (hdrFloats == nullptr)
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Failed to load hdr floats from data!", core::LogLevel::Warning);
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Image is too small to display!", core::LogLevel::Warning);
		stbi_image_free(hdrFloats);
		return false;
	}

	render::TextureData hdrTexture(render::E_ColorFormat::RGB16f, ivec2(width, height));
	hdrTexture.UploadData(static_cast<void const*>(hdrFloats), render::E_ColorFormat::RGB, render::E_DataType::Float);

	// we have our equirectangular texture on the GPU so we can clean up the load data on the CPU
	stbi_image_free(hdrFloats);
	hdrFloats = nullptr;

	render::TextureParameters params(false);
	params.wrapS = render::E_TextureWrapMode::ClampToEdge;
	params.wrapT = render::E_TextureWrapMode::ClampToEdge;
	hdrTexture.SetParameters(params);

	render::TextureData* envCubemap = render::EquirectangularToCubeMap(&hdrTexture, envMapAsset->m_CubemapRes);

	render::TextureData* irradianceMap = nullptr;
	render::TextureData* radianceMap = nullptr;
	render::PbrPrefilter::PrefilterCube(envCubemap, irradianceMap, radianceMap, 
		envMapAsset->m_CubemapRes, 
		envMapAsset->m_IrradianceRes, 
		envMapAsset->m_RadianceRes);

	//if (m_IsCompressed)
	//{
	//	CompressHDRCube(envCubemap);
	//	CompressHDRCube(irradianceMap);
	//	CompressHDRCube(radianceMap);
	//}

	SetData(new render::EnvironmentMap(envCubemap, irradianceMap, radianceMap));
	return true;
}

//----------------------------------------------
// EditableEnvironmentMapAsset::CompressHDRCube
//
// Convert a cubemap into a BC6H compressed cubemap
//
void EditableEnvironmentMapAsset::CompressHDRCube(render::TextureData*& cubeMap) const
{
	// temp tex pointer
	render::TextureData* const compressedTex = new render::TextureData(render::E_TextureType::CubeMap, 
		render::E_ColorFormat::BC6H_RGB, 
		cubeMap->GetResolution());

	// cache parameters
	render::TextureParameters params = cubeMap->GetParameters();

	// compress the cube map at all mip levels
	CompressedCube const cpuCube(*cubeMap, m_CompressionQuality);

	// delete old cubemap
	delete cubeMap;

	// upload new compressed data
	CompressedCube const* cubeMip = &cpuCube;
	int32 mipLevel = 0;
	while (cubeMip != nullptr)
	{
		compressedTex->UploadCompressed(reinterpret_cast<void const*>(cubeMip->GetCompressedData().data()), cubeMip->GetCompressedData().size(), mipLevel);

		++mipLevel;
		cubeMip = cubeMip->GetChildMip();
	}

	// upload params
	params.genMipMaps = false; // we already uploaded the mip maps
	compressedTex->SetParameters(params);

	// replace old pointer
	cubeMap = compressedTex;
}


} // namespace pl
} // namespace et
