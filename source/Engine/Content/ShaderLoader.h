#pragma once
#include "ContentLoader.h"

#include <Engine/Graphics/Shader.h>


class I_Uniform;
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

	bool Precompile( std::string &shaderContent, const std::string &assetFile, 
					 bool &useGeo, bool &useFrag,
					 std::string &vertSource, std::string &geoSource, std::string &fragSource );
	bool ReplaceInclude(std::string &line, const std::string &assetFile);

	bool GetUniformLocations(GLuint shaderProgram, std::map<uint32, I_Uniform*> &uniforms);

	ivec2 logPos;
	std::string loadingString;
};

