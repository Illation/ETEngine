#include "stdafx.h"
#include "ComponentPool.h"


namespace et {
namespace fw {


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
	size_t const byteIdx = m_Buffer.size();

	// add space in the buffer so we can create our new component
	m_Buffer.resize(byteIdx + typeSize);
	
	// call the copy constructor on the component in our new memory location
	ComponentRegistry::Instance().GetCopyAssign(m_ComponentType)(componentData, static_cast<void*>(&m_Buffer[byteIdx]));
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

	// call the destructor on the component we are erasing
	ComponentRegistry::Instance().GetDestructor(m_ComponentType)(static_cast<void const*>(m_Buffer.data() + byteIdx));

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

//----------------------
// ComponentPool::Clear
//
void ComponentPool::Clear()
{
	m_Buffer.clear();
}


} // namespace fw
} // namespace et
