/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlmisc.h>

#include "resource.h"

#include <gdiplus.h> 
#include <gdiplusheaders.h>
#include <gdiplusbitmap.h>
#include <gdipluscachedbitmap.h>
#include <gdiplusimaging.h>

#include "Direct2DUtility.h"
#include "ImageLayerOperation.h"

#pragma warning(disable:4244)

using namespace Gdiplus;

BOOL ImageFromIDResource(UINT nID, LPCTSTR sTR, Gdiplus::Image * & pImg);

#pragma once

class CPostView
{
public:

	ComPtr<ID2D1Bitmap> img_ = nullptr;

	CRect postRect_ = { 0, 0, 0, 0 };

	bool		imageLoadOK_ = false;

	int		miniWidth_ = 0;
	int		miniHeight_ = 0;

	int		filePicWidth_ = 0;
	int		filePicHeight_ = 0;

	//postPic location in CRect(postRect_) and size
	int		fileCurrentCX_ = 0;
	int		fileCurrentCY_ = 0;
	int		fileCurrentWidth_ = 0;
	int		fileCurrentHeight_ = 0;

public:

public:
	HRESULT CreateDeviceResources();
	HRESULT DiscardDeviceResources();

	CPostView();

	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	CPostView(HWND hWnd,
		CRect * lpRect,
		CBLayerPtr &bp);

	//////////////////////////////////////////////////////////////////////////////////////////////
	//���ⲿ�ļ���ָ����ͼ��ת����
	//////////////////////////////////////////////////////////////////////////////////////////////
	CPostView(HWND hWnd,
		CRect * lpRect,
		LPCTSTR picFilePathName);

	//////////////////////////////////////////////////////////////////////////////////////////////
	//�ӳ����ڲ��Ѿ����ڵ�Gdiplus::Bitmap������������
	//////////////////////////////////////////////////////////////////////////////////////////////
	CPostView(HWND hWnd,
		CRect * lpRect,
		ComPtr<IWICBitmapSource> wicBitmap);

	~CPostView();

protected:

	HWND pWnd_;
	int		alpha_ = 255;

protected:

	void LoadImgCreationAccordingToImg();
	void FixCurrentFile();

public:
	void ReLocatePostView(CRect * lpRec);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//CPostView��һ�������������Transparent�ͷ�͸����Post������
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	void PaintPostView(HDC hdc, bool bShowBorder = false);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//CRotateView��CWatermarkView�Ĺ�ͬ�����������Ϊ�����඼��͸��ͼ���ϲ���
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	void PaintTransparentView(HDC hdc, int alpha = 255);
};

class CRotateView : public CPostView
{
public:
	float		rWheel = 0.0f;
	float		xPosFloat = 0.0f;
	int		xPosInitial = 0;

	CRotateView(HWND hWnd, CRect * lpRect, LPCTSTR picFilePathName);
	~CRotateView();
	void PaintRotateView(HDC hdc, float rotateAngle);
};

class CWatermarkView : public CPostView
{
public:
	CWatermarkView(HWND hWnd, CRect * lpRect, LPCTSTR fileName, float displayRatio);
	CWatermarkView(HWND hWnd, CRect * lpRect, ComPtr<IWICBitmapSource> wicBitmap, float displayRatio);
	~CWatermarkView();
	CRect GetRect();
	void PrepareBitmemAndDC(float displayRatio);
};