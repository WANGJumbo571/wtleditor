#include "stdafx.h"
#include "mainfrm.h"
#include "gdiplus.h"
#include "ImageEditor.h"
#include "ImageRenderer.h"
#include "aboutdlg.h"
#include "combdlg.h"

CMainFrame * g_frameWork;
int callingTime = 0;

extern text_style g_textStyle;
bool g_bFullScreen = false;

BOOL CMainFrame::OnIdle()
{
	static int first = 1;

	if (first == 1)
	{
		ShowWindow(SW_SHOW);
		first++;
	}
	else if (first == 2)
	{
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, exstyle);
		first++;
	}

	imageEditor.UpdateUIFramework();

	COLORREF color = m_colorForeground.m_color;
	imageEditor.SetPenColor(D2D1::ColorF(static_cast<float>(GetRValue(color)) / 255.0f,
																static_cast<float>(GetGValue(color)) / 255.0f,
																static_cast<float>(GetBValue(color)) / 255.0f));

	UpdateLayout();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	g_frameWork = this;

	exstyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
	::SetWindowLong(m_hWnd, GWL_EXSTYLE, exstyle | WS_CLIPCHILDREN);

	m_colorForeground.SetColor(g_fontColor);
	
	m_layerOpacity.SetMax((LONG)100);
	m_layerOpacity.SetMin((LONG)0);
	m_durationTime.SetMax((LONG)1000);
	m_durationTime.SetMin((LONG)0);
	m_durationTime.SetVal((LONG)g_animDuration);

	CreateSimpleStatusBar();
	m_sbar.SubclassWindow(m_hWndStatusBar);
	int arrParts[] =
	{
		ID_DEFAULT_PANE,
		ID_CURRENT_LEFT,
		ID_CURRENT_TOP,
		ID_CURRENT_WIDTH,
		ID_CURRENT_HEIGHT
	};
	m_sbar.SetPanes(arrParts, sizeof(arrParts) / sizeof(int), false);

	RECT clientRect = { 0, 0, 0, 0 }; 
	m_hWndClient = imageEditor.Create(m_hWnd, clientRect, NULL, 
		WS_CHILD | WS_VISIBLE, NULL);
	
	::SetWindowPos(
							imageEditor.m_hWnd,
							0,
							0,
							0,
							PEDITOR_WIDTH,
							PEDITOR_HEIGHT,
							SWP_NOZORDER);
	::ShowWindow(imageEditor.m_hWnd, SW_SHOW);
	::UpdateWindow(imageEditor.m_hWnd);

	g_render->m_hwnd = imageEditor.m_hWnd;
	g_render->CreateSwapAndSetD2DContextTarget();
	
	imageEditor.ImageEditor_CreateStrokAndTextFormat();

	ComPtr<IShellItem> currentBrowseLocationItem;
	HRESULT hr = S_OK;
	reportError(174, (hr = ::SHCreateItemFromParsingName(L"K:\\Library\\military\\《background》\\theme #4\\",
																						nullptr,
																						IID_PPV_ARGS(&currentBrowseLocationItem))));
	reportError(175, (hr = imageEditor.SetCurrentLocation(currentBrowseLocationItem)));
	if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr)
	{
		::PostQuitMessage(0);
	}

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	ShowRibbonUI(true);

	CombDlg::COMB_EmptyList();
	return 0;
}

LRESULT CMainFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);
	rect.top += GetRibbonHeight();
	int cx = rect.right - rect.left;
	int cy = rect.bottom - rect.top - STATUS_PY;
	::SetWindowPos(
							imageEditor.m_hWnd,
							0,
							rect.left,
							rect.top,
							cx,
							cy,
							SWP_NOZORDER);
	::SetWindowPos(
							m_hWndStatusBar,
							0,
							rect.left,
							rect.top + cy,
							rect.right - rect.left,
							STATUS_PY,
							SWP_NOZORDER);
	return S_OK;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnMenuText(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetRibbonModes(UI_MAKEAPPMODE(1));
	return imageEditor.AnnotText(wNotifyCode, wID, hWndCtl, bHandled);
}

LRESULT CMainFrame::OnStopText(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::PostMessage(m_hWnd, WM_UPDATEROWCOL, 0, 0);

	SetRibbonModes(UI_MAKEAPPMODE(0));
	return imageEditor.AnnotStopText(wNotifyCode, wID, hWndCtl, bHandled);
}

LRESULT CMainFrame::OnMenuPen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetRibbonModes(UI_MAKEAPPMODE(2));
	return imageEditor.AnnotPencil(wNotifyCode, wID, hWndCtl, bHandled);
}

LRESULT CMainFrame::OnStopPen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::PostMessage(m_hWnd, WM_UPDATEROWCOL, 0, 0);

	SetRibbonModes(UI_MAKEAPPMODE(0));
	imageEditor.SetDrawingOperation(ImageOperationTypePen); 
	return 0;
}


LRESULT CMainFrame::OnMenuPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (imageEditor.CLBD_GetWicInfoFromClipboard() == false)
		return S_OK;
	SetRibbonModes(UI_MAKEAPPMODE(3));
	imageEditor.PAST_SetifLayerFromPasting(true);
	return imageEditor.AnnotLayer(wNotifyCode, wID, hWndCtl, bHandled);
}

LRESULT CMainFrame::OnMenuLayer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetRibbonModes(UI_MAKEAPPMODE(3));
	imageEditor.PAST_SetifLayerFromPasting(false);
	return imageEditor.AnnotLayer(wNotifyCode, wID, hWndCtl, bHandled);
}

LRESULT CMainFrame::OnStopLayer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::PostMessage(m_hWnd, WM_UPDATEROWCOL, 0, 0);

	SetRibbonModes(UI_MAKEAPPMODE(0));
	return imageEditor.AnnotStopLayer(wNotifyCode, wID, hWndCtl, bHandled);
}

extern bool g_fullScreen;

LRESULT CMainFrame::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (g_isDrawingRippleOn == true || g_isLayerAnimationOn == true || g_bFullScreen == true)
	{
		::PostMessage(imageEditor.m_hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
	}
	else {
		TrackRibbonMenu(ID_CONTEXTMAP, lParam);
	}
	return 0;
}

static CRect recBack;

void CMainFrame::OnMaximize()
{
	//::GetClientRect(imageEditor.m_hWnd, &recBack);
	//::SetWindowPos(
	//	imageEditor.m_hWnd,
	//	0,
	//	0,
	//	0,
	//	1920 + 2 * FRAME_PX,
	//	1080 + 3 * STATUS_PY + 2 * FRAME_PY,
	//	SWP_NOZORDER);
	//::UpdateWindow(imageEditor.m_hWnd);

	GetWindowPlacement(&prePlacement);

	m_FSRect.left = -FRAME_PX;
	m_FSRect.top = -(RIBBON_PY + STATUS_PY);
	m_FSRect.right = 1920 + FRAME_PX;
	m_FSRect.bottom = 1080 + 2 * STATUS_PY + FRAME_PY;
	MoveWindow(&m_FSRect, TRUE);
}

void CMainFrame::OffMaximize()
{
	//::SetWindowPos(
	//	imageEditor.m_hWnd,
	//	0,
	//	recBack.left,
	//	recBack.top,
	//	recBack.right - recBack.left,
	//	recBack.bottom - recBack.top,
	//	SWP_NOZORDER);
	//::UpdateWindow(imageEditor.m_hWnd);

	SetWindowPlacement(&prePlacement);
	g_bFullScreen = FALSE;
}

LRESULT CMainFrame::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

	lpMMI->ptMaxTrackSize.x = 2 * GetSystemMetrics(SM_CXSCREEN);
	lpMMI->ptMaxTrackSize.y = 2 * GetSystemMetrics(SM_CYSCREEN);
	return 0;
}
