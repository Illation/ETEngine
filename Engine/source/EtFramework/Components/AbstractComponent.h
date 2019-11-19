#pragma once


//forward declaration
class Entity;
class TransformComponent;


//---------------------
// AbstractComponent
//
// Base class for components of entities
//
class AbstractComponent
{
	friend class Entity;

	// construct destruct
	//--------------------
public:
	AbstractComponent() = default;
	virtual ~AbstractComponent();

private: // disable copying
	AbstractComponent(const AbstractComponent &obj);
	AbstractComponent& operator=(const AbstractComponent& obj);

	// interface
	//-----------
protected:
	virtual void Init() = 0;
	virtual void Deinit() = 0;
	virtual void Update() = 0;

	// root
	//-------
	void RootInit();
	void RootDeinit();

	// accessors
	//-----------
public:
	Entity* GetEntity() const { return m_Entity; }
	TransformComponent* GetTransform() const;

	// Data
	///////

private:

	Entity* m_Entity = nullptr;
	bool m_IsInitialized = false;
};

