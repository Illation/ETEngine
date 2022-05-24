#include "stdafx.h"
#include "RmlRenderInterface.h"

#include <EtCore/Content/ResourceManager.h>


namespace et {
namespace gui {


//======================
// RML Render Interface
//======================

// static
Rml::CompiledGeometryHandle const RmlRenderInterface::s_InvalidGeometry = 0u;
Rml::TextureHandle const RmlRenderInterface::s_InvalidTexture = 0u;


//-------------------------------------
// RmlRenderInterface::c-tor
//
RmlRenderInterface::RmlRenderInterface() 
	: Rml::RenderInterface()
{
	m_GeneratedParameters.minFilter = render::E_TextureFilterMode::Linear;
	m_GeneratedParameters.magFilter = render::E_TextureFilterMode::Linear;
	m_GeneratedParameters.mipFilter = render::E_TextureFilterMode::Nearest;
	m_GeneratedParameters.wrapS = render::E_TextureWrapMode::ClampToEdge;
	m_GeneratedParameters.wrapT = render::E_TextureWrapMode::ClampToEdge;
	m_GeneratedParameters.borderColor = vec4(0.f);
}

//-------------------------------------
// RmlRenderInterface::CompileGeometry
//
// Store some geometry for later rendering - implementing this means we don't use the simple RenderGeometry() method
//
Rml::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(Rml::Vertex* vertices,
	int32 numVertices, 
	int32* indices, 
	int32 numIndices, 
	Rml::TextureHandle textureHandle)
{
	// create handle
	std::pair<T_Geometries::iterator, bool> res = m_Geometries.emplace(++m_LastGeometryHandle, Geometry());
	if (!res.second)
	{
		return s_InvalidGeometry;
	}

	// transferred data
	Geometry& geometry = res.first->second;
	geometry.m_NumVertices = numVertices;
	geometry.m_NumIndices = numIndices;

	// texture
	if (textureHandle != 0u)
	{
		T_Textures::iterator const foundIt = m_Textures.find(textureHandle);
		ET_ASSERT(foundIt != m_Textures.cend());

		geometry.m_Texture = foundIt->second.Get();
	}

	// create buffers
	int64 const iBufferSize = static_cast<int64>(numIndices * sizeof(int32));
	int64 const vBufferSize = static_cast<int64>(numVertices * sizeof(Rml::Vertex));

	geometry.m_VertexBuffer = m_GraphicsContext->CreateBuffer();
	m_GraphicsContext->BindBuffer(render::E_BufferType::Vertex, geometry.m_VertexBuffer);
	m_GraphicsContext->SetBufferData(render::E_BufferType::Vertex, vBufferSize, reinterpret_cast<void const*>(vertices), render::E_UsageHint::Static);

	geometry.m_IndexBuffer = m_GraphicsContext->CreateBuffer();
	m_GraphicsContext->BindBuffer(render::E_BufferType::Index, geometry.m_IndexBuffer);
	m_GraphicsContext->SetBufferData(render::E_BufferType::Index, iBufferSize, reinterpret_cast<void const*>(indices), render::E_UsageHint::Static);

	// create a new vertex array and input layout
	geometry.m_VertexArray = m_GraphicsContext->CreateVertexArray();
	m_GraphicsContext->BindVertexArray(geometry.m_VertexArray);
	//AttributeDescriptor::DefineAttributeArray(mesh->GetSupportedFlags(), m_Material->GetLayoutFlags(), m_Material->GetAttributeLocations());

	// done
	m_GraphicsContext->BindVertexArray(0u);

	return m_LastGeometryHandle;
}

//--------------------------------------------
// RmlRenderInterface::RenderCompiledGeometry
//
void RmlRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f const& translation)
{

}

//---------------------------------------------
// RmlRenderInterface::ReleaseCompiledGeometry
//
void RmlRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry)
{
	T_Geometries::iterator const foundIt = m_Geometries.find(geometry);
	ET_ASSERT(foundIt != m_Geometries.cend());

	Geometry& geo = foundIt->second;
	m_GraphicsContext->DeleteBuffer(geo.m_VertexBuffer);
	m_GraphicsContext->DeleteBuffer(geo.m_IndexBuffer);
	m_GraphicsContext->DeleteVertexArray(geo.m_VertexArray);

	m_Geometries.erase(foundIt);
}

//-----------------------------------------
// RmlRenderInterface::EnableScissorRegion
//
// Called by RmlUi when it wants to enable or disable scissoring to clip content
//
void RmlRenderInterface::EnableScissorRegion(bool enable)
{
	UNUSED(enable);
}

//--------------------------------------
// RmlRenderInterface::SetScissorRegion
//
// Called by RmlUi when it wants to change the scissor region
//
void RmlRenderInterface::SetScissorRegion(int32 x, int32 y, int32 width, int32 height)
{
	UNUSED(x);
	UNUSED(y);
	UNUSED(width);
	UNUSED(height);
}

//---------------------------------
// RmlRenderInterface::LoadTexture
//
bool RmlRenderInterface::LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, Rml::String const& source)
{
	core::HashString const assetId(source.c_str());

	AssetPtr<render::TextureData> texture = core::ResourceManager::Instance()->GetAssetData<render::TextureData>(assetId);
	if (texture == nullptr)
	{
		textureHandle = s_InvalidTexture;
		return false;
	}

	std::pair<T_Textures::iterator, bool> res = m_Textures.emplace(++m_LastTextureHandle, texture);
	ET_ASSERT(res.second);

	textureHandle = res.first->first;
	textureDimensions = Rml::Vector2i(texture->GetResolution().x, texture->GetResolution().y);

	return true;
}

//-------------------------------------
// RmlRenderInterface::GenerateTexture
//
bool RmlRenderInterface::GenerateTexture(Rml::TextureHandle& textureHandle, Rml::byte const* source, Rml::Vector2i const& sourceDimensions)
{
	UniquePtr<render::TextureData> tex = Create<render::TextureData>(render::E_ColorFormat::RGBA8, ivec2(sourceDimensions.x, sourceDimensions.y));
	tex->UploadData(reinterpret_cast<void const*>(source), render::E_ColorFormat::RGBA, render::E_DataType::UByte, 0u);
	tex->SetParameters(m_GeneratedParameters);
	tex->GenerateMipMaps();

	std::pair<T_Textures::iterator, bool> res = m_Textures.emplace(++m_LastTextureHandle, std::move(tex));
	ET_ASSERT(res.second);

	textureHandle = res.first->first;

	return true;
}

//------------------------------------
// RmlRenderInterface::ReleaseTexture
//
void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle textureHandle)
{
	size_t const erased = m_Textures.erase(textureHandle);
	ET_ASSERT(erased == 1u);
}

//----------------------------------
// RmlRenderInterface::SetTransform
//
void RmlRenderInterface::SetTransform(Rml::Matrix4f const* transform)
{
	if (transform != nullptr)
	{
		memcpy(reinterpret_cast<void *>(&m_CurrentTransform.data), reinterpret_cast<void const*>(transform->data()), sizeof(float) * 16);
		// might need to transpose after

		m_HasTransform = true;
	}
	else
	{
		m_CurrentTransform = mat4();
		m_HasTransform = false;
	}
}


} // namespace gui
} // namespace et

