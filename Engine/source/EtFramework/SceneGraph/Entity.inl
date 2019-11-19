#pragma once


//========
// Entity
//========


// template accessors
//////////////////////

//----------------------
// Entity::HasComponent
//
template<class T>
bool Entity::HasComponent(bool searchChildren /*= false*/)
{
	return GetComponent<T>(searchChildren) != nullptr;
}

//----------------------
// Entity::GetComponent
//
template<class T>
T* Entity::GetComponent(bool searchChildren /*= false*/)
{
	const std::type_info& ti = typeid(T);
	for (auto *component : m_Components)
	{
		if (component && typeid(*component) == ti)
		{
			return static_cast<T*>(component);
		}
	}

	if (searchChildren)
	{
		for (auto *child : m_Children)
		{
			if (child->GetComponent<T>(searchChildren) != nullptr)
			{
				return child->GetComponent<T>(searchChildren);
			}
		}
	}

	return nullptr;
}

//-------------------
// Entity::GetChild
//
template<class T>
T* Entity::GetChild()
{
	const type_info& ti = typeid(T);
	for (auto *child : m_Children)
	{
		if (child && typeid(*child) == ti)
		{
			return static_cast<T*>(child);
		}
	}

	return nullptr;
}

//---------------------------
// Entity::GetChildrenOfType
//
template<class T>
std::vector<T*> Entity::GetChildrenOfType()
{
	const type_info& ti = typeid(T);
	std::vector<T*> children;

	for (auto *child : m_Children)
	{
		if (child && typeid(*child) == ti)
		{
			children.push_back(static_cast<T*>(child));
		}
	}

	return children;
}
