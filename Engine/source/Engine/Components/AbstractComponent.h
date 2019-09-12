#pragma once

//forward declaration
class Entity;
class TransformComponent;

class AbstractComponent
{
public:
	AbstractComponent(void);
	virtual ~AbstractComponent(void);

	Entity* GetEntity() const { return m_pEntity; }
	TransformComponent* GetTransform() const;

protected:

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void DrawForward() = 0;
	virtual void DrawShadow() {}
	virtual void PostDraw();

	Entity* m_pEntity = nullptr;
	bool m_IsInitialized = false;

private:

	friend class Entity;

	void RootInitialize();

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	AbstractComponent(const AbstractComponent &obj);
	AbstractComponent& operator=(const AbstractComponent& obj);
};

