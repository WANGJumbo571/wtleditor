// ThisApp类 接管程序与操作系统之间的通信

#pragma once

class ThisApp{
public:
    ThisApp(){};
    ~ThisApp(){};
    HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
    static void Render(ThisApp* pThis);
    void RunMessageLoop();
private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
    HWND						m_hwnd = nullptr;
    ImageRenderer			m_ImagaRenderer;
};