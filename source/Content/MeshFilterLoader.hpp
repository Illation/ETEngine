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
};

