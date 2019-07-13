#pragma once

class ShaderData;
class TextureData;
class LightMaterial;
class NullMaterial;
class MeshFilter;
class DirectionalShadowData;

class PointLightVolume : public Singleton<PointLightVolume>
{
public:
	PointLightVolume();
	virtual ~PointLightVolume();

	void Draw(vec3 pos, float radius, vec3 col);

private:
	friend class AbstractFramework; //should init and destroy singleton
	void Initialize();
	bool IsInitialized = false;

	LightMaterial* m_pMaterial;
	NullMaterial* m_pNullMaterial;
};

class DirectLightVolume : public Singleton<DirectLightVolume>
{
public:
	DirectLightVolume();
	virtual ~DirectLightVolume();

	void Draw(vec3 dir, vec3 col);
	void DrawShadowed(vec3 dir, vec3 col, DirectionalShadowData *pShadow);

protected:
	ShaderData* m_pShader;
	ShaderData* m_pShaderShadowed;
private:
	friend class AbstractFramework; //should init and destroy singleton
	void Initialize();
	bool IsInitialized = false;

	GLint m_uCol;
	GLint m_uDir;
	GLint m_uCamPos;
};