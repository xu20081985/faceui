#pragma once
#include "CCtrlBase.h"
class CDPProgress: public CCtrlBase
{
public:
    CDPProgress(CLayOut *pLayer): CCtrlBase(pLayer)
    {
        m_ColorBak = 0xffe3e3e3;
        m_ColorSrc = 0xffffae00;
        m_ProgressTotal = 100;
        m_ProgressCur = 0;
        m_textcolor = 0xff333333;
        m_bTouch = FALSE;
        m_bShowValue = TRUE;
        m_textsize = GetTextSize(TS_PROGRESS);
        m_active_height = 0;
        m_active_width = 0;
        memset(m_src, 0, sizeof(m_src));
    }
    ~CDPProgress()
    {
    }
    BOOL DoInit(ContentManage *);
    void Show(void);
    void Hide(void);
    void SetProgressTotal(DWORD total)
    {
        m_ProgressTotal = total;
    }
    void SetProgressCur(DWORD total)
    {
        m_ProgressCur = total;
    }
    BOOL DoResponse(DWORD x, DWORD y, DWORD statue);
private:
    DWORD m_ColorBak;
    DWORD m_ColorSrc;
    DWORD m_ProgressTotal;
    DWORD m_ProgressCur;
    DWORD m_textsize;
    DWORD m_textcolor;
    BOOL m_bTouch;
    BOOL m_bShowValue;
    DWORD m_active_height;
    DWORD m_active_width;
    char m_src[128];
};


