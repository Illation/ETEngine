#pragma once
#pragma warning(disable : 4201) //nameless struct union

#include <string>
#include <vector>

#include <EtCore/Helper/Singleton.h>
#include <Engine/Helper/MulticastDelegate.h>


struct Settings : public Singleton<Settings>
{
public:
	struct GraphicsSettings
	{
		GraphicsSettings();
		virtual ~GraphicsSettings();

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

		float GetAspectRatio();

		void Resize( int32 width, int32 height, bool broadcast = true );

		std::string Title;
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

