#pragma once
#include <rttr/type>

#include <EtFramework/ECS/RawComponentPointer.h>


namespace framework {


//-----------------------
// I_ComponentDescriptor
//
// Abstract class for generating components from data
//
class I_ComponentDescriptor
{
	// definitions
	//-------------
	RTTR_ENABLE()

	// construct destruct
	//--------------------
public:
	virtual ~I_ComponentDescriptor() = default;

	// interface
	//-----------
	virtual T_CompTypeIdx GetType() const = 0;
	virtual RawComponentPtr MakeRawData() = 0; // has ownership
};

//---------------------
// ComponentDescriptor
//
// Class to create a specific component descriptor
//
template<class TComponentType>
class ComponentDescriptor : public I_ComponentDescriptor
{
	// definitions
	//-------------
	RTTR_ENABLE(I_ComponentDescriptor)

	// construct destruct
	//--------------------
public:
	ComponentDescriptor() : I_ComponentDescriptor() {}
	virtual ~ComponentDescriptor() = default;

	// I_ComponentDescriptor interface
	//---------------------------------
	T_CompTypeIdx GetType() const override { return TComponentType::GetTypeIndex(); }
	RawComponentPtr MakeRawData() override { return RawComponentPtr(GetType(), reinterpret_cast<void*>(MakeData())); }

	// interface
	//-----------
	virtual TComponentType* MakeData() = 0;
};


} // namespace framework
