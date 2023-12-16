#pragma once

// ��;:  �������޸�ͷ�ļ� �����໥����

#include "RippleEffect.h"
#include "BasicTimer.h"
#include "ImageRenderer.h"
#include "ThisApp.h"
#include "HTimer.h"

#ifdef USING_MY_COMMON
#include <MyCommon.h>
#else
// �ض������
#ifdef _DEBUG
class DEBUG_OUT{
public:
    // ���Ӽ���
    void Acquire(){
        if (!m_counter){
            acquire();
        }
        ++m_counter;
    }
    // �ͷż���
    void Release(){
        if (m_counter && !(--m_counter)){
            release();
        }
    }
private:
    // ��������
    void acquire(){
        if (!m_pDebugOut){
            // �򿪿���̨
            ::AllocConsole();
            // �ض������
            freopen_s(&m_pDebugOut, "CONOUT$", "w+t", stdout);
            // �ض�������
            freopen_s(&m_pDebugIn, "CONIN$", "w+t", stdin);
            // �����ö�
            SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 200, 0, 0, SWP_NOSIZE);
            // ��������
            setlocale(LC_ALL, "");
        }
    }
    // �ͷŴ���
    void release(){
        if (m_pDebugOut){
            // �رտ���̨
            ::FreeConsole();
            // �ر����
            fclose(m_pDebugOut);
            // ����
            m_pDebugOut = nullptr;
            // �ر�����
            fclose(m_pDebugIn);
            // ����
            m_pDebugIn = nullptr;

        }
    }
    // ���캯��
    DEBUG_OUT(){};
    // ��������
    ~DEBUG_OUT(){ release(); }
    // ���ü���
    size_t      m_counter = 0;
    // �ض�������ļ�
    FILE*       m_pDebugOut = nullptr;
    // �ض��������ļ�
    FILE*       m_pDebugIn = nullptr;
public:
    // ��̬����
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