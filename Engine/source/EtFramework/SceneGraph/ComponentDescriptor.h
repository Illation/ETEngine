#pragma once


namespace framework {


//-----------------------
// I_ComponentDescriptor
//
// Abstract class for generating components from data
//
class I_ComponentDescriptor
{
public:
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
public:
	T_CompTypeIdx GetType() const override { return TComponentType::GetTypeIndex(); }
	RawComponentPtr MakeRawData() override { return RawComponentPtr(GetType(), reinterpret_cast<void*>(MakeData())); }

	virtual TComponentType* MakeData() = 0;
};


} // namespace framework
