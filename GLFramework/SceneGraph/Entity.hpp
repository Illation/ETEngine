#pragma once
class AbstractScene;
class Entity
{
public:
	Entity();
	virtual ~Entity();
	
protected:
	virtual void Initialize() {}
	virtual void Start() {}
	virtual void Draw() {}
	virtual void Update() {}

private:
	friend class AbstractScene;

	void RootInitialize();
	void RootStart();
	void RootDraw();
	void RootUpdate();

	bool m_IsInitialized = false;
	bool m_IsActive = true;

	AbstractScene* m_pParentScene;
};

