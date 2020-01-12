#pragma once


namespace fw {


//====================
// ECS Command Buffer
//====================


// modify component content
////////////////////////////

//---------------------------------
// EcsCommandBuffer::AddComponents
//
template<typename TComponentType, typename... Args>
void EcsCommandBuffer::AddComponents(T_EntityId const entity, TComponentType& component1, Args... args)
{
	std::vector<RawComponentPtr> components;
	detail::GenCompPtrList(components, component1, args...); 

	AddComponentList(entity, components);
}

//------------------------------------
// EcsCommandBuffer::RemoveComponents
//
template<typename TComponentType, typename... Args>
void EcsCommandBuffer::RemoveComponents(T_EntityId const entity)
{
	RemoveComponentTypes(entity, GenCompTypeList<TComponentType, Args...>());
}


} // namespace fw
