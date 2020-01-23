#include "stdafx.h"
#include "DebugRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/Camera.h>


namespace et {
namespace render {


DebugRenderer::~DebugRenderer()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteVertexArray(m_VAO);
	api->DeleteBuffer(m_VBO);
	m_Lines.clear();
	m_MetaData.clear();
}

void DebugRenderer::Initialize()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_pShader = core::ResourceManager::Instance()->GetAssetData<ShaderData>("DebugRenderer.glsl"_hash);

	api->SetShader(m_pShader.get());

	//Generate buffers and arrays
	m_VAO = api->CreateVertexArray();
	m_VBO = api->CreateBuffer();

	//bind
	api->BindVertexArray(m_VAO);
	api->BindBuffer(E_BufferType::Vertex, m_VBO);

	//set data and attributes
	api->SetBufferData(E_BufferType::Vertex, m_BufferSize, nullptr, E_UsageHint::Dynamic);

	//input layout
	api->SetVertexAttributeArrayEnabled(0, true);
	api->SetVertexAttributeArrayEnabled(1, true);

	api->DefineVertexAttributePointer(0, 3, E_DataType::Float, false, sizeof(LineVertex), offsetof(LineVertex, pos));
	api->DefineVertexAttributePointer(1, 4, E_DataType::Float, false, sizeof(LineVertex), offsetof(LineVertex, col));

	//unbind
	api->BindBuffer(E_BufferType::Vertex, 0);
	api->BindVertexArray(0);
}

void DebugRenderer::UpdateBuffer()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Bind Object vertex array
	api->BindVertexArray(m_VAO);

	//Send the vertex buffer again
	api->BindBuffer(E_BufferType::Vertex, m_VBO);

	bool bufferResize = m_Lines.size() * sizeof(LineVertex) > m_BufferSize;
	if (!m_VBO || bufferResize) //first creation or resize
	{
		if (bufferResize)
		{
			m_BufferSize = (uint32)m_Lines.size() * sizeof(LineVertex);
		}

		api->SetBufferData(E_BufferType::Vertex, m_BufferSize, m_Lines.data(), E_UsageHint::Dynamic);
	}
	else
	{
		void* p = api->MapBuffer(E_BufferType::Vertex, E_AccessMode::Write);
		memcpy(p, m_Lines.data(), sizeof(LineVertex)*m_Lines.size());
		api->UnmapBuffer(E_BufferType::Vertex);
	}


	api->BindBuffer(E_BufferType::Vertex, 0);
}

void DebugRenderer::Draw(Camera const& camera)
{
	if (m_Lines.size() <= 0)
	{
		return;
	}

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_pShader.get());
	m_pShader->Upload("uViewProj"_hash, camera.GetViewProj());

	UpdateBuffer();
	
	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::Zero);

	for (const auto& meta : m_MetaData)
	{
		api->SetLineWidth(meta.thickness);
		api->DrawArrays(E_DrawMode::Lines, meta.start, meta.size);
	}

	api->BindVertexArray(0);

	api->SetBlendEnabled(false);

	m_Lines.clear();
	m_MetaData.clear();
}

void DebugRenderer::CheckMetaData(float thickness)
{
	if (m_MetaData.size() == 0 || !etm::nearEquals(m_MetaData[m_MetaData.size() - 1].thickness, thickness))
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

void DebugRenderer::DrawLine(vec3 start, vec3 end, vec4 col /*= vec4(1)*/, float thickness /*= 1*/)
{
#if defined(ET_DEBUG)
	CheckMetaData(thickness);
	m_Lines.push_back(LineVertex(start, col));
	m_Lines.push_back(LineVertex(end, col));
#endif
}

void DebugRenderer::DrawLine(vec3 start, vec4 startCol, vec3 end, vec4 endCol, float thickness /*= 1*/)
{
#if defined(ET_DEBUG)
	CheckMetaData(thickness);
	m_Lines.push_back(LineVertex(start, startCol));
	m_Lines.push_back(LineVertex(end, endCol));
#endif
}

void DebugRenderer::DrawGrid(Camera const& camera, float pixelSpacingRad)
{
#if defined(ET_DEBUG)
	vec3 camPos = camera.GetPosition();
		
	//max draw distance of the grid
	constexpr float falloffAngle = etm::radians(75.f);
	float distLimit = sqrt(pow(camera.GetFarPlane(), 2) - camPos.y*camPos.y);
	float maxDist = std::min(tan(falloffAngle)*std::abs(camPos.y), distLimit);

	//figure out the spacing of lines
	static const float unit = 1;

	float spacing = tan((camera.GetFOV() / Viewport::GetCurrentViewport()->GetDimensions().x)*pixelSpacingRad)*std::abs(camPos.y);
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
	ivec2 lineIdx = etm::vecCast<int32>((floorPos / (float)spacingLower));
	vec2 basePos = etm::vecCast<float>(lineIdx) * (float)spacingLower;

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
		vec2 pos = etm::vecCast<float>(ivec2(lineIdx.x, i)) * (float)spacingLower;
		float lineFalloff = 1 - (abs(pos.y - basePos.y) / maxDist);
		float  lineDist = sin(etm::PI_DIV2*lineFalloff)*maxDist;
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
		vec2 pos = etm::vecCast<float>(ivec2(i, lineIdx.y)) * (float)spacingLower;
		float lineFalloff = 1 - (abs(pos.x - basePos.x) / maxDist);
		float  lineDist = sin(etm::PI_DIV2*lineFalloff)*maxDist;
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
#endif
}


} // namespace render
} // namespace et
