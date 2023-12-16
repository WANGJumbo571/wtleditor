#pragma once

// 用途:  包含待修改头文件 以免相互包含

#include "RippleEffect.h"
#include "BasicTimer.h"
#include "ImageRenderer.h"
#include "ThisApp.h"
#include "HTimer.h"

#ifdef USING_MY_COMMON
#include <MyCommon.h>
#else
// 重定向输出
#ifdef _DEBUG
class DEBUG_OUT{
public:
    // 增加计数
    void Acquire(){
        if (!m_counter){
            acquire();
        }
        ++m_counter;
    }
    // 释放计数
    void Release(){
        if (m_counter && !(--m_counter)){
            release();
        }
    }
private:
    // 创建窗口
    void acquire(){
        if (!m_pDebugOut){
            // 打开控制台
            ::AllocConsole();
            // 重定向输出
            freopen_s(&m_pDebugOut, "CONOUT$", "w+t", stdout);
            // 重定向输入
            freopen_s(&m_pDebugIn, "CONIN$", "w+t", stdin);
            // 窗口置顶
            SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 200, 0, 0, SWP_NOSIZE);
            // 设置区域
            setlocale(LC_ALL, "");
        }
    }
    // 释放窗口
    void release(){
        if (m_pDebugOut){
            // 关闭控制台
            ::FreeConsole();
            // 关闭输出
            fclose(m_pDebugOut);
            // 置零
            m_pDebugOut = nullptr;
            // 关闭输入
            fclose(m_pDebugIn);
            // 置零
            m_pDebugIn = nullptr;

        }
    }
    // 构造函数
    DEBUG_OUT(){};
    // 析构函数
    ~DEBUG_OUT(){ release(); }
    // 引用计数
    size_t      m_counter = 0;
    // 重定向输出文件
    FILE*       m_pDebugOut = nullptr;
    // 重定向输入文件
    FILE*       m_pDebugIn = nullptr;
public:
    // 静态变量
    static DEBUG_OUT s_out;
};

#   define DEBUGOUT DEBUG_OUT::s_out
#   define DEBUGOUT_ON DEBUGOUT.Acquire()
#   define DEBUGOUT_OFF DEBUGOUT.Release()
#   define DEBUGOUT_OUTA(format, ...) printf_s(format, __VA_ARGS__)
#   define DEBUGOUT_OUTW(format, ...) wprintf_s(format, __VA_ARGS__)
#   ifdef _UNICODE
#       define DEBUGOUT_OUT(format, ...) DEBUGOUT_OUTW(format, __VA_ARGS__)
#   else
#       define DEBUGOUT_OUT(format, ...) DEBUGOUT_OUTA(format, __VA_ARGS__)
#   endif
#else
// class DEBUG_OUT{    unsigned int unused; };
#   define DEBUGOUT_ON 0
#   define DEBUGOUT_OFF 0
#   define DEBUGOUT_OUT(format, ...) 1
#   define DEBUGOUT_OUTA(format, ...) 1
#   define DEBUGOUT_OUTW(format, ...) 1
#endif
#endif

#define WNDWIDTH 1024
#define WNDHEIGHT 768

#define lengthof(a) (sizeof(a)/sizeof(*(a)))