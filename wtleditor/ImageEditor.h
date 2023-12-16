//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================

#pragma once

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlgdi.h>
#include <atlribbon.h>

#include <gdiplus.h> 
#include <gdiplusheaders.h>
#include <gdiplusbitmap.h>
#include <gdipluscachedbitmap.h>
#include <gdiplusimaging.h>

#include "PointAnimation.h"
#include "ShellItemsLoader.h"
#include "SimpleImage.h"
#include "resource.h"
#include "Ribbon.h"
#include "ImageTextOperation.h"
#include "ImageLayerOperation.h"

#include "UIRibbon.h"
#include "aboutdlg.h"

#define WM_UPDATEROWCOL		(WM_USER + 1000)
#define MAXLAYERS						120
#define DISTANCE						20

extern WCHAR				g_textFormatFamilyName[];
extern FLOAT				g_textFormatFontSize;
extern COLORREF			g_fontColor;
extern COLORREF			g_bkColor;
extern int						g_charSpace;
extern int						g_lineSpace;
extern FLOAT				g_animDuration;

extern bool					g_ifShowCursor;
extern text_style			g_textStyle;

extern WCHAR* utf8ToUnicode(const char* zFilename);

#define CURSOR_TIMER	1001
#define DELAY_TIMER		1002

struct ImageItem
{
    ComPtr<IImage> Image;
    ComPtr<IPointAnimation> Animation;
};

class ImageEditorHandler : public CWindowImpl<ImageEditorHandler>
{
public:
	DECLARE_WND_CLASS(NULL)

	BEGIN_MSG_MAP(ImageEditorHandler)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, AnnotOnEraseBackground)
		MESSAGE_HANDLER(WM_SIZE, AnnotOnSize)
		MESSAGE_HANDLER(WM_KEYDOWN, AnnotKeyDown)
		MESSAGE_HANDLER(WM_CHAR, AnnotChar)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, AnnotOnLeftMouseButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, AnnotOnLeftMouseButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, AnnotOnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, AnnotOnMouseLeave)
		MESSAGE_HANDLER(WM_SETCURSOR, AnnotOnSetCursor)
		MESSAGE_HANDLER(WM_TIMER, AnnotOnTimer)
//		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)

		COMMAND_ID_HANDLER(ID_RIBBON_FONT, OnRibbonFont)
		COMMAND_ID_HANDLER(ID_WTL_MENU_NEW, AnnotNew)
		COMMAND_ID_HANDLER(ID_ANNO_FILE_OPEN, AnnotOpenFile)
		COMMAND_ID_HANDLER(ID_ANNO_FILE_SAVE, AnnotSaveFiles)
		COMMAND_ID_HANDLER(ID_WTL_PUBLISH, AnnotPublish)
		COMMAND_ID_HANDLER(HOME, AnnotHome)
		//COMMAND_ID_HANDLER(ID_ANNO_FILE_SAVE_AS, AnnotSaveFileAs)

		COMMAND_ID_HANDLER(rotateButton, AnnotR90)
		COMMAND_ID_HANDLER(rotate90Clockwise, AnnotR90)
		COMMAND_ID_HANDLER(rotate90CounterClockwise, AnnotR90CC)
		COMMAND_ID_HANDLER(flipHorizontal, AnnotFH)
		COMMAND_ID_HANDLER(flipVertical, AnnotFV)

		COMMAND_ID_HANDLER(ID_LAYERS, AnnotCombDlg)
		COMMAND_ID_HANDLER(layer_rotateButton, AnnotLayerR360)
		COMMAND_ID_HANDLER(ID_WTL_ROTATE_30, AnnotLayerR30)
		COMMAND_ID_HANDLER(layer_rotate90Clockwise, AnnotLayerR90)
		COMMAND_ID_HANDLER(layer_rotate90CounterClockwise, AnnotLayerR90CC)
		COMMAND_ID_HANDLER(layer_flipHorizontal, AnnotLayerFH)
		COMMAND_ID_HANDLER(layer_flipVertical, AnnotLayerFV)

		COMMAND_ID_HANDLER(IDC_WTL_PIC_SIZE_BIGGER, AnnotLayerBigger)
		COMMAND_ID_HANDLER(IDC_WTL_PIC_SIZE_SMALLER, AnnotLayerSmaller)
		COMMAND_ID_HANDLER(ID_WTL_DELETE_LAYER, AnnotDeleteLayer)
		COMMAND_ID_HANDLER(ID_WTL_TRANSLATION_LAYER, AnnotPast)
		COMMAND_ID_HANDLER(ID_WTL_FULLSCREEN, AnnotFullscreen)
		COMMAND_ID_HANDLER(ID_WTL_PAUSE, AnnotPause)
		COMMAND_ID_HANDLER(ID_WTL_ROLLUP, AnnotRollup)
		COMMAND_ID_HANDLER(ID_WTL_WRITE_OPACITY, AnnotWriteOpacity)
		COMMAND_ID_HANDLER(ID_WTL_WRITE_PUNCH, AnnotWritePunch)
		COMMAND_ID_HANDLER(ID_WTL_WRITE_WAVE, AnnotWriteWave)
		COMMAND_ID_HANDLER(ID_WTL_DRAW_POINT, AnnotDrawRipple)
		COMMAND_ID_HANDLER(ID_WTL_DRAW_SCROLL, AnnotDrawHScroll)
		COMMAND_ID_HANDLER(ID_VSCROLL, AnnotDrawVScroll)

		COMMAND_ID_HANDLER(ID_WTL_F, AnnotTextF)
		COMMAND_ID_HANDLER(ID_WTL_R, AnnotTextR)
		COMMAND_ID_HANDLER(ID_WTL_T, AnnotTextT)
		COMMAND_ID_HANDLER(ID_WTL_B, AnnotTextB)
		
		COMMAND_ID_HANDLER(IDC_WTL_MENU_COPY_TEXT, OnMenuCopyText)
		COMMAND_ID_HANDLER(IDC_WTL_MENU_CUT_TEXT, OnMenuCutText)
		COMMAND_ID_HANDLER(IDC_WTL_MENU_PASTE_TEXT, OnMenuPasteText)

		COMMAND_ID_HANDLER(cropButton, AnnotCrop)
		
		COMMAND_ID_HANDLER(ID_OPEN_SVG, AnnotOpenSVGFile)
		COMMAND_ID_HANDLER(ID_WTL_OPEN_LAYER_FILE, AnnotOpenLayerFile)
		COMMAND_ID_HANDLER(ID_LOCATE, AnnotLayerLocate)
		COMMAND_ID_HANDLER(ID_WTL_OPEN_PICS_FILE, AnnotOpenPicsFile)
		COMMAND_ID_HANDLER(ID_LOCATE_PICS, AnnotPicsLocate)
		COMMAND_ID_HANDLER(ID_LOCATE_SVG, AnnotSVGLocate)
		COMMAND_ID_HANDLER(ID_WTL_SHOW_PICS, AnnotShowPics)
		COMMAND_ID_HANDLER(ID_WTL_MENU_COPY, AnnotCopyCurrentImageDataToClipboard)
		COMMAND_ID_HANDLER(ID_WTL_MENU_NEW_CLIPBOARD, AnnotNewFromClipboard)
		COMMAND_ID_HANDLER(ID_WTL_RESIZE_IMAGE, AnnotResizeImage)

		COMMAND_ID_HANDLER(ID_BUTTON_ZOOM_IN, AnnotZoomIn)
		COMMAND_ID_HANDLER(ID_BUTTON_ZOOM_OUT, AnnotZoomOut)
		COMMAND_ID_HANDLER(ID_WTL_100, AnnotZoom100)
		COMMAND_ID_HANDLER(ID_WTL_FITIN, AnnotZoomFitIn)
		
		COMMAND_ID_HANDLER(ID_BUTTON_UNDO, AnnotUndo)
		COMMAND_ID_HANDLER(ID_BUTTON_REDO, AnnotRedo)
		
		COMMAND_ID_HANDLER(ID_STANDARD_COLORPICKER, AnnotColorPicker)
		COMMAND_ID_HANDLER(ID_ANNO_FILE_EXIT, AnnotExit)
		COMMAND_ID_HANDLER(POWER, AnnotExit)
	END_MSG_MAP()

public:
	HRESULT __stdcall SetCurrentLocation(IShellItem* shellFolder);
	HRESULT __stdcall SetDrawingOperation(__in ImageOperationType imageDrawingOperation);

	// self added funcs
	HRESULT __stdcall ROTA_SetRotationOperation(__in ImageOperationType imageDrawingOperation);

	HRESULT __stdcall SetPenColor(__in D2D1_COLOR_F penColor);
	HRESULT __stdcall SetPenSize(__in float penSize);
	HRESULT __stdcall OpenFile();
	HRESULT __stdcall SaveFiles();
	HRESULT __stdcall ZoomIn();
	HRESULT __stdcall ZoomOut();
	HRESULT __stdcall ZoomFull();
	HRESULT __stdcall CanUndo(__out bool* canUndo);
	HRESULT __stdcall CanRedo(__out bool* canRedo);
	HRESULT __stdcall Undo();
	HRESULT __stdcall Redo();

private:
	ULONG_PTR token;
	//CFont m_font;
	bool ifShutDown = false;

public:
	// Constructor/destructor
	ImageEditorHandler();
	~ImageEditorHandler();
	
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT AnnotOnTimer(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
	{
		if (wParam == CURSOR_TIMER)
		{
			KillTimer(CURSOR_TIMER);
			g_ifShowCursor = !g_ifShowCursor;
			if (ifShutDown == false)
			{
				InvalidateWindow();
				SetTimer(CURSOR_TIMER, 1000);
			}
		}
		else if (wParam == DELAY_TIMER)
		{
			KillTimer(DELAY_TIMER);
			CRect rect;
			GetClientRect(&rect);

			::PostMessage(m_hWnd, WM_LBUTTONDOWN, 0,
				((short)((rect.right - rect.left) / 2.0f)) | ((short)((rect.bottom - rect.top) / 2.0f) << 16));
			::PostMessage(m_hWnd, WM_LBUTTONUP, 0,
				((short)((rect.right - rect.left) / 2.0f)) | ((short)((rect.bottom - rect.top) / 2.0f) << 16));
		}
		return 0;
	}

	LRESULT AnnotOnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
	{
		unsigned int width = LOWORD(lParam);
		unsigned int height = HIWORD(lParam);
		OnSize(width, height);
		return 0;
	}
	LRESULT AnnotOnEraseBackground(UINT, WPARAM wParam, LPARAM lParam, BOOL& bhandled)
	{
		return 0;
	}
	LRESULT AnnotChar(UINT, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL &)
	{
		PAINTSTRUCT ps;
		::BeginPaint(m_hWnd, &ps);
		OnRender();
		::EndPaint(m_hWnd, &ps);
		return 0;
	}
	LRESULT AnnotKeyDown(UINT, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT AnnotOnLeftMouseButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT AnnotOnLeftMouseButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT AnnotOnMouseMove(UINT message, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT AnnotOnMouseLeave(UINT message, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT AnnotOnSetCursor(UINT message, WPARAM wParam, LPARAM lParam, BOOL &)
	{
		return 0;
	}
	LRESULT OnRibbonFont(WORD hwParam, WORD lwParam, HWND, BOOL&);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT AnnotOpenLayerFile(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotLayerLocate(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotOpenPicsFile(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotCombDlg(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotPicsLocate(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotSVGLocate(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotShowPics(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotNew(WORD, WORD wID, HWND, BOOL&);
	void AnnotInnerNew(COLORREF bgColor);
	LRESULT AnnotNewFromClipboard(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotOpenFile(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotSaveFiles(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotDrawRipple(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotDrawHScroll(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotDrawVScroll(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotHome(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotResizeImage(WORD, WORD wID, HWND, BOOL&);
	LRESULT OnMenuPasteText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMenuCopyText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMenuCutText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT AnnotR90(WORD, WORD wID, HWND, BOOL&)
	{
		SetDrawingOperation(ImageOperationTypeRotateClockwise);
		return 0;
	}
	LRESULT AnnotR90CC(WORD, WORD wID, HWND, BOOL&)
	{
		SetDrawingOperation(ImageOperationTypeRotateCounterClockwise);
		return 0;
	}
	LRESULT AnnotFH(WORD, WORD wID, HWND, BOOL&)
	{
		SetDrawingOperation(ImageOperationTypeFlipHorizontal);
		return 0;
	}
	LRESULT AnnotFV(WORD, WORD wID, HWND, BOOL&)
	{
		SetDrawingOperation(ImageOperationTypeFlipVertical);
		return 0;
	}
	LRESULT AnnotLayerR90(WORD, WORD wID, HWND, BOOL&)
	{
		ROTA_SetRotationOperation(ImageOperationTypeRotateClockwise);
		return 0;
	}
	LRESULT AnnotWriteWave(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotRollup(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotPublish(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotPause(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotFullscreen(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotPast(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotLayerR360(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotLayerR30(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotLayerR90CC(WORD, WORD wID, HWND, BOOL&)
	{
		ROTA_SetRotationOperation(ImageOperationTypeRotateCounterClockwise);
		return 0;
	}
	LRESULT AnnotLayerFH(WORD, WORD wID, HWND, BOOL&)
	{
		ROTA_SetRotationOperation(ImageOperationTypeFlipHorizontal);
		return 0;
	}
	LRESULT AnnotLayerFV(WORD, WORD wID, HWND, BOOL&)
	{
		ROTA_SetRotationOperation(ImageOperationTypeFlipVertical);
		return 0;
	}
	LRESULT AnnotTextR(WORD, WORD wID, HWND, BOOL&)
	{
		g_textStyle = style_r;
		return 0;
	}
	LRESULT AnnotTextF(WORD, WORD wID, HWND, BOOL&)
	{
		g_textStyle = style_f;
		return 0;
	}
	LRESULT AnnotTextT(WORD, WORD wID, HWND, BOOL&)
	{
		g_textStyle = style_c;
		return 0;
	}
	LRESULT AnnotTextB(WORD, WORD wID, HWND, BOOL&)
	{
		g_textStyle = style_b;
		return 0;
	}
	LRESULT AnnotLayerBigger(WORD, WORD wID, HWND, BOOL&)
	{
		ROTA_SetRotationOperation(ImageOperationTypeSizeBigger);
		return 0;
	}
	LRESULT AnnotLayerSmaller(WORD, WORD wID, HWND, BOOL&)
	{
		ROTA_SetRotationOperation(ImageOperationTypeSizeSmaller);
		return 0;
	}
	LRESULT AnnotZoomIn(WORD, WORD wID, HWND, BOOL&)
	{
		ZoomIn();
		return 0;
	}
	LRESULT AnnotZoomOut(WORD, WORD wID, HWND, BOOL&)
	{
		ZoomOut();
		return 0;
	}
	LRESULT AnnotZoomFitIn(WORD, WORD wID, HWND, BOOL&)
	{
		ZoomFull();
		return 0;
	}
	LRESULT AnnotZoom100(WORD, WORD wID, HWND, BOOL&)
	{
		float scale;
		m_images[m_currentIndex].Image->GetScale(&scale);
		m_currentZoom = scale;
		
		CalculatePanBoundary();
		PanImage(D2D1::Point2F(0, 0), true);
		return 0;
	}
	LRESULT AnnotUndo(WORD, WORD wID, HWND, BOOL&)
	{
		PAST_Deactivate();
		Undo();
		return 0;
	}
	LRESULT AnnotRedo(WORD, WORD wID, HWND, BOOL&)
	{
		PAST_Deactivate();
		Redo();
		return 0;
	}
	LRESULT AnnotPencil(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotOpenSVGFile(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotCrop(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotText(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotStopText(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotLayer(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotWriteOpacity(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotWritePunch(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotDeleteLayer(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotStopLayer(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotColorPicker(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotCopyCurrentImageDataToClipboard(WORD, WORD wID, HWND, BOOL&);
	LRESULT AnnotExit(WORD, WORD wID, HWND, BOOL&)
	{
		ifShutDown = true;
		::PostQuitMessage(0);
		return 0;
	}

	HRESULT OnRender();
	HRESULT OnSize(unsigned int width, unsigned int height);
	HRESULT OnMouseWheel(D2D1_POINT_2F mousePosition, short delta, int keys);
	HRESULT OnMouseEnter(D2D1_POINT_2F mousePosition);

	HRESULT UpdateUIFramework();

public:
	// Editor image
	std::vector<ImageItem> m_images;

	// Image information
	int						m_currentRangeStart;
	int						m_currentIndex;
	int						m_currentRangeEnd;
	float						m_currentZoom;
	float						m_maxSlideDistance;

	// 选中的图片、文字层的位置和大小信息
	int						m_currentLeft = 0;
	int						m_currentTop = 0;
	int						m_currentWidth = 0;
	int						m_currentHeight = 0;

	ComPtr<IImageOperation>	m_currentOperation;

	CString									m_layerPicFileName;

private:
	// Constants
	static const int BackgroundColor;
	static const int PreviousNextImageRangeCount;

	static const float ImageMargin;
	static const float KeyboardPanDistance;
	static const float PreviousNextImageMargin;
	static const float SlideAnimationDuration;
	static const float ZoomMinimum;
	static const float ZoomMaximum;
	static const float ZoomStep;
	static const float StrokeSizes[];

	// Direct2D rendering resources
	RenderingParameters							m_drawingObjects;
	ComPtr<ID2D1SolidColorBrush>			m_solidBrush;
	ComPtr<ID2D1StrokeStyle>					strokeStyleCustomOffsetZero;

	ComPtr<IDWriteTextFormat>				m_textFormat;
	ComPtr<IDWriteTextLayout>				m_textLayout;

	ComPtr<IDWriteTextFormat>				m_copyFrameDataFormat;
	ComPtr<IDWriteTextLayout>				m_copyFrameDataLayout;

	// Direct2D rendering parameters
	RenderingParameters							m_renderingParameters;

	D2D1_POINT_2F									m_currentPanPoint;
	D2D1_RECT_F										m_currentPanBoundary;

	D2D1_RECT_F										m_imageBoundaryRect;
	D2D1_RECT_F										m_currentClipBoundary;
	D2D1_RECT_F										m_currentClipDrawBox;

	D2D1_COLOR_F									m_penColor;
	float														m_penSize;

	// Animation information
	bool															m_animationEnabled;
	bool															m_switchingImages;
	ComPtr<IUIAnimationManager>				m_animationManager;
	ComPtr<IUIAnimationTransitionLibrary>	m_transitionLibrary;
	ComPtr<IUIAnimationVariable>				m_transformationAnimationVariable;

	// Mouse information
	D2D1_POINT_2F									m_mouseDownPosition;
	D2D1_POINT_2F									m_previousMousePosition;

	// Drawing operations
	ImageOperationType							m_currentDrawingOperationType;
	ImageOperationType							m_prevDrawingOperationType;

	// Control whether the pic/text rotation animation should go on and on

	bool		m_isDrawing;
	bool		m_isPasting;

	bool		m_isTexting;
	bool		m_isLayering;

	bool		m_isClipping;
	bool		m_startClipping;
	bool		m_isCopying;
	bool		m_isRotation;
	bool		m_isFlip;
	bool		m_isMouseCursorInWindow = false;

	//--------------------------------------------------------------------------------------------------------------------------
	// Text data
	//--------------------------------------------------------------------------------------------------------------------------
	FLOAT									m_charSpace;
	FLOAT									m_lineSpace;
	FLOAT									m_charSize;
	D2D1_COLOR_F					m_textColor;

	//--------------------------------------------------------------------------------------------------------------------------
	// Layer data
	//--------------------------------------------------------------------------------------------------------------------------
	ComPtr<ID2D1Bitmap>		m_layerBitmap;
	bool										m_ifLayerFromPasting = false;

	//--------------------------------------------------------------------------------------------------------------------------
	// Pics data
	//--------------------------------------------------------------------------------------------------------------------------
	CString									m_picsFirstPicFileName;

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	CString									m_svgFileName;

	//--------------------------------------------------------------------------------------------------------------------------
	// Paste drawing data
	//--------------------------------------------------------------------------------------------------------------------------
	ComPtr<IWICBitmapSource>	m_pasteWicBitmapFromClipboard;

	//--------------------------------------------------------------------------------------------------------------------------
	// Copy frame drawing data
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1::Matrix3x2F	m_copyMatrix;
	D2D1_POINT_2F	m_copyFramePoint;
	D2D1_POINT_2F	m_copyStartPoint;
	D2D1_POINT_2F	m_copyFinalPoint;
	FLOAT					m_copyBorderWidth;
	FLOAT					m_copyBorderHeight;

public:

	//--------------------------------------------------------------------------------------------------------------------------
	// Direct2D resources
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT ImageEditor_CreateStrokAndTextFormat();
	HRESULT DiscardDeviceResources();
	HRESULT ManageImageResources();

	//--------------------------------------------------------------------------------------------------------------------------
	// Calculate methods
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT CalculatePanBoundary();
	HRESULT CalculateImagePositions();

	//--------------------------------------------------------------------------------------------------------------------------
	// Scroll/Zoom operations
	//--------------------------------------------------------------------------------------------------------------------------
	bool PreviousImage();
	bool NextImage();
	void PanImage(D2D1_POINT_2F offset, bool snapToBounds);

	//--------------------------------------------------------------------------------------------------------------------------
	// Draw operations
	//--------------------------------------------------------------------------------------------------------------------------
	void DrawClientArea();
	void InvalidateWindow();

	//--------------------------------------------------------------------------------------------------------------------------
	// Animation methods
	//--------------------------------------------------------------------------------------------------------------------------
	void SetupAnimation();
	void SetupTransformationAnimation();
	void CleanupAnimation();

	//--------------------------------------------------------------------------------------------------------------------------
	// Load/Save operations
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT Reset();
	HRESULT LoadShellItems(const std::vector<ComPtr<IShellItem> >* shellItems, IShellItem* currentItem);

	//--------------------------------------------------------------------------------------------------------------------------
	// Save methods
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT SaveFileAtIndex(int index);
	void ShowSaveFailure(int imageIndex);

	//--------------------------------------------------------------------------------------------------------------------------
	// Draw methods
	//--------------------------------------------------------------------------------------------------------------------------
	void DrawAnimatedImages(int imageIndex);
	void DrawImages(int imageIndex);

	ImageOperationType GetOperationType();

	D2D1_POINT_2F GetAbsolutePosition(D2D1_POINT_2F mousePosition, bool withBorder = true);
	D2D1_POINT_2F RemoveRenderingTransformations(D2D1_POINT_2F mousePosition);

	inline D2D1_POINT_2F GetCenter()
	{
		return D2D1::Point2F(
			g_logicalSize.width / 2,
			g_logicalSize.height / 2);
	}

	void ImageEditorHandler::UpdateMouseCursor(D2D1_POINT_2F mousePosition);
	bool IsImageHit(D2D1_POINT_2F mousePoint);
	D2D1_POINT_2F AdjustToClipRect(D2D1_POINT_2F absPoint);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	void	TEXT_CreateTextItem(D2D1_POINT_2F mousePosition);
	bool TEXT_IsTextHit(D2D1_POINT_2F mousePosition);
	bool TEXT_IsActive(ComPtr<IImageOperation> operation);
	void	TEXT_HitTextBorderTest(ComPtr<IImageTextOperation> textOperation, D2D1_POINT_2F mousePosition, bool& lefttop, bool&edit, bool&right, bool&left, bool&bottom);
	void	TEXT_AdjustingCenterOrBorder(D2D1_POINT_2F mousePosition);
	void	TEXT_SetCursor(bool center, bool edit, bool right, bool left, bool bottom);
	void	TEXT_StartTracking(D2D1_POINT_2F mousePosition);
	void	TEXT_CreateNewText(D2D1_POINT_2F mousePosition);
	void	TEXT_TrackFirstWord(CBTextPtr& bp, int i);
	void	TEXT_TrackFollowingWord(CBTextPtr& bp, int i);
	void	TEXT_UpdateFont(CHARFORMAT2& cf);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	void	PLYR_CreateLayerItem(D2D1_POINT_2F mousePosition, bool ifLocateToCenter = false);
	bool PLYR_IsLayerHit(D2D1_POINT_2F mousePosition);
	bool PLYR_IsActive(ComPtr<IImageOperation> operation);
	bool PLYR_HitLayerBorderTest(ComPtr<IImageLayerOperation> layer, D2D1_POINT_2F mousePosition);
	void	PLYR_SetCursor(bool center);
	void	PLYR_StartTracking(D2D1_POINT_2F mousePosition);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	void PICS_CreatePicsItem(FLOAT picX, FLOAT picY);
	void SVG_CreateSVGItem();

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	void PAST_CalculatePastingMatrix(CBPastingPtr& bp, ComPtr<IImage> image);
	void PAST_Deactivate();

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	bool ROTA_GetRelativeAndMousePt(ComPtr<IImageTextOperation> textOperation, CBRotatablePtr& bp, D2D1_POINT_2F mousePosition, D2D1_POINT_2F * relative, D2D1_POINT_2F * mouse);
	bool ROTA_GetRelativeAndMousePt(ComPtr<IImageLayerOperation> layerOperation, CBRotatablePtr& bp, D2D1_POINT_2F mousePosition, D2D1_POINT_2F * relative, D2D1_POINT_2F * mouse);
	void	ROTA_ShowPosition(CBRotatablePtr&bp);

	void						CLBD_CopyToClipboard(D2D1_POINT_2F copyFinalPoint, FLOAT copyBorderWidth, FLOAT copyBorderHeight);
	void						CLBD_CopyToClipboard(CString m_strResult);
	bool						CLBD_IfAvailablePasteText();
	bool						CLBD_IfAvailableCopyText();
	WCHAR *				CLBD_GetTextFromClipboard();
	D2D1_POINT_2F	CLBD_GetCopyPoint(__in D2D1_POINT_2F mousePosition);
	ComPtr<IWICBitmapSource> CLBD_GetWicBitmapFromClipboard();

	ComPtr<IWICBitmapSource> makeAWindow(ComPtr<IWICBitmapSource> source);
	
	void ANIM_Write(write_style wStyle);

public:
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	void PAST_SelectLayer(UINT index);
	void PAST_SelectLayer(ComPtr<IImageOperation> thisOper);
	void PAST_UnselectLayer();
	void PAST_SetPastOpacity(LONG lVal);
	void PAST_SetifLayerFromPasting(bool ifFromPasting) { m_ifLayerFromPasting = ifFromPasting; }
	
	void ANIM_SetDurationTime(LONG IVal);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	void		TEXT_SetCharSpace(LONG val);
	LONG	TEXT_GetCharSpace() { return (LONG)m_charSpace; }
	void		TEXT_SetLineSpace(LONG val);
	LONG	TEXT_GetLineSpace() { return (LONG)m_lineSpace; }
	void		TEXT_SetCharSize(LONG val);
	LONG	TEXT_GetCharSize() { return (LONG)m_charSize; }

	bool	CLBD_GetWicInfoFromClipboard();
	void OnMaximize();
	void OffMaximize();
};