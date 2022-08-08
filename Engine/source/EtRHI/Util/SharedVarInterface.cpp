#include "stdafx.h"
#include "SharedVarInterface.h"


namespace et {
namespace rhi {


// static
Ptr<I_SharedVarController const> I_SharedVarController::s_GlobalInstance;


//----------------------------------
// I_SharedVarController::SetGlobal
//
void I_SharedVarController::SetGlobal(Ptr<I_SharedVarController const> const controller)
{
	s_GlobalInstance = controller;
}

//----------------------------------
// I_SharedVarController::GetGlobal
//
et::rhi::I_SharedVarController const* I_SharedVarController::GetGlobal()
{
	return s_GlobalInstance.Get();
}


} // namespace rhi
} // namespace et
