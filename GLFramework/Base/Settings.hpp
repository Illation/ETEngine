#pragma once
#include "../Helper/Singleton.h"
#include <string>
#include <SDL.h>

struct Settings : public Singleton<Settings>
{
public:
	Settings():Window(WindowSettings())
	{

	}
	~Settings() {}

	struct WindowSettings
	{
		WindowSettings() :
			Fullscreen(true),
			AspectRatio(Width / (float)Height),
			Title("OpenGl Framework"),
			pWindow(nullptr)
		{
			Width = Fullscreen ? 1920 : 1280;
			Height = Fullscreen ? 1080 : 720;
		}
		void VSync(const bool enabled){SDL_GL_SetSwapInterval(enabled);}

		bool Fullscreen;
		int Width;
		int Height;
		float AspectRatio;
		std::string Title;
		SDL_Window* pWindow;
	}Window;
};

