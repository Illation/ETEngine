#pragma once
#include <EtCore/Util/LinkerUtils.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


namespace et {
namespace demo {


//---------------------------------
// ControledLightComponent
//
// Tag component that marks a light for rotation and brightness controlls
//
struct ControledLightComponent final : public fw::SimpleComponentDescriptor
{
	ECS_DECLARE_COMPONENT
	RTTR_ENABLE(fw::SimpleComponentDescriptor)
	DECLARE_FORCED_LINKING()
};


} // namespace demo
} // namespace et

