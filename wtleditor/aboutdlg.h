// aboutdlg.h : interface of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <atlctrls.h>
#include "postview.h"
#include <atlctrls.h>
#include <ShlObj.h>
#include <ShlGuid.h>
#include "ShellMgr.h"
#include "imageRenderer.h"
#include "ImageEditor.h"

extern BOOL ImageFromIDResource(UINT nID, LPCTSTR sTR, Gdiplus::Image * & pImg);

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };
	
	CPostView * initialPost_ = NULL;

	int		titlePic_width;
	int		titlePic_height;

	CFont fontBold;
	CFont fontSmall;

	CEdit infoEdit;

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	#define ABOUT_DISTANCE 6

	#define CALC_AND_DRAW(size, recta, rectb) \
		SIZE size; \
		GetTextExtentPoint32(current_dc.m_hDC, name.GetBuffer(256), name.GetLength(), &size); \
		RECT rectb = { (dialogWidth - size.cx) / 2, recta.bottom + ABOUT_DISTANCE, \
		(dialogWidth + size.cx) / 2, recta.bottom + ABOUT_DISTANCE + size.cy }; \
		current_dc.DrawTextW(name.GetBuffer(256), name.GetLength(), &rectb, DT_CENTER | DT_VCENTER);

	LRESULT OnPaint(UINT , WPARAM , LPARAM , BOOL& )
	{
		CPaintDC current_dc(m_hWnd);

		SelectObject(current_dc.m_hDC, fontBold.m_hFont);
		current_dc.SetTextColor(0x000000);
		current_dc.SetBkColor(0xf0f0f0);

		RECT recClient;
		GetClientRect(&recClient);
		int dialogWidth = recClient.right - recClient.left;

		initialPost_->PaintPostView(current_dc.m_hDC);
		titlePic_width = 200;
		titlePic_height = 200;

		CString name(L"wtleditor");
		SIZE name_size;
		GetTextExtentPoint32(current_dc.m_hDC, name.GetBuffer(256), name.GetLength(), &name_size);
		RECT rect00 = {	(dialogWidth - name_size.cx) / 2,	titlePic_height + 30,
								(dialogWidth + name_size.cx) / 2,	titlePic_height + 30 + name_size.cy };
		current_dc.DrawTextW(name.GetBuffer(256), name.GetLength(), &rect00, DT_CENTER | DT_VCENTER);

		SelectObject(current_dc.m_hDC, fontSmall.m_hFont);
		WCHAR buf[100];
		::LoadString(GetModuleHandle(NULL), IDR_VERSION, buf, 100);

		name = buf;
		CALC_AND_DRAW(name_size01, rect00, rect01);
		
		name = L"(C) 2023";
		CALC_AND_DRAW(name_size02, rect01, rect02);

		name = L"Windows Template Library";
		CALC_AND_DRAW(name_size03, rect02, rect03);

		return 0;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		extern bool g_bAboutDialogShow;
		g_bAboutDialogShow = false;

		ShowWindow(SW_HIDE);
		return 0;
	}
};

extern CAboutDlg *g_aboutDlg;
