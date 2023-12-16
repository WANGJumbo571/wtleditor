#include "stdafx.h"
#include "external.h"
//#include "mainfrm.h"
//#include "ImageEditor.h"
//extern CMainFrame* g_frameWork;

WCHAR* utf8ToUnicode(const char* zFilename)
{
	static WCHAR nullString[1];
	nullString[0] = L'\0';

	if (zFilename == nullptr)
		return nullString;

	int nChar;
	WCHAR* zWideFilename;

	nChar = MultiByteToWideChar(CP_UTF8, 0, zFilename, -1, NULL, 0);
	zWideFilename = static_cast<WCHAR*>(malloc((nChar + 1) * sizeof(zWideFilename[0])));
	if (zWideFilename == 0)
	{
		return nullString;
		//return 0;
	}
	nChar = MultiByteToWideChar(CP_UTF8, 0, zFilename, -1, zWideFilename, nChar);
	if (nChar == 0)
	{
		free(zWideFilename);
		zWideFilename = 0;
		return nullString;
	}
	zWideFilename[nChar] = 0;
	return zWideFilename;
}

char* unicodeToUtf8(const WCHAR* zWideFilename)
{
	int nByte;
	char* zFilename;

	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, 0, 0, 0, 0);
	zFilename = static_cast<char*>(malloc(nByte + 1));
	if (zFilename == 0)
	{
		return 0;
	}
	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, zFilename, nByte, 0, 0);
	if (nByte == 0)
	{
		free(zFilename);
		zFilename = 0;
	}
	zFilename[nByte] = 0;
	return zFilename;
}

void reportError(int location, HRESULT hr)
{
	if (hr != S_OK)
	{
		CString message;
		message.Format(L"Error: Location %d Error number: 0x%x", location, hr);
		::MessageBox(NULL, message.GetBuffer(100), L"ERROR", MB_OK);
	}
}

byte* FReadData(std::string filename, LONG* len)
{
	*len = 0;

	FILE* file = fopen(filename.c_str(), "rb");
	if (file == NULL)
	{
		CString message;
		message.Format(L"Error: Can't open file %s", utf8ToUnicode(filename.c_str()));
		::MessageBox(NULL, message.GetBuffer(100), L"ERROR", MB_OK);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	LONG length = ftell(file);
	fseek(file, 0, SEEK_SET);

	byte* fileData = new byte[length];

	fread(fileData, 1, length, file);

	*len = length;
	return fileData;
}

BOOL BitmapFromIDResource(UINT nID, LPCTSTR sTR, Gdiplus::Bitmap*& pBitmap)
{
	//HINSTANCE hInst = theApp.m_hInstance;
	//HMODULE ghmodule = GetModuleHandle(L"launcher.exe");
	HRSRC hRsrc = ::FindResource(g_hInstance, MAKEINTRESOURCE(nID), sTR);
	if (!hRsrc)
	{
		DWORD error = GetLastError();
		return FALSE;
	}
	// load resource into memory
	//DWORD len = ::SizeofResource(GetModuleHandle(NULL), hRsrc);
	DWORD len = ::SizeofResource(g_hInstance, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(g_hInstance, hRsrc);
	if (!lpRsrc)
		return FALSE;
	// Allocate global memory on which to create stream
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem, lpRsrc, len);
	IStream* pstm;
	CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);
	// load from stream
	pBitmap = Gdiplus::Bitmap::FromStream(pstm);
	// free/release stuff
	GlobalUnlock(m_hMem);
	pstm->Release();
	FreeResource(lpRsrc);
	return TRUE;
}
