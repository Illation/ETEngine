#include "stdafx.h"
#include "RmlRenderInterface.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtGUI/Context/RmlUtil.h>


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

	std::vector<uint8> emptyTexData({
		255, 255, 255, 255,    255, 255, 255, 255,
		255, 255, 255, 255,    255, 255, 255, 255
		});
	m_EmptyWhiteTex2x2 = GenTextureInternal(reinterpret_cast<void const*>(emptyTexData.data()), ivec2(2));
}

//-------------------------------------
// RmlRenderInterface::SetShader
//
void RmlRenderInterface::SetShader(AssetPtr<render::ShaderData> const& shader, AssetPtr<render::ShaderData> const& textShader)
{
	m_Shader = shader;
	m_TextShader = textShader;
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

	// texture
	//---------
	if (textureHandle != 0u)
	{
		T_Textures::iterator const foundIt = m_Textures.find(textureHandle);
		ET_ASSERT(foundIt != m_Textures.cend());

		geometry.m_Texture = foundIt->second.Get();
		geometry.m_Font = foundIt->second.GetFont();
	}
	else
	{
		geometry.m_Texture = m_EmptyWhiteTex2x2;
	}

	// create a new vertex array and input layout with vertex and index buffers
	//--------------------------------------------------------------------------
	geometry.m_VertexArray = m_GraphicsContext->CreateVertexArray();
	m_GraphicsContext->BindVertexArray(geometry.m_VertexArray);

	// index buffer is the same for generic and text geometry
	geometry.m_NumIndices = numIndices;
	int64 const iBufferSize = static_cast<int64>(geometry.m_NumIndices * sizeof(uint32));

	geometry.m_IndexBuffer = m_GraphicsContext->CreateBuffer();
	m_GraphicsContext->BindBuffer(render::E_BufferType::Index, geometry.m_IndexBuffer);
	m_GraphicsContext->SetBufferData(render::E_BufferType::Index, iBufferSize, reinterpret_cast<void const*>(indices), render::E_UsageHint::Static);

	// the vertex data needs to be treated differently depending on if it's generic or for text
	geometry.m_VertexBuffer = m_GraphicsContext->CreateBuffer();
	m_GraphicsContext->BindBuffer(render::E_BufferType::Vertex, geometry.m_VertexBuffer);

	if (geometry.m_Font == nullptr)
	{
		geometry.m_NumVertices = numVertices;

		int32 const vertSize = sizeof(Rml::Vertex);
		int64 const vBufferSize = static_cast<int64>(geometry.m_NumVertices * vertSize);
		m_GraphicsContext->SetBufferData(render::E_BufferType::Vertex, vBufferSize, reinterpret_cast<void const*>(vertices), render::E_UsageHint::Static);

		m_GraphicsContext->SetVertexAttributeArrayEnabled(0, true);
		m_GraphicsContext->SetVertexAttributeArrayEnabled(1, true);
		m_GraphicsContext->SetVertexAttributeArrayEnabled(2, true);

		m_GraphicsContext->DefineVertexAttributePointer(0, 2, render::E_DataType::Float, false, vertSize, offsetof(Rml::Vertex, position));
		m_GraphicsContext->DefineVertexAttributePointer(1, 4, render::E_DataType::UByte, true, vertSize, offsetof(Rml::Vertex, colour));
		m_GraphicsContext->DefineVertexAttributePointer(2, 2, render::E_DataType::Float, false, vertSize, offsetof(Rml::Vertex, tex_coord));
	}
	else
	{
		// reconvert stored geometry data from how it was packed into Rml::Vertices in the font engine interface
		int32 const numChars = numIndices / 6;
		ET_ASSERT(numChars * 6 == numIndices);
		geometry.m_NumVertices = numChars * 4;

		uint8 const* dataPtr = reinterpret_cast<uint8 const*>(vertices);
		FontParameters const& fontParams = *reinterpret_cast<FontParameters const*>(dataPtr);
		dataPtr += FontParameters::GetVCount() * sizeof(Rml::Vertex);

		// vertex buffer
		int32 const vertSize = sizeof(TextVertex);
		int64 const bufferSize = static_cast<int64>(geometry.m_NumVertices * vertSize);
		m_GraphicsContext->SetBufferData(render::E_BufferType::Vertex, bufferSize, reinterpret_cast<void const*>(dataPtr), render::E_UsageHint::Static);

		m_GraphicsContext->SetVertexAttributeArrayEnabled(0, true);
		m_GraphicsContext->SetVertexAttributeArrayEnabled(1, true);
		m_GraphicsContext->SetVertexAttributeArrayEnabled(2, true);

		m_GraphicsContext->DefineVertexAttributePointer(0, 2, render::E_DataType::Float, false, vertSize, offsetof(TextVertex, m_Position));
		m_GraphicsContext->DefineVertexAttributePointer(1, 2, render::E_DataType::Float, false, vertSize, offsetof(TextVertex, m_TexCoord));
		m_GraphicsContext->DefineVertexAttribIPointer(2, 1, render::E_DataType::UByte, vertSize, offsetof(TextVertex, m_Channel));

		// setup instance buffer
		geometry.m_InstanceCount = fontParams.m_LayerCount;
		std::vector<TextLayer> layerInstances;
		layerInstances.reserve(fontParams.m_LayerCount);
		for (size_t layerIdx = 0u; layerIdx < fontParams.m_LayerCount; ++layerIdx)
		{
			layerInstances.push_back(fontParams.m_Layers.Get()[layerIdx]);
			TextLayer& inst = layerInstances.back();
			if (layerIdx == fontParams.m_MainLayerIdx)
			{
				inst.m_Color = fontParams.m_MainLayerColor;
			}

			inst.m_SdfThreshold = math::Clamp01(inst.m_SdfThreshold + fontParams.m_SdfThreshold);
			inst.m_MinThreshold = math::Clamp01(inst.m_MinThreshold + fontParams.m_SdfThreshold);
		}

		geometry.m_VertexBufferInstances = m_GraphicsContext->CreateBuffer();
		m_GraphicsContext->BindBuffer(render::E_BufferType::Vertex, geometry.m_VertexBufferInstances);
		m_GraphicsContext->SetBufferData(render::E_BufferType::Vertex,
			layerInstances.size() * sizeof(TextLayer),
			layerInstances.data(),
			render::E_UsageHint::Static);

		m_GraphicsContext->SetVertexAttributeArrayEnabled(3, true);
		m_GraphicsContext->SetVertexAttributeArrayEnabled(4, true);
		m_GraphicsContext->SetVertexAttributeArrayEnabled(5, true);
		m_GraphicsContext->SetVertexAttributeArrayEnabled(6, true);
		m_GraphicsContext->SetVertexAttributeArrayEnabled(7, true);
		m_GraphicsContext->DefineVertexAttributePointer(3, 2, render::E_DataType::Float, false, sizeof(TextLayer), offsetof(TextLayer, m_Offset));
		m_GraphicsContext->DefineVertexAttributePointer(4, 4, render::E_DataType::Float, false, sizeof(TextLayer), offsetof(TextLayer, m_Color));
		m_GraphicsContext->DefineVertexAttributePointer(5, 1, render::E_DataType::Float, false, sizeof(TextLayer), offsetof(TextLayer, m_SdfThreshold));
		m_GraphicsContext->DefineVertexAttributePointer(6, 1, render::E_DataType::Float, false, sizeof(TextLayer), offsetof(TextLayer, m_MinThreshold));
		m_GraphicsContext->DefineVertexAttribIPointer(7, 1, render::E_DataType::UByte, sizeof(TextLayer), offsetof(TextLayer, m_IsBlurred));
		m_GraphicsContext->DefineVertexAttribDivisor(3, 1);
		m_GraphicsContext->DefineVertexAttribDivisor(4, 1);
		m_GraphicsContext->DefineVertexAttribDivisor(5, 1);
		m_GraphicsContext->DefineVertexAttribDivisor(6, 1);
		m_GraphicsContext->DefineVertexAttribDivisor(7, 1);
	}

	// done
	m_GraphicsContext->BindVertexArray(0u);

	return m_LastGeometryHandle;
}

//--------------------------------------------
// RmlRenderInterface::RenderCompiledGeometry
//
void RmlRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f const& translation)
{
	// the geometry in question
	T_Geometries::const_iterator const foundIt = m_Geometries.find(geometry);
	ET_ASSERT(foundIt != m_Geometries.cend());

	Geometry const& geo = foundIt->second;

	// scissor rectangle
	if (m_IsScissorEnabled)
	{
		if (m_HasTransform)
		{
			// no real scissor rectangle needed
			m_GraphicsContext->SetScissorEnabled(false);
			m_GraphicsContext->SetStencilEnabled(true);

			// avoid redrawing stencil buffer
			if (!(math::nearEqualsV(m_ScissorPos, m_LastScissorPos) 
				&& math::nearEqualsV(m_ScissorSize, m_LastScissorSize) 
				&& math::nearEqualsM(m_CurrentTransform, m_LastTransform)))
			{
				m_LastScissorPos = m_ScissorPos;
				m_LastScissorSize = m_ScissorSize;
				m_LastTransform = m_CurrentTransform;

				// clear stencil buffer
				m_GraphicsContext->SetStencilMask(0xFFFFFFFFu);
				m_GraphicsContext->Clear(render::E_ClearFlag::CF_Stencil);

				// fill stencil buffer
				m_GraphicsContext->SetDepthMask(false);
				m_GraphicsContext->SetStencilFunction(render::T_StencilFunc::Never, 1, 0xFFFFFFFFu);
				m_GraphicsContext->SetStencilOperation(render::E_StencilOp::Replace, render::E_StencilOp::Keep, render::E_StencilOp::Keep);

				if (m_NullShader == nullptr)
				{
					m_NullShader = render::RenderingSystems::Instance()->GetNullMaterial()->GetShaderAsset();
				}

				m_GraphicsContext->SetShader(m_NullShader.get());

				mat4 quadTransform = math::translate(vec3(1.f, 1.f, 0.f)) * math::scale(vec3(0.5f, 0.5f, 1.f)) * // move to 0/0/0
					math::scale(vec3(math::vecCast<float>(m_ScissorSize), 0.f)) * math::translate(vec3(math::vecCast<float>(m_ScissorPos), 0.f)); // rec 
				quadTransform = quadTransform * m_CurrentTransform; // apply transform to scissor rectangle
				m_NullShader->Upload("model"_hash, quadTransform);

				m_NullShader->Upload("worldViewProj"_hash, m_ViewProj); // convert from UI to screen coordinates and perform vertical flip

				render::RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<render::primitives::Quad>();

				// reset for normal rendering
				m_GraphicsContext->SetDepthMask(true);
				m_GraphicsContext->SetStencilMask(0u);
				m_GraphicsContext->SetStencilFunction(render::T_StencilFunc::Equal, 1, 0xFFFFFFFFu);
			}
		}
		else
		{
			m_GraphicsContext->SetScissorEnabled(true);
			m_GraphicsContext->SetStencilEnabled(false);
			m_GraphicsContext->SetScissor(ivec2(m_ScissorPos.x, m_ViewDimensions.y - (m_ScissorPos.y + m_ScissorSize.y)), m_ScissorSize);
		}
	}
	else
	{
		m_GraphicsContext->SetScissorEnabled(false);
		m_GraphicsContext->SetStencilEnabled(false);
	}

	// what to draw
	m_GraphicsContext->BindVertexArray(geo.m_VertexArray); // this should be unbound at the end of rendering a context

	// update shader parameters - shader should already be set
	render::ShaderData const* shader = nullptr;
	if (geo.m_Font != nullptr)
	{
		shader = m_TextShader.get();
	}
	else
	{
		shader = m_Shader.get();
	}

	ET_ASSERT(shader != nullptr);
	m_GraphicsContext->SetShader(shader);

	shader->Upload("uTranslation"_hash, vec2(translation.x, translation.y));
	shader->Upload("uTransform"_hash, m_CurrentTransform);
	shader->Upload("uTexture"_hash, geo.m_Texture.Get());

	// draw differently depending on if we are handling text or not
	if (geo.m_Font != nullptr)
	{
		//shader->Upload("uSdfSize"_hash, geo.m_Font->GetSdfSize());
		shader->Upload("uUseAntiAliasing"_hash, true);

		m_GraphicsContext->DrawElementsInstanced(render::E_DrawMode::Triangles, geo.m_NumIndices, render::E_DataType::UInt, 0, geo.m_InstanceCount);
	}
	else
	{
		m_GraphicsContext->DrawElements(render::E_DrawMode::Triangles, geo.m_NumIndices, render::E_DataType::UInt, 0);
	}
}

//---------------------------------------------
// RmlRenderInterface::ReleaseCompiledGeometry
//
void RmlRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry)
{
	T_Geometries::iterator const foundIt = m_Geometries.find(geometry);
	ET_ASSERT(foundIt != m_Geometries.cend());

	Geometry& geo = foundIt->second;
	if (geo.m_Font != nullptr)
	{
		m_GraphicsContext->DeleteBuffer(geo.m_VertexBufferInstances);
	}

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
	m_IsScissorEnabled = enable;
}

//--------------------------------------
// RmlRenderInterface::SetScissorRegion
//
// Called by RmlUi when it wants to change the scissor region
//
void RmlRenderInterface::SetScissorRegion(int32 x, int32 y, int32 width, int32 height)
{
	m_ScissorPos = ivec2(x, y);
	m_ScissorSize = ivec2(width, height);
}

//---------------------------------
// RmlRenderInterface::LoadTexture
//
bool RmlRenderInterface::LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, Rml::String const& source)
{
	core::HashString const assetId(source.c_str());

	std::pair<T_Textures::iterator, bool> res;

	AssetPtr<SdfFont> const font = core::ResourceManager::Instance()->GetAssetData<SdfFont>(assetId, false);
	if (font != nullptr)
	{
		res = m_Textures.emplace(++m_LastTextureHandle, font);
		textureDimensions = RmlUtil::ToRml(font->GetAtlas()->GetResolution());
	}
	else
	{
		AssetPtr<render::TextureData> texture = core::ResourceManager::Instance()->GetAssetData<render::TextureData>(assetId);
		if (texture == nullptr)
		{
			textureHandle = s_InvalidTexture;
			return false;
		}

		res = m_Textures.emplace(++m_LastTextureHandle, texture);
		textureDimensions = RmlUtil::ToRml(texture->GetResolution());
	}

	ET_ASSERT(res.second);
	textureHandle = res.first->first;

	return true;
}

//-------------------------------------
// RmlRenderInterface::GenerateTexture
//
bool RmlRenderInterface::GenerateTexture(Rml::TextureHandle& textureHandle, Rml::byte const* source, Rml::Vector2i const& sourceDimensions)
{
	std::pair<T_Textures::iterator, bool> res = m_Textures.emplace(++m_LastTextureHandle, 
		std::move(GenTextureInternal(reinterpret_cast<void const*>(source), ivec2(sourceDimensions.x, sourceDimensions.y))));
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

//----------------------------------------
// RmlRenderInterface::GenTextureInternal
//
UniquePtr<render::TextureData> RmlRenderInterface::GenTextureInternal(void const* data, ivec2 dimensions)
{
	UniquePtr<render::TextureData> tex = Create<render::TextureData>(render::E_ColorFormat::RGBA8, dimensions);
	tex->UploadData(data, render::E_ColorFormat::RGBA, render::E_DataType::UByte, 0u);
	tex->SetParameters(m_GeneratedParameters);
	tex->GenerateMipMaps();

	return std::move(tex);
}


} // namespace gui
} // namespace et

