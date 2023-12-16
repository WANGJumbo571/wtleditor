#pragma once

#include "stdafx.h"
#include "aboutdlg.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	RECT recClient;
	GetClientRect(&recClient);

	int left = (recClient.right - recClient.left - 200) / 2;
	CRect rect = { left, 20, left + 200, 20 + 200 };
	initialPost_ = new CPostView(m_hWnd, &rect, L"K:\\����\\vs2022\\wtleditor\\wtleditor\\res\\Finder.png");

	LOGFONT logFontBold;
	memset(&logFontBold, 0, sizeof(logFontBold));
	logFontBold.lfHeight = -MulDiv(14, GetDeviceCaps(GetDC(), LOGPIXELSY), 72);
	logFontBold.lfWeight = FW_BOLD;    // ����İ���(Ĭ��,FW_BOLDΪ�Ӵ�)
	logFontBold.lfCharSet = GB2312_CHARSET;   // �ַ���(Ĭ��)
	_swprintf(logFontBold.lfFaceName, L"%s", L"΢���ź�");
	fontBold.CreateFontIndirect(&logFontBold);

	LOGFONT logFontSmall;
	memset(&logFontSmall, 0, sizeof(logFontSmall));
	logFontSmall.lfHeight = -MulDiv(10, GetDeviceCaps(GetDC(), LOGPIXELSY), 72);
	logFontSmall.lfCharSet = GB2312_CHARSET;   // �ַ���(Ĭ��)
	_swprintf(logFontSmall.lfFaceName, L"%s", L"΢���ź�");
	fontSmall.CreateFontIndirect(&logFontSmall);

	return TRUE;
}
