#include "stdafx.h"
#include "DebugRenderer.h"


#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/Shader.h>

#include <EtRendering/GraphicsTypes/Camera.h>


namespace et {
namespace render {


//================
// Debug Renderer
//================


//----------------------
// DebugRenderer::c-tor
//
DebugRenderer::~DebugRenderer()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DeleteVertexArray(m_VAO);
	device->DeleteBuffer(m_VBO);
	m_Lines.clear();
	m_MetaData.clear();
}

//---------------------------
// DebugRenderer::Initialize
//
void DebugRenderer::Initialize()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	m_Shader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/DebugRenderer.glsl"));

	device->SetShader(m_Shader.get());

	//Generate buffers and arrays
	m_VAO = device->CreateVertexArray();
	m_VBO = device->CreateBuffer();

	//bind
	device->BindVertexArray(m_VAO);
	device->BindBuffer(rhi::E_BufferType::Vertex, m_VBO);

	//set data and attributes
	device->SetBufferData(rhi::E_BufferType::Vertex, m_BufferSize, nullptr, rhi::E_UsageHint::Dynamic);

	//input layout
	device->SetVertexAttributeArrayEnabled(0, true);
	device->SetVertexAttributeArrayEnabled(1, true);

	device->DefineVertexAttributePointer(0, 3, rhi::E_DataType::Float, false, sizeof(LineVertex), offsetof(LineVertex, pos));
	device->DefineVertexAttributePointer(1, 4, rhi::E_DataType::Float, false, sizeof(LineVertex), offsetof(LineVertex, col));

	//unbind
	device->BindBuffer(rhi::E_BufferType::Vertex, 0);
	device->BindVertexArray(0);
}


//------------------------------
// DebugRenderer::CheckMetaData
//
void DebugRenderer::DrawLine(vec3 start, vec3 end, vec4 col /*= vec4(1)*/, float thickness /*= 1*/)
{
	CheckMetaData(thickness);
	m_Lines.push_back(LineVertex(start, col));
	m_Lines.push_back(LineVertex(end, col));
}

//-------------------------
// DebugRenderer::DrawLine
//
void DebugRenderer::DrawLine(vec3 start, vec4 startCol, vec3 end, vec4 endCol, float thickness /*= 1*/)
{
	CheckMetaData(thickness);
	m_Lines.push_back(LineVertex(start, startCol));
	m_Lines.push_back(LineVertex(end, endCol));
}

//-------------------------
// DebugRenderer::DrawGrid
//
void DebugRenderer::DrawGrid(Camera const& camera, float pixelSpacingRad)
{
	vec3 camPos = camera.GetPosition();
		
	//max draw distance of the grid
	constexpr float falloffAngle = math::radians(75.f);
	float distLimit = sqrt(pow(camera.GetFarPlane(), 2) - camPos.y*camPos.y);
	float maxDist = std::min(tan(falloffAngle)*std::abs(camPos.y), distLimit);

	//figure out the spacing of lines
	static const float unit = 1;

	float spacing = tan((camera.GetFOV() / rhi::Viewport::GetCurrentViewport()->GetDimensions().x)*pixelSpacingRad)*std::abs(camPos.y);
	int32 digitCount = 0;
	float num = abs(spacing);
	while (num >= unit)
	{
		num /= 10;
		digitCount++;
	}
	int32 spacingLower = 10 ^ (digitCount);
	int32 spacingHigher = spacingLower * 10;

	//how many lines do we draw?
	int32 linesDrawn = (int32)(maxDist / (float)spacingLower);

	vec2 floorPos = vec2(camPos.x, camPos.z);//height is 0
	ivec2 lineIdx = math::vecCast<int32>((floorPos / (float)spacingLower));
	vec2 basePos = math::vecCast<float>(lineIdx) * (float)spacingLower;

	//Line thickness and alpha
	constexpr float thicknessHigher = 5;
	constexpr float thicknessLower = 2;

	float maxAlpha = 0.5f;
	float fade = (spacing - spacingLower) / (spacingHigher - spacingLower);
	fade = 1;
	float thickness = thicknessLower + ((thicknessHigher - thicknessLower)*fade);

	const vec3 greyCol(0.75f);
	const vec3 redCol(1, 0, 0);
	const vec3 blueCol(0, 0, 1);

	//Create the lines
	std::vector<LineVertex> normalLines;
	std::vector<LineVertex> thickLines;

	//lateral (X)
	for (int32 i = lineIdx.y - linesDrawn; i < lineIdx.y + linesDrawn; ++i)
	{
		vec2 pos = math::vecCast<float>(ivec2(lineIdx.x, i)) * (float)spacingLower;
		float lineFalloff = 1 - (abs(pos.y - basePos.y) / maxDist);
		float  lineDist = sin(math::PI_DIV2*lineFalloff)*maxDist;
		vec3 p0 = vec3(pos.x-lineDist, 0, pos.y);
		vec3 p1 = vec3(pos.x, 0, pos.y);
		vec3 p2 = vec3(pos.x+lineDist, 0, pos.y);

		vec4 col;
		vec4 colA;
		std::vector<LineVertex>* lines = nullptr;
		if (i % 10 == 0)
		{
			//thick line
			if (i == 0)
			{
				//base red line
				col = vec4(redCol, lineFalloff*maxAlpha);
				colA = vec4(redCol, 0);
			}
			else
			{
				col = vec4(greyCol, lineFalloff*maxAlpha);
				colA = vec4(greyCol, 0);
			}
			lines = &thickLines;
		}
		else
		{
			//normal line
			col = vec4(greyCol, lineFalloff*fade*maxAlpha);
			colA = vec4(greyCol, 0);
			lines = &normalLines;
		}
		lines->push_back(LineVertex(p0, colA));
		lines->push_back(LineVertex(p1, col));
		lines->push_back(LineVertex(p1, col));
		lines->push_back(LineVertex(p2, colA));
	}
	//depth (Z)
	for (int32 i = lineIdx.x - linesDrawn; i < lineIdx.x + linesDrawn; ++i)
	{
		vec2 pos = math::vecCast<float>(ivec2(i, lineIdx.y)) * (float)spacingLower;
		float lineFalloff = 1 - (abs(pos.x - basePos.x) / maxDist);
		float  lineDist = sin(math::PI_DIV2*lineFalloff)*maxDist;
		vec3 p0 = vec3(pos.x, 0, pos.y - lineDist);
		vec3 p1 = vec3(pos.x, 0, pos.y);
		vec3 p2 = vec3(pos.x, 0, pos.y + lineDist);

		vec4 col;
		vec4 colA;
		std::vector<LineVertex>* lines = nullptr;
		if (i % 10 == 0)
		{
			//thick line
			if (i == 0)
			{
				//base blue line
				col = vec4(blueCol, lineFalloff*maxAlpha);
				colA = vec4(blueCol, 0);
			}
			else
			{
				col = vec4(greyCol, lineFalloff*maxAlpha);
				colA = vec4(greyCol, 0);
			}
			lines = &thickLines;
		}
		else
		{
			//normal line
			col = vec4(greyCol, lineFalloff*fade*maxAlpha);
			colA = vec4(greyCol, 0);
			lines = &normalLines;
		}
		lines->push_back(LineVertex(p0, colA));
		lines->push_back(LineVertex(p1, col));
		lines->push_back(LineVertex(p1, col));
		lines->push_back(LineVertex(p2, colA));
	}
	//Append line data
	LineMetaData meta;
	meta.thickness = thickness;
	if (m_MetaData.size() > 0)
	{
		meta.start = m_MetaData[m_MetaData.size() - 1].start + m_MetaData[m_MetaData.size() - 1].size;
	}
	meta.size = (uint32)normalLines.size() * sizeof(LineVertex);
	m_MetaData.push_back(meta);
	m_Lines.insert(m_Lines.end(), normalLines.begin(), normalLines.end());

	meta.thickness = thicknessHigher;
	meta.start += meta.size;
	meta.size = (uint32)thickLines.size() * sizeof(LineVertex);
	m_MetaData.push_back(meta);
	m_Lines.insert(m_Lines.end(), thickLines.begin(), thickLines.end());
}

//---------------------
// DebugRenderer::Draw
//
void DebugRenderer::Draw(Camera const& camera)
{
	if (m_Lines.size() <= 0)
	{
		return;
	}

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->SetShader(m_Shader.get());
	m_Shader->Upload("uViewProj"_hash, camera.GetViewProj());

	UpdateBuffer();

	device->SetBlendEnabled(true);
	device->SetBlendEquation(rhi::E_BlendEquation::Add);
	device->SetBlendFunction(rhi::E_BlendFactor::One, rhi::E_BlendFactor::Zero);

	for (const auto& meta : m_MetaData)
	{
		device->SetLineWidth(meta.thickness);
		device->DrawArrays(rhi::E_DrawMode::Lines, meta.start, meta.size);
	}

	device->BindVertexArray(0);

	device->SetBlendEnabled(false);

	m_Lines.clear();
	m_MetaData.clear();
}

//-----------------------------
// DebugRenderer::UpdateBuffer
//
void DebugRenderer::UpdateBuffer()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	//Bind Object vertex array
	device->BindVertexArray(m_VAO);

	//Send the vertex buffer again
	device->BindBuffer(rhi::E_BufferType::Vertex, m_VBO);

	bool bufferResize = m_Lines.size() * sizeof(LineVertex) > m_BufferSize;
	if (!m_VBO || bufferResize) //first creation or resize
	{
		if (bufferResize)
		{
			m_BufferSize = (uint32)m_Lines.size() * sizeof(LineVertex);
		}

		device->SetBufferData(rhi::E_BufferType::Vertex, m_BufferSize, m_Lines.data(), rhi::E_UsageHint::Dynamic);
	}
	else
	{
		void* p = device->MapBuffer(rhi::E_BufferType::Vertex, rhi::E_AccessMode::Write);
		memcpy(p, m_Lines.data(), sizeof(LineVertex)*m_Lines.size());
		device->UnmapBuffer(rhi::E_BufferType::Vertex);
	}


	device->BindBuffer(rhi::E_BufferType::Vertex, 0);
}

//------------------------------
// DebugRenderer::CheckMetaData
//
void DebugRenderer::CheckMetaData(float thickness)
{
	if (m_MetaData.size() == 0 || !math::nearEquals(m_MetaData[m_MetaData.size() - 1].thickness, thickness))
	{
		m_MetaData.push_back(LineMetaData());
		m_MetaData[m_MetaData.size() - 1].thickness = thickness;
		if (m_MetaData.size() > 1)
		{
			m_MetaData[m_MetaData.size() - 1].start = m_MetaData[m_MetaData.size() - 2].start + m_MetaData[m_MetaData.size() - 2].size;
		}
	}
	m_MetaData[m_MetaData.size() - 1].size += sizeof(LineVertex) * 2;
}


} // namespace render
} // namespace et

#endif // ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
