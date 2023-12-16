#include "stdafx.h"
#include "ThisApp.h"
#include "HScrollEffect.h"

#define TITLE L"Title"

bool g_exitFlag = false;

static std::thread* s_thread;

// 渲染窗口
void ThisApp::Render(ThisApp* pThis){
    // 渲染
    while (true){
        pThis->m_ImagaRenderer.HScroll_Render();
		if (g_isDrawingScrollOn == false)
		{
			break;
		}
    }
}

// 初始化
HRESULT ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow){
    HRESULT hr = E_FAIL;
    //register window class
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ThisApp::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"Direct2DTemplate";
    wcex.hIcon = nullptr;
    // 注册窗口
    RegisterClassEx(&wcex);
    // 创建窗口
    RECT window_rect = { 0, 0, WNDWIDTH, WNDHEIGHT };
    DWORD window_style = WS_CAPTION | WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&window_rect, window_style, FALSE);
    window_rect.right -= window_rect.left;
    window_rect.bottom -= window_rect.top;
    window_rect.left = (GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
    window_rect.top = (GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;

    m_hwnd = CreateWindowExW(0, wcex.lpszClassName, TITLE, window_style,
        window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, 0, 0, hInstance, this);
    hr = m_hwnd ? S_OK : E_FAIL;
    // 设置窗口句柄
    if (SUCCEEDED(hr)) 
	{
        m_ImagaRenderer.m_hwnd = m_hwnd;
    }
    // 显示窗口
    if (SUCCEEDED(hr)) {
        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);

		g_logicalSize.width = WNDWIDTH;
		g_logicalSize.height = WNDHEIGHT;
		
		m_ImagaRenderer.CreateResources();
		m_ImagaRenderer.CreateSwapAndSetD2DContextTarget();

		m_ImagaRenderer.HScroll_CreateEffect(L"view.png");
		m_ImagaRenderer.HScroll_UpdateEffectScale();
		m_ImagaRenderer.m_isRippleAnimating = true;
		g_isDrawingScrollOn = true;

		m_ImagaRenderer.HScroll_Update();

		s_thread = new std::thread(Render, this);
    }
    return hr;
}

// 消息循环
void ThisApp::RunMessageLoop(){
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

extern float so_skewX;
extern float so_skewY;
extern float so_xRatio;

// 窗口过程函数
LRESULT CALLBACK ThisApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        ThisApp *pOurApp = (ThisApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pOurApp)
            );

        result = 1;
    }
    else
    {
        ThisApp *pOurApp = reinterpret_cast<ThisApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA
            )));

		static int cnt = 0;

        bool wasHandled = false;
        if (pOurApp){
            switch (message)
            {
			case WM_LBUTTONDOWN:
				//pOurApp->m_ImagaRenderer.Ripple_UpdatePointer(D2D1::Point2F(LOWORD(lParam), HIWORD(lParam)));
				result = 1;
				wasHandled = true;
				break;
			
			case WM_KEYDOWN:
				if (wParam == VK_F1)
				{
					g_HscrollEffect->SetValue(HWAVE_PROP_XRATIO, so_xRatio - 0.02f);
				}
				else if (wParam == VK_F2)
				{
					g_HscrollEffect->SetValue(HWAVE_PROP_XRATIO, so_xRatio + 0.02f);
				}
				result = 1;
				wasHandled = true;
				break;

			case WM_CLOSE:
				g_isDrawingScrollOn = false;
				s_thread->join();
                DestroyWindow(hwnd);
                result = 1;
                wasHandled = true;
                break;
            
			case WM_DESTROY:
				PostQuitMessage(0);
                result = 1;
                wasHandled = true;
                break;
            }
        }
        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }
    return result;
}
