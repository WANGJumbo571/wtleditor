#include "stdafx.h"
#include "resource.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "ImageEditor.h"
#include "resizedlg.h"

int g_resize_width = 1920;
int g_resize_height = 1080;

static WNDPROC OldWndProc = NULL;
static CResizeDlg * resizeDlg_ptr = NULL;

#define SET_UPDOWN_1_VALUE(value) \
	resizeDlg_ptr->upDownQuality1_.SetPos(value); \
	resizeDlg_ptr->setQualityEditValue(resizeDlg_ptr->edit1_, value);

#define SET_UPDOWN_2_VALUE(value) \
	resizeDlg_ptr->upDownQuality2_.SetPos(value); \
	resizeDlg_ptr->setQualityEditValue(resizeDlg_ptr->edit2_, value);

#define CHECK_WIDTH() \
	if (d1 < 1)	d1 = 1; \
	if (d1 > resizeDlg_ptr->max_width_) d1 = resizeDlg_ptr->max_width_;

#define CHECK_HEIGHT() \
	if (d2 < 1)	d2 = 1; \
	if (d2 > resizeDlg_ptr->max_height_) d2 = resizeDlg_ptr->max_height_;

// 该对话框新的窗口回调函数，过滤WM_KEYDOWN消息。
static LRESULT CALLBACK NewResizeEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_GETDLGCODE)
	{
		return (DLGC_WANTALLKEYS | CallWindowProc(OldWndProc, hWnd, message, wParam, lParam)); 
	}

	if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
	{
		if (wParam == VK_RETURN)
		{
			if (resizeDlg_ptr->bLinked_ == true)
			{
				int d1 = 0; 
				int d2 = 0;
				bool setEditsAndUpdowns = false;

				if (hWnd == resizeDlg_ptr->edit1_.m_hWnd)
				{
					d1 = resizeDlg_ptr->getQualityValue(resizeDlg_ptr->edit1_);
					d2 = (int)(((double)(d1)) / resizeDlg_ptr->width_ * resizeDlg_ptr->height_);
					CHECK_WIDTH();
					CHECK_HEIGHT();
					setEditsAndUpdowns = true;
				}
				else if (hWnd == resizeDlg_ptr->edit2_.m_hWnd)
				{
					d2 = resizeDlg_ptr->getQualityValue(resizeDlg_ptr->edit2_);
					d1 = (int)(((double)(d2)) / resizeDlg_ptr->height_ * resizeDlg_ptr->width_);
					CHECK_WIDTH();
					CHECK_HEIGHT();
					setEditsAndUpdowns = true;
				}

				if (setEditsAndUpdowns)
				{
					SET_UPDOWN_1_VALUE(d1);
					SET_UPDOWN_2_VALUE(d2);
				}
			}
			else
			{
				if (hWnd == resizeDlg_ptr->edit1_.m_hWnd)
				{
					int d1 = resizeDlg_ptr->getQualityValue(resizeDlg_ptr->edit1_);
					CHECK_WIDTH();
					SET_UPDOWN_1_VALUE(d1);
				}
				else if (hWnd == resizeDlg_ptr->edit2_.m_hWnd)
				{
					int d2 = resizeDlg_ptr->getQualityValue(resizeDlg_ptr->edit2_);
					CHECK_HEIGHT();
					SET_UPDOWN_2_VALUE(d2);
				}
			}
			return TRUE;
		}

		if ((wParam < '0' || wParam > '9') && wParam != VK_LEFT && 	wParam != VK_RIGHT &&
			wParam != VK_DELETE && wParam != VK_BACK)
			return TRUE;
	}
	return CallWindowProc(OldWndProc, hWnd, message, wParam, lParam);
}

void createFontAvailable(HDC hdc, LOGFONT & logfont, WCHAR * faceName, int height, BOOL ifBold)
{
	memset(&logfont, 0, sizeof(LOGFONT));
	logfont.lfHeight = -MulDiv(height, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	logfont.lfWidth = 0;    // 字体的宽度(默认)
	if (ifBold)
		logfont.lfWeight = FW_BOLD;    // 字体的磅数(默认,FW_BOLD为加粗)
	else
		logfont.lfWeight = 0;
	logfont.lfItalic = 0;    // 斜体
	logfont.lfUnderline = 0; // 下划线
	logfont.lfStrikeOut = 0; // 删除线
	logfont.lfCharSet = GB2312_CHARSET;   // 字符集(默认)
	_swprintf(logfont.lfFaceName, L"%s", faceName);
}


#define SAFE_DELETE(p) \
{ if (p != nullptr) delete(p); 	p = nullptr; }

CResizeDlg::~CResizeDlg()
{
	SAFE_DELETE(initialPost_);
	SAFE_DELETE(resizePost_);
	SAFE_DELETE(linkedPost_);
	SAFE_DELETE(notlinkedPost_);
}

int CResizeDlg::GetFinalWidth()
{
	int w = getQualityValue(edit1_);
	if (w < 1) w = 1;
	if (w > max_width_) w = max_width_;
	return w;
}

int CResizeDlg::GetFinalHeight()
{
	int h = getQualityValue(edit2_);
	if (h < 1)	h = 1;
	if (h > max_height_) h = max_height_;
	return h;
}

void 	CResizeDlg::GetDlgRect(int id, CRect * lprect)
{
	CWindow title = GetDlgItem(id);
	title.GetWindowRect(lprect);
	ScreenToClient(lprect);
}

void CResizeDlg::setInitialSize(int width, int height)
{
	width_ = width;
	height_ = height;

	max_width_ = width_ * 3;
	max_height_ = height_ * 3;
}

LRESULT CResizeDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	CRect rec;

	//resizePost_ is not transparent
	GetDlgRect(IDC_STATIC_9, &rec);
	resizePost_		= new CPostView(m_hWnd, &rec, bp.title1PicFileName.GetBuffer(100));

	//both linkedPost_ and notlinkedPost_ are not transparent
	GetDlgRect(IDC_STATIC_10, &rec);
	linkedPost_		= new CPostView(m_hWnd, &rec, IDR_MYPNG_RESIZE_LINKED);
	notlinkedPost_	= new CPostView(m_hWnd, &rec, IDR_MYPNG_RESIZE_UNLINKED);

	GetDlgRect(IDC_STATIC_8, &rec);
	if (bp.ifHasTitle2Pic)
	{
		if (bp.ifTitle2PicFromFile)
		{
			initialPost_ = new CPostView(m_hWnd, &rec, bp.title2PicFileName.GetBuffer(100));
		}
		else
		{
			initialPost_ = new CPostView(m_hWnd, &rec, bp.title2PicWicBitmap);
		}
	}
	
	LOGFONT logFont;

	createFontAvailable(GetDC(), logFont, L"微软雅黑", 10, TRUE);
	font10b_.CreateFontIndirect(&logFont);

	createFontAvailable(GetDC(), logFont, L"微软雅黑", 8, FALSE);
	font8_.CreateFontIndirect(&logFont);

	GetDlgRect(IDC_STATIC_5, &rec);
	edit1_.Create(m_hWnd, CRect(CPoint(rec.left, rec.top), CSize(60, 20)), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		0, ContentEditupDown1);

	//UDS_SETBUDDYINT: 自动设置Edit的整型值
	upDownQuality1_.Create(m_hWnd, CRect(CPoint(), CSize()), NULL, WS_CHILDWINDOW | WS_BORDER | WS_VISIBLE
		| UDS_AUTOBUDDY | UDS_ALIGNRIGHT
		| UDS_ARROWKEYS | UDS_HOTTRACK, 0, ContentQualityUpDown1);
	upDownQuality1_.SetRange(1, max_width_);

	upDownQuality1_.SetPos(width_);
	setQualityEditValue(edit1_, width_);

	edit2_.Create(m_hWnd, CRect(CPoint(rec.left, rec.top+28), CSize(60, 20)), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		0, ContentEditupDown2);

	//UDS_SETBUDDYINT: 自动设置Edit的整型值
	upDownQuality2_.Create(m_hWnd, CRect(CPoint(), CSize()), NULL, WS_CHILDWINDOW | WS_BORDER | WS_VISIBLE
		| UDS_AUTOBUDDY | UDS_ALIGNRIGHT
		| UDS_ARROWKEYS | UDS_HOTTRACK, 0, ContentQualityUpDown2);
	upDownQuality2_.SetRange(1, max_height_);

	upDownQuality2_.SetPos(height_);
	setQualityEditValue(edit2_, height_);

	resizeDlg_ptr = this;

	//edit1_.SetFont(AtlGetStockFont(DEFAULT_GUI_FONT));
	OldWndProc = (WNDPROC)edit1_.SetWindowLongPtr(GWLP_WNDPROC, (LONG_PTR)NewResizeEditProc);
	OldWndProc = (WNDPROC)edit2_.SetWindowLongPtr(GWLP_WNDPROC, (LONG_PTR)NewResizeEditProc);

	return TRUE;
}

LRESULT CResizeDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC pdc(m_hWnd);
	HDC hdc = pdc.m_hDC;

	SetTextColor(hdc, 0x000000);
	SetBkColor(hdc, 0xf0f0f0);

	SelectObject(hdc, font10b_.m_hFont);

	CRect rec;

	GetDlgRect(IDC_STATIC_1, &rec);
	CString name = bp.titleName;
	DrawTextW(hdc, name.GetBuffer(256), name.GetLength(), &rec, DT_LEFT | DT_TOP);

	GetDlgRect(IDC_STATIC_3, &rec);
	name = L"图像大小";
	DrawTextW(hdc, name.GetBuffer(256), name.GetLength(), &rec, DT_LEFT | DT_TOP);

	SelectObject(hdc, font8_.m_hFont);

	name = L"文件名：【未命名的新创建图像】";
	GetDlgRect(IDC_STATIC_2, &rec);
	DrawTextW(hdc, name.GetBuffer(256), name.GetLength(), &rec, DT_LEFT | DT_TOP);

	Gdiplus::Color regionBorderColor(200, 0x77, 0x77, 0x77);
	Gdiplus::Pen pen(regionBorderColor);
	Gdiplus::Graphics renderer(hdc);
	Point pt1 = { 0, rec.bottom + 1 };
	Point pt2 = { 600, rec.bottom + 1 };
	renderer.DrawLine(&pen, pt1, pt2);
	
	SetWindowText(bp.dialogName.GetBuffer(100));

	if (bp.ifHasTitle2Pic)
	{
		initialPost_->PaintPostView(hdc);
	}

	resizePost_->PaintPostView(hdc);

	if (bLinked_)
		linkedPost_->PaintPostView(hdc);
	else
		notlinkedPost_->PaintPostView(hdc);

	return 0;
}

LRESULT CResizeDlg::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	int xPos = (short)LOWORD(lParam);// 鼠标位置X
	int yPos = (short)HIWORD(lParam);// 鼠标位置Y

	CRect rec;
	GetDlgRect(IDC_STATIC_10, &rec);
	if (xPos > rec.left && xPos < rec.right && yPos > rec.top && yPos < rec.bottom)
	{
		bLinked_ = !bLinked_;

		HDC hdc = GetDC();
		if (bLinked_)
			linkedPost_->PaintPostView(hdc);
		else
			notlinkedPost_->PaintPostView(hdc);
	}
	return 0;
}

LRESULT CResizeDlg::OnReset(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	setQualityEditValue(edit1_, width_);
	upDownQuality1_.SetPos(width_);

	setQualityEditValue(edit2_, height_);
	upDownQuality2_.SetPos(height_);
	return 0;
}

LRESULT CResizeDlg::OnDefault(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (max(width_, height_) <= 2000)
		return 0;

	if (width_ >= height_)
	{
		FLOAT scale = width_ / 2000.0f;
		setQualityEditValue(edit1_, 2000);
		upDownQuality1_.SetPos(2000);

		setQualityEditValue(edit2_, (int)(height_ / scale));
		upDownQuality2_.SetPos((int)(height_ / scale));
	}
	else
	{
		FLOAT scale = height_ / 2000.0f;
		setQualityEditValue(edit1_, (int)(width_ / scale));
		upDownQuality1_.SetPos((int)(width_ / scale));

		setQualityEditValue(edit2_, 2000);
		upDownQuality2_.SetPos(2000);
	}
	return 0;
}

LRESULT CResizeDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	g_resize_width = GetFinalWidth();
	g_resize_height = GetFinalHeight();

	EndDialog(wID);
	return 0;
}

#define BUFLEN 70

int CResizeDlg::getQualityValue(CEdit &edit)
{
	wchar_t buf[BUFLEN] = { 0 };
	edit.GetWindowText(buf, BUFLEN-1);

	for (int i = 0; i < BUFLEN; i++)
	if (buf[i] != '\0' && (buf[i] < '0' || buf[i] > '9')) buf[i] = '0';
	buf[BUFLEN - 1] = '\0';
	
	return _wtoi(buf);
}

void CResizeDlg::setQualityEditValue(CEdit &edit, int value)
{
	wchar_t buf[BUFLEN] = { 0 };
	wsprintf(buf, L"%d", value);
	edit.SetWindowText(buf);
}

LRESULT CResizeDlg::onUpDownChange1(LPNMHDR pnmh)
{
	auto upDownData = (LPNMUPDOWN)pnmh;
	setQualityEditValue(edit1_, upDownData->iPos);
	if (bLinked_) 
	{
		int d1 = upDownData->iPos;
		int d2 = (int)(((double)d1) / width_ * height_);
		CHECK_HEIGHT();
		upDownQuality2_.SetPos(d2);
		setQualityEditValue(edit2_, d2);
	}
	return 0;
}

LRESULT CResizeDlg::onUpDownChange2(LPNMHDR pnmh)
{
	auto upDownData = (LPNMUPDOWN)pnmh;
	setQualityEditValue(edit2_, upDownData->iPos);
	if (bLinked_)
	{
		int d2 = upDownData->iPos;
		int d1 = (int)(((double)d2) / height_ * width_);
		CHECK_WIDTH();
		upDownQuality1_.SetPos(d1);
		setQualityEditValue(edit1_, d1);
	}
	return 0;
}

