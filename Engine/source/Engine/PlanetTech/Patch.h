#pragma once
#include <EtCore/Content/AssetPointer.h>

class ShaderData;
class Planet;

struct PatchVertex
{
	PatchVertex(vec2 position, vec2 morphVec)
	{
		pos = position;
		morph = morphVec;
	}
	vec2 pos;
	vec2 morph;
};
struct PatchInstance
{
	PatchInstance(BYTE Level, vec3 A, vec3 R, vec3 S)
	{
		level = Level;
		a = A;
		r = R;
		s = S;
	}
	int32 level;
	vec3 a;
	vec3 r;
	vec3 s;
};
class Patch
{
public:
	Patch(int16 levels = 5);
	~Patch();

	void SetPlanet(Planet* pPlanet) { m_pPlanet = pPlanet; }

	uint32 GetVertexCount() { return (uint32)m_Vertices.size(); }

	void Init();
	void GenerateGeometry(int16 levels);
	void BindInstances(std::vector<PatchInstance> &instances);
	void UploadDistanceLUT(std::vector<float> &distances);
	void Draw();
private:
	std::vector<PatchVertex>m_Vertices;
	std::vector<uint32>m_Indices;

	Planet *m_pPlanet = nullptr;

	int32 m_NumInstances = 0;

	int16 m_Levels;
	uint32 m_RC;

	//OpenGl stuff
	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_EBO;
	GLuint m_VBOInstance;

	AssetPtr<ShaderData> m_pPatchShader;

	float m_MorphRange = 0.5f;

	//shading
	vec3 m_Ambient = vec3(0.05f, 0.05f, 0.08f);
};
