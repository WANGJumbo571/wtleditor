//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================

#include "stdafx.h"
#include "Direct2DUtility.h"

using namespace Hilo::Direct2DHelpers;

HRESULT Direct2DUtility::SaveBitmapToFile(IWICBitmapSource* updatedBitmap,	const wchar_t * newname)
{
	ComPtr<IWICStream> pStream;
	ComPtr<IWICBitmapEncoder> pEncoder;
	ComPtr<IWICBitmapFrameEncode> pFrameEncode;
	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;

	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		hr = g_pWICFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		hr = pStream->InitializeFromFilename(newname, GENERIC_WRITE);
	}
	if (SUCCEEDED(hr))
	{
		CString name = newname;
		if (name.Find(L".jpg") != -1 || name.Find(L".jpeg") != -1)
		{
			hr = g_pWICFactory->CreateEncoder(GUID_ContainerFormatJpeg, NULL, &pEncoder);
		}
		else
		{
			hr = g_pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
		}
	}
	if (SUCCEEDED(hr))
	{
		hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
	}
	if (SUCCEEDED(hr))
	{
		hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL);
	}
	// Use IWICBitmapFrameEncode to encode the bitmap into the picture format you want.
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->Initialize(NULL);
	}

	UINT sc_bitmapWidth;
	UINT sc_bitmapHeight;

	updatedBitmap->GetSize(&sc_bitmapWidth, &sc_bitmapHeight);

	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->SetSize(sc_bitmapWidth, sc_bitmapHeight);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->SetPixelFormat(&format);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->WriteSource(updatedBitmap, NULL);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->Commit();
	}
	if (SUCCEEDED(hr))
	{
		hr = pEncoder->Commit();
	}
	return hr;
}

// Creates a 32-bit DIB from the specified WIC bitmap.
HBITMAP Direct2DUtility::CreateHBITMAP(IWICBitmapSource * ipBitmapSource)
{
	HBITMAP hbmp = NULL;

	UINT width = 0;
	UINT height = 0;

	if (FAILED(ipBitmapSource->GetSize(&width, &height)) || width == 0 || height == 0)
		return hbmp;

	BITMAPINFO bminfo;

	ZeroMemory(&bminfo, sizeof(bminfo));
	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bminfo.bmiHeader.biWidth = width;
	bminfo.bmiHeader.biHeight = ((LONG)height);
	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 32;
	bminfo.bmiHeader.biCompression = BI_RGB;

	// create a DIB section that can hold the image
	void * pvImageBits = NULL;
	HDC hdcScreen = GetDC(NULL);
	hbmp = CreateDIBSection(hdcScreen, &bminfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
	ReleaseDC(NULL, hdcScreen);

	if (hbmp == NULL)
		return hbmp;

	// extract the image into the HBITMAP
	const UINT cbStride = width * 4;
	const UINT cbImage = cbStride * height;
	if (FAILED(ipBitmapSource->CopyPixels(NULL, cbStride, cbImage, static_cast<BYTE *>(pvImageBits))))
	{
		// couldn't extract image; delete HBITMAP
		DeleteObject(hbmp);
		hbmp = NULL;
	}
	return hbmp;
}

ComPtr<IWICBitmapSource> Direct2DUtility::CreateWICBitmapFromBitmap(ID2D1Bitmap * bitmap)
{
	D2D1_SIZE_F size = bitmap->GetSize();

	ComPtr<IWICBitmap> wicBitmap;
	HRESULT hr = S_OK;
	reportError(114, (hr = g_pWICFactory->CreateBitmap(
		static_cast<unsigned int>(size.width),
		static_cast<unsigned int>(size.height),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapCacheOnLoad,
		&wicBitmap)));

	ComPtr<ID2D1RenderTarget> wicRenderTarget;
	D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
	reportError(115, (hr = g_pD2DFactory->CreateWicBitmapRenderTarget(wicBitmap, rtProps, &wicRenderTarget)));

	wicRenderTarget->BeginDraw();
	wicRenderTarget->DrawBitmap(bitmap);
	wicRenderTarget->EndDraw();

	return static_cast<ComPtr<IWICBitmapSource>>(wicBitmap);
}
