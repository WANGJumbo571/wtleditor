#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlstr.h>
#include <WinInet.h>
#include <xstring>

extern CAppModule _Module;

#include <UIAnimation.h>
#include <commctrl.h>

#include <ShellAPI.h>
#include <ShlObj.h>
#include <StructuredQuery.h>
#include <PropKey.h>

#define WIN32_LEAN_AND_MEAN     
#define FIXED_DPI (96.f)

template<class Interface>
inline void SafeRelease(Interface*& pInterfaceToRelease) {
	if (pInterfaceToRelease != nullptr) {
		pInterfaceToRelease->Release();
		pInterfaceToRelease = nullptr;
	}
}

template <typename Interface>
inline Interface* SafeAcquire(Interface* newObject)
{
	if (newObject != nullptr)
		((IUnknown*)newObject)->AddRef();

	return newObject;
}

inline void SafeCloseHandle(HANDLE& handle) {
	if (handle) {
		::CloseHandle(handle);
		handle = nullptr;
	}
}

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <thread>
#include <cwchar>

#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXMath.h>

#include <d2d1effectauthor_1.h>
#include <d2d1effecthelpers.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "dwrite.lib" )
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib" )
#pragma comment(lib, "windowscodecs.lib" )

#include <vector>

// Useful macros
#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#ifndef IDC_PEN
#define IDC_PEN MAKEINTRESOURCE(32631)
#endif

// Common constants
const double PI = 3.14159265358979323846;


#define WNDWIDTH 1024
#define WNDHEIGHT 768

#define lengthof(a) (sizeof(a)/sizeof(*(a)))

#include "ComPtr.h"
#include "sharedobject.h"
#include "comhelpers.h"
#include "AnimationUtility.h"
#include "Direct2DUtility.h"
#include "ImageRenderer.h"
#include "ImageEditor.h"
