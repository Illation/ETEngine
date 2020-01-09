#include "stdafx.h"
#include "AudioListenerSystem.h"

#include <AL/al.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Systems/TransformSystem.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Audio/OpenAL_ETM.h>


namespace fw {


//=======================
// Audio Listener System 
//=======================


//----------------------------
// AudioListenerSystem::c-tor
//
// system dependencies
//
AudioListenerSystem::AudioListenerSystem()
{
	DeclareDependencies<TransformSystem::Compute>(); // update lights after updating transforms, though we don't need to wait for flags to update
}

//------------------------------
// AudioListenerSystem::Process
//
// Update active audio listener properties
//
void AudioListenerSystem::Process(ComponentRange<AudioListenerSystemView>& range) const
{
#ifndef ET_SHIPPING
	uint32 count = 0u;
	AudioManager* const audioMan = AudioManager::GetInstance();
#endif 

	for (AudioListenerSystemView& view : range)
	{
		// debugging
		//------------
#ifndef ET_SHIPPING
		++count;
#endif 

		// gain
		//------
		alListenerf(AL_GAIN, view.listener->m_Gain);
		ET_ASSERT(!audioMan->TestALError("set gain error"));

		// transformation
		//----------------

		// delta calculation
		vec3 const pos = ALvec3(view.transf->GetWorldPosition());
		vec3 const vel = pos - view.listener->m_PrevPos;
		view.listener->m_PrevPos = pos;

		// pos and speed
		alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
		ET_ASSERT(!audioMan->TestALError("set position error"));

		alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
		ET_ASSERT(!audioMan->TestALError("set velocity error"));

		// orientation
		vec3 const fwd = ALvec3(view.transf->GetForward());
		vec3 const up = ALvec3(view.transf->GetUp());
		ALfloat listenerOri[] = { fwd.x, fwd.y, fwd.z, up.x, up.y, up.z };

		alListenerfv(AL_ORIENTATION, listenerOri);
		ET_ASSERT(!audioMan->TestALError("set orientation error"));
	}

	ET_ASSERT(count <= 1u, "there should only be a single active audio listener");
}


} // namespace fw

