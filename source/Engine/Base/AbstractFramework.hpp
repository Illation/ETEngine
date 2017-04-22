#pragma once

#include "../staticDependancies/glad/glad.h"
#include <SDL.h>
#include <SDL_opengl.h>

#include <iostream>

//Static stuff
static void sdl_die(const char * message)
{
	fprintf(stderr, "%s: %s\n", message, SDL_GetError());
	exit(2);
}
static void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	(void)source; (void)type; (void)id;
	(void)severity; (void)length; (void)userParam;
	std::cout << std::endl;
	fprintf(stderr, "%s\n", message);
	if (severity == GL_DEBUG_SEVERITY_HIGH)
	{
		fprintf(stderr, "Aborting...\n");
		abort();
	}
	std::cout << std::endl;
}

class AbstractFramework
{
public:
	AbstractFramework();
	virtual ~AbstractFramework();

	void Run();

protected:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	void ClearTarget();

private:
	void InitializeSDL();
	void InitializeWindow();
	void InitializeDevIL();
	void BindOpenGL();
	void InitializeDebug();
	void InitializeGame();

	void GameLoop();

private:
	SDL_GLContext m_GlContext = nullptr;
};

