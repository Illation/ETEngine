#include "stdafx.h"
#include "EditableEnvironmentMap.h"

#include <stb/stb_image.h>

#include <EtBuild/EngineVersion.h>

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
		.property("Cubemap Resolution", &EditableEnvironmentMapAsset::m_CubemapRes)
		.property("Irradiance Resolution", &EditableEnvironmentMapAsset::m_IrradianceRes)
		.property("Radiance Resolution", &EditableEnvironmentMapAsset::m_RadianceRes)
	END_REGISTER_CLASS_POLYMORPHIC(EditableEnvironmentMapAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableEnvironmentMapAsset) // force the asset class to be linked as it is only used in reflection


// static
std::string const EditableEnvironmentMapAsset::s_EnvMapPostFix("_envCube");
std::string const EditableEnvironmentMapAsset::s_IrradiancePostFix("_irradianceCube");
std::string const EditableEnvironmentMapAsset::s_RadiancePostFix("_radianceCube");


//---------------------------------------------
// EditableEnvironmentMapAsset::LoadFromMemory
//
// Loads an equirectangular texture, converts it to a cubemap, and prefilters irradiance and radiance cubemaps for IBL
//
bool EditableEnvironmentMapAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	render::TextureData* envCubemap = nullptr;
	render::TextureData* irradianceMap = nullptr;
	render::TextureData* radianceMap = nullptr;
	if (!CreateTextures(data, envCubemap, irradianceMap, radianceMap))
	{
		return false;
	}

	if (m_IsCompressed)
	{
		CompressHDRCube(envCubemap);
		CompressHDRCube(irradianceMap);
		CompressHDRCube(radianceMap);
	}

	SetData(new render::EnvironmentMap(envCubemap, irradianceMap, radianceMap));
	return true;
}

//---------------------------------------------------------
// EditableEnvironmentMapAsset::SetupRuntimeAssetsInternal
//
void EditableEnvironmentMapAsset::SetupRuntimeAssetsInternal()
{
	render::EnvironmentMapAsset* const mainAsset = new render::EnvironmentMapAsset(*static_cast<render::EnvironmentMapAsset*>(m_Asset));
	m_RuntimeAssets.emplace_back(mainAsset, true);

	render::TextureAsset* const envCubeMapAsset = new render::TextureAsset();
	envCubeMapAsset->SetName(core::FileUtil::RemoveExtension(mainAsset->GetName()) + s_EnvMapPostFix + "." + render::TextureFormat::s_TextureFileExt);
	envCubeMapAsset->SetPath(mainAsset->GetPath());
	envCubeMapAsset->SetPackageId(mainAsset->GetPackageId());
	envCubeMapAsset->m_ForceResolution = true;
	render::PbrPrefilter::PopulateCubeTextureParams(envCubeMapAsset->m_Parameters);
	m_RuntimeAssets.emplace_back(envCubeMapAsset, true);

	render::TextureAsset* const irradianceAsset = new render::TextureAsset();
	irradianceAsset->SetName(core::FileUtil::RemoveExtension(mainAsset->GetName()) + s_IrradiancePostFix + "." + render::TextureFormat::s_TextureFileExt);
	irradianceAsset->SetPath(mainAsset->GetPath());
	irradianceAsset->SetPackageId(mainAsset->GetPackageId());
	irradianceAsset->m_ForceResolution = true;
	render::PbrPrefilter::PopulateCubeTextureParams(irradianceAsset->m_Parameters);
	irradianceAsset->m_Parameters.genMipMaps = false;
	m_RuntimeAssets.emplace_back(irradianceAsset, true);

	render::TextureAsset* const radianceAsset = new render::TextureAsset();
	radianceAsset->SetName(core::FileUtil::RemoveExtension(mainAsset->GetName()) + s_RadiancePostFix + "." + render::TextureFormat::s_TextureFileExt);
	radianceAsset->SetPath(mainAsset->GetPath());
	radianceAsset->SetPackageId(mainAsset->GetPackageId());
	radianceAsset->m_ForceResolution = true;
	render::PbrPrefilter::PopulateCubeTextureParams(radianceAsset->m_Parameters);
	m_RuntimeAssets.emplace_back(radianceAsset, true);

	mainAsset->SetReferenceIds(std::vector<core::HashString>({ envCubeMapAsset->GetId(), irradianceAsset->GetId(), radianceAsset->GetId() }));
}

//-----------------------------------------------
// EditableEnvironmentMapAsset::GenerateInternal
//
bool EditableEnvironmentMapAsset::GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath)
{
	ET_UNUSED(buildConfig);
	ET_UNUSED(dbPath);

	auto const endsWithFn = [](std::string const& fullString, std::string const& ending)
		{
			if (fullString.length() >= ending.length())
			{
				return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
			}
			else
			{
				return false;
			}
		};

	// Write Data
	//--------------------
	RuntimeAssetData* mainData = nullptr;
	RuntimeAssetData* envData = nullptr;
	RuntimeAssetData* irradianceData = nullptr;
	RuntimeAssetData* radianceData = nullptr;
	for (RuntimeAssetData& data : m_RuntimeAssets)
	{
		data.m_HasGeneratedData = true;

		std::string const name = core::FileUtil::RemoveExtension(data.m_Asset->GetName());
		if (name == core::FileUtil::RemoveExtension(m_Asset->GetName()))
		{
			mainData = &data;
		}
		else if (endsWithFn(name, s_EnvMapPostFix))
		{
			envData = &data;
		}
		else if (endsWithFn(name, s_IrradiancePostFix))
		{
			irradianceData = &data;
		}
		else if (endsWithFn(name, s_RadiancePostFix))
		{
			radianceData = &data;
		}
		else
		{
			ET_ERROR("unexpected asset name '%s'", name.c_str());
			data.m_HasGeneratedData = false;
		}
	}

	ET_ASSERT(mainData != nullptr);
	ET_ASSERT(envData != nullptr);
	ET_ASSERT(irradianceData != nullptr);
	ET_ASSERT(radianceData != nullptr);

	// Create textures
	//------------------------
	render::TextureData* envCubemap = nullptr;
	render::TextureData* irradianceMap = nullptr;
	render::TextureData* radianceMap = nullptr;
	if (!CreateTextures(m_Asset->GetLoadData(), envCubemap, irradianceMap, radianceMap))
	{
		return false;
	}

	// generate
	//----------
	if (!GenerateTextureData(envData->m_GeneratedData, envCubemap))
	{
		ET_WARNING("failed to generate base map data");
		delete envCubemap;
		delete irradianceMap;
		delete radianceMap;
		return false;
	}

	if (!GenerateTextureData(irradianceData->m_GeneratedData, irradianceMap))
	{
		ET_WARNING("failed to generate irradiance map data");
		delete envCubemap;
		delete irradianceMap;
		delete radianceMap;
		return false;
	}

	if (!GenerateTextureData(radianceData->m_GeneratedData, radianceMap))
	{
		ET_WARNING("failed to generate radiance map data");
		delete envCubemap;
		delete irradianceMap;
		delete radianceMap;
		return false;
	}

	GenerateBinEnvMap(mainData->m_GeneratedData, envData->m_Asset->GetId(), irradianceData->m_Asset->GetId(), radianceData->m_Asset->GetId());

	delete envCubemap;
	delete irradianceMap;
	delete radianceMap;

	return true;
}

//---------------------------------------------
// EditableEnvironmentMapAsset::CreateTextures
//
bool EditableEnvironmentMapAsset::CreateTextures(std::vector<uint8> const& data, 
	render::TextureData*& env, 
	render::TextureData*& irradiance,
	render::TextureData*& radiance) const
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
		ET_WARNING("Failed to load hdr floats from data!");
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		ET_WARNING("Image is too small to display!");
		stbi_image_free(hdrFloats);
		return false;
	}

	render::TextureData hdrTexture(render::E_ColorFormat::RGB16f, ivec2(width, height));
	hdrTexture.UploadData(static_cast<void const*>(hdrFloats), render::E_ColorFormat::RGB, render::E_DataType::Float, 0u);

	// we have our equirectangular texture on the GPU so we can clean up the load data on the CPU
	stbi_image_free(hdrFloats);
	hdrFloats = nullptr;

	render::TextureParameters params(false);
	params.wrapS = render::E_TextureWrapMode::ClampToEdge;
	params.wrapT = render::E_TextureWrapMode::ClampToEdge;
	hdrTexture.SetParameters(params);

	env = render::EquirectangularToCubeMap(&hdrTexture, m_CubemapRes);
	render::PbrPrefilter::PrefilterCube(env, irradiance, radiance, m_CubemapRes, m_IrradianceRes, m_RadianceRes);

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
	compressedTex->SetParameters(params);

	// replace old pointer
	cubeMap = compressedTex;
}

//--------------------------------------------------
// EditableEnvironmentMapAsset::GenerateTextureData
//
bool EditableEnvironmentMapAsset::GenerateTextureData(std::vector<uint8>& data, render::TextureData const* const texture) const
{
	ivec2 const res = texture->GetResolution();
	core::BinaryWriter binWriter(data);

	if (m_IsCompressed)
	{
		// compress the cube map at all mip levels
		CompressedCube const cpuCube(*texture, m_CompressionQuality);

		// precalculate buffer size
		//--------------------------
		size_t bufferSize = 0u;
		uint8 mipCount = 0u;

		CompressedCube const* cubeMip = &cpuCube;
		while (cubeMip != nullptr)
		{
			bufferSize += cubeMip->GetCompressedData().size();
			++mipCount;

			cubeMip = cubeMip->GetChildMip();
		}

		// init binary writer
		//--------------------
		TextureCompression::WriteTextureHeader(binWriter, 
			bufferSize, 
			render::E_TextureType::CubeMap,
			static_cast<uint32>(res.x), 
			static_cast<uint32>(res.y), 
			mipCount - 1u, 
			render::E_ColorFormat::BC6H_RGB);
		binWriter.Write(render::E_DataType::Invalid);
		binWriter.Write(render::E_ColorFormat::Invalid);

		// write image data per level
		//----------------------------
		cubeMip = &cpuCube;
		while (cubeMip != nullptr)
		{
			binWriter.WriteData(cubeMip->GetCompressedData().data(), cubeMip->GetCompressedData().size());

			cubeMip = cubeMip->GetChildMip();
		}
	}
	else
	{
		static render::E_ColorFormat const s_Layout = render::E_ColorFormat::BGR;
		static render::E_DataType const s_DataType = render::E_DataType::Half;
		static size_t const s_PixelSize = static_cast<size_t>(render::TextureFormat::GetChannelCount(s_Layout)) 
			* static_cast<size_t>(render::DataTypeInfo::GetTypeSize(s_DataType));
		static size_t const s_MinMipSize = s_PixelSize * 4u * 4u * render::TextureData::s_NumCubeFaces; // make sure we don't have less than 4x4 textures

		ET_ASSERT(res.x >= 4u && res.y >= 4u);
			
		// precalculate buffer size
		//--------------------------
		uint8 mipCount = 0u;
		size_t bufferSize = s_PixelSize * res.x * res.y * render::TextureData::s_NumCubeFaces;
		{
			uint8 const texMipCount = static_cast<uint8>(texture->GetNumMipLevels()); 
			size_t mipSize = bufferSize;
			for (uint8 mipIdx = 0u; mipIdx < texMipCount; ++mipIdx)
			{
				mipSize /= 4u;
				if (mipSize < s_MinMipSize)
				{
					break;
				}

				bufferSize += mipSize;
				mipCount++;
			}
		}

		// init binary writer
		//--------------------
		TextureCompression::WriteTextureHeader(binWriter,
			bufferSize,
			render::E_TextureType::CubeMap,
			static_cast<uint32>(res.x),
			static_cast<uint32>(res.y),
			mipCount,
			render::E_ColorFormat::RGB16f);
		binWriter.Write(s_DataType);
		binWriter.Write(s_Layout);

		// write image data per level
		//----------------------------
		render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

		size_t mipSize = s_PixelSize * res.x * res.y * render::TextureData::s_NumCubeFaces;
		size_t bufferPos = binWriter.GetBufferPosition();
		for (uint8 mipIdx = 0u; mipIdx <= mipCount; ++mipIdx)
		{
			api->GetTextureData(*texture, mipIdx, s_Layout, s_DataType, reinterpret_cast<void*>(data.data() + bufferPos));
			bufferPos += mipSize;
			mipSize /= 4u;
		}
	}

	return true;
}

//------------------------------------------------
// EditableEnvironmentMapAsset::GenerateBinEnvMap
//
void EditableEnvironmentMapAsset::GenerateBinEnvMap(std::vector<uint8>& data, 
	core::HashString const env,
	core::HashString const irradiance,
	core::HashString const radiance) const
{
	core::BinaryWriter binWriter(data);
	binWriter.FormatBuffer(render::EnvironmentMapAsset::s_Header.size() +
		build::Version::s_Name.size() + 1u +
		sizeof(T_Hash) + 
		sizeof(T_Hash) + 
		sizeof(T_Hash));

	binWriter.WriteString(render::EnvironmentMapAsset::s_Header);
	binWriter.WriteNullString(build::Version::s_Name);

	binWriter.Write(env.Get());
	binWriter.Write(irradiance.Get());
	binWriter.Write(radiance.Get());
}


} // namespace pl
} // namespace et
