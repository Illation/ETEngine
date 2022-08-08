#pragma once
#include <EtRHI/GraphicsContext/GraphicsTypes.h>


namespace et {
namespace rhi {


//-----------------------
// I_SharedVarController
//
// Interface for a uniform buffer that adds globally shared variables to shaders
//
class I_SharedVarController 
{
	// static
	//--------
	static Ptr<I_SharedVarController const> s_GlobalInstance;
	
public:
	static void SetGlobal(Ptr<I_SharedVarController const> const controller);
	static I_SharedVarController const* GetGlobal();

	// interface
	//-----------
	virtual ~I_SharedVarController() = default;

	virtual T_BufferLoc const GetBufferLocation() const = 0;
	virtual uint32 const GetBufferBinding() const = 0;
	virtual std::string const& GetBlockName() const = 0;
};


} // namespace rhi
} // namespace et

