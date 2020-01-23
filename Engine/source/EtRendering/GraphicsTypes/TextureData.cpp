#include "stdafx.h"
#include "TextureData.h"

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <EtCore/Reflection/Registration.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace render {


//==============
// Texture Data
//==============


//---------------------------------
// TextureData::c-tor
//
// Create the texture and generate a new GPU texture resource
//
TextureData::TextureData(ivec2 const res, E_ColorFormat const intern, E_ColorFormat const format, E_DataType const type, int32 const depth)
	: m_Resolution(res)
	, m_Internal(intern)
	, m_Format(format)
	, m_DataType(type)
	, m_Depth(depth)
	, m_TargetType((depth > 1) ? E_TextureType::Texture3D : E_TextureType::Texture2D)
{
	m_Location = Viewport::GetCurrentApiContext()->GenerateTexture();
}

//---------------------------------
// TextureData::TextureData
//
// Create a texture of a specific type with a preexisting handle
//
TextureData::TextureData(E_TextureType const targetType, ivec2 const res)
	: m_TargetType(targetType)
	, m_Resolution(res)
	, m_Internal(E_ColorFormat::RGB)
	, m_Format(E_ColorFormat::RGB)
	, m_DataType(E_DataType::Float)
{
	m_Location = Viewport::GetCurrentApiContext()->GenerateTexture();
}

//---------------------------------
// TextureData::d-tor
//
// Destroys the GPU resource
//
TextureData::~TextureData()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
	if (m_Handle != 0u)
	{
		api->SetTextureHandleResidency(m_Handle, false);
	}

	api->DeleteTexture(m_Location);
}

//---------------------------------
// TextureData::Build
//
// send the data to the GPU location. Can be initialized without any image data
//
void TextureData::Build(void* data)
{
	ET_ASSERT(m_Handle == 0u, "Shouldn't build after a handle was created!");

	Viewport::GetCurrentApiContext()->SetTextureData(*this, data);
}

//---------------------------------
// TextureData::SetParameters
//
// Sets parameters on a texture, can optionally be enforced
// This function should be called either way when a new texture is created, 
//  - because if we leave the default parameters OpenGL expects us to generate a mip map, or we can disable mip maps which will change the min filter
//
void TextureData::SetParameters(TextureParameters const& params, bool const force)
{
	ET_ASSERT(m_Handle == 0u, "Shouldn't set parameters after a handle was created!");

	Viewport::GetCurrentApiContext()->SetTextureParams(*this, m_MipLevels, m_Parameters, params, force);
}

//---------------------------------
// TextureData::Resize
//
// returns true if regenerated 
//  - if the texture is a framebuffer texture upscaling won't work properly unless it is reatached to the framebuffer object
//
bool TextureData::Resize(ivec2 const& newSize)
{
	bool const hasHandle = (m_Handle != 0u);

	bool const regenerate = (newSize.x > m_Resolution.x) || (newSize.y > m_Resolution.y) || hasHandle;

	m_Resolution = newSize;

	if (regenerate)
	{
		I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

		if (hasHandle)
		{
			api->SetTextureHandleResidency(m_Handle, false);
			m_Handle = 0u;
		}

		api->DeleteTexture(m_Location);
		m_Location = api->GenerateTexture();
	}

	Build();

	if (regenerate)
	{
		SetParameters(m_Parameters, true);
	}

	if (hasHandle)
	{
		CreateHandle();
	}

	return regenerate;
}

//---------------------------------
// TextureData::CreateHandle
//
// Create a handle that allows bindless use of the texture. After the handle is created, no other modifying operations should be done (except resize)
//
void TextureData::CreateHandle()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_Handle = api->GetTextureHandle(m_Location);
	api->SetTextureHandleResidency(m_Handle, true); // #todo: in the future we should have a system that makes inactive handles non resident after a while
}

//===================
// Texture Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_POLYMORPHIC_CLASS(TextureAsset, "texture asset")
		.property("use SRGB", &TextureAsset::m_UseSrgb)
		.property("force resolution", &TextureAsset::m_ForceResolution)
		.property("parameters", &TextureAsset::m_Parameters)
	END_REGISTER_POLYMORPHIC_CLASS(TextureAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(TextureAsset) // force the shader class to be linked as it is only used in reflection


//---------------------------------
// TextureAsset::LoadFromMemory
//
// Load texture data from binary asset content, and place it on the GPU
//
bool TextureAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// check image format

	stbi_set_flip_vertically_on_load(false);
	int32 width = 0;
	int32 height = 0;
	int32 channels = 0;

	static int32 const s_TargetNumChannels = 3; // for now we only support opaque textures

	// option to load 16 bit texture
	uint8* bits = stbi_load_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &channels, s_TargetNumChannels);

	if (bits == nullptr)
	{
		LOG("TextureAsset::LoadFromMemory > Failed to load texture bytes from data!", core::LogLevel::Warning);
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		LOG("TextureAsset::LoadFromMemory > Image is too small to display!", core::LogLevel::Warning);
		stbi_image_free(bits);
		return false;
	}

	render::GraphicsSettings const& graphicsSettings = RenderingSystems::Instance()->GetGraphicsSettings();
	if (!etm::nearEquals(graphicsSettings.TextureScaleFactor, 1.f))
	{
		if (!m_ForceResolution)
		{
			// resize
			int32 const outWidth = static_cast<int32>(static_cast<float>(width) * graphicsSettings.TextureScaleFactor);
			int32 const outHeight = static_cast<int32>(static_cast<float>(height) * graphicsSettings.TextureScaleFactor);
			uint8* outBits = new uint8[outWidth * outHeight * channels];

			stbir_resize_uint8(bits, width, height, 0, outBits, outWidth, outHeight, 0, channels);

			stbi_image_free(bits);
			bits = outBits;
			width = outWidth;
			height = outHeight;
		}
	}

	// convert data type
	// check number of channels

	//Upload to GPU
	m_Data = new TextureData(ivec2(width, height), (m_UseSrgb ? E_ColorFormat::SRGB : E_ColorFormat::RGB), E_ColorFormat::RGB, E_DataType::UByte);
	m_Data->Build((void*)bits);
	m_Data->SetParameters(m_Parameters);

	m_Data->CreateHandle();

	stbi_image_free(bits);
	bits = nullptr;

	return true;
}


} // namespace render
} // namespace et
