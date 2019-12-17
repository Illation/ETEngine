#include "stdafx.h"
#include "ComponentPool.h"


namespace framework {


//================
// Component Pool
//================


//--------------------
// ComponentPool::At
//
// typeless element access
//
void* ComponentPool::At(size_t const idx)
{
	size_t const byteIdx = idx * ComponentRegistry::Instance().GetSize(m_ComponentType);
	ET_ASSERT(byteIdx < m_Buffer.size());

	return &m_Buffer[byteIdx];
}

//--------------------
// ComponentPool::At
//
// typeless const element access
//
void const* ComponentPool::At(size_t const idx) const
{
	size_t const byteIdx = idx * ComponentRegistry::Instance().GetSize(m_ComponentType);
	ET_ASSERT(byteIdx < m_Buffer.size());

	return &m_Buffer[byteIdx];
}

//------------------------
// ComponentPool::GetSize
//
// count of components being stored
//
size_t ComponentPool::GetSize() const
{
	size_t const typeSize = ComponentRegistry::Instance().GetSize(m_ComponentType);
	ET_ASSERT(typeSize != 0u);
	return m_Buffer.size() / typeSize;
}

//------------------------
// ComponentPool::Append
//
// Add an component to the end of the buffer (without knowing its static type data)
//
void ComponentPool::Append(void const* const componentData)
{
	size_t const typeSize = ComponentRegistry::Instance().GetSize(m_ComponentType);
	m_Buffer.insert(m_Buffer.end(), static_cast<uint8 const*>(componentData), static_cast<uint8 const*>(componentData) + typeSize);
}

//------------------------
// ComponentPool::Erase
//
// Swaps the content with the last component and removes the last element from the list
//
void ComponentPool::Erase(size_t const idx)
{
	size_t const typeSize = ComponentRegistry::Instance().GetSize(m_ComponentType);
	size_t const byteIdx = idx * typeSize;

	ET_ASSERT(byteIdx < m_Buffer.size()); // ensure we are not out of range

	if (m_Buffer.size() == typeSize) // only one element is left so we can just clear all data in the buffer
	{
		m_Buffer.clear();
		return;
	}

	size_t const lastIdx = m_Buffer.size() - typeSize;

	// copy the last element into the position of the element to erase
	memcpy(m_Buffer.data() + byteIdx, m_Buffer.data() + lastIdx, typeSize);

	// 'pop_back' last component
	m_Buffer.resize(m_Buffer.size() - typeSize);
}


} // namespace framework
