#pragma once
#include <string>
#include <vector>
#include <typeinfo>
#include <functional>
class AbstractComponent;
class AbstractScene;
class TransformComponent;
class Entity
{
public:
	Entity();
	virtual ~Entity();

	void AddChild(Entity* pEntity);
	void RemoveChild(Entity* pEntity);

	void AddComponent(AbstractComponent* pComp);
	void RemoveComponent(AbstractComponent* pComp);

	const std::string& GetTag() const { return m_Tag; }
	void SetTag(const std::string& tag) { m_Tag = tag; }

	TransformComponent* GetTransform() const { return m_pTransform; }

	AbstractScene* GetScene();
	Entity* GetParent() const { return m_pParentEntity; }

	template<class T> 
	bool HasComponent(bool searchChildren = false)
	{
		return GetComponent<T>(searchChildren) != nullptr;
	}

	template<class T> 
	T* GetComponent(bool searchChildren = false)
	{
		const std::type_info& ti = typeid(T);
		for (auto *component : m_pComponentVec)
		{
			if (component && typeid(*component) == ti)
				return static_cast<T*>(component);
		}

		if (searchChildren)
		{
			for (auto *child : m_pChildVec)
			{
				if (child->GetComponent<T>(searchChildren) != nullptr)
					return child->GetComponent<T>(searchChildren);
			}
		}

		return nullptr;
	}

	template<class T> 
	std::vector<T*> GetComponents(bool searchChildren = false)
	{
		const type_info& ti = typeid(T);
		std::vector<T*> components;
		for (auto *component : m_pComponentVec)
		{
			if (component && typeid(*component) == ti)
				components.push_back(static_cast<T*>(component));
		}
		if (searchChildren)
		{
			for (auto *child : m_pChildVec)
			{
				auto childComponents = child->GetComponents<T>(searchChildren);

				for (auto *childComp : childComponents)
					components.push_back(static_cast<T*>(childComp));
			}
		}
		return components;
	}

	template<class T> 
	T* GetChild()
	{
		const type_info& ti = typeid(T);
		for (auto *child : m_pChildVec)
		{
			if (child && typeid(*child) == ti)
				return static_cast<T*>(child);
		}
		return nullptr;
	}

	template<class T> 
	std::vector<T*> GetChildren()
	{
		const type_info& ti = typeid(T);
		std::vector<T*> children;

		for (auto *child : m_pChildVec)
		{
			if (child && typeid(*child) == ti)
				children.push_back(static_cast<T*>(child));
		}
		return children;
	}

protected:
	virtual void Initialize() {}
	virtual void Start() {}
	virtual void Draw() {}
	virtual void DrawForward() {}
	virtual void Update() {}

private:
	friend class AbstractScene;

	void RootInitialize();
	void RootStart();
	void RootDraw();
	void RootDrawForward();
	void RootUpdate();

	std::vector<Entity*> m_pChildVec;
	std::vector<AbstractComponent*> m_pComponentVec;

	bool m_IsInitialized = false;
	bool m_IsActive = true;
	AbstractScene* m_pParentScene = nullptr;
	Entity* m_pParentEntity = nullptr;
	TransformComponent* m_pTransform = nullptr;
	std::string m_Tag;
};

