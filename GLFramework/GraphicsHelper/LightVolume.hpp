#pragma once
#include "../StaticDependancies/glad/glad.h"

class ShaderData;
class TextureData;
class LightMaterial;
class NullMaterial;
class MeshFilter;

class PointLightVolume : public Singleton<PointLightVolume>
{
public:
	PointLightVolume();
	virtual ~PointLightVolume();

	void Draw(glm::vec3 pos, float radius, glm::vec3 col);

private:
	friend class AbstractFramework; //should init and destroy singleton
	void Initialize();
	bool IsInitialized = false;

	MeshFilter* m_pMeshFilter;
	LightMaterial* m_pMaterial;
	NullMaterial* m_pNullMaterial;
};

class DirectLightVolume : public Singleton<DirectLightVolume>
{
public:
	DirectLightVolume();
	virtual ~DirectLightVolume();

	void Draw(glm::vec3 dir, glm::vec3 col);

protected:
	ShaderData* m_pShader;
private:
	friend class AbstractFramework; //should init and destroy singleton
	void Initialize();
	bool IsInitialized = false;

	GLint m_uCol;
	GLint m_uDir;
	GLint m_uCamPos;

	GLuint m_QuadVAO = 0;
	GLuint m_QuadVBO = 0;
};