#pragma once
#include "UIUtil.hpp"

class UIComponent;

class UIContainer
{
public:
	//Should set the world position and return a rectangle in world position and its local scale
	virtual iRect CalculateDimensions(const ivec2 &worldPos) = 0;

	virtual void Update() {}

	//offset to world position
	void SetLocalPos( ivec2 pos ) { m_Rect.pos = pos; }

	//Drawing - return true if it has children, level indicates the depth 
	//breadth first in most cases, unless we are drawing a frame
	virtual bool Draw( uint16 level ) = 0;

protected:
	iRect m_Rect;
	ivec2 m_WorldPos;

	uint16 m_Level = std::numeric_limits<uint16>::max();
};

class UIFixedContainer : public UIContainer
{
public:
	UIFixedContainer() :UIContainer() {}
	iRect CalculateDimensions(const ivec2 &worldPos);
	//overridable for containers that need to resize buffers etc
	virtual void SetSize(ivec2 size) { m_Rect.size = size; }
	//Guaranteed to only set the size
	virtual void SetSizeOnly(ivec2 size) { m_Rect.size = size; }
	ivec2 GetSize() const { return m_Rect.size; }
};

class UIDynamicBox : public UIContainer
{
public:
	enum class Mode : uint8
	{
		HORIZONTAL,
		VERTICAL
	};

	enum class Positioning : uint8
	{
		DYNAMIC,	
		UI_RELATIVE	
	};

	UIDynamicBox( UIDynamicBox::Mode mode ) :m_Mode( mode ) {}
	virtual ~UIDynamicBox();

	iRect CalculateDimensions( const ivec2 &worldPos);
	virtual bool Draw( uint16 level );

	virtual void Update()override;

	void AddChild( UIContainer* child, Positioning positioning );
private:
	UIDynamicBox::Mode m_Mode;

	std::vector<UIContainer*> m_RelativeChildren;
	std::vector<UIContainer*> m_DynamicChildren;
};

class UIPortal : public UIFixedContainer
{
public:
	UIPortal() :UIFixedContainer() {}
	virtual ~UIPortal() { delete m_Child; }

	virtual bool Draw(uint16 level);

	virtual void Update()override { if (m_Child)m_Child->Update(); }

	void SetChild(UIContainer* child) { m_Child = child; }
	void SetColor(vec4 val) { m_Color = val; }

private:
	UIContainer* m_Child = nullptr;

	vec4 m_Color = vec4(1);
};

class UISplitter : public UIFixedContainer
{
public:
	enum class Mode : uint8
	{
		HORIZONTAL,
		VERTICAL
	}; 

	UISplitter(UISplitter::Mode mode) :UIFixedContainer(), m_Mode(mode) {}
	virtual ~UISplitter();
	virtual bool Draw(uint16 level);

	void Update() override;

	void SetSize(ivec2 size)override;
	void SetSizeOnly(ivec2 size)override;
	void SetSplitPercentage(float perc);

	void SetFirst(UIFixedContainer* child) { m_First = child; }
	void SetSecond(UIFixedContainer* child) { m_Second = child; }
private:
	void RecalculateSplit(bool sizeOnly = false);

	UIFixedContainer* m_First = nullptr;
	UIFixedContainer* m_Second = nullptr;

	float m_SplitRegionPix = 5;
	bool m_DragActive = false;

	float m_SplitPercentage = 0.2f;
	UISplitter::Mode m_Mode;
};