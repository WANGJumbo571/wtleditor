#include "stdafx.h"

#include "BasicTimer.h"
#include "external.h"
#include "HScrollEffect.h"
#include "ImageRenderer.h"
#include "mainfrm.h"
#include "RippleEffect.h"
#include "time.h"
#include "VScrollEffect.h"

extern CMainFrame* g_frameWork;

ComPtr<ID3D11Device2>					g_d3dDevice;
ComPtr<ID3D11DeviceContext3>		g_d3dContext;
ComPtr<ID3D11RenderTargetView>	g_d3dRenderTargetView;
ComPtr<ID3D11DepthStencilView>	g_d3dDepthStencilView;
ComPtr<IDXGISwapChain1>			g_swapChain;
ComPtr<ID2D1Factory6>					g_pD2DFactory;
ComPtr<ID2D1Device5>					g_pD2DDevice;
ComPtr<ID2D1DeviceContext5>		g_pD2DDeviceContext;
ComPtr<IWICImagingFactory2>		g_pWICFactory;
ComPtr<IDWriteFactory2>				g_pDWriteFactory;
ComPtr<IDWriteTextFormat>			g_pTextFormatMain;
ComPtr<ID2D1Bitmap1>					g_pD2DTargetBimtap;
ComPtr<ID2D1Bitmap1>					g_d2dTargetBitmap;
ComPtr<ID2D1SolidColorBrush>		g_solidBrush;
ComPtr<ID2D1RenderTarget>			g_wicRenderTarget;
ComPtr<ID2D1GradientMesh>			g_gradientMesh;

ComPtr<ID2D1Effect>						g_RippleEffect;
ComPtr<ID2D1Effect>						g_HscrollEffect;
ComPtr<ID2D1Effect>						g_VscrollEffect;

D3D11_VIEWPORT							g_screenViewport;
D3D_FEATURE_LEVEL						g_d3dFeatureLevel;
D3D_FEATURE_LEVEL						g_featureLevel;
DXGI_PRESENT_PARAMETERS			g_dxgiParameters;

D2D1_SIZE_F		g_logicalSize;
bool					g_isDrawingRippleOn = false;
bool					g_isDrawingScrollOn = false;
bool					g_isDrawingVScrollOn = false;
bool					g_isLayerAnimationOn = false;
bool					g_isDrawingVScroll2000 = false;
bool					g_isDrawingScroll2000 = false;

#undef 	PixelFormat

// ImageRenderer类构造函数
ImageRenderer::ImageRenderer()
{
    g_dxgiParameters.DirtyRectsCount = 0;
    g_dxgiParameters.pDirtyRects = nullptr;
    g_dxgiParameters.pScrollRect = nullptr;
    g_dxgiParameters.pScrollOffset = nullptr;
}

// ImageRenderer析构函数
ImageRenderer::~ImageRenderer()
{
}

// 创建资源
void ImageRenderer::CreateResources()
{
	// Initialize Direct2D resources.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	HRESULT hr = S_OK;
	// Initialize the Direct2D Factory.

	reportError(1, (hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory6),
		&options,
		(void**)&g_pD2DFactory
	)));

	// Initialize the DirectWrite Factory.
	reportError(2, (hr =
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory2),
			(IUnknown**)&g_pDWriteFactory
		)));

	// Initialize the Windows Imaging Component (WIC) Factory.
	reportError(3, (hr =
		CoCreateInstance(
			CLSID_WICImagingFactory2,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&g_pWICFactory)
		)));

	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the Direct3D 11 API device object and a corresponding context.
	ComPtr<ID3D11Device>				device;
	ComPtr<ID3D11DeviceContext> context;

	reportError(4, (hr = D3D11CreateDevice(
		nullptr,                    // Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
		0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		creationFlags,              // Set debug and Direct2D compatibility flags.
		featureLevels,              // List of feature levels this app can support.
		ARRAYSIZE(featureLevels),   // Size of the list above.
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Runtime apps.
		&device,                    // Returns the Direct3D device created.
		&g_d3dFeatureLevel,         // Returns feature level of device created.
		&context                    // Returns the device immediate context.
	)));

	if (FAILED(hr))
	{
		// If the initialization fails, fall back to the WARP device.
		// For more information on WARP, see: 
		// http://go.microsoft.com/fwlink/?LinkId=286690
		reportError(5, (hr =
			D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
				0,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&device,
				&g_d3dFeatureLevel,
				&context
			)));
	}

	device.QueryInterface(&g_d3dDevice);
	context.QueryInterface(&g_d3dContext);

	ComPtr<IDXGIDevice3> dxgiDevice;
	// Create the Direct2D device object and a corresponding context.
	g_d3dDevice->QueryInterface(__uuidof(IDXGIDevice3), (void**)&dxgiDevice);

	reportError(6, (hr = g_pD2DFactory->CreateDevice(dxgiDevice, &g_pD2DDevice)));

	reportError(7, (hr = g_pD2DDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&g_pD2DDeviceContext)));

	reportError(13, (hr = RippleEffect::Register(g_pD2DFactory)));
	reportError(57, (hr = HScrollEffect::Register(g_pD2DFactory)));
	reportError(157, (hr = VScrollEffect::Register(g_pD2DFactory)));

	g_RippleEffect = nullptr;
	reportError(15, (hr = g_pD2DDeviceContext->CreateEffect(CLSID_CustomRippleEffect, &g_RippleEffect)));

	g_HscrollEffect = nullptr;
	reportError(55, (hr = g_pD2DDeviceContext->CreateEffect(CLSID_CustomWaveEffectH, &g_HscrollEffect)));

	g_VscrollEffect = nullptr;
	reportError(155, (hr = g_pD2DDeviceContext->CreateEffect(CLSID_CustomWaveEffectV, &g_VscrollEffect)));
}

// These resources need to be recreated every time the window size is changed.
void ImageRenderer::CreateSwapAndSetD2DContextTarget()
{
	HRESULT hr = S_OK;
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	g_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	g_d3dRenderTargetView = nullptr;
	g_pD2DDeviceContext->SetTarget(nullptr);
	g_d2dTargetBitmap = nullptr;
	g_d3dDepthStencilView = nullptr;
	g_d3dContext->Flush();

	// Calculate the necessary render target size in pixels.
	m_outputSize.width = g_logicalSize.width;
	m_outputSize.height = g_logicalSize.height;

	// Prevent zero size DirectX content from being created.
	m_outputSize.width = max(m_outputSize.width, 1);
	m_outputSize.height = max(m_outputSize.height, 1);

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	bool swapDimensions = false;
	m_d3dRenderTargetSize.width = swapDimensions ? m_outputSize.height : m_outputSize.width;
	m_d3dRenderTargetSize.height = swapDimensions ? m_outputSize.width : m_outputSize.height;

	if (g_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = S_OK;
		reportError(17, (hr = g_swapChain->ResizeBuffers(
			2, // Double-buffered swap chain.
			lround(m_d3dRenderTargetSize.width),
			lround(m_d3dRenderTargetSize.height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
		)));

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			return;
		}
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		// 获取窗口大小
		RECT rect;
		::GetClientRect(m_hwnd, &rect);
		rect.right -= rect.left;
		rect.bottom -= rect.top;
		// 交换链信息
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
		swapChainDesc.Width = rect.right;
		swapChainDesc.Height = rect.bottom;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		swapChainDesc.BufferCount = 2;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// 应用商店程序
		//swapChainDesc.Scaling = DXGI_SCALING_NONE;
		//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		// 桌面应用程序
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
		ComPtr<IDXGIDevice3> dxgiDevice;
		g_d3dDevice->QueryInterface(__uuidof(IDXGIDevice3), (void**)&dxgiDevice);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		reportError(18, (hr = dxgiDevice->GetAdapter(&dxgiAdapter)));

		ComPtr<IDXGIFactory2> dxgiFactory;
		reportError(19, (hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))));

		// 利用窗口句柄创建交换链
		reportError(20, (hr = dxgiFactory->CreateSwapChainForHwnd(
			g_d3dDevice,
			m_hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&g_swapChain
		)));

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
		// ensures that the application will only render after each VSync, minimizing power consumption.
		reportError(21, (hr = dxgiDevice->SetMaximumFrameLatency(1)));
	}

	// Set the proper orientation for the swap chain, and generate 2D and
	// 3D matrix transformations for rendering to the rotated swap chain.
	// Note the rotation angle for the 2D and 3D transforms are different.
	// This is due to the difference in coordinate spaces.  Additionally,
	// the 3D matrix is specified explicitly to avoid rounding errors.

	//reportError(22, (hr = g_swapChain->SetRotation(DXGI_MODE_ROTATION_IDENTITY)));
	hr = g_swapChain->SetRotation(DXGI_MODE_ROTATION_IDENTITY);

	// Create a render target view of the swap chain back buffer.
	ComPtr<ID3D11Texture2D> backBuffer;
	reportError(23, (hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))));

	reportError(24, (hr = g_d3dDevice->CreateRenderTargetView(
		backBuffer,
		nullptr,
		&g_d3dRenderTargetView
	)));

	// Create a depth stencil view for use with 3D rendering if needed.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		lround(m_d3dRenderTargetSize.width),
		lround(m_d3dRenderTargetSize.height),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
	);

	ComPtr<ID3D11Texture2D> depthStencil;
	reportError(25, (hr = g_d3dDevice->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		&depthStencil
	)));

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	reportError(26, (hr = g_d3dDevice->CreateDepthStencilView(
		depthStencil,
		&depthStencilViewDesc,
		&g_d3dDepthStencilView
	)));

	// Set the 3D rendering viewport to target the entire window.
	g_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		m_d3dRenderTargetSize.width,
		m_d3dRenderTargetSize.height
	);

	g_d3dContext->RSSetViewports(1, &g_screenViewport);

	// Create a Direct2D target bitmap associated with the
	// swap chain back buffer and set it as the current target.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

	ComPtr<IDXGISurface2> dxgiBackBuffer;
	reportError(27, (hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))));

	reportError(28, (hr = g_pD2DDeviceContext->CreateBitmapFromDxgiSurface(
		dxgiBackBuffer,
		&bitmapProperties,
		&g_d2dTargetBitmap
	)));

	g_pD2DDeviceContext->SetTarget(g_d2dTargetBitmap);

	// Grayscale text anti-aliasing is recommended for all Windows Runtime apps.
	g_pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

void ImageRenderer::Present()
{
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = S_OK;
	reportError(89, (hr = g_swapChain->Present(1, 0)));

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely
	// overwritten. If dirty or scroll rects are used, this call should be removed.
	g_d3dContext->DiscardView(g_d3dRenderTargetView);

	// Discard the contents of the depth stencil.
	g_d3dContext->DiscardView(g_d3dDepthStencilView);

	// If the device was removed either by a disconnection or a driver upgrade, we 
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		//HandleDeviceLost();
	}
	else
	{
		//DX::ThrowIfFailed(hr);
		reportError(90, hr);
	}
}

void ImageRenderer::Ripple_CreateEffect(CString sourceFileName)
{
	HRESULT hr = S_OK;
	// Create WIC Decoder to read JPG file.
	ComPtr<IWICBitmapDecoder> decoder;
	reportError(8, (hr = g_pWICFactory->CreateDecoderFromFilename(
		sourceFileName.GetBuffer(100),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&decoder
	)));

	ComPtr<IWICBitmapFrameDecode> frame;
	reportError(9, (hr = decoder->GetFrame(0, &frame)));

	m_RformatConverter = nullptr;
	reportError(10, (hr = g_pWICFactory->CreateFormatConverter(&m_RformatConverter)));
	reportError(11, (hr = m_RformatConverter->Initialize(
		frame,
		GUID_WICPixelFormat32bppPBGRA, // 32bppPBGRA is used by Direct2D.
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom
	)));

	UINT width;
	UINT height;
	reportError(12, (hr = m_RformatConverter->GetSize(&width, &height)));
	m_RimageSize = D2D1::SizeU(width, height);

	// Load the image from WIC using ImageSource. The image is scaled using
	// ID2D1TransformedImageSource which is dependent on the scale factor.
	m_RimageSource = nullptr;
	reportError(14, (hr = g_pD2DDeviceContext->CreateImageSourceFromWic(m_RformatConverter, &m_RimageSource)));
}

void ImageRenderer::Ripple_UpdateEffectScale()
{
	float scale = g_logicalSize.width / m_RimageSize.width;

	// When using ID2D1ImageSource, the recommend method of applying a transform
	// is to use ID2D1TransformedImageSource. It is inexpensive to recreate this object.
	D2D1_TRANSFORMED_IMAGE_SOURCE_PROPERTIES props =
	{
		D2D1_ORIENTATION_DEFAULT,
		scale,
		scale,
		D2D1_INTERPOLATION_MODE_LINEAR, // This is ignored when using DrawImage.
		D2D1_TRANSFORMED_IMAGE_SOURCE_OPTIONS_NONE
	};
	m_RtransformedRippleImage = nullptr;
	HRESULT hr = S_OK;
	reportError(16, (hr = g_pD2DDeviceContext->CreateTransformedImageSource(
		m_RimageSource,
		&props,
		&m_RtransformedRippleImage)));

	g_RippleEffect->SetInput(0, m_RtransformedRippleImage);
}

void ImageRenderer::Ripple_UpdatePointer(D2D1_POINT_2F pointer)
{
	m_timer.Reset();

	g_RippleEffect->SetValue(RIPPLE_PROP_CENTER, pointer);
	m_isRippleAnimating = true;
}

void ImageRenderer::Ripple_Update()
{
	if (m_isRippleAnimating)
	{
		m_timer.Update();
		float delta = m_timer.m_total;

		// Stop animating after four seconds.
		if (delta >= 4)
		{
			delta = 4;
			m_isRippleAnimating = false;
		}
		HRESULT hr = S_OK;
		// Increase the spread over time to make the visible area of the waves spread out.
		hr = g_RippleEffect->SetValue(RIPPLE_PROP_SPREAD, 0.01f + delta / 10.0f);

		// Reduce the amplitude over time to make the waves decay in intensity.
		hr = g_RippleEffect->SetValue(RIPPLE_PROP_AMPLITUDE, 60.0f - delta * 15.0f);

		// Reduce the frequency over time to make each individual wave spread out.
		hr = g_RippleEffect->SetValue(RIPPLE_PROP_FREQUENCY, 140.0f - delta * 30.0f);

		// Change the phase over time to make each individual wave travel away from the center.
		hr = g_RippleEffect->SetValue(RIPPLE_PROP_PHASE, -delta * 20.0f);
	}
}

void ImageRenderer::Ripple_Render()
{
	g_render->Ripple_Update();
	g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::CornflowerBlue));
	g_pD2DDeviceContext->DrawImage(g_RippleEffect);
}

void ImageRenderer::HScroll_CreateEffect(CString sourceFileName)
{
	HRESULT hr = S_OK;
	ComPtr<IWICBitmapDecoder> decoder;
	reportError(48, (hr = g_pWICFactory->CreateDecoderFromFilename(
		sourceFileName.GetBuffer(100),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&decoder
	)));

	ComPtr<IWICBitmapFrameDecode> frame;
	reportError(49, (hr = decoder->GetFrame(0, &frame)));

	m_HformatConverter = nullptr;
	reportError(50, (hr = g_pWICFactory->CreateFormatConverter(&m_HformatConverter)));
	reportError(51, (hr = m_HformatConverter->Initialize(
		frame,
		GUID_WICPixelFormat32bppPBGRA, // 32bppPBGRA is used by Direct2D.
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom
	)));

	UINT width;
	UINT height;
	reportError(52, (hr = m_HformatConverter->GetSize(&width, &height)));
	m_HimageSize = D2D1::SizeU(width, height);

	if (height == 2000)
	{
		g_isDrawingScroll2000 = true;
	}
	else
	{
		g_isDrawingScroll2000 = false;
	}

	m_HimageSource = nullptr;
	reportError(54, (hr = g_pD2DDeviceContext->CreateImageSourceFromWic(m_HformatConverter, &m_HimageSource)));
}

void ImageRenderer::HScroll_UpdateEffectScale()
{
	//float scale = g_logicalSize.width / m_HimageSize.width;
	D2D1_TRANSFORMED_IMAGE_SOURCE_PROPERTIES props =
	{
		D2D1_ORIENTATION_DEFAULT,
		1.0f,
		1.0f,
		D2D1_INTERPOLATION_MODE_LINEAR, // This is ignored when using DrawImage.
		D2D1_TRANSFORMED_IMAGE_SOURCE_OPTIONS_NONE
	};
	m_HtransformedWaveImage = nullptr;
	HRESULT hr = S_OK;
	reportError(56, (hr = g_pD2DDeviceContext->CreateTransformedImageSource(
		m_HimageSource,
		&props,
		&m_HtransformedWaveImage)));

	g_HscrollEffect->SetInput(0, m_HtransformedWaveImage);
}

static std::vector<ComPtr<ID2D1Bitmap1> > storedBitmaps;
static const int QUEUESIZE = 500;
static int index = 0;

void ImageRenderer::HScroll_Update()
{
	index = 0;
	static ComPtr<ID2D1Bitmap> bitmap;
	if (bitmap == nullptr) 
	{
		bitmap = LoadBitmapFromFile(L"K:\\娱乐\\ico\\wtlpainter\\view-coasts\\ILWJ78328865.png", 0, 0);
	}
	storedBitmaps.clear();

	D2D1_RECT_F localBounds;
	ComPtr<ID2D1Image> image;
	g_HscrollEffect->GetOutput(&image);
	g_pD2DDeviceContext->GetImageLocalBounds(image, &localBounds);

	for (int i = 0; i < QUEUESIZE; i++)
	{
		g_HscrollEffect->SetValue(HWAVE_PROP_XRATIO, 1.0f / QUEUESIZE * (i + 1));
		//g_HscrollEffect->SetValue(HWAVE_PROP_SKEW_Y, 0.0f);
		//g_HscrollEffect->SetValue(HWAVE_PROP_SKEW_X, 0.1f);

		ComPtr<ID2D1Image> oldTarget;
		ComPtr<ID2D1Bitmap1> targetBitmap;

		D2D1_BITMAP_PROPERTIES1 bitmapProperties =
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
			);

		D2D1_SIZE_U size = D2D1::SizeU(g_logicalSize.width, g_logicalSize.height);
		g_pD2DDeviceContext->CreateBitmap(size, 0, 0, bitmapProperties, &targetBitmap);

		g_pD2DDeviceContext->GetTarget(&oldTarget);
		g_pD2DDeviceContext->SetTarget(targetBitmap);

		g_pD2DDeviceContext->BeginDraw();
		g_pD2DDeviceContext->Clear(D2D1::ColorF(0x105e06));

		if (g_isDrawingScroll2000)
		{
			D2D1_POINT_2F pt = D2D1::Point2F(-localBounds.left + (g_logicalSize.width - (localBounds.right - localBounds.left)) / 2, -localBounds.top - 50);
			g_pD2DDeviceContext->DrawImage(g_HscrollEffect, & pt);
		}
		else
		{
			D2D1_RECT_F rectDest = D2D1::RectF(0, 0, g_logicalSize.width, g_logicalSize.height);
			g_pD2DDeviceContext->DrawBitmap(bitmap, &rectDest);

			D2D1_POINT_2F pt = D2D1::Point2F(-localBounds.left + (g_logicalSize.width - (localBounds.right - localBounds.left)) / 2, 
																	-localBounds.top + (g_logicalSize.height - (localBounds.bottom - localBounds.top)) / 2);
			g_pD2DDeviceContext->DrawImage(g_HscrollEffect, &pt);
		}
		g_pD2DDeviceContext->EndDraw();

		g_pD2DDeviceContext->SetTarget(oldTarget);
		storedBitmaps.push_back(targetBitmap);
	}
}

void ImageRenderer::HScroll_Render()
{
	if (storedBitmaps.size() == 0)
		return;

	g_pD2DDeviceContext->DrawBitmap(storedBitmaps.at(index));
	if (++index >= QUEUESIZE)
	{
		index = QUEUESIZE - 1;
	}
}

void ImageRenderer::VScroll_CreateEffect(CString sourceFileName)
{
	HRESULT hr = S_OK;
	// Create WIC Decoder to read JPG file.
	ComPtr<IWICBitmapDecoder> decoder;
	reportError(148, (hr = g_pWICFactory->CreateDecoderFromFilename(
		sourceFileName.GetBuffer(100),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&decoder
	)));

	ComPtr<IWICBitmapFrameDecode> frame;
	reportError(149, (hr = decoder->GetFrame(0, &frame)));

	m_VformatConverter = nullptr;
	reportError(150, (hr = g_pWICFactory->CreateFormatConverter(&m_VformatConverter)));
	reportError(151, (hr = m_VformatConverter->Initialize(
		frame,
		GUID_WICPixelFormat32bppPBGRA, // 32bppPBGRA is used by Direct2D.
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom
	)));

	UINT width;
	UINT height;
	reportError(152, (hr = m_VformatConverter->GetSize(&width, &height)));
	m_VimageSize = D2D1::SizeU(width, height);

	if (height == 2000)
	{
		g_isDrawingVScroll2000 = true;
	}
	else
	{
		g_isDrawingVScroll2000 = false;
	}

	m_VimageSource = nullptr;
	reportError(154, (hr = g_pD2DDeviceContext->CreateImageSourceFromWic(m_VformatConverter, &m_VimageSource)));
}

void ImageRenderer::VScroll_UpdateEffectScale()
{
	//float scale = g_logicalSize.width / m_VimageSize.width;
	D2D1_TRANSFORMED_IMAGE_SOURCE_PROPERTIES props =
	{
		D2D1_ORIENTATION_DEFAULT,
		1.0f,
		1.0f,
		D2D1_INTERPOLATION_MODE_LINEAR, // This is ignored when using DrawImage.
		D2D1_TRANSFORMED_IMAGE_SOURCE_OPTIONS_NONE
	};
	m_VtransformedWaveImage = nullptr;
	HRESULT hr = S_OK;
	reportError(156, (hr = g_pD2DDeviceContext->CreateTransformedImageSource(
		m_VimageSource,
		&props,
		&m_VtransformedWaveImage)));

	g_VscrollEffect->SetInput(0, m_VtransformedWaveImage);
}

static std::vector<ComPtr<ID2D1Bitmap1> > VstoredBitmaps;

void ImageRenderer::VScroll_Update()
{
	index = 0;
	ComPtr<ID2D1Bitmap> bitmap;
	bitmap = LoadBitmapFromFile(L"K:\\娱乐\\ico\\wtlpainter\\view-coasts\\ILWJ78328865.png", 0, 0);
	VstoredBitmaps.clear();

	D2D1_RECT_F localBounds;
	ComPtr<ID2D1Image> image;
	g_VscrollEffect->GetOutput(&image);
	g_pD2DDeviceContext->GetImageLocalBounds(image, &localBounds);

	for (int i = 0; i < QUEUESIZE; i++)
	{
		g_VscrollEffect->SetValue(VWAVE_PROP_XRATIO, 1.0f / QUEUESIZE * (i + 1));
		//g_HscrollEffect->SetValue(HWAVE_PROP_SKEW_Y, 0.0f);
		//g_HscrollEffect->SetValue(HWAVE_PROP_SKEW_X, 0.1f);

		ComPtr<ID2D1Image> oldTarget;
		ComPtr<ID2D1Bitmap1> targetBitmap;

		D2D1_BITMAP_PROPERTIES1 bitmapProperties =
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
			);

		D2D1_SIZE_U size = D2D1::SizeU(g_logicalSize.width, g_logicalSize.height);
		g_pD2DDeviceContext->CreateBitmap(size, 0, 0, bitmapProperties, &targetBitmap);

		g_pD2DDeviceContext->GetTarget(&oldTarget);
		g_pD2DDeviceContext->SetTarget(targetBitmap);

		g_pD2DDeviceContext->BeginDraw();
		g_pD2DDeviceContext->Clear(D2D1::ColorF(0x105e06));

		if (g_isDrawingVScroll2000)
		{
			D2D1_POINT_2F pt = D2D1::Point2F(-localBounds.left + (g_logicalSize.width - (localBounds.right - localBounds.left)) / 2, -localBounds.top - 90);
			g_pD2DDeviceContext->DrawImage(g_VscrollEffect, &pt);
		}
		else
		{
			D2D1_RECT_F rectDest = D2D1::RectF(0, 0, g_logicalSize.width, g_logicalSize.height);
			g_pD2DDeviceContext->DrawBitmap(bitmap, &rectDest);
			D2D1_POINT_2F pt = D2D1::Point2F(-localBounds.left + (g_logicalSize.width - (localBounds.right - localBounds.left)) / 2, -localBounds.top);
			g_pD2DDeviceContext->DrawImage(g_VscrollEffect, &pt);
		}
		g_pD2DDeviceContext->EndDraw();
		g_pD2DDeviceContext->SetTarget(oldTarget);

		VstoredBitmaps.push_back(targetBitmap);
	}
}

void ImageRenderer::VScroll_Render()
{
	if (VstoredBitmaps.size() == 0)
		return;

	if (g_isDrawingVScroll2000)
	{
		g_pD2DDeviceContext->Clear(D2D1::ColorF(0x105e06));
	}
	g_pD2DDeviceContext->DrawBitmap(VstoredBitmaps.at(index));

	if (++index >= QUEUESIZE)
	{
		index = QUEUESIZE - 1;
	}
}

ComPtr<ID2D1Bitmap> ImageRenderer::LoadBitmapFromFile(
	const wchar_t* uri,
	unsigned int destinationWidth,
	unsigned int destinationHeight)
{
	ComPtr<ID2D1Bitmap> bitmap;
	ComPtr<IWICBitmapSource> source = LoadWicBitmapFromFile(uri, destinationWidth, destinationHeight);
	g_pD2DDeviceContext->CreateBitmapFromWicBitmap(source, &bitmap);
	return bitmap;
}

ComPtr<ID2D1Bitmap> ImageRenderer::LoadBitmapFromFile(
	ID2D1RenderTarget* renderTarget,
	const wchar_t* uri,
	unsigned int destinationWidth,
	unsigned int destinationHeight)
{
	ComPtr<ID2D1Bitmap> bitmap;
	ComPtr<IWICBitmapSource> source = LoadWicBitmapFromFile(uri, destinationWidth, destinationHeight);
	renderTarget->CreateBitmapFromWicBitmap(source, &bitmap);
	return bitmap;
}

ComPtr<IWICBitmapSource> ImageRenderer::LoadWicBitmapFromFile(
	const wchar_t* uri,
	unsigned int destinationWidth,
	unsigned int destinationHeight)
{
	HRESULT hr = S_OK;

	ComPtr<IWICBitmapDecoder > decoder;
	ComPtr<IWICBitmapFrameDecode > bitmapSource;
	ComPtr<IWICFormatConverter > converter;
	ComPtr<IWICBitmapScaler > scaler;

	if (SUCCEEDED(hr))
	{
		reportError(29, (hr = g_pWICFactory->CreateDecoderFromFilename(
			uri,
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&decoder)));
	}

	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		reportError(30, (hr = decoder->GetFrame(0, &bitmapSource)));
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		reportError(31, (hr = g_pWICFactory->CreateFormatConverter(&converter)));
	}

	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			unsigned int originalWidth, originalHeight;
			reportError(32, (hr = bitmapSource->GetSize(&originalWidth, &originalHeight)));
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					float scalar = static_cast<float>(destinationHeight) / static_cast<float>(originalHeight);
					destinationWidth = static_cast<unsigned int>(scalar * static_cast<float>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					float scalar = static_cast<float>(destinationWidth) / static_cast<float>(originalWidth);
					destinationHeight = static_cast<unsigned int>(scalar * static_cast<float>(originalHeight));
				}

				reportError(33, (hr = g_pWICFactory->CreateBitmapScaler(&scaler)));
				if (SUCCEEDED(hr))
				{
					reportError(34, (hr = scaler->Initialize(
						bitmapSource,
						destinationWidth,
						destinationHeight,
						WICBitmapInterpolationModeLinear)));
				}
				if (SUCCEEDED(hr))
				{
					reportError(35, (hr = converter->Initialize(
						scaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						nullptr,
						0.f,
						WICBitmapPaletteTypeMedianCut)));
				}
			}
		}
		else // Don't scale the image.
		{
			reportError(36, (hr = converter->Initialize(
				bitmapSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				nullptr,
				0.f,
				WICBitmapPaletteTypeMedianCut)));
		}
	}

	return static_cast<ComPtr<IWICBitmapSource>>(converter);
}

