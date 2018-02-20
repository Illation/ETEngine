#pragma once

class DebugRenderer : public Singleton<DebugRenderer>
{
public:
	void DrawLine(vec3 start, vec3 end, vec4 col = vec4(1), float thickness = 1);
	void DrawLine(vec3 start, vec4 startCol, vec3 end, vec4 endCol, float thickness = 1);

	void DrawGrid(float pixelSpacingRad = etm::radians(75));

private:
	friend class RenderPipeline;

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

	void Initialize();
	void UpdateBuffer();

	void Draw();

	void CheckMetaData(float thickness);

	//Linebuffer
	std::vector<LineVertex> m_Lines;
	uint32 m_BufferSize = 0;
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;

	//Metadata
	std::vector<LineMetaData> m_MetaData;

	//Shader and its uniforms
	ShaderData* m_pShader = nullptr;
	GLint m_uWVP = 0;

private:
	friend class Singleton<DebugRenderer>;
	//Private constructor and destructor for singleton
	DebugRenderer();
	virtual ~DebugRenderer();
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	DebugRenderer(const DebugRenderer& t);
	DebugRenderer& operator=(const DebugRenderer& t);
};