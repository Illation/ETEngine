#pragma once
#include "../staticDependancies/glad/glad.h"
#include "ContentLoader.hpp"
#include <string>

class ShaderData;
class ShaderLoader : public ContentLoader<ShaderData>
{
public:
	ShaderLoader();
	~ShaderLoader();
protected:

	virtual ShaderData* LoadContent(const std::string& assetFile);
	virtual void Destroy(ShaderData* objToDestroy);

private:
	GLuint CompileShader(const std::string &shaderSourceStr, GLenum type);
	bool ReplaceInclude(std::string &line, const std::string &assetFile);
};

