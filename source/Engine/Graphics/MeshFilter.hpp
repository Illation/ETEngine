#pragma once
#include "../StaticDependancies/glad/glad.h"
#include <vector>
#include <map>
#include <string>
//Use lefthanded coordinate system
#ifndef GLM_LEFT_HANDED
	#define GLM_LEFT_HANDED
#endif
#include <glm\glm.hpp>
#include "../Helper/MathHelper.hpp"

class Material;

enum VertexFlags
{
	POSITION = 1 << 0,
	NORMAL   = 1 << 1,
	BINORMAL = 1 << 2,
	TANGENT  = 1 << 3,
	COLOR    = 1 << 4,
	TEXCOORD = 1 << 5
};

struct AttributeDescriptor
{
	std::string name;
	GLenum dataType;
	uint32 dataSize;
};

struct VertexObject
{
	uint32 flags;
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

	static std::map<VertexFlags, AttributeDescriptor> LayoutAttributes;
	size_t GetIndexCount() { return m_IndexCount; }

	Sphere* GetBoundingSphere() { return &m_BoundingSphere; }
private:
	friend class MeshFilterLoader;
	friend class ModelComponent;

	int32 GetVertexObjectId(uint32 flags);
	void BuildVertexBuffer(Material* pMaterial);
	bool HasElement(uint32 flags){ return (m_SupportedFlags&flags) > 0 ? true : false; }

	void CalculateBoundingVolumes();
	std::string PrintFlags(uint32 flags);

	size_t m_VertexCount = 0, m_IndexCount = 0;
	uint32 m_SupportedFlags = 0;
	std::vector<glm::vec3> m_Positions;
	std::vector<glm::vec3> m_Normals;
	std::vector<glm::vec3> m_BiNormals;
	std::vector<glm::vec3> m_Tangents;
	std::vector<glm::vec4> m_Colors;
	std::vector<glm::vec2> m_TexCoords;

	std::vector<GLuint> m_Indices;

	std::vector<VertexObject> m_Objects;

	std::string m_Name;
	glm::vec4 m_DefaultColor;

	Sphere m_BoundingSphere;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	MeshFilter(const MeshFilter &obj);
	MeshFilter& operator=(const MeshFilter& obj);
};