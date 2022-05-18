#pragma once
#include "RmlSystemInterface.h"

#include <EtGUI/Rendering/RmlRenderInterface.h>


namespace et {
namespace gui {


//---------------------------------
// RmlGlobal
//
// Singleton that manages global access into the RmlUi library
//
class RmlGlobal final
{
	// definitions
	//-------------
private:
	friend class RefPtr<RmlGlobal>;
	friend class Create<RmlGlobal>;

	static RefPtr<RmlGlobal> s_Instance;

	// singleton access
	//------------------
public:
	static RefPtr<RmlGlobal> GetInstance();
	static void Destroy();
	static bool IsInitialized();

	// construct destruct
	//--------------------
private:
	RmlGlobal();
	~RmlGlobal();


	// Data
	///////
	
	RmlSystemInterface m_SystemInterface;
	RmlRenderInterface m_RenderInterface;
};


} // namespace gui
} // namespace et
