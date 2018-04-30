#pragma once
#include "ContentLoader.hpp"
#include <string>

class MeshFilter;
class MeshFilterLoader : public ContentLoader<MeshFilter>
{
public:
	MeshFilterLoader();
	~MeshFilterLoader();

protected:
	virtual MeshFilter* LoadContent(const std::string& assetFile);
	virtual void Destroy(MeshFilter* objToDestroy);

private:
	MeshFilter* LoadAssimp(const std::vector<uint8>& binaryContent, const std::string &ext);
	MeshFilter* LoadGLTF(const std::vector<uint8>& binaryContent, const std::string path, const std::string &ext);

	ivec2 logPos;
	std::string loadingString;
};

