#pragma once
#include "../StaticDependancies/glad/glad.h"

class ShaderData;
class TextureData;
class LightMaterial;
class NullMaterial;
class MeshFilter;

class LightVolume : public Singleton<LightVolume>
{
public:
	LightVolume();
	virtual ~LightVolume();

	void Draw(glm::vec3 pos, float radius, glm::vec3 col);

protected:
	ShaderData* m_pShader;
private:
	friend class AbstractFramework; //should init and destroy singleton
	void Initialize();
	bool IsInitialized = false;

	MeshFilter* m_pMeshFilter;
	LightMaterial* m_pMaterial;
	NullMaterial* m_pNullMaterial;

	std::string m_ShaderFile;
};

