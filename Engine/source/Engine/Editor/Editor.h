#pragma once
#include <EtCore/Helper/Singleton.h>
#include <EtCore/UpdateCycle/Tickable.h>
#include <EtCore/Content/AssetPointer.h>

#include <Engine/UI/UIUtil.h>


#ifdef EDITOR

class TextureData;
class EditorRenderer;
class UIViewport;
class UIFixedContainer;
class UIPortal;
class SpriteFont;


class Editor : public Singleton<Editor>, public I_Tickable
{
public:

	void Initialize();

	void OnTick() override;
	void DrawSceneVisualizers();
	void Draw();

	GLuint GetSceneTarget();

	void OnWindowResize(ivec2 EditorDimensions);
	void CalculateViewportSize(ivec2 FullWindowDimensions, bool resizeBuffer = false);

	UIViewport* GetViewport() { return m_Viewport; }

	SpriteFont* EditorFont() const { return m_pEditorFont; }

private:
	UIViewport* m_Viewport = nullptr;
	UIFixedContainer* m_Root = nullptr;
	UIPortal* m_pToolbar = nullptr;

	AssetPtr<SpriteFont> m_EditorFont;

private:
	//Disable constructors
	friend class Singleton<Editor>;

	Editor();
	virtual ~Editor();

	Editor( const Editor& t );
	Editor& operator=( const Editor& t );
};


#endif