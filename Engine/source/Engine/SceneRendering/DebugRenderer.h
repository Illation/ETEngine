#pragma once
#include <EtCore/Content/AssetPointer.h>

class DebugRenderer final
{
private:
	friend class SceneRenderer;

	struct LineVertex
	{
		LineVertex(vec3 p, vec4 c) :pos(p), col(c) {}
		vec3 pos;
		vec4 col;
	};
	struct LineMetaData
	{
		float thickness = 1;
		uint32 start = 0;
		uint32 size = 0;
	};

	DebugRenderer() = default;
	~DebugRenderer();

	void Initialize();

public:
	void DrawLine(vec3 start, vec3 end, vec4 col = vec4(1), float thickness = 1);
	void DrawLine(vec3 start, vec4 startCol, vec3 end, vec4 endCol, float thickness = 1);

	void DrawGrid(float pixelSpacingRad = etm::radians(75));

private:
	void UpdateBuffer();
	void Draw();
	void CheckMetaData(float thickness);

	//Linebuffer
	std::vector<LineVertex> m_Lines;
	uint32 m_BufferSize = 0;
	T_ArrayLoc m_VAO = 0;
	T_BufferLoc m_VBO = 0;

	//Metadata
	std::vector<LineMetaData> m_MetaData;

	//Shader and its uniforms
	AssetPtr<ShaderData> m_pShader;
};