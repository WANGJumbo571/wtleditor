// WTLApp.cpp : main source file for WTLApp.exe
//

#include "stdafx.h"
#include "included.h"

#include "sys/stat.h"

#include <algorithm>
#include <atlmisc.h>
#include <time.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlribbon.h>

#include "resource.h"

#include "mainfrm.h"
#include "tinyxml2.h"

using namespace tinyxml2;

CAppModule			_Module;
HINSTANCE				g_hInstance;
bool							g_ifShowCursor = false;
text_style					g_textStyle = style_f;
ImageRenderer*		g_render;

extern WCHAR			g_textFormatFamilyName[];
extern FLOAT			g_textFormatFontSize;

#ifdef _DEBUG
DEBUG_OUT DEBUGOUT;
#endif

int Run()
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	g_render = new ImageRenderer();
	g_render->CreateResources();

	CMainFrame wndMain;

	int frameWidth = PEDITOR_WIDTH + 2 * FRAME_PX;
	int frameHeight = RIBBON_PY + PEDITOR_HEIGHT + STATUS_PY + FRAME_PY;
	int startx = SCREEN_WIDTH / 2 - frameWidth / 2;
	int starty = (SCREEN_HEIGHT - TASKBAR_HEIGHT) / 2 - frameHeight / 2;
	if (startx < 0)		startx = 0;
	if (starty < 0)		starty = 0;
	RECT rect = { startx, starty, startx + frameWidth, starty + frameHeight };

	if(wndMain.CreateEx(NULL, &rect) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(SW_HIDE);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpstrCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;

	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("defaults.xml") != XML_SUCCESS)
	{
		AtlMessageBox(NULL, L"Error in reading defaults.xml.", IDR_MAINFRAME);
		return 0;
	}

	XMLElement* durationEle = doc.FirstChildElement("document")->FirstChildElement("Duration");
	g_animDuration = (FLOAT)durationEle->IntAttribute("duration", 20);

	XMLElement* element = doc.FirstChildElement("document")->FirstChildElement("FontFamily");
	SecureHelper::strcpy_x(g_textFormatFamilyName, LF_FACESIZE, utf8ToUnicode(element->GetText()));
	g_textFormatFontSize = (FLOAT)element->IntAttribute("size", 30);

	int nRet = 0;
	if (RunTimeHelper::IsRibbonUIAvailable())
	{
		nRet = Run();
	}
	else
	{
		AtlMessageBox(NULL, L"Cannot run with this version of Windows", IDR_MAINFRAME);
	}

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

// 应用程序入口
int WINAPI WinMain3(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	DEBUGOUT_ON;
	DEBUGOUT_OUTW(L"Battle Control  Online! \n");
	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			ThisApp app;
			if (SUCCEEDED(app.Initialize(hInstance, nCmdShow)))
			{
				app.RunMessageLoop();
			}
		}
		CoUninitialize();
	}
	DEBUGOUT_OUTW(L"Battle Control  Terminated! \n");
	DEBUGOUT_OFF;
	return 0;
}

BOOL ImageFromIDResource(UINT nID, LPCTSTR sTR, Gdiplus::Image*& pImg)
{
	HRSRC hRsrc = ::FindResource(NULL, MAKEINTRESOURCE(nID), sTR);
	if (!hRsrc)
	{
		DWORD error = GetLastError();
		return FALSE;
	}
	// load resource into memory
	//DWORD len = ::SizeofResource(GetModuleHandle(NULL), hRsrc);
	DWORD len = ::SizeofResource(NULL, hRsrc);
	//BYTE* lpRsrc = (BYTE*)LoadResource(NULL, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(NULL, hRsrc);
	if (!lpRsrc)
		return FALSE;
	// Allocate global memory on which to create stream
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem, lpRsrc, len);
	IStream* pstm;
	CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);
	// load from stream
	pImg = Gdiplus::Image::FromStream(pstm);
	// free/release stuff
	GlobalUnlock(m_hMem);
	pstm->Release();
	FreeResource(lpRsrc);
	return TRUE;
}
