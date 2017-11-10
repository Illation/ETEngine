#include "stdafx.hpp"
#include "Settings.hpp"

#ifdef EDITOR
#include "../Editor/Editor.hpp"
#endif

Settings::Settings() :Window( WindowSettings() ), Graphics( GraphicsSettings() )
{

}

Settings::~Settings()
{
	SDL_DestroyWindow( Window.pWindow );
}

Settings::GraphicsSettings::GraphicsSettings() :
	NumCascades( 3 ),
	NumPCFSamples( 3 ),
	CSMDrawDistance( 200 ),
	NumBlurPasses( 5 )
{
}
Settings::GraphicsSettings::~GraphicsSettings()
{
}

Settings::WindowSettings::WindowSettings() :
	Fullscreen( false ),
	Title( "ETEngine" ),
	pWindow( nullptr )
{
	std::vector<ivec2> resolutions;
	resolutions.push_back( ivec2( 1280, 720 ) );
	resolutions.push_back( ivec2( 1920, 1080 ) );
	resolutions.push_back( ivec2( 2560, 1440 ) );

	uint32 baseRes = 2;

	Width = Fullscreen ? resolutions[baseRes].x : resolutions[baseRes - 1].x;
	Height = Fullscreen ? resolutions[baseRes].y : resolutions[baseRes - 1].y;
#ifdef EDITOR
	EditorWidth = Width;
	EditorHeight = Height;
#endif
}
Settings::WindowSettings::~WindowSettings()
{

}

float Settings::WindowSettings::GetAspectRatio()
{
	return (float)Width / (float)Height;
}

void Settings::WindowSettings::Resize( int32 width, int32 height )
{
#ifdef EDITOR
	EditorWidth = width;
	EditorHeight = height;
	Editor::GetInstance()->OnWindowResize();
	Dimensions = Editor::GetInstance()->GetViewport().size;
#else
	Width = width;
	Height = height;
#endif
	WindowResizeEvent.Broadcast();
}