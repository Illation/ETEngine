#include "stdafx.h"
#include "PlaylistSystem.h"

#include <EtFramework/Systems/AudioSourceSystem.h>


namespace demo {


//=================
// Playlist System
//=================


//-----------------------
// PlaylistSystem::c-tor
//
PlaylistSystem::PlaylistSystem()
{
	DeclareDependents<fw::AudioSourceSystem::State>(); // update before track changes
}

//-------------------------
// PlaylistSystem::Process
//
void PlaylistSystem::Process(fw::ComponentRange<PlaylistSystemView>& range)
{
	// common variables
	InputManager* const input = InputManager::GetInstance();

	if (input->GetKeyState(E_KbdKey::P) == E_KeyState::Pressed)
	{
		for (PlaylistSystemView& view : range)
		{
			if (view.source->GetState() != fw::AudioSourceComponent::E_PlaybackState::Playing)
			{
				// start playing the first song if it's not playing already
				if ((view.playlist->currentTrack == std::numeric_limits<size_t>::max()) && view.playlist->tracks.size() > 0u)
				{
					view.playlist->currentTrack = 0u;

					view.source->SetAudioData(view.playlist->tracks[view.playlist->currentTrack].GetId());
				}

				view.source->SetState(fw::AudioSourceComponent::E_PlaybackState::Playing);
			}
			else 
			{
				view.source->SetState(fw::AudioSourceComponent::E_PlaybackState::Paused);
			}
		}
	}

	if (input->GetKeyState(E_KbdKey::LeftBracket) == E_KeyState::Pressed)
	{
		for (PlaylistSystemView& view : range)
		{
			if (view.playlist->tracks.size() > 0u)
			{
				if (view.playlist->currentTrack == std::numeric_limits<size_t>::max())
				{
					view.playlist->currentTrack = view.playlist->tracks.size() - 1;
				}
				else
				{
					view.playlist->currentTrack--;
					if (view.playlist->currentTrack >= view.playlist->tracks.size())
					{
						view.playlist->currentTrack = view.playlist->tracks.size() - 1;
					}
				}

				view.source->SetAudioData(view.playlist->tracks[view.playlist->currentTrack].GetId());
			}
		}
	}

	if (input->GetKeyState(E_KbdKey::RightBracket) == E_KeyState::Pressed)
	{
		for (PlaylistSystemView& view : range)
		{
			if (view.playlist->tracks.size() > 0u)
			{
				if (view.playlist->currentTrack == std::numeric_limits<size_t>::max())
				{
					view.playlist->currentTrack = 0u;
				}
				else
				{
					view.playlist->currentTrack = (view.playlist->currentTrack + 1u) % view.playlist->tracks.size();
				}

				view.source->SetAudioData(view.playlist->tracks[view.playlist->currentTrack].GetId());
			}
		}
	}
}


} // namespace demo

