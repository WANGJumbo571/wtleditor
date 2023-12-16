#pragma once

#include <GdiPlus.h>
#include <functional>

#include <atlcrack.h>
#include <atlmisc.h>

#pragma warning(disable:4244)

using namespace std;

#define THUMB_SIZE 12
#define TIMER_ID_ANIMATED_SHIMMER 2001

extern BOOL BitmapFromIDResource(UINT nID, LPCTSTR sTR, Gdiplus::Bitmap*& pBitmap);

class CSliderBar :public CWindowImpl<CSliderBar> //, CWindow>
{
public:
	CSliderBar() : solidBrush(Gdiplus::Color(0xffffffff))
	{}

	~CSliderBar() {
		delete bmpShimmer_;
	}

	BEGIN_MSG_MAP_EX(CSliderBar)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkg)
		MSG_WM_PAINT(OnPaint)
		END_MSG_MAP()

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		if (abs(point.x - x_pointCurrent_) < THUMB_SIZE / 2 &&
			(abs(point.y - rect_.Height() / 2) < THUMB_SIZE / 2))
		{
			SetCapture();
			x_pointLastTime_ = point.x;
		}
		else if (point.x < x_pointCurrent_ && abs(point.y - rect_.Height() / 2) < THUMB_SIZE / 2)
		{
			int value;
			if (value_ > 90)
				value = 90;
			else if (value_ > 67)
				value = 67;
			else if (value_ > 50)
				value = 50;
			else if (value_ > 25)
				value = 25;
			else
				value = value_ - 1;
			if (value < nMin_)
				value = nMin_;
			setValue(value);

			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			//�ص�save quality�Ի���ĵ�ǰ����ֵ�ı��Ĵ�����
			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (!funcValueChanging_)
				return;
			funcValueChanging_(value);
		}
		else if (point.x > x_pointCurrent_ && abs(point.y - rect_.Height() / 2) < THUMB_SIZE / 2)
		{
			int value;
			if (value_ < 25)
				value = 25;
			else if (value_ < 50)
				value = 50;
			else if (value_ < 67)
				value = 67;
			else if (value_ < 90)
				value = 90;
			else
				value = value_ + 1;
			if (value > nMax_)
				value = nMax_;
			setValue(value);

			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			//�ص�save quality�Ի���ĵ�ǰ����ֵ�ı��Ĵ�����
			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (!funcValueChanging_)
				return;
			funcValueChanging_(value);
		}
	}

	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		if (GetCapture() == m_hWnd)
			ReleaseCapture();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//�������sliderbar�Ļ�����ֵ
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if (nFlags != MK_LBUTTON || GetCapture() != m_hWnd)
			return;

		int x_offset = point.x - x_pointLastTime_;

		x_pointCurrent_ += x_offset;

		if (x_pointCurrent_ > x_pointEnd_)
			x_pointCurrent_ = x_pointEnd_;
		if (x_pointCurrent_ < x_pointStart_)
			x_pointCurrent_ = x_pointStart_;

		x_pointLastTime_ = x_pointCurrent_;

		InvalidateRect(NULL);
		UpdateWindow();

		if (!funcValueChanging_)
			return;

		auto value = (x_pointCurrent_ - x_pointStart_) * (nMax_ - nMin_) / rect_drawable_.Width() + nMin_;

		if (value < nMin_) value = nMin_;
		if (value > nMax_) value = nMax_;

		if (value_ != value)
		{
			value_ = value;
			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			//�ص�save quality�Ի���ĵ�ǰ����ֵ�ı��Ĵ�����
			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			funcValueChanging_(value);
		}
	}

	LRESULT OnEraseBkg(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		bHandled = TRUE;
		return TRUE;
	}

	LRESULT OnTimer(UINT_PTR ptr)
	{
		int timer_id = (int)ptr;
		if (timer_id == TIMER_ID_ANIMATED_SHIMMER)
		{
			KillTimer(TIMER_ID_ANIMATED_SHIMMER);

			InvalidateRect(NULL);
			UpdateWindow();

			nShimmerStart_ += 2;
			if (nShimmerStart_ > 200)
				nShimmerStart_ = -100;

			SetTimer(TIMER_ID_ANIMATED_SHIMMER, 10);
		}
		return TRUE;
	}

	void OnPaint(CDCHandle)
	{
		CPaintDC cdc(m_hWnd);
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 1.1 ����Drawable����Bitmap
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Bitmap bmpDrawable(rect_drawable_.Width(), rect_drawable_.Height());
		Gdiplus::Graphics graphicsBmpDrawable(&bmpDrawable);
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 1.2 �������Drawable�Ļ���BitmapͼƬ�����ֵ����
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Color colorused(0x7a, 0x9b, 0x07);
		solidBrush.SetColor(colorused);
		graphicsBmpDrawable.FillRectangle(&solidBrush,
			RectF(0,
				0,
				rect_drawable_.Width(),
				rect_drawable_.Height()));

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 1.3 ��ShimmerͼƬ�������Drawable
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::CachedBitmap cachedBmpShimmer((Bitmap*)bmpShimmer_, &graphicsBmpDrawable);
		graphicsBmpDrawable.DrawCachedBitmap(&cachedBmpShimmer, nShimmerStart_, 0);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 1.4 ���Drawable�Ļ���Bitmapͼ���Ҳ�ǰ�����Ը���Drawable���Ҳ�����
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Color colorUnUsed(0xa0, 0xa0, 0xa0);
		solidBrush.SetColor(colorUnUsed);
		graphicsBmpDrawable.FillRectangle(&solidBrush,
			RectF(x_pointCurrent_,
				0,
				rect_drawable_.Width() - x_pointCurrent_,
				rect_drawable_.Height()));

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2.1 ����SliderBar�������ڵĻ���Bitmap������AntiAlias
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Bitmap bmp(rect_.Width(), rect_.Height());
		Gdiplus::Graphics graphicsBmp(&bmp);
		graphicsBmp.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
		graphicsBmp.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2.2 ���SliderBar���ڵĻ���Bitmapͼ�ı���ɫ
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Color colorBackground(0xf0, 0xf0, 0xf0);
		solidBrush.SetColor(colorBackground);
		graphicsBmp.FillRectangle(&solidBrush, RectF(0.0, 0.0, rect_.Width(), rect_.Height()));

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2.3 ��Drawable����Bitmapͼ���SliderBar���ڻ���Bitmapͼ
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::CachedBitmap cachedBmpDrawable(&bmpDrawable, &graphicsBmp);
		graphicsBmp.DrawCachedBitmap(&cachedBmpDrawable, rect_drawable_.left, rect_drawable_.top);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2.4 ��SliderBar���ڻ���Bitmap���м仭THUMBԲ��
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Color color(0x4f, 0x61, 0x11);
		solidBrush.SetColor(color);
		graphicsBmp.FillEllipse(&solidBrush,
			x_pointCurrent_ - THUMB_SIZE / 2,
			(rect_.Height() - THUMB_SIZE) / 2,
			THUMB_SIZE,
			THUMB_SIZE);

		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 3 ���ˢ����������
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Graphics graphics(cdc.m_hDC);
		Gdiplus::CachedBitmap cachedBmpWindow(&bmp, &graphics);
		graphics.DrawCachedBitmap(&cachedBmpWindow, 0, 0);
	}

	void setFuncValueChanging(function<void(int)>& func)
	{
		funcValueChanging_ = func;
	}

	void setValueRange(int nMin, int nMax)
	{
		nMin_ = nMin;
		nMax_ = nMax;
	}

	void setValue(int value)
	{
		value_ = value;

		x_pointCurrent_ = (value_ - nMin_) / ((float)(nMax_ - nMin_)) * rect_drawable_.Width() + rect_drawable_.left;

		InvalidateRect(NULL);
		UpdateWindow();
	}

	void updateLayout(CRect rect)
	{
		rect_.left = 0;
		rect_.top = 0;
		rect_.right = rect.Width();
		rect_.bottom = rect.Height();

		rect_drawable_.left = rect_.left + 6;
		rect_drawable_.top = rect_.Height() / 2 - 2;
		rect_drawable_.right = rect_.right - 6;
		rect_drawable_.bottom = rect_.Height() / 2 + 2;

		x_pointStart_ = rect_drawable_.left;
		x_pointEnd_ = rect_drawable_.right;

		BitmapFromIDResource(IDR_MYPNG_SHIMMER, L"MYPNG", bmpShimmer_);
		nShimmerStart_ = -100;
		SetTimer(TIMER_ID_ANIMATED_SHIMMER, 700);
	}

private:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//sliderbar�����˵Ļص�������ڣ�ÿ�����ؼ��������valueֵ�����ı�ʱ������
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	function<void(int)> funcValueChanging_;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//����SliderBar���ڵĿ�Ⱥ͸߶�
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	CRect rect_;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//��ֵ100��bar��sliderbar�������λ�úʹ�С
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	CRect rect_drawable_;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//��ֵbar�ڴ��������ʼ�ͽ�����λ�ã�������λ��λ
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int x_pointStart_;
	int x_pointEnd_;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//bar�м��THUMB�ڴ����е�λ�ã���ֵ��ʵ�ʵ�����Ϊ��λ
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int x_pointCurrent_;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//�������һ�μ�¼������ x ֵ
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int x_pointLastTime_;

	int nMin_;
	int nMax_;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//sliderbar����������յĵ�ǰֵ�������ڡ�nMin, nMax����
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int value_;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//���⶯���õ���Bitmap�ͻ�ͼ����ʼλ��
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	Gdiplus::Bitmap* bmpShimmer_;
	int nShimmerStart_;

	Gdiplus::SolidBrush solidBrush;
};
