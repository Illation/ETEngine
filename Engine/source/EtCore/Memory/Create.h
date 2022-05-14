#pragma once


namespace et {


//--------
// Create
//
// Utility class for creating smart pointer wrapped objects
//
template <typename TDataType>
class Create final
{
	// construct destruct
	//--------------------
public:
	template<typename... Args>
	Create(Args&&... args) // Construct from arguments - eventually this should be the only place we allocate heap memory across the engine
	{
		// exceptions are not handled for performance reasons
		m_Ptr = new TDataType(std::forward<Args>(args)...);
	}
	Create(TDataType* rawPtr) : m_Ptr(rawPtr) {} // this version is for reflection to work and otherwise not to be used

	Create(Create const&) = delete;
	Create& operator=(Create const&) = delete;

	Create(Create&& moving);
	Create& operator=(Create&& moving);

	~Create();

	// functionality
	//---------------
	TDataType* Release();

	// Data
	///////

private:
	TDataType* m_Ptr = nullptr;
};


} // namespace et


#include "Create.inl"
