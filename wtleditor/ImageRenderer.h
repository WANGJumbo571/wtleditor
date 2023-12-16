#pragma once
#include "BasicTimer.h"

extern ComPtr<ID3D11Device2>					g_d3dDevice;
extern ComPtr<ID3D11DeviceContext3>		g_d3dContext;
extern ComPtr<ID3D11RenderTargetView>	g_d3dRenderTargetView;
extern ComPtr<ID3D11DepthStencilView>	g_d3dDepthStencilView;
extern ComPtr<IDXGISwapChain1>				g_swapChain;
extern ComPtr<ID2D1Factory6>					g_pD2DFactory;
extern ComPtr<ID2D1Device5>					g_pD2DDevice;
extern ComPtr<ID2D1DeviceContext5>		g_pD2DDeviceContext;
extern ComPtr<IWICImagingFactory2>		g_pWICFactory;
extern ComPtr<IDWriteFactory2>					g_pDWriteFactory;
extern ComPtr<IDWriteTextFormat>			g_pTextFormatMain;
extern ComPtr<ID2D1Bitmap1>					g_pD2DTargetBimtap;
extern ComPtr<ID2D1Bitmap1>					g_d2dTargetBitmap;
extern ComPtr<ID2D1SolidColorBrush>		g_solidBrush;
extern ComPtr<ID2D1RenderTarget>			g_wicRenderTarget;
extern ComPtr<ID2D1GradientMesh>			g_gradientMesh;

extern ComPtr<ID2D1Effect>						g_RippleEffect;
extern ComPtr<ID2D1Effect>						g_HscrollEffect;
extern ComPtr<ID2D1Effect>						g_VscrollEffect;

extern D3D11_VIEWPORT								g_screenViewport;
extern D3D_FEATURE_LEVEL							g_d3dFeatureLevel;
extern D3D_FEATURE_LEVEL							g_featureLevel;
extern DXGI_PRESENT_PARAMETERS				g_dxgiParameters;

extern D2D1_SIZE_F										g_logicalSize;
extern bool													g_isDrawingRippleOn;
extern bool													g_isDrawingScrollOn;
extern bool													g_isDrawingVScrollOn;
extern bool													g_isLayerAnimationOn;
extern bool													g_isDrawingVScroll2000;
extern bool													g_isDrawingScroll2000;

extern void reportError(int location, HRESULT hr);

class ImageRenderer{

public:
	ImageRenderer();
    ~ImageRenderer();

	HWND	m_hwnd;

	void CreateResources();
	void CreateSwapAndSetD2DContextTarget();

	void Present();

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Ripple_CreateEffect(CString sourceFileName);
	void Ripple_UpdateEffectScale();
	void Ripple_Render();
	void Ripple_Update();
	void Ripple_UpdatePointer(D2D1_POINT_2F pointer);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void HScroll_CreateEffect(CString sourceFileName);
	void HScroll_UpdateEffectScale();
	void HScroll_Update();
	void HScroll_Render(); 

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void VScroll_CreateEffect(CString sourceFileName);
	void VScroll_UpdateEffectScale();
	void VScroll_Update();
	void VScroll_Render();

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void GradientMesh_CreateGradientMesh();
	void GradientMesh_Render();

	ComPtr<IWICBitmapSource>	LoadWicBitmapFromFile(const wchar_t* uri, unsigned int destinationWidth, unsigned int destinationHeight);
	ComPtr<ID2D1Bitmap>			LoadBitmapFromFile(const wchar_t* uri, unsigned int destinationWidth, unsigned int destinationHeight);
	ComPtr<ID2D1Bitmap>			LoadBitmapFromFile(ID2D1RenderTarget* renderTarget, const wchar_t* uri, unsigned int destinationWidth, unsigned int destinationHeight);

private:

	D2D1_SIZE_F													m_outputSize;
	D2D1_SIZE_F													m_d3dRenderTargetSize;

	ComPtr<IWICFormatConverter>					m_RformatConverter;
	ComPtr<ID2D1TransformedImageSource>	m_RtransformedRippleImage;
	ComPtr<ID2D1ImageSourceFromWic>			m_RimageSource;
	D2D1_SIZE_U												m_RimageSize;

	ComPtr<IWICFormatConverter>					m_HformatConverter;
	ComPtr<ID2D1TransformedImageSource>	m_HtransformedWaveImage;
	ComPtr<ID2D1ImageSourceFromWic>			m_HimageSource;
	D2D1_SIZE_U												m_HimageSize;

	ComPtr<IWICFormatConverter>					m_VformatConverter;
	ComPtr<ID2D1TransformedImageSource>	m_VtransformedWaveImage;
	ComPtr<ID2D1ImageSourceFromWic>			m_VimageSource;
	D2D1_SIZE_U												m_VimageSize;

	BasicTimer													m_timer;

public:
	bool																m_isRippleAnimating;
};

extern ImageRenderer* g_render;
