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
			//回调save quality对话框的当前滑动值改变后的处理函数
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
			//回调save quality对话框的当前滑动值改变后的处理函数
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
	//接替计算sliderbar的滑动现值
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
			//回调save quality对话框的当前滑动值改变后的处理函数
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
		// 1.1 创建Drawable缓存Bitmap
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Bitmap bmpDrawable(rect_drawable_.Width(), rect_drawable_.Height());
		Gdiplus::Graphics graphicsBmpDrawable(&bmpDrawable);
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 1.2 填充整个Drawable的缓存Bitmap图片的最大值背景
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Color colorused(0x7a, 0x9b, 0x07);
		solidBrush.SetColor(colorused);
		graphicsBmpDrawable.FillRectangle(&solidBrush,
			RectF(0,
				0,
				rect_drawable_.Width(),
				rect_drawable_.Height()));

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 1.3 以Shimmer图片滑动填充Drawable
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::CachedBitmap cachedBmpShimmer((Bitmap*)bmpShimmer_, &graphicsBmpDrawable);
		graphicsBmpDrawable.DrawCachedBitmap(&cachedBmpShimmer, nShimmerStart_, 0);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 1.4 填充Drawable的缓存Bitmap图的右部前景，以覆盖Drawable的右部背景
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Color colorUnUsed(0xa0, 0xa0, 0xa0);
		solidBrush.SetColor(colorUnUsed);
		graphicsBmpDrawable.FillRectangle(&solidBrush,
			RectF(x_pointCurrent_,
				0,
				rect_drawable_.Width() - x_pointCurrent_,
				rect_drawable_.Height()));

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2.1 创建SliderBar整个窗口的缓存Bitmap并设置AntiAlias
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Bitmap bmp(rect_.Width(), rect_.Height());
		Gdiplus::Graphics graphicsBmp(&bmp);
		graphicsBmp.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
		graphicsBmp.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2.2 填充SliderBar窗口的缓存Bitmap图的背景色
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Color colorBackground(0xf0, 0xf0, 0xf0);
		solidBrush.SetColor(colorBackground);
		graphicsBmp.FillRectangle(&solidBrush, RectF(0.0, 0.0, rect_.Width(), rect_.Height()));

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2.3 以Drawable缓冲Bitmap图填充SliderBar窗口缓存Bitmap图
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::CachedBitmap cachedBmpDrawable(&bmpDrawable, &graphicsBmp);
		graphicsBmp.DrawCachedBitmap(&cachedBmpDrawable, rect_drawable_.left, rect_drawable_.top);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2.4 在SliderBar窗口缓存Bitmap的中间画THUMB圆形
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		Gdiplus::Color color(0x4f, 0x61, 0x11);
		solidBrush.SetColor(color);
		graphicsBmp.FillEllipse(&solidBrush,
			x_pointCurrent_ - THUMB_SIZE / 2,
			(rect_.Height() - THUMB_SIZE) / 2,
			THUMB_SIZE,
			THUMB_SIZE);

		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 3 最后刷满到窗口中
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
	//sliderbar的主人的回调函数入口，每当本控件所代表的value值发生改变时调用它
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	function<void(int)> funcValueChanging_;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//保留SliderBar窗口的宽度和高度
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	CRect rect_;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//满值100的bar在sliderbar窗口里的位置和大小
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	CRect rect_drawable_;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//满值bar在窗口里的起始和结束的位置，以像素位单位
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int x_pointStart_;
	int x_pointEnd_;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//bar中间的THUMB在窗口中的位置，其值以实际的像素为单位
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int x_pointCurrent_;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//缓存的上一次记录的鼠标的 x 值
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int x_pointLastTime_;

	int nMin_;
	int nMax_;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//sliderbar所代表的最终的当前值，区间在《nMin, nMax》里
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int value_;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//亮光动画用到的Bitmap和画图的起始位置
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	Gdiplus::Bitmap* bmpShimmer_;
	int nShimmerStart_;

	Gdiplus::SolidBrush solidBrush;
};
