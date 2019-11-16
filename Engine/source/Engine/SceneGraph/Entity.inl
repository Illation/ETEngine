#pragma once




template<class T>
bool Entity::HasComponent(bool searchChildren /*= false*/)
{
	return GetComponent<T>(searchChildren) != nullptr;
}

template<class T>
T* Entity::GetComponent(bool searchChildren /*= false*/)
{
	const std::type_info& ti = typeid(T);
	for (auto *component : m_pComponentVec)
	{
		if (component && typeid(*component) == ti)
		{
			return static_cast<T*>(component);
		}
	}

	if (searchChildren)
	{
		for (auto *child : m_pChildVec)
		{
			if (child->GetComponent<T>(searchChildren) != nullptr)
			{
				return child->GetComponent<T>(searchChildren);
			}
		}
	}

	return nullptr;
}

template<class T>
std::vector<T*>
Entity::GetComponents(bool searchChildren /*= false*/)
{
	const type_info& ti = typeid(T);
	std::vector<T*> components;
	for (auto *component : m_pComponentVec)
	{
		if (component && typeid(*component) == ti)
		{
			components.push_back(static_cast<T*>(component));
		}
	}

	if (searchChildren)
	{
		for (auto *child : m_pChildVec)
		{
			auto childComponents = child->GetComponents<T>(searchChildren);

			for (auto *childComp : childComponents)
			{
				components.push_back(static_cast<T*>(childComp));
			}
		}
	}

	return components;
}

template<class T>
T* Entity::GetChild()
{
	const type_info& ti = typeid(T);
	for (auto *child : m_pChildVec)
	{
		if (child && typeid(*child) == ti)
		{
			return static_cast<T*>(child);
		}
	}

	return nullptr;
}

template<class T>
std::vector<T*> Entity::GetChildrenOfType()
{
	const type_info& ti = typeid(T);
	std::vector<T*> children;

	for (auto *child : m_pChildVec)
	{
		if (child && typeid(*child) == ti)
		{
			children.push_back(static_cast<T*>(child));
		}
	}

	return children;
}
