#pragma once
#include <rttr/type.h>


namespace framework {


// definitions
//*************
typedef uint16 T_CompTypeIdx; 
static constexpr T_CompTypeIdx INVALID_COMP_TYPE_IDX = std::numeric_limits<T_CompTypeIdx>::max();


//--------------------
// ComponentRegistry
//
// Contains a static enumeration of component info
//
class ComponentRegistry
{
	// definitions
	//-------------
	struct ComponentTypeInfo
	{
	public:
		ComponentTypeInfo(rttr::type const& t, size_t const size) : type(t), data_size(size) {}

		rttr::type type;
		size_t data_size;
	};

public:
	static constexpr T_CompTypeIdx s_InvalidTypeIdx = std::numeric_limits<T_CompTypeIdx>::max();

	// static access
	//---------------
	static ComponentRegistry& Instance();

	// construct destruct
	//--------------------
private:
	ComponentRegistry() = default;
public:
	ComponentRegistry(ComponentRegistry const&) = delete;
	void operator=(ComponentRegistry const&) = delete;

	// functionality
	//---------------
	template<typename TComponentType>
	T_CompTypeIdx Register(); // returns the type index

	// accessors
	//-----------
	size_t GetSize(T_CompTypeIdx const idx) const;
	rttr::type const& GetType(T_CompTypeIdx const idx) const;

	T_CompTypeIdx GetTypeIdx(rttr::type const& type) const;

	// Data
	///////

private:
	std::vector<ComponentTypeInfo> m_ComponentTypes;
};


} // namespace framework


// macros
//********

//-----------------------
// ECS_DECLARE_COMPONENT
//
// Declare this in a components header to add type info
//
#define ECS_DECLARE_COMPONENT \
private: \
static framework::T_CompTypeIdx const s_TypeIndex; \
public: \
static framework::T_CompTypeIdx GetTypeIndex() { return s_TypeIndex; } \
private:

//------------------------
// ECS_REGISTER_COMPONENT
//
// Declare this in a components source file after registering it with RTTR
//
#define ECS_REGISTER_COMPONENT(TComponentType) \
framework::T_CompTypeIdx const TComponentType::s_TypeIndex = framework::ComponentRegistry::Instance().Register<TComponentType>();


#include "ComponentRegistry.inl"
