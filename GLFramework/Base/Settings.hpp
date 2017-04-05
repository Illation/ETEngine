#pragma once
#include "../Helper/Singleton.h"
#include <string>
#include <SDL.h>
#include <vector>

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
			AspectRatio(Width / (float)Height),
			Title("OpenGl Framework"),
			pWindow(nullptr)
		{
			std::vector<glm::ivec2> resolutions;
			resolutions.push_back(glm::ivec2(1280, 720));
			resolutions.push_back(glm::ivec2(1920, 1080));
			resolutions.push_back(glm::ivec2(2560, 1440));

			unsigned int baseRes = 2;

			Width = Fullscreen ? resolutions[baseRes].x : resolutions[baseRes-1].x;
			Height = Fullscreen ? resolutions[baseRes].y : resolutions[baseRes-1].y;
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

