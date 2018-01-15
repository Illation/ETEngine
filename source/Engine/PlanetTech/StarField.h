#pragma once
#include "../SceneGraph/Entity.hpp"

class StarField : public Entity
{
public:
	StarField(const std::string &dataFile);
protected:
	virtual void Initialize();
	virtual void Update();
	virtual void Draw() {}
	virtual void DrawForward();

private:
	std::vector<vec4> m_Stars;
	std::string m_DataFile;
};
