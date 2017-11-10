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
	Settings();
	~Settings();

	struct GraphicsSettings
	{
		GraphicsSettings();
		~GraphicsSettings();

		void VSync( const bool enabled ) { SDL_GL_SetSwapInterval( enabled ); }

		//Shadow Quality
		int32 NumCascades;
		float CSMDrawDistance;
		int32 NumPCFSamples;

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

		void Resize( int32 width, int32 height );

		std::string Title;
		SDL_Window* pWindow;
		bool Fullscreen;
		union
		{
			struct
			{
				int32 Width;
				int32 Height;
			};
			ivec2 Dimensions;
		};
	#ifdef EDITOR
		union
		{
			struct
			{
				int32 EditorWidth;
				int32 EditorHeight;
			};
			ivec2 EditorDimensions;
		};
	#endif
	}Window;
};

