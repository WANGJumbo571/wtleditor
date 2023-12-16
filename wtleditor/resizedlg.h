/////////////////////////////////////////////////////////////////////////////
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

#include "postview.h"
#include "resource.h"

extern CString	g_strCurrentFileName;
extern int g_resize_width;
extern int g_resize_height;

#pragma once

typedef enum ViewWindowId1_ {
	ContentQualityUpDown1 = WM_USER + 1000,
	ContentEditupDown1
} ViewWindowId1;

typedef enum ViewWindowId2_ {
	ContentQualityUpDown2 = WM_USER + 1010,
	ContentEditupDown2
} ViewWindowId2;

class CBResize 
{
public:
	CString dialogName;
	bool ifHasTitle2Pic = false;
	bool ifTitle2PicFromFile = false;
	CString titleName;
	CString title1PicFileName;
	CString title2PicFileName;
	ComPtr<IWICBitmapSource> title2PicWicBitmap;
};

//CSimpleDlg<IDD_RESIZE_BOX, FALSE>
class CResizeDlg : public CDialogImpl<CResizeDlg> 
{
public:
	enum { IDD = IDD_RESIZE_BOX };

	BEGIN_MSG_MAP_EX(CResizeDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		COMMAND_ID_HANDLER(IDC_RESIZE_RESET, OnReset)
		COMMAND_ID_HANDLER(IDDefault, OnDefault)
		COMMAND_RANGE_HANDLER(IDOK, IDNO, OnCloseCmd)
		NOTIFY_HANDLER_EX(ContentQualityUpDown1, UDN_DELTAPOS, onUpDownChange1)
		NOTIFY_HANDLER_EX(ContentQualityUpDown2, UDN_DELTAPOS, onUpDownChange2)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	CPostView * initialPost_ = NULL;
	CPostView * resizePost_ = NULL;
	CPostView * linkedPost_ = NULL;
	CPostView * notlinkedPost_ = NULL;

	int width_ = 0;
	int height_ = 0;

	int max_width_ = 1;
	int max_height_ = 1;

	bool bLinked_ = true;

	CFont font10b_;
	CFont font8_;

	CEdit edit1_;
	CUpDownCtrl upDownQuality1_;

	CEdit edit2_;
	CUpDownCtrl upDownQuality2_;

	CBResize bp;

public:
	~CResizeDlg();

	int getQualityValue(CEdit &edit);
	void setQualityEditValue(CEdit & edit, int value);

	LRESULT onUpDownChange1(LPNMHDR pnmh);
	LRESULT onUpDownChange2(LPNMHDR pnmh);

	void onTrackBarValueChange(int val);

	int GetFinalWidth();
	int GetFinalHeight();

	void 	GetDlgRect(int id, CRect * lprect);

	void setInitialSize(int width, int height);

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);

	LRESULT OnReset(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDefault(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
