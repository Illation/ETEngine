#pragma once
#include "../Graphics/FrameBuffer.hpp"

class Gbuffer : public FrameBuffer
{
public:
	Gbuffer(bool demo = false);
	~Gbuffer();

	void SetAmbCol(glm::vec3 col) { m_AmbientColor = col; }

private:
	void AccessShaderAttributes();
	void UploadDerivedVariables();

	//CameraPos
	GLint m_uCamPos;
	//Phong parameters
	GLint m_uAmbCol;
	glm::vec3 m_AmbientColor;
};

