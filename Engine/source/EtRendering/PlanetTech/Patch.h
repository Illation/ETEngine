#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRHI/GraphicsTypes/Shader.h>


namespace et {
namespace render {


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
	Patch() = default;
	~Patch();

	uint32 GetVertexCount() { return (uint32)m_Vertices.size(); }

	void Init(int16 const levels = 5);
	void GenerateGeometry(int16 levels);
	void BindInstances(std::vector<PatchInstance> const& instances);
	void UploadDistanceLUT(std::vector<float> const& distances);
	void Draw(Planet const& planet, mat4 const& transform);
private:
	std::vector<PatchVertex>m_Vertices;
	std::vector<uint32>m_Indices;

	int32 m_NumInstances = 0;

	int16 m_Levels;
	uint32 m_RC;

	//OpenGl stuff
	rhi::T_ArrayLoc m_VAO;
	rhi::T_BufferLoc m_VBO;
	rhi::T_BufferLoc m_EBO;
	rhi::T_BufferLoc m_VBOInstance;

	AssetPtr<rhi::ShaderData> m_pPatchShader;

	float m_MorphRange = 0.5f;

	//shading
	vec3 m_Ambient = vec3(0.05f, 0.05f, 0.08f);
};


} // namespace render
} // namespace et
