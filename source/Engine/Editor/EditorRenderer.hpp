#pragma once
#ifdef EDITOR

class EditorRenderer
{
public:

	EditorRenderer();
	virtual ~EditorRenderer();

	void Initialize();

	void Draw();
	void DrawRenderers();
};

#endif