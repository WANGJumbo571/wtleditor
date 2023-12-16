#include "stdafx.h"
#include "ImageRenderer.h"
#include "postview.h"

using namespace Hilo::Direct2DHelpers;

static ComPtr<ID2D1SolidColorBrush> d2_solidBrush_; 
static ComPtr<ID2D1DCRenderTarget> d2_renderTarget_;

HRESULT CPostView::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (d2_renderTarget_ == nullptr)
	{
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_IGNORE),
			0,
			0,
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT
		);

		if (SUCCEEDED(hr))
		{
			hr = g_pD2DFactory->CreateDCRenderTarget(&props, &d2_renderTarget_);
		}

		if (SUCCEEDED(hr))
		{
			hr = d2_renderTarget_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &d2_solidBrush_);
		}
	}
	return hr;
}

HRESULT CPostView::DiscardDeviceResources()
{
	return S_OK;
}

CPostView::CPostView(HWND hWnd,
									CRect * lpRect,
									CBLayerPtr &bp)
{
	CreateDeviceResources();

	postRect_ = *lpRect;
	miniWidth_ = postRect_.Width();
	miniHeight_ = postRect_.Height();

	pWnd_ = hWnd;

	d2_renderTarget_->CreateBitmapFromWicBitmap(bp.m_layerWicBitmap, &img_);

	LoadImgCreationAccordingToImg();
}

	//////////////////////////////////////////////////////////////////////////////////////////////
	//从外部文件名指定的图像转换。
	//////////////////////////////////////////////////////////////////////////////////////////////
CPostView::CPostView(HWND hWnd, 
					CRect * lpRect, 
					LPCTSTR picFilePathName) 
{

	CreateDeviceResources();
	
	postRect_ = *lpRect;
	miniWidth_ = postRect_.Width();
	miniHeight_ = postRect_.Height();

	pWnd_ = hWnd;

	img_ = g_render->LoadBitmapFromFile(d2_renderTarget_, picFilePathName, 0, 0);

	LoadImgCreationAccordingToImg();
}

	//////////////////////////////////////////////////////////////////////////////////////////////
	//从程序内部已经存在的Gdiplus::Bitmap的内容中来。
	//////////////////////////////////////////////////////////////////////////////////////////////
CPostView::CPostView(HWND hWnd,
									CRect * lpRect, 
									ComPtr<IWICBitmapSource> wicBitmap) 
{
	CreateDeviceResources();
	
	postRect_ = *lpRect;
	miniWidth_ = postRect_.Width();
	miniHeight_ = postRect_.Height();

	pWnd_ = hWnd;

	d2_renderTarget_->CreateBitmapFromWicBitmap(wicBitmap, &img_);

	LoadImgCreationAccordingToImg();
}

CPostView::~CPostView() {
	img_ = nullptr;
}

void CPostView::LoadImgCreationAccordingToImg()
{
	if (img_ == nullptr || img_->GetSize().width == 0 || img_->GetSize().height == 0)
	{
		imageLoadOK_ = false;
		ATLASSERT(false);
	}
	else
		imageLoadOK_ = true;

	filePicWidth_ = img_->GetSize().width;
	filePicHeight_ = img_->GetSize().height;

	FixCurrentFile();
}

void CPostView::FixCurrentFile()
{
	if (imageLoadOK_ == false)
	{
		fileCurrentWidth_ = fileCurrentHeight_ = fileCurrentCX_ = fileCurrentCY_ = 0;
		ATLASSERT(false);
		return;
	}

	int original_PicWidth;
	int original_PicHeight;
	int showWidth;
	int showHeight;

	original_PicWidth = showWidth = filePicWidth_;
	original_PicHeight = showHeight = filePicHeight_;
	//////////////////////////////////////////////////////////////////////////////////////////////
	//图片在miniWidth_，miniHeight_大小的ChildView窗口里显示
	//////////////////////////////////////////////////////////////////////////////////////////////
	if (original_PicWidth >= miniWidth_)
	{
		if (original_PicHeight >= miniHeight_)
		{
			if (((double)original_PicWidth) / filePicHeight_ >= ((double)miniWidth_) / miniHeight_)
			{
				showWidth = miniWidth_;
				showHeight = (int)(((double)(showWidth)) / original_PicWidth * original_PicHeight);
			}
			else
			{
				showHeight = miniHeight_;
				showWidth = (int)(((double)(showHeight)) / original_PicHeight * original_PicWidth);
			}
		}
		else
		{
			showWidth = miniWidth_;
			showHeight = (int)(((double)(showWidth)) / original_PicWidth * original_PicHeight);
		}
	}
	else
	{
		if (original_PicHeight >= miniHeight_)
		{
			showHeight = miniHeight_;
			showWidth = (int)(((double)(showHeight)) / original_PicHeight * original_PicWidth);
		}
		else
		{
			showWidth = original_PicWidth;
			showHeight = original_PicHeight;
		}
	}

	//得出图片在ChildView中的实际显示的大小
	fileCurrentWidth_ = showWidth;
	fileCurrentHeight_ = showHeight;

	//缺省模式为initialShow=true，把图片显示在窗口的中央
	fileCurrentCX_ = miniWidth_ / 2 - fileCurrentWidth_ / 2;
	fileCurrentCY_ = miniHeight_ / 2 - fileCurrentHeight_ / 2;

	if (fileCurrentCX_ < 0)
		fileCurrentCX_ = 0;
	if (fileCurrentCY_ < 0)
		fileCurrentCY_ = 0;
}

void CPostView::ReLocatePostView(CRect * lpRec)
{
	postRect_ = *lpRec;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//CPostView的一般性输出函数，Transparent和非透明的Post调用它
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPostView::PaintPostView(HDC hdc, bool bShowBorder)
{
	HRESULT hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		RECT rect = { 0, 0, 1920, 1080 };
		d2_renderTarget_->BindDC(hdc, &rect);

		d2_renderTarget_->BeginDraw();
		d2_renderTarget_->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_RECT_F	dest = { 0, 0, (FLOAT)filePicWidth_, (FLOAT)filePicHeight_ };

		D2D1::Matrix3x2F	matrix1 =
			D2D1::Matrix3x2F::Scale(fileCurrentWidth_ / (float)filePicWidth_, fileCurrentHeight_ / (float)filePicHeight_) *
			D2D1::Matrix3x2F::Translation(postRect_.left + fileCurrentCX_, postRect_.top + fileCurrentCY_);

		d2_renderTarget_->SetTransform(matrix1);
		d2_renderTarget_->DrawBitmap(img_, dest, alpha_ / 255.0f);

		if (bShowBorder)
		{
			d2_renderTarget_->DrawRectangle(dest, d2_solidBrush_, 3);
		}

		hr = d2_renderTarget_->EndDraw();
	}
}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//CRotateView和CWatermarkView的共同输出函数，因为两个类都在透明图像上操作
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPostView::PaintTransparentView(HDC hdc, int alpha)
{
	alpha_ = alpha;
	PaintPostView(hdc);
}
	
CRotateView::CRotateView(HWND hWnd, CRect * lpRect, LPCTSTR picFilePathName) 
	: CPostView(hWnd, lpRect, picFilePathName)
{
	rWheel = postRect_.Width() / 2;
	xPosFloat = postRect_.left;
	xPosInitial = xPosFloat;
}
	
CRotateView::~CRotateView() {
}
	
void CRotateView::PaintRotateView(HDC hdc, float rotateAngle)
{
	HRESULT hr;
	RECT rc;

	GetClientRect(pWnd_, &rc);

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		hr = d2_renderTarget_->BindDC(hdc, &rc);

		d2_renderTarget_->BeginDraw();
		d2_renderTarget_->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_RECT_F	dest0;
		dest0 = { (FLOAT)postRect_.left, (FLOAT)postRect_.top, (FLOAT)postRect_.left + miniWidth_ + 4, (FLOAT)postRect_.top + miniHeight_ };
		d2_renderTarget_->FillRectangle(dest0, d2_solidBrush_);

		D2D1_RECT_F	dest = { 0, 0, (FLOAT)filePicWidth_, (FLOAT)filePicHeight_ };
		D2D1_POINT_2F pt = { filePicWidth_ / 2.0f, (FLOAT)filePicHeight_ / 2.0f };

		D2D1::Matrix3x2F	matrix1 = D2D1::Matrix3x2F::Rotation(rotateAngle, pt);
		D2D1::Matrix3x2F	matrix2 = 
			D2D1::Matrix3x2F::Scale(fileCurrentWidth_ / (float)filePicWidth_, fileCurrentHeight_ / (float)filePicHeight_) *
			D2D1::Matrix3x2F::Translation((FLOAT)postRect_.left + fileCurrentCX_, (FLOAT)postRect_.top + fileCurrentCY_);
													
		d2_renderTarget_->SetTransform(matrix1 * matrix2);
		d2_renderTarget_->DrawBitmap(img_, dest, alpha_ / 255.0f);

		hr = d2_renderTarget_->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		DiscardDeviceResources();
	}

	postRect_.left = xPosFloat;
	postRect_.right = postRect_.left + miniWidth_;
}

CWatermarkView::CWatermarkView(HWND hWnd, CRect * lpRect, LPCTSTR fileName, float displayRatio) 
	: CPostView(hWnd, lpRect, fileName)
{
	PrepareBitmemAndDC(displayRatio);
}
	
CWatermarkView::CWatermarkView(HWND hWnd, CRect * lpRect, ComPtr<IWICBitmapSource> wicBitmap, float displayRatio) 
	: CPostView(hWnd, lpRect, wicBitmap)
{
	PrepareBitmemAndDC(displayRatio);
}

CWatermarkView::~CWatermarkView()
{
}

CRect CWatermarkView::GetRect()
{
	return postRect_;
}

void CWatermarkView::PrepareBitmemAndDC(float displayRatio)
{
	ATLASSERT(img_ != nullptr);

	filePicWidth_ = img_->GetSize().width;
	filePicHeight_ = img_->GetSize().height;

	fileCurrentWidth_ = miniWidth_ = (int)(filePicWidth_ * displayRatio);
	fileCurrentHeight_ = miniHeight_ = (int)(filePicHeight_ * displayRatio);

	postRect_.right = postRect_.left + miniWidth_;
	postRect_.bottom = postRect_.top + miniHeight_;

	fileCurrentCX_ = 0;
	fileCurrentCY_ = 0;
}
