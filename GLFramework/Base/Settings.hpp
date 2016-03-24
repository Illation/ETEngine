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
			Fullscreen(false),
			Width(1280),
			Height(720),
			AspectRatio(Width / (float)Height),
			Title("OpenGl Framework"),
			pWindow(nullptr)
		{
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

