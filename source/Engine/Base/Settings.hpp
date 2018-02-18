#pragma once
#pragma warning(disable : 4201) //nameless struct union
#include "../Helper/Singleton.hpp"
#include "../Helper/MulticastDelegate.hpp"
#include <string>
#include <SDL.h>
#include <vector>

struct Settings : public Singleton<Settings>
{
public:
	struct GraphicsSettings
	{
		GraphicsSettings();
		virtual ~GraphicsSettings();

		void VSync( const bool enabled ) { SDL_GL_SetSwapInterval( enabled ); }

		bool UseFXAA;

		//Shadow Quality
		int32 NumCascades;
		float CSMDrawDistance;
		int32 NumPCFSamples;

		int32 PbrBrdfLutSize;

		float TextureScaleFactor;

		//Bloom Quality
		int32 NumBlurPasses;
	}Graphics;

	struct WindowSettings
	{
		MulticastDelegate WindowResizeEvent;

		WindowSettings();
		virtual ~WindowSettings();

		void VSync( const bool enabled ) { SDL_GL_SetSwapInterval( enabled ); }
		float GetAspectRatio();

		void Resize( int32 width, int32 height, bool broadcast = true );

		std::string Title;
		SDL_Window* pWindow;
		bool Fullscreen;
		union
		{
			struct
			{
				int32 Width;
				int32 Height;
	#if EDITOR
				int32 EditorWidth;
				int32 EditorHeight;
	#endif
			};
	#if EDITOR
			struct
			{
				ivec2 Dimensions;
				ivec2 EditorDimensions;
			};
	#else
			ivec2 Dimensions;
	#endif
		};
	}Window;
private:
	friend class Singleton<Settings>;

	Settings();
	virtual ~Settings();

	Settings( const Settings& t );
	Settings& operator=( const Settings& t );
};

