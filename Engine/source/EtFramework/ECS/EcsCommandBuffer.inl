#pragma once


namespace fw {


namespace detail {

	//-------------------------------
	// EcsCommandBuffer::AddToBuffer
	//
	// variadic template recursively adds components to a raw component pointer list and finally add the list to the buffer
	// adding the components in the last recursion call prevents components from going out of scope for some reason
	//
	template<typename TComponentType>
	void AddToBuffer(EcsCommandBuffer& buffer, T_EntityId const entity, std::vector<RawComponentPtr>& list, TComponentType& component)
	{
		list.emplace_back(MakeRawComponent(component));
		buffer.AddComponentList(entity, list);
	}

	template<typename TComponentType, typename... Args>
	void AddToBuffer(EcsCommandBuffer& buffer, T_EntityId const entity, std::vector<RawComponentPtr>& list, TComponentType& component1, Args... args)
	{
		list.emplace_back(MakeRawComponent(component1));
		AddToBuffer(buffer, entity, list, args...);
	}

}


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
	std::vector<RawComponentPtr> list;
	detail::AddToBuffer(*this, entity, list, component1, args...);
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
