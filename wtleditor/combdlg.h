#pragma once

#include <atlctrls.h>
#include "postview.h"
#include <atlctrls.h>
#include <ShlObj.h>
#include <ShlGuid.h>
#include <atlmisc.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlcrack.h>

#include <gdiplus.h> 
#include <gdiplusheaders.h>
#include <gdiplusbitmap.h>
#include <gdipluscachedbitmap.h>
#include <gdiplusimaging.h>

#include "ShellMgr.h"
#include "imageRenderer.h"
#include "ImageEditor.h"
#include "postview.h"
#include "external.h"
#include "csliderbar.h"
#include "mainfrm.h"

#define QUALITY_INITIAL_VALUE 0

typedef enum ViewWindowId3_ {
	ContentQualityUpDown3 = WM_USER + 1100,
	ContentEditupDown3
} ViewWindowId3;

class CombDlg : public CDialogImpl<CombDlg>
{
public:
	enum { IDD = IDD_COMBBOX };

	CPostView* initialPost_ = NULL;

	int		titlePic_width;
	int		titlePic_height;

	CSliderBar trackBar_;
	CEdit edit3_;
	CUpDownCtrl upDownQuality3_;

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		NOTIFY_CODE_HANDLER(NM_CLICK, OnLVItemClick)
		NOTIFY_HANDLER_EX(ContentQualityUpDown3, UDN_DELTAPOS, onUpDownChange3)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnLVItemClick(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void onTrackBarValueChange(int val);

	int GetFinalValue()
	{
		int v = getQualityValue3();
		if (v < 1) v = 1;
		if (v > 100) v = 100;
		return v;
	}

	void 	GetDlgRect(int id, CRect* lprect)
	{
		CWindow title = GetDlgItem(id);
		title.GetWindowRect(lprect);
		ScreenToClient(lprect);
	}

#define BUFLEN 70

	int getQualityValue3()
	{
		wchar_t buf[BUFLEN] = { 0 };
		edit3_.GetWindowText(buf, BUFLEN - 1);
		buf[4] = '\0';
		for (int i = 0; i < BUFLEN; i++)
			if (buf[i] != '\0' && (buf[i] < '0' || buf[i] > '9')) buf[i] = '0';
		buf[BUFLEN - 1] = '\0';
		return _wtoi(buf);
	}

	void setQualityEditValue3(int value)
	{
		wchar_t buf[BUFLEN] = { 0 };
		wsprintf(buf, L"%d", value);
		edit3_.SetWindowText(buf);
	}

	LRESULT onUpDownChange3(LPNMHDR pnmh)
	{
		auto upDownData = (LPNMUPDOWN)pnmh;
		setQualityEditValue3(upDownData->iPos);
		trackBar_.setValue((int)upDownData->iPos);
		return 0;
	}

	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&)
	{
		CPaintDC current_dc(m_hWnd);

		initialPost_->PaintPostView(current_dc.m_hDC);
		titlePic_width = 200;
		titlePic_height = 200;

		return 0;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		extern bool g_bCombDialogShow;
		g_bCombDialogShow = false;

		ShowWindow(SW_HIDE);
		return 0;
	}

	static int GetStopCharLoc(WCHAR* namebuf, int len, HDC hdc);
	static void OnPicLayerSelected(UINT layerId);

	static void COMB_InitList();
	static void COMB_EmptyList();
	static void COMB_EmptySpinners();
	static void COMB_SetSpinners(ComPtr<IImageOperation> item);
	static bool COMB_HasListMember();
	static void COMB_ActivateItem(ComPtr<IImageOperation> item);
	static void COMB_DeactivateItemWithFollower(ComPtr<IImageOperation> item);
	static void COMB_DeactivateEveryItem();
	static void COMB_SetItemActive(ComPtr<IImageOperation> item, bool active);
	static void	COMB_InsertListView(HBITMAP hbitmap, WCHAR* namebuf, int itemNum);
	static void	COMB_UpdateTextLayerString(ComPtr<IImageOperation> item);
	static HBITMAP	COMB_MakeHBITMAP(WCHAR* filename);
};

extern CombDlg* g_combDlg;