#pragma once
class GameObject
{
public:
	GameObject();
	virtual ~GameObject();
	
	void RootInitialize();
	void RootStart();
	void RootDraw();
	void RootUpdate();

protected:
	virtual void Initialize() {}
	virtual void Start() {}
	virtual void Draw() {}
	virtual void Update() {}
};

