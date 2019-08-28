#pragma once
#include "VertexInfo.h"

class Material;

namespace glTF { class MeshFilterConstructor; };

struct VertexObject
{
	T_VertexFlags flags;
	GLuint array;
	GLuint buffer;
	GLuint index;
private:
	friend class MeshFilter;
	void Release()
	{
		glDeleteBuffers(1, &index);
		glDeleteBuffers(1, &buffer);
		glDeleteVertexArrays(1, &array);
	}
};

class MeshFilter
{
public:
	MeshFilter();
	~MeshFilter();

	const VertexObject& GetVertexObject(Material* pMaterial);

	size_t GetIndexCount() { return m_IndexCount; }

	Sphere* GetBoundingSphere();

	std::string GetName() const { return m_Name; }

private://for construction purposes
	friend class glTF::MeshFilterConstructor;
	std::vector<uint32>& GetIndices() { return m_Indices; }
	std::vector<vec3>& GetPositions() { return m_Positions; }
	std::vector<vec3>& GetNormals() { return m_Normals; }
	std::vector<vec3>& GetTangents() { return m_Tangents; }
	std::vector<vec4>& GetColors() { return m_Colors; }
	std::vector<vec2>& GetTexCoords() { return m_TexCoords; }

	void SetName(std::string val) { m_Name = val; }

	//if tangentInfo array is empty, generate tangent space using mikkTSpace algorithm
	bool ConstructTangentSpace(std::vector<vec4>& tangentInfo);
	void CalculateBoundingVolumes();
	
private:
	friend class MeshFilterLoader;
	friend class ModelComponent;

	int32 GetVertexObjectId(T_VertexFlags const flags);
	void BuildVertexBuffer(Material* pMaterial);
	bool HasElement(uint32 flags){ return (m_SupportedFlags&flags) > 0 ? true : false; }

	size_t m_VertexCount = 0, m_IndexCount = 0;
	T_VertexFlags m_SupportedFlags = 0;
	std::vector<vec3> m_Positions;
	std::vector<vec3> m_Normals;
	std::vector<vec3> m_BiNormals;
	std::vector<vec3> m_Tangents;
	std::vector<vec4> m_Colors;
	std::vector<vec2> m_TexCoords;

	std::vector<GLuint> m_Indices;

	std::vector<VertexObject> m_Objects;

	std::string m_Name;

	Sphere m_BoundingSphere;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	MeshFilter(const MeshFilter &obj);
	MeshFilter& operator=(const MeshFilter& obj);
};