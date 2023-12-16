/////////////////////////////////////////////////////////////////////////////
// mainfrm.h : interface of the CMainFrame class
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <ShlObj.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlribbon.h>
#include <atlmisc.h>

#include "Ribbon.h"
#include "resource.h"
#include "ImageEditor.h"
#include "ImageRenderer.h"

#define FRAME_PX				8
#define FRAME_PY				8
#define STATUS_PY				22
#define RIBBON_PY				166
#define PEDITOR_WIDTH		1300
#define PEDITOR_HEIGHT		731
#define SCREEN_WIDTH		1920
#define SCREEN_HEIGHT		1080
#define TASKBAR_HEIGHT		40

extern int callingTime;

class CMainFrame : 
	public CRibbonFrameWindowImpl<CMainFrame, ATL::CWindow>, 
	public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
	
	ImageEditorHandler				imageEditor;
	ImageRenderer						imageRenderer;

	CMultiPaneStatusBarCtrl		m_sbar;

	WINDOWPLACEMENT prePlacement;
	CRect m_FSRect;

	CRibbonColorCtrl<ID_STANDARD_COLORPICKER>			m_colorForeground;
	CRibbonFontCtrl<ID_RIBBON_FONT>								m_font;
	//ExtraCRibbonComboCtrl<ID_LAYERS, MAXLAYERS>			m_layers;
	CRibbonSpinnerCtrl<ID_PICOPACITY>								m_layerOpacity;
	CRibbonSpinnerCtrl<ID_DURATION>								m_durationTime;
	CRibbonSpinnerCtrl<ID_CHAR_SPACE>								m_charSpace;
	CRibbonSpinnerCtrl<ID_LINE_SPACE>								m_lineSpace;
	CRibbonSpinnerCtrl<ID_CHAR_SIZE>								m_charSize;

	BEGIN_RIBBON_CONTROL_MAP(CMainFrame)
		RIBBON_CONTROL(m_colorForeground)
		RIBBON_CONTROL(m_font)
		//RIBBON_CONTROL(m_layers)
		RIBBON_CONTROL(m_layerOpacity)
		RIBBON_CONTROL(m_durationTime)
		RIBBON_CONTROL(m_charSpace)
		RIBBON_CONTROL(m_lineSpace)
		RIBBON_CONTROL(m_charSize)
	END_RIBBON_CONTROL_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_UPDATEROWCOL, OnUpdateRowCol)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)

		COMMAND_ID_HANDLER(ID_WTL_MENU_TEXT, OnMenuText)
		COMMAND_ID_HANDLER(ID_WTL_STOP_TEXT, OnStopText)

		COMMAND_ID_HANDLER(ID_WTL_MENU_PEN, OnMenuPen)
		COMMAND_ID_HANDLER(ID_WTL_STOP_PEN, OnStopPen)

		COMMAND_ID_HANDLER(ID_WTL_MENU_PASTE, OnMenuPaste)
		COMMAND_ID_HANDLER(ID_WTL_MENU_LAYER, OnMenuLayer)
		COMMAND_ID_HANDLER(ID_WTL_STOP_LAYER, OnStopLayer)

		RIBBON_SPINNER_CONTROL_HANDLER(ID_PICOPACITY, OnSpinnerOpacity)
		RIBBON_SPINNER_CONTROL_HANDLER(ID_DURATION, OnSpinnerDuration)
		RIBBON_SPINNER_CONTROL_HANDLER(ID_CHAR_SPACE, OnSpinnerCharSpace)
		RIBBON_SPINNER_CONTROL_HANDLER(ID_LINE_SPACE, OnSpinnerLineSpace)
		RIBBON_SPINNER_CONTROL_HANDLER(ID_CHAR_SIZE, OnSpinnerCharSize)
		//RIBBON_COMBO_CONTROL_HANDLER(ID_LAYERS, OnPicLayerSelected)
		CHAIN_MSG_MAP(CRibbonFrameWindowImpl<CMainFrame>)
		CHAIN_COMMANDS_MEMBER((imageEditor))
	END_MSG_MAP()

	bool OnRibbonQueryFont(UINT nId, CHARFORMAT2& cf)
	{
		ZeroMemory(&cf, sizeof(CHARFORMAT2));
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE;
		cf.crTextColor = g_fontColor;  // Yellow in BGR order.
		cf.yHeight = g_textFormatFontSize * 20;
		SecureHelper::strcpy_x(cf.szFaceName, LF_FACESIZE, g_textFormatFamilyName);
		return true;
	}
	
	LPCWSTR OnRibbonQueryItemText(UINT32 uCtrlID, UINT32 uItem)
	{
		return L" ";
	}

	bool OnRibbonQuerySpinnerValue(UINT nCmdID, REFPROPERTYKEY key, LONG* pVal)
	{
		if (nCmdID == ID_CHAR_SPACE)
		{
			if (key == UI_PKEY_DecimalValue)
			{
				*pVal = (LONG)imageEditor.TEXT_GetCharSpace();
				return true;
			}
		}
		if (nCmdID == ID_LINE_SPACE)
		{
			if (key == UI_PKEY_DecimalValue)
			{
				*pVal = (LONG)imageEditor.TEXT_GetLineSpace();
				return true;
			}
		}
		if (nCmdID == ID_CHAR_SIZE)
		{
			if (key == UI_PKEY_DecimalValue)
			{
				*pVal = (LONG)imageEditor.TEXT_GetCharSize();
				return true;
			}
		}		
		return false;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CRibbonFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle();

public:

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void OnMaximize();
	void OffMaximize();

	LRESULT OnUpdateRowCol(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (wParam)
		{
			WCHAR szBuff[200];
			wsprintf(szBuff, L"Left: %d", imageEditor.m_currentLeft);
			m_sbar.SetPaneText(ID_CURRENT_LEFT, szBuff);
			wsprintf(szBuff, L"Top: %d", imageEditor.m_currentTop);
			m_sbar.SetPaneText(ID_CURRENT_TOP, szBuff);
			wsprintf(szBuff, L"Width: %d", imageEditor.m_currentWidth);
			m_sbar.SetPaneText(ID_CURRENT_WIDTH, szBuff);
			wsprintf(szBuff, L"Height: %d", imageEditor.m_currentHeight);
			m_sbar.SetPaneText(ID_CURRENT_HEIGHT, szBuff);
		}
		else
		{
			WCHAR * szBuff = L"";
			m_sbar.SetPaneText(ID_CURRENT_LEFT, szBuff);
			m_sbar.SetPaneText(ID_CURRENT_TOP, szBuff);
			m_sbar.SetPaneText(ID_CURRENT_WIDTH, szBuff);
			m_sbar.SetPaneText(ID_CURRENT_HEIGHT, szBuff);
		}
		return 0;
	}

	void COMB_SetSpinnerOpacity(LONG IVal)
	{
		m_layerOpacity.SetVal(IVal, true);
	}

	void COMB_SetSpinnerDuration(LONG IVal)
	{
		m_durationTime.SetVal(IVal, true);
	}

	LRESULT OnSpinnerOpacity(WORD /*wID*/, LONG lVal, BOOL& /*bHandled*/)
	{
		imageEditor.PAST_SetPastOpacity(lVal);
		imageEditor.Invalidate();
		return 0;
	}

	LRESULT OnSpinnerDuration(WORD /*wID*/, LONG lVal, BOOL& /*bHandled*/)
	{
		imageEditor.ANIM_SetDurationTime(lVal);
		imageEditor.Invalidate();
		return 0;
	}

	LRESULT OnSpinnerCharSpace(WORD /*wID*/, LONG lVal, BOOL& /*bHandled*/)
	{
		imageEditor.TEXT_SetCharSpace(lVal - 1);
		imageEditor.Invalidate();
		return 0;
	}

	LRESULT OnSpinnerLineSpace(WORD /*wID*/, LONG lVal, BOOL& /*bHandled*/)
	{
		imageEditor.TEXT_SetLineSpace(lVal - 1);
		imageEditor.Invalidate();
		return 0;
	}

	LRESULT OnSpinnerCharSize(WORD /*wID*/, LONG lVal, BOOL& /*bHandled*/)
	{
		imageEditor.TEXT_SetCharSize(lVal - 1);
		imageEditor.Invalidate();
		return 0;
	}
	
	LRESULT OnMenuText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStopText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnMenuPen(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStopPen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnMenuPaste(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMenuLayer(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStopLayer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnPicLayerSelected(WORD /*wNotifyCode*/, WORD /*wID*/, UINT, BOOL& /*bHandled*/);
	
private:
	LONG exstyle;
};

