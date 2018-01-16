#pragma once
#include "../SceneGraph/Entity.hpp"

class StarField : public Entity
{
public:
	StarField(const std::string &dataFile);
	virtual ~StarField();

	void SetRadius(float radius) { m_Radius = radius; }
	void SetMaxStars(uint32 maxStars) { m_MaxStars = maxStars; }
	void SetDrawnStars(uint32 drawnStars) { m_DrawnStars = drawnStars; }
	void SetBrightnessMultiplier(float mult) { m_BrightnessMult = mult; }

protected:
	virtual void Initialize();
	virtual void Update() {}
	virtual void Draw() {}
	virtual void DrawForward();

private:
	std::vector<vec4> m_Stars;
	std::string m_DataFile;

	ShaderData* m_pShader  = nullptr;
	TextureData* m_pSprite = nullptr;

	GLuint m_VAO = 0;
	GLuint m_VBO = 0;

	uint32 m_MaxStars = 0;
	uint32 m_DrawnStars = 1000;

	float m_Radius = 0.005f;
	float m_BrightnessMult = 100;
};
