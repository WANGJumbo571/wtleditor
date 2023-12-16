#pragma once

#include "stdafx.h"
#include "atlmisc.h"
#include "ImageOperation.h"
#include "CBRotatablePtr.h"

typedef enum { style_f, style_c, style_r, style_b } text_style;

class DX_DECLARE_INTERFACE("43D782E6-A59B-404D-9BBC-2C77391A7F60") CBSingleCharLayout : public IUnknown
{
public:
	CBSingleCharLayout() {}
	~CBSingleCharLayout() {}

	WCHAR m_charActer;
	ComPtr<IDWriteTextLayout> m_charLayout;
	FLOAT m_charLayoutWidth;
	FLOAT m_charLayoutHeight;
	D2D1_RECT_F m_charLayoutRect;
	
	bool QueryInterfaceHelper(const IID &iid, void **object)
	{
		return CastHelper<CBSingleCharLayout>::CastTo(iid, this, object);
	}
};

class CBTextPtr : public CBRotatablePtr, public CBPastingPtr
{
public:
	CBTextPtr() { m_textString = L""; }
	~CBTextPtr() {}

	// marked if the text is showed every line from upper to lower direction(vertically).
	text_style	m_textStyle;

	// 是否是左右摇摆动画，即摇摆幅度
	DOUBLE m_waveAngle;

	bool													m_textIsActive;
	ComPtr<ID2D1SolidColorBrush>		m_textBrush;
	ComPtr<IDWriteTextFormat>			m_textFormat;
	WCHAR											m_textFormatFamilyName[32];
	FLOAT												m_textFormatFontSize;
	std::vector<ComPtr<CBSingleCharLayout> >	m_textLayout;
	std::vector<int>								m_textRowStart;
	FLOAT												m_textSingleCharWidth;
	FLOAT												m_textSingleCharHeight;
	CString												m_textString;
	int													m_textPoint;

	// m_textIfMarked true if high-lighted text(from position m_textMarkLeft to m_textMarkRight)
	// should be drawed.
	bool	m_textIfMarked;
	int	m_textMarkStart;
	int	m_textMarkShift;
	int	m_textMarkLeft;
	int	m_textMarkRight;

	D2D1_POINT_2F	m_textCursorTop;
	D2D1_POINT_2F	m_textCursorBottom;
	FLOAT					m_textBoxWidth;
	FLOAT					m_textBoxHeight;
	FLOAT					m_textCharSpace;
	FLOAT					m_textLineSpace;
	D2D1_COLOR_F	m_textColor;
	FLOAT					m_textOpacity;
	D2D1_RECT_F		m_textDest;

	// 文字框的左上角，用来移动文字框
	bool	m_tbIsActiveLeftTop;
	// 文字框的中间编辑部分
	bool	m_tbIsActiveEdit;
	// 文字框的右边界，用来改变文字框的宽度
	bool	m_tbIsActiveRight;
	bool m_tblIsActiveLeft;
	// 文字框的下边界，用来改变文字框的高度
	bool	m_tbIsActiveBottom;
};


__interface  DX_DECLARE_INTERFACE("42D782E6-A59B-404D-9BBD-2C27391A7F60") IImageTextOperation : public IUnknown
{
	HRESULT _stdcall GetTextEquipment(__out CBTextPtr * te);
	HRESULT _stdcall SetTextEquipment(__in CBTextPtr te);
};

class ImageTextOperation : public IImageOperation, public IImageTextOperation
{
public:

	HRESULT _stdcall GetTextEquipment(__out CBTextPtr * te);
	HRESULT _stdcall SetTextEquipment(__in CBTextPtr te);

	void SetActive(bool isActive);
	bool GetActive() { return textInfo.m_textIsActive; }
	void SetOpacity(FLOAT opacity);
	FLOAT GetOpacity();
	void		SetDuration(FLOAT duration) { m_duration = duration; }
	FLOAT	GetDuration() { return m_duration; }

	static void TEXT_FrameOutText(CBTextPtr & bp);

	static void TEXT_KeyDown_Up(CBTextPtr &bp, int &textPoint);
	static void TEXT_KeyDown_Down(CBTextPtr &bp, int &textPoint);
	static void TEXT_KeyDown_Left(CBTextPtr &bp);
	static void TEXT_KeyDown_Right(CBTextPtr &bp);

	static void TEXT_KeyDown_Shift_Up(CBTextPtr &bp);
	static void TEXT_KeyDown_Shift_Down(CBTextPtr &bp);
	static void TEXT_KeyDown_Shift_Left(CBTextPtr &bp);
	static void TEXT_KeyDown_Shift_Right(CBTextPtr &bp);

	static void TEXT_KeyDown_Delete(CBTextPtr &bp);
	static void TEXT_KeyDown_Back(CBTextPtr &bp);
	static void TEXT_KeyDown_Escape(CBTextPtr& bp);

protected:
	ImageTextOperation(CBTextPtr te);
	virtual ~ImageTextOperation();

	// Interface helper
	bool QueryInterfaceHelper(const IID &iid, void **object)
	{
		return CastHelper<IImageOperation>::CastTo(iid, this, object) ||
			CastHelper<IImageTextOperation>::CastTo(iid, this, object);
	}

private:
	CBTextPtr		textInfo;
	FLOAT			m_duration = 0.0f;
};
