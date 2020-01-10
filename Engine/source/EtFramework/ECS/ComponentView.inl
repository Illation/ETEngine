#pragma once


namespace fw {


//================
// Component View
//================


//-------------------------
// ComponentView::Declare
//
// initialize a read access
//
template<typename TComponentType>
void ComponentView::Declare(ReadAccess<TComponentType>& read)
{
	m_Accessors.push_back(Accessor(reinterpret_cast<void**>(&(read.m_Data)), TComponentType::GetTypeIndex(), true));
}

//-------------------------
// ComponentView::Declare
//
// initialize a write access
//
template<typename TComponentType>
void ComponentView::Declare(WriteAccess<TComponentType>& write)
{
	m_Accessors.push_back(Accessor(reinterpret_cast<void**>(&(write.m_Data)), TComponentType::GetTypeIndex(), false));
}

//-------------------------
// ComponentView::Declare
//
// initialize a read access to a parents components
//
template<typename TComponentType>
void ComponentView::Declare(ParentRead<TComponentType>& parentRead)
{
	m_ParentAccessors.push_back(Accessor(reinterpret_cast<void**>(&(parentRead.m_Data)), TComponentType::GetTypeIndex(), true));
}

//-------------------------
// ComponentView::Declare
//
// initialize a read access to an entities components
//
template<typename TComponentType>
void ComponentView::Declare(EntityRead<TComponentType>& read)
{
	m_ControllerPtrs.emplace_back(&read.m_Ecs);
}

//-------------------------
// ComponentView::Include
//
// Add a component type to the signature, useful for flag components that don't need to be read
//
template<typename TComponentType>
void ComponentView::Include()
{
	m_Includes.emplace_back(TComponentType::GetTypeIndex());
}


// signature generation
//----------------------

//-------------------
// SignatureFromView
//
template<typename TViewType>
ComponentSignature SignatureFromView()
{
	TViewType temp;
	return ComponentSignature(temp.GetTypeList());
}


} // namespace fw


