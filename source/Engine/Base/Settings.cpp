#include "stdafx.hpp"
#include "Settings.hpp"

#ifdef EDITOR
#include "../Editor/Editor.hpp"
#include "UI/UIViewport.h"
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
	PbrBrdfLutSize( 512 ),
	TextureScaleFactor( 0.25f ),
	NumBlurPasses( 5 ),
	UseFXAA(true)
{
}
Settings::GraphicsSettings::~GraphicsSettings()
{
}

Settings::WindowSettings::WindowSettings() :
	Fullscreen( false ),
	Title( "ETEngine" ),
	pWindow( nullptr ),
	Dimensions(ivec2(1920, 1080))
{
#ifdef EDITOR
	EditorWidth = Width;
	EditorHeight = Height;
	//Editor::GetInstance()->CalculateViewportSize(EditorDimensions);
	//Dimensions = Editor::GetInstance()->GetViewport()->GetSize();
#endif
}
Settings::WindowSettings::~WindowSettings()
{

}

float Settings::WindowSettings::GetAspectRatio()
{
	return (float)Width / (float)Height;
}

void Settings::WindowSettings::Resize( int32 width, int32 height, bool broadcast )
{
#ifdef EDITOR
	EditorWidth = width;
	EditorHeight = height;
	Editor::GetInstance()->CalculateViewportSize( ivec2( width, height ) );
	Dimensions = Editor::GetInstance()->GetViewport()->GetSize();
	if (broadcast) Editor::GetInstance()->OnWindowResize(EditorDimensions);
#else
	Width = width;
	Height = height;
#endif
	if(broadcast) WindowResizeEvent.Broadcast();
}