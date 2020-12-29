#include "stdafx.h"
#include "SwirlyLightSystem.h"

#include <EtFramework/Systems/TransformSystem.h>


namespace et {
namespace demo {


//======================
// Swirly Light System
//======================


//-------------------------------
// SwirlyLightSystem::c-tor
//
SwirlyLightSystem::SwirlyLightSystem()
{
	DeclareDependents<fw::TransformSystem::Compute>(); // update before transform matrix recalculation
}

//------------------------------
// SwirlyLightSystem::Process
//
void SwirlyLightSystem::Process(fw::ComponentRange<SwirlyLightSystemView>& range)
{
	// common vars
	float const dt = core::ContextManager::GetInstance()->GetActiveContext()->time->DeltaTime();

	for (SwirlyLightSystemView& view : range)
	{
		// update first angle
		view.swirly->timeOffset1 += dt;
		view.swirly->angle1 += dt * view.swirly->timeMult1 * (1.f + 0.9f * std::cosf(view.swirly->timeOffset1));

		// update second angle
		view.swirly->timeOffset2 += dt;
		view.swirly->angle2 += dt * view.swirly->timeMult2 * (1.f + 0.9f * std::cosf(view.swirly->timeOffset2));

		// calc position
		vec3 pos = view.swirly->origin;
		pos.x += view.swirly->radius1 * std::cosf(view.swirly->angle1);
		pos.y -= view.swirly->radius2 * std::sinf(view.swirly->angle2);
		pos.z -= view.swirly->radius1 * std::sinf(view.swirly->angle1);

		view.transf->SetPosition(pos);

		// update light brightness
		view.light->SetBrightness(std::max(view.light->GetBrightness() + std::cosf(view.swirly->angle1) * 3.5f, 0.f));
	}
}


} // namespace demo
} // namespace et
