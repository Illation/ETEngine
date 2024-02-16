#include "stdafx.h"
#include "RmlRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/Util/PrimitiveRenderer.h>

#include <EtGUI/Context/RmlUtil.h>


namespace et {
namespace gui {


//======================
// RML Render Interface
//======================

// static
Rml::CompiledGeometryHandle const RmlRenderer::s_InvalidGeometry = 0u;
Rml::TextureHandle const RmlRenderer::s_InvalidTexture = 0u;


//-------------------------------------
// RmlRenderInterface::c-tor
//
RmlRenderer::RmlRenderer() 
	: Rml::RenderInterface()
{
	m_GeneratedParameters.minFilter = rhi::E_TextureFilterMode::Linear;
	m_GeneratedParameters.magFilter = rhi::E_TextureFilterMode::Linear;
	m_GeneratedParameters.mipFilter = rhi::E_TextureFilterMode::Nearest;
	m_GeneratedParameters.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
	m_GeneratedParameters.wrapT = rhi::E_TextureWrapMode::ClampToEdge;
	m_GeneratedParameters.borderColor = vec4(0.f);

	// default white texture
	std::vector<uint8> emptyTexData({
		255, 255, 255, 255,    255, 255, 255, 255,
		255, 255, 255, 255,    255, 255, 255, 255
		});
	m_EmptyWhiteTex2x2 = GenTextureInternal(reinterpret_cast<void const*>(emptyTexData.data()), ivec2(2));

	// immediate vertex buffer
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	m_VertexArray = device->CreateVertexArray();
	device->BindVertexArray(m_VertexArray);

	m_VertexBuffer = device->CreateBuffer();
	device->BindBuffer(rhi::E_BufferType::Vertex, m_VertexBuffer);
	device->SetBufferData(rhi::E_BufferType::Vertex, m_VertexBufferSize, nullptr, rhi::E_UsageHint::Dynamic);

	SetGenericInputLayout(device);

	m_IndexBuffer = device->CreateBuffer();
	device->BindBuffer(rhi::E_BufferType::Vertex, m_IndexBuffer);
	device->SetBufferData(rhi::E_BufferType::Vertex, m_IndexBufferSize, nullptr, rhi::E_UsageHint::Dynamic);

	device->BindBuffer(rhi::E_BufferType::Vertex, 0);
	device->BindVertexArray(0);
}

//-------------------------------------
// RmlRenderInterface::d-tor
//
RmlRenderer::~RmlRenderer()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DeleteBuffer(m_IndexBuffer);
	device->DeleteBuffer(m_VertexBuffer);
	device->DeleteVertexArray(m_VertexArray);
}

//-------------------------------------
// RmlRenderInterface::SetShader
//
void RmlRenderer::SetShader(AssetPtr<rhi::ShaderData> const& shader, AssetPtr<rhi::ShaderData> const& textShader)
{
	m_Shader = shader;
	m_TextShader = textShader;
}


//-------------------------------------
// RmlRenderInterface::RenderGeometry
//
// for rendering geometry immediately to the screen 
//  - this function should only be used in rare circumstances, as normally compiled geometry is more efficient
//  - this only handles generic geometry, text geometry is assumed to always be compiled
//
void RmlRenderer::RenderGeometry(Rml::Vertex* vertices,
	int32 numVertices,
	int32* indices,
	int32 numIndices,
	Rml::TextureHandle textureHandle,
	Rml::Vector2f const& translation)
{
	// predraw scissor / stencil region
	//----------------------------------
	SetupScissorRectangle();

	// update vertex and index buffers
	//---------------------------------
	m_RenderDevice->BindVertexArray(m_VertexArray);

	m_RenderDevice->BindBuffer(rhi::E_BufferType::Vertex, m_VertexBuffer);
	int64 const vbSize = sizeof(Rml::Vertex) * static_cast<int64>(numVertices);
	if (vbSize > m_VertexBufferSize)
	{
		m_VertexBufferSize = vbSize;
		m_RenderDevice->SetBufferData(rhi::E_BufferType::Vertex, vbSize, reinterpret_cast<void const*>(vertices), rhi::E_UsageHint::Dynamic);
	}
	else
	{
		void* p = m_RenderDevice->MapBuffer(rhi::E_BufferType::Vertex, rhi::E_AccessMode::Write);
		memcpy(p, vertices, static_cast<size_t>(vbSize));
		m_RenderDevice->UnmapBuffer(rhi::E_BufferType::Vertex);
	}

	m_RenderDevice->BindBuffer(rhi::E_BufferType::Vertex, 0u);

	m_RenderDevice->BindBuffer(rhi::E_BufferType::Index, m_IndexBuffer);
	int64 const ibSize = sizeof(int32) * static_cast<int64>(numIndices);
	if (ibSize > m_IndexBufferSize)
	{
		m_IndexBufferSize = ibSize;
		m_RenderDevice->SetBufferData(rhi::E_BufferType::Index, ibSize, reinterpret_cast<void const*>(indices), rhi::E_UsageHint::Dynamic);
	}
	else
	{
		void* p = m_RenderDevice->MapBuffer(rhi::E_BufferType::Index, rhi::E_AccessMode::Write);
		memcpy(p, indices, static_cast<size_t>(ibSize));
		m_RenderDevice->UnmapBuffer(rhi::E_BufferType::Index);
	}

	// setup shading parameters and draw
	//-----------------------------------
	m_RenderDevice->SetShader(m_Shader.get());

	m_Shader->Upload("uTranslation"_hash, vec2(translation.x, translation.y));
	m_Shader->Upload("uTransform"_hash, m_CurrentTransform);

	if (textureHandle != 0u)
	{
		T_Textures::iterator const foundIt = m_Textures.find(textureHandle);
		ET_ASSERT(foundIt != m_Textures.cend());

		m_Shader->Upload("uTexture"_hash, static_cast<rhi::TextureData const*>(foundIt->second.Get().Get()));
	}
	else
	{
		m_Shader->Upload("uTexture"_hash, static_cast<rhi::TextureData const*>(m_EmptyWhiteTex2x2.Get()));
	}

	m_RenderDevice->DrawElements(rhi::E_DrawMode::Triangles, static_cast<uint32>(numIndices), rhi::E_DataType::UInt, 0);

	// done
	//------
	m_RenderDevice->BindVertexArray(0u);
}

//-------------------------------------
// RmlRenderInterface::CompileGeometry
//
// Store some geometry for later rendering - implementing this means we don't use the simple RenderGeometry() method
//
Rml::CompiledGeometryHandle RmlRenderer::CompileGeometry(Rml::Vertex* vertices,
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
	geometry.m_VertexArray = m_RenderDevice->CreateVertexArray();
	m_RenderDevice->BindVertexArray(geometry.m_VertexArray);

	// index buffer is the same for generic and text geometry
	geometry.m_NumIndices = numIndices;
	int64 const iBufferSize = static_cast<int64>(geometry.m_NumIndices * sizeof(uint32));

	geometry.m_IndexBuffer = m_RenderDevice->CreateBuffer();
	m_RenderDevice->BindBuffer(rhi::E_BufferType::Index, geometry.m_IndexBuffer);
	m_RenderDevice->SetBufferData(rhi::E_BufferType::Index, iBufferSize, reinterpret_cast<void const*>(indices), rhi::E_UsageHint::Static);

	// the vertex data needs to be treated differently depending on if it's generic or for text
	geometry.m_VertexBuffer = m_RenderDevice->CreateBuffer();
	m_RenderDevice->BindBuffer(rhi::E_BufferType::Vertex, geometry.m_VertexBuffer);

	if (geometry.m_Font == nullptr)
	{
		geometry.m_NumVertices = numVertices;

		int32 const vertSize = sizeof(Rml::Vertex);
		int64 const vBufferSize = static_cast<int64>(geometry.m_NumVertices * vertSize);
		m_RenderDevice->SetBufferData(rhi::E_BufferType::Vertex, vBufferSize, reinterpret_cast<void const*>(vertices), rhi::E_UsageHint::Static);

		SetGenericInputLayout(m_RenderDevice.Get());
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
		m_RenderDevice->SetBufferData(rhi::E_BufferType::Vertex, bufferSize, reinterpret_cast<void const*>(dataPtr), rhi::E_UsageHint::Static);

		m_RenderDevice->SetVertexAttributeArrayEnabled(0, true);
		m_RenderDevice->SetVertexAttributeArrayEnabled(1, true);
		m_RenderDevice->SetVertexAttributeArrayEnabled(2, true);

		m_RenderDevice->DefineVertexAttributePointer(0, 2, rhi::E_DataType::Float, false, vertSize, offsetof(TextVertex, m_Position));
		m_RenderDevice->DefineVertexAttributePointer(1, 2, rhi::E_DataType::Float, false, vertSize, offsetof(TextVertex, m_TexCoord));
		m_RenderDevice->DefineVertexAttribIPointer(2, 1, rhi::E_DataType::UByte, vertSize, offsetof(TextVertex, m_Channel));

		// setup instance buffer
		ET_ASSERT(fontParams.m_LayerCount > 0u);
		geometry.m_InstanceCount = static_cast<uint64>(fontParams.m_LayerCount);
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

		geometry.m_VertexBufferInstances = m_RenderDevice->CreateBuffer();
		m_RenderDevice->BindBuffer(rhi::E_BufferType::Vertex, geometry.m_VertexBufferInstances);
		m_RenderDevice->SetBufferData(rhi::E_BufferType::Vertex,
			layerInstances.size() * sizeof(TextLayer),
			layerInstances.data(),
			rhi::E_UsageHint::Static);

		m_RenderDevice->SetVertexAttributeArrayEnabled(3, true);
		m_RenderDevice->SetVertexAttributeArrayEnabled(4, true);
		m_RenderDevice->SetVertexAttributeArrayEnabled(5, true);
		m_RenderDevice->SetVertexAttributeArrayEnabled(6, true);
		m_RenderDevice->SetVertexAttributeArrayEnabled(7, true);
		m_RenderDevice->DefineVertexAttributePointer(3, 2, rhi::E_DataType::Float, false, sizeof(TextLayer), offsetof(TextLayer, m_Offset));
		m_RenderDevice->DefineVertexAttributePointer(4, 4, rhi::E_DataType::Float, false, sizeof(TextLayer), offsetof(TextLayer, m_Color));
		m_RenderDevice->DefineVertexAttributePointer(5, 1, rhi::E_DataType::Float, false, sizeof(TextLayer), offsetof(TextLayer, m_SdfThreshold));
		m_RenderDevice->DefineVertexAttributePointer(6, 1, rhi::E_DataType::Float, false, sizeof(TextLayer), offsetof(TextLayer, m_MinThreshold));
		m_RenderDevice->DefineVertexAttribIPointer(7, 1, rhi::E_DataType::UByte, sizeof(TextLayer), offsetof(TextLayer, m_IsBlurred));
		m_RenderDevice->DefineVertexAttribDivisor(3, 1);
		m_RenderDevice->DefineVertexAttribDivisor(4, 1);
		m_RenderDevice->DefineVertexAttribDivisor(5, 1);
		m_RenderDevice->DefineVertexAttribDivisor(6, 1);
		m_RenderDevice->DefineVertexAttribDivisor(7, 1);
	}

	// done
	m_RenderDevice->BindVertexArray(0u);

	return m_LastGeometryHandle;
}

//--------------------------------------------
// RmlRenderInterface::RenderCompiledGeometry
//
void RmlRenderer::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f const& translation)
{
	// the geometry in question
	T_Geometries::const_iterator const foundIt = m_Geometries.find(geometry);
	ET_ASSERT(foundIt != m_Geometries.cend());

	Geometry const& geo = foundIt->second;

	// scissor rectangle
	SetupScissorRectangle();

	// what to draw
	m_RenderDevice->BindVertexArray(geo.m_VertexArray); // this should be unbound at the end of rendering a context

	// update shader parameters - shader should already be set
	rhi::ShaderData const* shader = nullptr;
	if (geo.m_Font != nullptr)
	{
		shader = m_TextShader.get();
	}
	else
	{
		shader = m_Shader.get();
	}

	ET_ASSERT(shader != nullptr);
	m_RenderDevice->SetShader(shader);

	shader->Upload("uTranslation"_hash, vec2(translation.x, translation.y));
	shader->Upload("uTransform"_hash, m_CurrentTransform);
	shader->Upload("uTexture"_hash, geo.m_Texture.Get());

	// draw differently depending on if we are handling text or not
	if (geo.m_Font != nullptr)
	{
		//shader->Upload("uSdfSize"_hash, geo.m_Font->GetSdfSize());
		shader->Upload("uUseAntiAliasing"_hash, true);

		m_RenderDevice->DrawElementsInstanced(rhi::E_DrawMode::Triangles, geo.m_NumIndices, rhi::E_DataType::UInt, 0, geo.m_InstanceCount);
	}
	else
	{
		m_RenderDevice->DrawElements(rhi::E_DrawMode::Triangles, geo.m_NumIndices, rhi::E_DataType::UInt, 0);
	}
}

//---------------------------------------------
// RmlRenderInterface::ReleaseCompiledGeometry
//
void RmlRenderer::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry)
{
	T_Geometries::iterator const foundIt = m_Geometries.find(geometry);
	ET_ASSERT(foundIt != m_Geometries.cend());

	Geometry& geo = foundIt->second;
	if (geo.m_Font != nullptr)
	{
		m_RenderDevice->DeleteBuffer(geo.m_VertexBufferInstances);
	}

	m_RenderDevice->DeleteBuffer(geo.m_VertexBuffer);
	m_RenderDevice->DeleteBuffer(geo.m_IndexBuffer);
	m_RenderDevice->DeleteVertexArray(geo.m_VertexArray);

	m_Geometries.erase(foundIt);
}

//-----------------------------------------
// RmlRenderInterface::EnableScissorRegion
//
// Called by RmlUi when it wants to enable or disable scissoring to clip content
//
void RmlRenderer::EnableScissorRegion(bool enable)
{
	m_IsScissorEnabled = enable;
}

//--------------------------------------
// RmlRenderInterface::SetScissorRegion
//
// Called by RmlUi when it wants to change the scissor region
//
void RmlRenderer::SetScissorRegion(int32 x, int32 y, int32 width, int32 height)
{
	m_ScissorPos = ivec2(x, y);
	m_ScissorSize = ivec2(width, height);
}

//---------------------------------
// RmlRenderInterface::LoadTexture
//
bool RmlRenderer::LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, Rml::String const& source)
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
		AssetPtr<rhi::TextureData> texture = core::ResourceManager::Instance()->GetAssetData<rhi::TextureData>(assetId);
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
bool RmlRenderer::GenerateTexture(Rml::TextureHandle& textureHandle, Rml::byte const* source, Rml::Vector2i const& sourceDimensions)
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
void RmlRenderer::ReleaseTexture(Rml::TextureHandle textureHandle)
{
	size_t const erased = m_Textures.erase(textureHandle);
	ET_ASSERT(erased == 1u);
}

//----------------------------------
// RmlRenderInterface::SetTransform
//
void RmlRenderer::SetTransform(Rml::Matrix4f const* transform)
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
UniquePtr<rhi::TextureData> RmlRenderer::GenTextureInternal(void const* data, ivec2 dimensions)
{
	UniquePtr<rhi::TextureData> tex = Create<rhi::TextureData>(rhi::E_ColorFormat::RGBA8, dimensions);
	tex->UploadData(data, rhi::E_ColorFormat::RGBA, rhi::E_DataType::UByte, 0u);
	tex->SetParameters(m_GeneratedParameters);
	tex->GenerateMipMaps();

	return std::move(tex);
}

//-------------------------------------------
// RmlRenderInterface::SetGenericInputLayout
//
void RmlRenderer::SetGenericInputLayout(rhi::I_RenderDevice* const device) const
{
	device->SetVertexAttributeArrayEnabled(0, true);
	device->SetVertexAttributeArrayEnabled(1, true);
	device->SetVertexAttributeArrayEnabled(2, true);

	device->DefineVertexAttributePointer(0, 2, rhi::E_DataType::Float, false, sizeof(Rml::Vertex), offsetof(Rml::Vertex, position));
	device->DefineVertexAttributePointer(1, 4, rhi::E_DataType::UByte, true, sizeof(Rml::Vertex), offsetof(Rml::Vertex, colour));
	device->DefineVertexAttributePointer(2, 2, rhi::E_DataType::Float, false, sizeof(Rml::Vertex), offsetof(Rml::Vertex, tex_coord));
}

//-------------------------------------------
// RmlRenderInterface::SetupScissorRectangle
//
void RmlRenderer::SetupScissorRectangle()
{
	if (m_IsScissorEnabled)
	{
		if (m_HasTransform)
		{
			// no real scissor rectangle needed
			m_RenderDevice->SetScissorEnabled(false);
			m_RenderDevice->SetStencilEnabled(true);

			// avoid redrawing stencil buffer
			if (!(math::nearEqualsV(m_ScissorPos, m_LastScissorPos)
				&& math::nearEqualsV(m_ScissorSize, m_LastScissorSize)
				&& math::nearEqualsM(m_CurrentTransform, m_LastTransform)))
			{
				m_LastScissorPos = m_ScissorPos;
				m_LastScissorSize = m_ScissorSize;
				m_LastTransform = m_CurrentTransform;

				// clear stencil buffer
				m_RenderDevice->SetStencilMask(0xFFFFFFFFu);
				m_RenderDevice->Clear(rhi::E_ClearFlag::CF_Stencil);

				// fill stencil buffer
				m_RenderDevice->SetDepthMask(false);
				m_RenderDevice->SetStencilFunction(rhi::T_StencilFunc::Never, 1, 0xFFFFFFFFu);
				m_RenderDevice->SetStencilOperation(rhi::E_StencilOp::Replace, rhi::E_StencilOp::Keep, rhi::E_StencilOp::Keep);

				if (m_NullShader == nullptr)
				{
					m_NullShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("FwdNullShader.glsl"));
				}

				m_RenderDevice->SetShader(m_NullShader.get());

				mat4 quadTransform = math::translate(vec3(1.f, 1.f, 0.f)) * math::scale(vec3(0.5f, 0.5f, 1.f)) * // move to 0/0/0
					math::scale(vec3(math::vecCast<float>(m_ScissorSize), 0.f)) * math::translate(vec3(math::vecCast<float>(m_ScissorPos), 0.f)); // rec 
				quadTransform = quadTransform * m_CurrentTransform; // apply transform to scissor rectangle
				m_NullShader->Upload("model"_hash, quadTransform);

				m_NullShader->Upload("worldViewProj"_hash, m_ViewProj); // convert from UI to screen coordinates and perform vertical flip

				rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

				// reset for normal rendering
				m_RenderDevice->SetDepthMask(true);
				m_RenderDevice->SetStencilMask(0u);
				m_RenderDevice->SetStencilFunction(rhi::T_StencilFunc::Equal, 1, 0xFFFFFFFFu);
			}
		}
		else
		{
			m_RenderDevice->SetScissorEnabled(true);
			m_RenderDevice->SetStencilEnabled(false);
			m_RenderDevice->SetScissor(ivec2(m_ScissorPos.x, m_ViewDimensions.y - (m_ScissorPos.y + m_ScissorSize.y)), m_ScissorSize);
		}
	}
	else
	{
		m_RenderDevice->SetScissorEnabled(false);
		m_RenderDevice->SetStencilEnabled(false);
	}
}


} // namespace gui
} // namespace et

