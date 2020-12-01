#pragma once
#include <EtRendering/GraphicsTypes/ParameterBlock.h>


namespace et {
namespace render {


class Material;


//---------------------------------
// I_Material
//
// Interface that allows rendering systems to use materials and their instances in a unified way
//
class I_Material
{
	RTTR_ENABLE()
public:
	virtual ~I_Material() = default;

	virtual Material const* GetBaseMaterial() const = 0;
	virtual T_ConstParameterBlock GetParameters() const = 0;
};


} // namespace render
} // namespace et
